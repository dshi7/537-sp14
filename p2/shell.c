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
#include "single.h"


void executeCurrentLine(char* cmd_line) {  


  int   mode_code;  //  indicate the mode of multiple commands
  int   status = 0;
  int   i;
  pid_t child_pid[512];
  char* mysh_argv[512];

  /* strtok will make change on cmd_line, so make a copy */
  char  cmd_line2[512];
  strcpy(cmd_line2, cmd_line);


  parseSingleCommand(cmd_line, mysh_argv);



  /* check the command line mode */
  mode_code = getCommandMode(cmd_line2);


  /* int mode_code : 
   *    indicate the executing mode of the input command line
   *
   * return 0 if this is a single command
   * return 1 if this is a set of sequential commands 
   * return 2 if this is a set of parallel commands 
   * return 3 (illegal) 
   *
   * */
  if(mode_code<=1) {

    /* parse the multiple commands */
    parseSequentialCommands(cmd_line2, mysh_argv);

    i = 0;

    while(mysh_argv[i])  {

      /* execute multiple commands in sequential */
      executeSingleJob(child_pid+i, mysh_argv[i]);

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
      executeSingleJob(child_pid+i, mysh_argv[i]);
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
    exit(EXIT_FAILURE);

}


int main(int argc, char *argv[]) {

  char  cmd_line[512];

  FILE *fp;
  char  file_line[512];

  /* check batch mode */
  if(argv[1]!=NULL) {


    if(argv[2]!=NULL)
      printErrorMsg();

    fp = fopen(argv[1], "r");

    if(fp==NULL) {
      printErrorMsg();
      return 1;
    }


    while(fgets(file_line, 512, fp)!=NULL) {


      write(STDOUT_FILENO, file_line, strlen(file_line));
      strtok( file_line, "\n");

      /* make sure that all the lines are finished in order */
      if(fork()==0) {

        executeCurrentLine(file_line);
        exit(EXIT_SUCCESS);

      }
      else {
        wait(NULL);

        /* check empty lines */
//        if(isEmptyLine(file_line))
//          continue;

      }
    }

  }

  else {

    while (1) {

      /* print the shell prompt */
      echoPrompt();

      /* read the command */
      fgets(cmd_line, 512, stdin);

      /* remove the trailing newline char from fget() input */
      strtok( cmd_line, "\n");

      /* check if in batch mode */
      executeCurrentLine(cmd_line);

    }

  }


  return  0;
}
