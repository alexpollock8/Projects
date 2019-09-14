/*Alex Pollock
  115332645
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <sysexits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "command.h"
#include "executor.h"

static void print_tree(struct tree *t);

int execute_aux(struct tree *t, int input_fd, int output_fd);

int execute(struct tree *t) {
  return execute_aux(t, STDIN_FILENO, STDOUT_FILENO);
  
  /* print_tree(t); */


}

int execute_aux(struct tree *t, int input_fd, int output_fd){
  pid_t child_pid;
  int pipe_fd[2], status1;

  if(t == NULL){
    exit(0);
  }

  /* Pipe conjunction */
  if(t->conjunction == PIPE){
    pid_t  child_pid2;

    /* Checking for ambiguous input/output redirect */
    if((t->left)->output != NULL && (t->right)->input != NULL){
      printf("Ambiguous output redirect.\n");
      return 0;
    }
    else if(t->left != NULL && (t->left)->output != NULL){
      printf("Ambiguous output redirect.\n");
      return 0;
    }
    else if(t->right != NULL && (t->right)->input != NULL){
      printf("Ambiguous input redirect.\n");
      return 0;
    }
    
    if(pipe(pipe_fd) < 0){
      perror("pipe error");
    }
    if((child_pid = fork()) < 0){
      err(EX_OSERR, "fork error");
    }
    /* Parent code waiting for child to finish */
    if(child_pid){
      wait(&status1);
      /* Creating second child to execute the right subtree */
      if((child_pid2 = fork()) < 0){
	err(EX_OSERR, "fork error");
      } else if(child_pid2){
	close(pipe_fd[0]);
	close(pipe_fd[1]);
	wait(&status1);
      }
      /* Second child */
      else {
	/* Redirecting input to pipe read end */
	close(pipe_fd[1]);
	execute_aux(t->right, pipe_fd[0], output_fd);
	close(pipe_fd[0]);
	exit(EX_OSERR);
      }
      return status1;
    }
    /* First child to execute the left subtree */
    else {
      /* Redirecting output to pipe write end */
      close(pipe_fd[0]);
      execute_aux(t->left, input_fd, pipe_fd[1]);
      close(pipe_fd[1]);
      exit(EX_OSERR);
    } 
  }

  /* None conjunction */
  else if(t->conjunction == NONE){
    /* Command given is exit, will exit */
    if(strcmp(t->argv[0], "exit") == 0){
      exit(0);
    }
    else if(strcmp(t->argv[0], "cd") == 0){
      /* No argument given go to user's home directory */
      if(t->argv[1] == NULL){
	t->argv[1] = getenv("HOME");
      }
      if(chdir(t->argv[1]) == -1){
	perror(t->argv[1]);
	exit(EX_OSERR);
      }
    }
    /* Command other than exit or cd */
    else {
      if((child_pid = fork()) < 0){
	perror("fork error");
      }
      /* Parent code waits for child to complete */
      else if(child_pid){
	wait(&status1);
	return status1;
	
      } else {
	/* Input redirection, open argument read only */
	if(t->input != NULL){
      	  /* Read from redirected input ignoring what is recieved from parent */
	  if((input_fd = open(t->input, O_RDONLY)) < 0){
	    err(EX_OSERR, "File opening failed"); 
	  }
	}
	/* Redirect parent input to stdin if parent input not given */
	if(input_fd != STDIN_FILENO){
	  if(dup2(input_fd, STDIN_FILENO) < 0){
	    err(EX_OSERR, "dup2 error");
	  }
	}

	/* Output redirection, open argument for write only */
	if(t->output != NULL){
	  if((output_fd = open(t->output, O_WRONLY | O_CREAT | O_TRUNC)) < 0){
	    err(EX_OSERR, "File opening failed");
	  }
	}
	/* Redirect parent output to stdout if parent output not given */
	if(output_fd != STDOUT_FILENO){
	  if(dup2(output_fd, STDOUT_FILENO) < 0){
	    err(EX_OSERR, "dup2 error");
	  }
	}
 
	
	execvp(t->argv[0], t->argv);
	close(input_fd);
	close(output_fd);
	fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
	exit(EX_OSERR);
      }
    }
    return 0;
  }
  
  /* And conjunction executes left subtree first and upon successful completion
     the right subtree is executed */
  else if(t->conjunction == AND){
    if(!execute_aux(t->left, input_fd, output_fd)){
      execute_aux(t->right, input_fd, output_fd);
    }
  }
 
  /* Subshell conjunction */
  else if(t->conjunction == SUBSHELL){
    if((child_pid = fork()) < 0){
      perror("fork error");
    }
    /* Parent code checks status of child */
    if(child_pid){
      wait(&status1);
      return status1;
    }
    /* Child code to execute left subtree */
    else {
      execute_aux(t->left, input_fd, output_fd);
      fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
      exit(EX_OSERR);
    }
  }

  return 0;
}
  
  
static void print_tree(struct tree *t) {
  if (t != NULL) {
    print_tree(t->left);

    if (t->conjunction == NONE) {
      printf("NONE: %s, ", t->argv[0]);
    } else {
      printf("%s, ", conj[t->conjunction]);
    }
    printf("IR: %s, ", t->input);
    printf("OR: %s\n", t->output);

    print_tree(t->right);
  }
}

