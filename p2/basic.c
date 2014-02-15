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


/* check if this is a built-in command  */
int isBuiltInCommand(char**mysh_argv) {
  return !strcmp(mysh_argv[0],"cd") 
    || !strcmp(mysh_argv[0],"pwd") ;
}


/* execute a non-quit single command */ 
void  executeSingleCommand(char *file_ptr, pid_t *child_pid, char** sgl_cmd_argv) {

  char  def_out[512];

  if(file_ptr!=NULL) {
    /* the output file is specified */
    strcpy(def_out, file_ptr);
    char  *token = strtok(def_out," \t");
    strcpy(def_out, token);
  }

  char  cwd[512];

  int   file_descriptor = -1;

  *child_pid = fork();

  if ( *child_pid==-1 ) {

    /* fork error */
    perror("Cannot create a child process\n");
    exit(EXIT_FAILURE);

  }
  else if ( *child_pid==0 ) {

    /* in the child process */

    /* duplicate file descriptor if file redirection is required */
    if (file_ptr) {
      file_descriptor = open(def_out, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    }

    if(file_descriptor!=-1)
      dup2(file_descriptor, STDOUT_FILENO);

    /* check if the command is built-in */
    if(isBuiltInCommand(sgl_cmd_argv)) {

      /* if true
       *  then execute the built-in command */
      if(!strcmp(sgl_cmd_argv[0],"pwd")) {
        if(getcwd(cwd,sizeof(cwd))) 
          fprintf(stdout, "%s\n", cwd);
        else
          printErrorMsg();
      }
      else if(!strcmp(sgl_cmd_argv[0],"cd")) {

      }
    }
    else {

      /* if false
       *  execute the command line by calling execvp */
      execvp(sgl_cmd_argv[0], sgl_cmd_argv);

    }

    if(file_descriptor!=-1)
      close(file_descriptor);


    /* kill the child process */
    exit(EXIT_SUCCESS);
  }
}


