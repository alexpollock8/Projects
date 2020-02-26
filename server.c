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
#include <sys/poll.h>
#include <sys/fcntl.h>
#include <sysexits.h>
#include "hash.h"

struct server_args {
	int port;
	uint8_t *salt;
	size_t salt_len;
};

enum message {INIT, ACK, HASH, CLOSED};

struct client {
	struct checksum_ctx *ctx;
  enum message msg;
  int hashnum;
  int i;
	size_t data_len;
	uint8_t *buffer_out;
	uint8_t *buffer_in;
  size_t out_len;
	size_t in_len;
};

void error(char *err_msg){
  perror(err_msg);
  exit(1);
}

error_t server_parser(int key, char *arg, struct argp_state *state) {
	struct server_args *args = state->input;
	error_t ret = 0;
	switch(key) {
	case 'p':
		/* Validate that port is correct and a number, etc!! */
		args->port = atoi(arg);
		if (args->port == 0 || args->port <= 1024) {
			argp_error(state, "Invalid option for a port");
		}
		break;
	case 's':
		args->salt_len = strlen(arg);
		args->salt = malloc(args->salt_len);
		memcpy(args->salt, arg, args->salt_len);
		break;
	default:
		ret = ARGP_ERR_UNKNOWN;
		break;
	}
	return ret;
}

void *server_parseopt(int argc, char *argv[], struct server_args *args) {

	memset(args, 0, sizeof(*args));

	struct argp_option options[] = {
		{ "port", 'p', "port", 0, "The port to be used for the server" ,0},
		{ "salt", 's', "salt", 0, "The salt to be used for the server. Zero by default", 0},
		{0}
	};
	struct argp argp_settings = { options, server_parser, 0, 0, 0, 0, 0 };
	if (argp_parse(&argp_settings, argc, argv, 0, NULL, args) != 0) {
		printf("Got an error condition when parsing\n");
    exit(EX_USAGE);
	}

  if (!args->port) {
		fputs("A port number must be specified\n", stderr);
		exit(EX_USAGE);
	}

	printf("Got port %d and salt %s with length %ld\n", args->port, args->salt, args->salt_len);
	return args;
}

/* Handles messages and requests coming from clients */
void comm(int sockfd, struct client *clients) {
  ssize_t bytes, bytes_in;
	uint8_t *buffer_out = clients->buffer_out;
	uint8_t *buffer_in = clients->buffer_in;
  size_t balance, buff;

  /* Handles incoming messages from clients depending on the message format */
	switch (clients->msg) {
  /* initilzation message */
  case INIT:
		bytes =  8 - clients->in_len;
		break;
  /* Acknowledgement message */
	case ACK:
		bytes =  8 - clients->in_len;
		break;
  /* Hash request */
	case HASH:
		bytes = clients->data_len - clients->in_len;
		if (bytes > UPDATE_PAYLOAD_SIZE) {
			bytes = UPDATE_PAYLOAD_SIZE - clients->in_len % UPDATE_PAYLOAD_SIZE;
		} else if (clients->out_len) {
			return;
		}
		break;
	default:;
	}

	/* Reads in contents from the clients */
	balance = clients->in_len % UPDATE_PAYLOAD_SIZE;
	if((bytes_in = recv(sockfd, buffer_in + balance, bytes, 0)) < 0){
		error("Error on recv");
	}

	clients->in_len = clients->in_len + bytes_in;

	/* If bytes receieved from client is 0 close connection */
	if (!bytes_in) {
		clients->msg = CLOSED;
	}
	/* If not 0 read in hash request information */
	else if (bytes_in == bytes) switch (clients->msg) {
	case INIT:
		if (ntohs(*(int16_t *)buffer_in) != 1) {
			clients->msg = CLOSED;
			printf("Client type is incorrect");
		}
    printf("Number of hash requests: %d\n", clients->hashnum);
		clients->hashnum = ntohl(*(uint32_t *)&buffer_in[4]);
		clients->in_len = 0;
		*(uint16_t *)buffer_out = htons(2);
		*(uint32_t *)&buffer_out[4] = htonl(40 * clients->hashnum);
		clients->out_len = 4;
		clients->msg = ACK;
		break;
	case ACK:
		if (ntohs(*(int16_t *)buffer_in) != 3) {
			clients->msg = CLOSED;
			printf("Client type is incorrect");
		} else {
			clients->data_len = ntohl(*(uint32_t *)&buffer_in[4]);
			clients->in_len = 0;
			clients->msg = HASH;
			printf("Hashing payload");
		}
		break;
	case HASH:
		if (clients->in_len == clients->data_len) {
			buff = clients->in_len % UPDATE_PAYLOAD_SIZE;
			*(uint16_t *)buffer_out = htons(4);
			*(uint32_t *)&buffer_out[4] = htonl(clients->i++);
			checksum_finish(clients->ctx, buffer_in, buff, buffer_out + 4);
			clients->out_len = 40;
			if (clients->i < clients->hashnum) {
				clients->in_len = 0;
				clients->msg = ACK;
				checksum_reset(clients->ctx);
			}
		} else {
			checksum_update(clients->ctx, buffer_in);
		}
		break;
	default:;
	}
}

