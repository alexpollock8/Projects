/* Alex Pollock
   UID: 115332645
   DID: apolloc1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "document.h"
#include <sysexits.h>
#include <ctype.h>

int remove_space(char *line);
int remove_quotations(char *line, char *p);


int main(int argc, char *argv[]){
  FILE *input;
  char line[MAX_STR_SIZE + 1], filename[MAX_STR_SIZE + 1];
  Document main_document;

 
  /*More than 0 or 1 arguments*/
  if(argc >= 3){
    fprintf(stderr, "Invalid Command");
    exit(EX_USAGE);
  }

  /*No file when program started*/
  if(argc == 1){
    input = stdin;
  }
  
  /*File specified when program started*/
  if(argc == 2){
    strcpy(filename, argv[1]);
    if((input = fopen(filename, "r")) == NULL){
      fprintf(stderr, "%s cannot be opened.\n", filename);
      exit(EX_OSERR);
    }
  }
  
  init_document(&main_document, "main_document");
  
  if(argc == 1){
    printf("> ");
  }
  
  while(fgets(line, 1024, input) != NULL){
    char command[1024] = "", new_line[80 + 1], replacement[80 + 1], data[80];
    int x = 0, y = 0, values_read = 0;
    char *p;
    
    if(isspace(line[0]) && (strlen(line) < 10)){
      sscanf(line, "%c", command);
    } else{
      sscanf(line, "%s", command);
    }

    /*This command adds a paragraph to the document, printing invalid command 
      to standard output when the input does not match. If command not
      successful then prints the command failed.*/
    if(strcmp(command, "add_paragraph_after") == 0){
      values_read = sscanf(line, "%s%d%s", command, &x, replacement);
      if((values_read != 2) || (x < 0)){
	fprintf(stdout, "Invalid Command\n");
      } else {
	if(add_paragraph_after(&main_document, x) == FAILURE){
	  printf("%s failed\n", command);
	}
      }
    }
    
    /*This command adds a line after specified line, printing invalid command
      when input does not match. If command not successful then prints command
      failed */
    if(strcmp(command, "add_line_after") == 0){
      values_read = sscanf(line, "%s%d%d", command, &x, &y);
      p = strstr(line, "*");
      if((values_read != 3) || (x <= 0) || (y < 0) || !p){
	fprintf(stdout, "Invalid Command\n");
      } else {
	p++;
	strcpy(replacement, p);
	if(add_line_after(&main_document, x, y, replacement) == FAILURE){
	  printf("%s failed\n", command);
	}
      }
    }

    /*This command prints document. If data appears after command invalid
      command will be printed.*/
    if(strcmp(command, "print_document") == 0){
      values_read = sscanf(line, "%s%s", command, new_line);
      if(values_read != 1){
	fprintf(stdout, "Invalid Command\n");
      } else {
	print_document(&main_document);
      }
    }

    /*This command quits the user interface. Prints invalid command when data
      appears after command.*/
    if(strcmp(command, "quit") == 0){
      values_read = sscanf(line, "%s%s", command, new_line);
      if(values_read != 1){
	fprintf(stdout, "Invalid Command\n");
      } else {
	exit(EXIT_SUCCESS);
      }
    }

    /*This command exits the user interface. Prints invalid command when data
      appears after commmand.*/
    if(strcmp(command, "exit") == 0){
      values_read = sscanf(line, "%s%s", command, new_line);
      if(values_read != 1){
	fprintf(stdout, "Invalid Command\n");
      } else {
	exit(EXIT_SUCCESS);
      }
	
    }

    /*This command adds a line to the given paragraph. Prints invalid command 
      when invalid data given. If command fails prints command failed.*/
    if(strcmp(command, "append_line") == 0){
      values_read = sscanf(line, "%s%d", command, &x);
      p = strstr(line, "*");
      if((values_read != 2) || (x <= 0) || !p){
	fprintf(stdout, "Invalid Command\n");
      } else{
	p++;
	strcpy(replacement, p);
	if(append_line(&main_document, x, replacement) == FAILURE){
	  printf("%s failed\n", command);
	}
      }
    }

    /*This command removes given line from the paragraph. Prints invalid command
      if data missing. If commmand fails prints command failed.*/
    if(strcmp(command, "remove_line") == 0){
      values_read = sscanf(line, "%s%d%d%s", command, &x, &y, new_line);
      if((values_read != 3) || (x <= 0) || (y <= 0)){
	fprintf(stdout, "Invalid Command\n");
      } else{
	if(remove_line(&main_document, x, y) == FAILURE){
	  printf("%s failed\n", command);
	}
      }
    }

    /*This command loads data from given file. Prints invalid command if data
      missing. If command fails prints command failed.*/
    if(strcmp(command, "load_file") == 0){
      values_read = sscanf(line, "%s%s%s", command, new_line, data);
      if(values_read != 2){
	fprintf(stdout, "Invalid Command\n");
      } else{
	if(load_file(&main_document, new_line) == FAILURE){
	  printf("%s failed\n", command);
	}
      }
    }

    /*Replaces the given text with the replacement text. If data missing prints
      invalid command. If command fails prints command failed.*/
    if(strcmp(command, "replace_text") == 0){
      int new_line_length = 0, i = 0;
      char *p1, *line1;
      values_read = sscanf(line, "%s%s", command, new_line);
      p = strstr(line, " ");
      new_line_length = strlen(new_line);
      for(i = 0; i <= new_line_length; i++){
	p++;
      }
      if(values_read != 2 || !p){
	fprintf(stdout, "Invalid Command\n");
      } else{
	p++;
	remove_space(p);
	p1 = p;
	if(p1[0] == '\"'){
	  remove_quotations(p, p1);
	}
	strcpy(replacement, p1);
	line1 = new_line;
	if(line1[0] == '\"'){
	  remove_quotations(new_line, line1);
	}
	if(replace_text(&main_document, line1, replacement) == FAILURE){
	  printf("%s failed\n", command);
	}
      }
    }

    /*This command highlights the given text, printing invalid command when 
      target string is missing.*/
    if(strcmp(command, "highlight_text") == 0){
      char *p1;
      values_read = sscanf(line, "%s", command);
      p = strstr(line, " ");
      
      if(values_read != 1 || !p){
	fprintf(stdout, "Invalid Command\n");
      } else{
	remove_space(p);
	p1 = p;
	if(p1[0] == '\"'){
	  remove_quotations(p, p1);
	}
	highlight_text(&main_document, p1);
      }
    }

    /*This command removes the given string from the data. If target missing
      invalid command will be printed.*/
    if(strcmp(command, "remove_text") == 0){
      char *p1;
      
      values_read = sscanf(line, "%s%s", command, new_line);
      p = strstr(line, " ");
      if(values_read != 2 || !p){
	fprintf(stdout, "Invalid Command\n");
      } else{
	remove_space(p);
	p1 = p;
	if(p1[0] == '\"'){
	  remove_quotations(p, p1);
	}
	remove_text(&main_document, p);
      }
    }

    /*This command saves current document to specified file. Prints invalid
      command if data invalid.*/
    if(strcmp(command, "save_document") == 0){
      values_read = sscanf(line, "%s%s%s", command, new_line, replacement);
      if(values_read != 2){
	fprintf(stdout, "Invalid Command\n");
      } else{
	if(save_document(&main_document, new_line) == FAILURE){
	  printf("%s failed", command);
	}
      }
    }

    /*This command resets the current document. Prints invalid command if data
      appears after command.*/
    if(strcmp(command, "reset_document") == 0){
      values_read = sscanf(line, "%s%s", command, new_line);
      if(values_read != 1){
	fprintf(stdout, "Invalid Command\n");
      } else{
	reset_document(&main_document);
      }
    }
    if(argc == 1){
      printf("> ");
    }
  }
  fclose(input);

  return 0;
}

/*Removes spaces from the given string.*/
int remove_space(char *line){
  int index = 0, length = 0;

  length = strlen(line);
  while(isspace(line[0])){
    for(index = 0; index < length; index++){
      line[index] = line[index + 1];
    }
  }
  return 0;
}

/*If string has quotations this function will remove them.*/
int remove_quotations(char *line, char *p){
  int index = 0;
  
  line = strstr(line, "\"");
  line++;
  while(line[0] != '\"'){
    p[index++] = line[0];
    line++;
  }
  p[index] = '\0';
  line++;

  return 0;
}
