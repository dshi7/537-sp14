/*
 * =====================================================================================
 *
 *       Filename:  parser.c
 *
 *    Description:  implement parser-related functions defined in "parser.h"
 *
 *        Version:  1.0
 *        Created:  02/14/2014 04:52:38 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Daohang Shi, dshi7@wisc.edu
 *
 * =====================================================================================
 */

#include "basic.h"
#include "parser.h"

/* parse sequential multiple commands */
void  parseSequentialCommands(char* cmd_line, char**mysh_argv) {
  *mysh_argv = strtok(cmd_line,";");
  while (*mysh_argv!=NULL) 
    *(++mysh_argv)=strtok(NULL,";");
}


/* parse parallel multiple commands */
void  parseParallelCommands(char* cmd_line, char**mysh_argv) {
  *mysh_argv = strtok(cmd_line,"+");
  while (*mysh_argv!=NULL) 
    *(++mysh_argv)=strtok(NULL,"+");
}


/* parse a single command */
void  parseSingleCommand(char* cmd_line, char**mysh_argv) {

  *mysh_argv = strtok(cmd_line," \t");
  while (*mysh_argv!=NULL)  
    *(++mysh_argv)=strtok(NULL," \t");

}


/* execute a single command */ 
void  executeSingleCommand(pid_t *child_pid, char** sgl_cmd_argv) {

/* char*sgl_cmd_argv[512]; */
  char  cwd[512];

//  parseSingleCommand(sgl_cmd, sgl_cmd_argv);

  *child_pid = fork();

  if ( *child_pid==-1 ) {

    /* fork error */
    perror("Cannot create a child process\n");
    exit(EXIT_FAILURE);

  }
  else if ( *child_pid==0 ) {

    /* in the child process */

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

    /* kill the child process */
    exit(EXIT_SUCCESS);
  }
}


