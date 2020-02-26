#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <math.h>
#include <sys/stat.h>
#include <sysexits.h>
#include <time.h>


struct client_args {
	struct sockaddr_in server_addr;
	int hashnum;
	int smin;
	int smax;
	FILE *filename;
	struct stat file_info;
};

void error(char *error_msg){
  perror(error_msg);
  exit(1);
}


error_t client_parser(int key, char *arg, struct argp_state *state) {
	struct client_args *args = state->input;
	int port_num;
	error_t ret = 0;
	switch(key) {
	case 'a':
		memset(&args->server_addr, 0, sizeof(args->server_addr));
    args->server_addr.sin_family = AF_INET;

    /* Converts the network address */
		if (inet_pton(AF_INET, arg, &args->server_addr.sin_addr.s_addr) != 1) {
			argp_error(state, "Invalid address");
		}
		break;
	case 'p':
		/* Validate that port is correct and a number, etc!! */
    port_num = atoi(arg);
		if (port_num <= 0) {
			argp_error(state, "Invalid option for a port, must be a number");
		}
    args->server_addr.sin_port = htons(port_num);
		break;
	case 'n':
		/* validate argument makes sense */
		args->hashnum = atoi(arg);
    if(args->hashnum < 0){
      argp_error(state, "Invalid value for hash requests, must be >= 0");
    }
		break;
	case 300:
		/* validate arg */
		args->smin = atoi(arg);
    if (args->smin < 1) {
			argp_error(state, "Invalid value for min size, must be >= 1");
		}
		break;
	case 301:
		/* validate arg */
		args->smax = atoi(arg);
    if (args->smax > pow(2, 24)) {
			argp_error(state, "Invalid value for max size, must be <= 2^24");
		}
		break;
	case 'f':
		/* validate file */
		args->filename = fopen(arg, "r");
    if (args->filename == NULL) {
			argp_error(state, "Invalid file, could not be opened");
		} else if (fstat(fileno(args->filename), &args->file_info) < 0){
			error("Error on fstat");
		}
		break;
	default:
		ret = ARGP_ERR_UNKNOWN;
		break;
	}
	return ret;
}

void client_parseopt(int argc, char *argv[], struct client_args *args) {
	struct argp_option options[] = {
		{ "addr", 'a', "addr", 0, "The IP address the server is listening at", 0},
		{ "port", 'p', "port", 0, "The port that is being used at the server", 0},
		{ "hashreq", 'n', "hashreq", 0, "The number of hash requests to send to the server", 0},
		{ "smin", 300, "minsize", 0, "The minimum size for the data payload in each hash request", 0},
		{ "smax", 301, "maxsize", 0, "The maximum size for the data payload in each hash request", 0},
		{ "file", 'f', "file", 0, "The file that the client reads data from for all hash requests", 0},
		{0}
	};

	struct argp argp_settings = { options, client_parser, 0, 0, 0, 0, 0 };

	memset(args, 0, sizeof(*args));

	if (argp_parse(&argp_settings, argc, argv, 0, NULL, args) != 0) {
		printf("Got error in parse\n");
	}

  /* Program exits if any of arugments are left out */
  if (!args->server_addr.sin_addr.s_addr) {
    fputs("Address not specified\n", stderr);
    exit(EX_USAGE);
  }
  if (!args->server_addr.sin_port) {
    fputs("Port not specified\n", stderr);
    exit(EX_USAGE);
  }
  if (!args->hashnum) {
    fputs("Hash number not specified\n", stderr);
    exit(EX_USAGE);
  }
  if (!args->smin) {
    fputs("Smin not specified\n", stderr);
    exit(EX_USAGE);
  }
  if (!args->smax) {
    fputs("Smax not specified\n", stderr);
    exit(EX_USAGE);
  }
  if (!args->filename) {
    fputs("File name not specified\n", stderr);
    exit(EX_USAGE);
  }
}

int main (int argc, char *argv[]) {
  int sockfd, i, l, j;
  struct client_args args;
  uint8_t *buff_in = malloc(40);
  unsigned int balance = 0;
	ssize_t bytes;
  size_t buff_in_size, buff_out_size;

	srand(time(NULL));

  /* Parse client arguments */
  client_parseopt(argc, argv, &args);

	/* Set the socket */
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sockfd < 0){
    error("Error opening socket");
  }

  /* Connects socket to specified address */
  if(connect(sockfd, (struct sockaddr *)&args.server_addr, sizeof(args.server_addr)) < 0){
    error("Error connecting");
  }

	uint8_t *buff_out = malloc(8 + args.smax);
	/* initilzation message */
  buff_out_size = 8;
	*(uint32_t *)buff_out = htonl(1);

  /* Send the initilzation message to server with the number of hash requests */
	*(uint32_t *)&buff_out[4] = htonl(args.hashnum);
	while(buff_out_size > balance){
    bytes = send(sockfd, balance + buff_out, buff_out_size - balance, 0);
    if(bytes < 0){
      error("Error on sending ");
    }
    balance = balance + bytes;
  }

  /* Read acknowledgement from server */
	if (ntohs(*(int16_t *)buff_in) != 2) {
		error("Server type is incorrect");
	}
  if((bytes = recv(sockfd, &buff_in[4], 4, 0)) < 0){
    error("Error on reading from server");
  }

	/* Converts integer from host byte order to network byte order */
  *(uint16_t *)buff_out = htons(3);

  /* Loop through for each hash request */
	for(i = 0; i < args.hashnum; i++) {
    /* Samples random number l from smin to smax */
		l = (rand() % (args.smax - args.smin + 1)) + args.smin;

    /* Reads in next l bytes from source file */
		*(uint32_t *)&buff_out[4] = htonl(l);
		fgets((void *)&buff_out[8], l, args.filename);
    balance = 0;

    /* Constructs and sends HashRequest message using l data bytes */
    buff_out_size = 8 + l;
		while (balance < buff_out_size) {
			bytes = send(sockfd, balance + buff_out, buff_out_size - balance, 0);
			if (bytes < 0) {
				error("Error on sending the hash request");
			}
			balance = bytes + balance;
		}

    /* Recieves HashResponses message */
		balance = 0;
		buff_in_size = 40;
		if (ntohs(*(int16_t *)buff_in) != 4) {
			error("Server type is incorrect");
		}
		while (buff_in_size > balance) {
			bytes = recv(sockfd, balance + &buff_in[4], buff_in_size - balance, 0);
			if (bytes < 0) {
				error("Error on receiving the hash response");
			}
			if (bytes == 0) {
				error("Connection closed by host");
			}
			balance = bytes + balance;
		}

    /* Outputting each HashResponse */
		printf("%u: 0x", ntohl(*(uint32_t *)buff_in));
		for (j = 0; j < 32; j++) printf("%02x", buff_in[i+4]);
		printf("\n");
	}

	/* Closing and freeing memory */
  close(sockfd);
  fclose(args.filename);
  free(buff_out);
  free(buff_in);
}
