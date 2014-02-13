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

void  echoPrompt() {
  fprintf(stdout, "537sh> ");
}

void  occurError() {
  fprintf(stderr, "An error has occured\n");
  exit(0);
}

int main(int argc, char *argv[]){

  char  command_line[512];
  pid_t pid;
  int status = 0;

  /*  init the shell  */
  echoPrompt();

  while(fgets(command_line, 512, stdin)) {
    /* remove the trailing newline char from fget() input */
    strtok( command_line, "\n");

    /* check if the input command is "quit" */
    if(!strcmp(command_line, "quit")){
      exit(0);
    }

    /* create child process */
    pid = fork();
    if(pid == 0){
      execvp("/bin/ls", "ls", "-al", "*.c", NULL);
      break;
    }

    echoPrompt();
  }

  return  0;
}
