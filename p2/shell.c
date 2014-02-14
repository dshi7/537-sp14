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

#include "basic.h"
#include "parser.h"

void  echoPrompt() {
  fprintf(stdout, "537sh> ");
}

void  printErrorMsg() {
  fprintf(stderr, "An error has occured\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

  char  cmd_line[512];
  char  cwd[512];
  char  *mysh_argv[512];
  //  char  *sgl_cmd_argv[512];
  int   mode_code;  //  indicate the mode of multiple commands
  int   status = 0;
  pid_t child_pid[512];
  int   i;

  while (1) {

    /* print the shell prompt */
    echoPrompt();

    /* read the command */
    fgets(cmd_line, 512, stdin);

    /* remove the trailing newline char from fget() input */
    strtok( cmd_line, "\n");

    /* check the command line mode */
    mode_code = getCommandMode(cmd_line);
    /* int mode_code : 
     *    indicate the executing mode of the input command line
     *
     * return 0 if this is a single command
     * return 1 if this is a set of sequential commands 
     * return 2 if this is a set of parallel commands 
     * return 3 (illegal) 
     *
     * */
    if(mode_code==0) {

      /* parse the command */
      parseSingleCommand(cmd_line, mysh_argv);

      /* execute the single command */
      executeSingleCommand(child_pid, *mysh_argv);

      /* wait the current child process to terminate
       * in the parent process */
      waitpid(child_pid[0], &status, 0);
    }
    else if(mode_code==1) {

      /* parse the multiple commands */
      parseSequentialCommands(cmd_line, mysh_argv);

      i = 0;
      while(mysh_argv[i])  {

        /* execute multiple commands in sequential */
        executeSingleCommand(child_pid+i, *(mysh_argv+i));

        /* wait the current child process to terminate
         * in the parent process */
        if (child_pid[i]>0)  
          waitpid(child_pid[i++], &status, 0);
      }

    }
    else if(mode_code==2) {

      /* parse the multiple commands */
      parseParallelCommands(cmd_line, mysh_argv);

      i = 0;
      while(mysh_argv[i])  {
        /* execute multiple commands in parallel */
        executeSingleCommand(child_pid+i, *(mysh_argv+i));
        ++i;
      }

      /* wait all the current child processes to terminate
       * in the parent process */
      i = 0;
      while(mysh_argv[i])  {
        if (child_pid[i]>0)  
          waitpid(child_pid[i++], &status, 0);
      }

    }
    else if(mode_code==3) {
      /* sequantial and parallel mode cannot exist in one line */
      printErrorMsg();
    }
    else
      

    continue;

    if(isBuiltInCommand(mysh_argv)) {
      if(!strcmp(mysh_argv[0],"quit")) 
        exit(EXIT_SUCCESS);
      if(!strcmp(mysh_argv[0],"pwd")) {
        if(getcwd(cwd,sizeof(cwd))) 
          fprintf(stdout, "%s\n", cwd);
        else
          printErrorMsg();
      }
    }
  }

  return  0;
}
