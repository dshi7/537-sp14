/*
 * =====================================================================================
 *
 *       Filename:  mysh.c
 *
 *    Description:  main function to implement a simple shell
 *
 *        Version:  1.0
 *        Created:  02/09/2014 04:18:09 PM
 *       Compiler:  gcc
 *
 *         Author:  Daohang Shi, dshi7@wisc.edu
 *   Organization:  University of Wisconsin Madison 
 *
 * =====================================================================================
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

typedef struct  process {
  struct process *next;
  char **argv;
  pid_t pid;
  char  completed;
  char  stopped;
  int status;
} process;

typedef struct  job {
  struct job *next;
  char  *command;
  process *first_process;
  pid_t pgid;
} job;

void  echoPrompt() {
  fprintf(stdout, "537sh> ");
}

void  occurError() {
  fprintf(stderr, "An error has occured\n");
  exit(0);
}

int main(int argc, char *argv[]) {

  char  cmd_line[512];
  int   status = 0;
  pid_t child_pid=1;

  while (1) {

    wait(&status);

    /* print the shell prompt */
    echoPrompt();

    /* read the command */
    fgets(cmd_line, 512, stdin);

    /* remove the trailing newline char from fget() input */
    strtok( cmd_line, "\n");

    /* check if the input command is "quit" */
    if(!strcmp(cmd_line, "quit")){
      exit(EXIT_SUCCESS);
    }

    /* parse the command */
    //cmd = parseCommandLine(cmd_line);

    child_pid = fork();
    if ( child_pid==-1 ) {
      /* fork error */
      perror("Cannot create a child process\n");
      exit(EXIT_FAILURE);
    }
    else if ( child_pid==0 ) {
      /* execute the command line by calling execvp */
      fprintf(stdout,"helloworld\n");
      fprintf(stdout,"create a child %d under parent %d\n", getpid(), getppid());
      //execute_basic_shell_cmd(cmd);
      exit(EXIT_SUCCESS);
    }
    else {
    }
  }

  return  0;
}
