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
  char **mysh_argv;
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

int isWhiteSpace(char ch) {
  /* check if ch is tab, space */
  return ch==9 || ch==32;
}

int isEndOfCmd(char ch) {
  /* check if ch is nul, ";", "|", "+" */
  return ch==0 || ch==59 || ch==124 || ch==43;
}

/* parse a single command */
void  parseCommand(char* cmd_line, char**mysh_argv) {
  *mysh_argv = strtok(cmd_line," \t");
  while (*mysh_argv!=NULL) 
    *(++mysh_argv)=strtok(NULL," \t");
}

int main(int argc, char *argv[]) {

  char  cmd_line[512];
  char  *mysh_argv[512];
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
    parseCommand(cmd_line, mysh_argv);

    child_pid = fork();
    if ( child_pid==-1 ) {
      /* fork error */
      perror("Cannot create a child process\n");
      exit(EXIT_FAILURE);
    }
    else if ( child_pid==0 ) {
      /* execute the command line by calling execvp */
      //execute_basic_shell_cmd(cmd);
      execvp(mysh_argv[0], mysh_argv);
      exit(EXIT_SUCCESS);
    }
    else {
    }
  }

  return  0;
}
