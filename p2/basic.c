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
  fprintf(stderr, "An error has occured\n");
  exit(EXIT_FAILURE);
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


/* check if it is a built in command */
int isBuiltInCommand(char** sgl_cmd_argv) {
  
  if(!strcmp(sgl_cmd_argv[0],"pwd") && sgl_cmd_argv[1]==NULL)
    return 1;

  if(!strcmp(sgl_cmd_argv[0],"cd"))
    return 1;

  return 0;

}