/* Handles the client by accepting the connection and initalizes its contents */
int clients_in(int sockfd, struct client *clients) {
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
  int nsockfd;

  /* Alllows client to connect and fails if error */
	nsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
	if (nsockfd < 0) {
		error("Error on accepting");
	}

  /* Handles the files */
	fcntl(nsockfd, F_SETFL, O_NONBLOCK);

  /* Initalizes the client structure */
	memset(clients, 0, sizeof(*clients));
	clients->msg = INIT;
	clients->buffer_in = malloc(40);
	clients->buffer_out = malloc(UPDATE_PAYLOAD_SIZE);
	clients->in_len = 0;
	clients->out_len = 0;

	return nsockfd;
}

int main (int argc, char *argv[]) {
  int sockfd, client_in, i;
  struct server_args args;
  struct client clients[10];
  struct sockaddr_in server_addr;
  struct pollfd clients_fd[11];
	ssize_t bytes_sent;


  /* Handles command line arguments */
  server_parseopt(argc, argv, &args);

  /* Create socket and verification */
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sockfd < 0){
    error("Error opening socket");
  }

  fcntl(sockfd, F_SETFL, O_NONBLOCK);

  /* Initalizes the clients file descriptors */
  clients_fd[0].fd = sockfd;
  clients_fd[0].events = POLLIN;
  for(i = 0; i < 10; i++){
    clients_fd[i + 1].fd = -1;
    clients_fd[i + 1].events = POLLIN;
  }

  /* Assign IP and port */
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(args.port);

  /* Binding newly created socket to given IP and verification */
  if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
    error("Error on binding");
  }

  /* Ready for listening */
  if((listen(sockfd, 1024)) < 0){
    error("Error on listening");
  }

  for(;;)
  switch (poll(clients_fd, 11, -1)){
		case -1:
      printf("Error on incoming clients");
			break;
    case 0:
      printf("Currently waiting for the connections");
      break;

    default:
			client_in = clients_fd[0].revents & POLLIN;

      for(i = 0; i < 10; i++){
				/* Server will handle the incoming client */
        if(client_in && clients_fd[i + 1].fd < 0){
          clients_fd[i + 1].fd = clients_in(sockfd, &clients[i]);
					client_in = 0;
					clients[i].ctx = checksum_create(args.salt, args.salt_len);
				}
				/* Server will then handle the message */
				if (clients_fd[i + 1].revents & POLLIN){
					comm(clients_fd[i + 1].fd, &clients[i]);

					/* If connection is closed destroy and free memory */
          if(clients[i].msg == CLOSED){
            close(clients_fd[i + 1].fd);
            clients_fd[i + 1].fd = -999;
            clients_fd[i + 1].revents = 0;
            checksum_destroy(clients[i].ctx);
            free(clients[i].buffer_out);
            free(clients[i].buffer_in);
          }
          if(clients[i].out_len){
            clients_fd[i + 1].events |= POLLOUT;
          }
        }

				/* Server will send data if data is available to be sent to client */
        if(clients_fd[i + 1].revents & POLLOUT){
          bytes_sent = send(clients_fd[i + 1].fd, clients[i].buffer_out, clients[i].out_len, MSG_NOSIGNAL);
          if(bytes_sent < 0){
            error("Error on sending");
          }
          clients[i].out_len = clients[i].out_len - bytes_sent;
          memmove(clients[i].buffer_out, clients[i].buffer_out + bytes_sent, clients[i].out_len);

          if(!clients[i].out_len){
            clients_fd[i + 1].events &= ~POLLOUT;
          }
        }
      }
      break;
    }
  }
