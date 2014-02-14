/*
 * =====================================================================================
 *
 *       Filename:  basic.c
 *
 *    Description:  implement some basic functions defined in "basic.h"
 *
 *        Version:  1.0
 *        Created:  02/14/2014 05:01:38 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Daohang Shi, dshi7@wisc.edu
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include"basic.h"
#include"parser.h"


/* get the mode code */
int getCommandMode(char* cmd_line) {
  int isSequential = 0;
  int isParallel = 0;
  while (*cmd_line) {
    if (*cmd_line==';')
      isSequential = 1;
    if (*cmd_line=='+')
      isParallel = 2;
    cmd_line++;
  }
  return  isSequential+isParallel;
}


/* check if this is a built-in command  */
int isBuiltInCommand(char**mysh_argv) {
  return !strcmp(mysh_argv[0],"cd") 
    || !strcmp(mysh_argv[0],"pwd") 
    || !strcmp(mysh_argv[0],"quit");
}


/* execute a single command */ 
void  executeSingleCommand(pid_t *child_pid, char* sgl_cmd) {

  char*sgl_cmd_argv[512];
  parseSingleCommand(sgl_cmd, sgl_cmd_argv);


  *child_pid = fork();

  if ( *child_pid==-1 ) {
    /* fork error */
    perror("Cannot create a child process\n");
    exit(EXIT_FAILURE);
  }
  else if ( *child_pid==0 ) {
    /* execute the command line by calling execvp */
    sleep(5);
    execvp(sgl_cmd_argv[0], sgl_cmd_argv);
    exit(EXIT_SUCCESS);
  }
}


