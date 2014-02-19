/*
 * =====================================================================================
 *
 *       Filename:  basic.c
 *
 *    Description:  implement some basic functions defined in "basic.h"
 *
 *        Created:  02/14/2014 05:01:38 PM
 *       Compiler:  gcc
 *
 *         Author:  Gary D.Shi  dshi7@wisc.edu
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include"basic.h"


/* print the shell prompt and wait for stdin */
void  echoPrompt() {
  fprintf(stdout, "537sh> ");
}


/* print the required error message */
void  printErrorMsg() {
  char  error_message[30]="An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}


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


/* check if empty line */
int isEmptyLine(char* cmd_line) {
  char* token;
  token = strtok(cmd_line,"\t ");
  return token==NULL;
}

/* check if it is a built in command */
int isBuiltInCommand(char** sgl_cmd_argv) {

  
  if(!strcmp(sgl_cmd_argv[0],"quit") && sgl_cmd_argv[1]==NULL)
    return 1;

  if(!strcmp(sgl_cmd_argv[0],"pwd") && sgl_cmd_argv[1]==NULL)
    return 1;

  if(!strcmp(sgl_cmd_argv[0],"cd"))
    return 1;

  return 0;

}
