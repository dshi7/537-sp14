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

void  echoPrompt() {
  fprintf(stdout, "537sh> ");
}

void  printErrorMsg() {
  fprintf(stderr, "An error has occured\n");
  exit(EXIT_FAILURE);
}

/* 
 * return 0 if this is a single command
 * return 1 if this is a set of sequential commands 
 * return 2 if this is a set of parallel commands 
 * return 3 (illegal) 
 * */
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

/* check if this is a built-in command */
int isBuiltInCommand(char**mysh_argv) {
  return !strcmp(mysh_argv[0],"cd") 
    || !strcmp(mysh_argv[0],"pwd") 
    || !strcmp(mysh_argv[0],"quit");
}

/*  function to execute a single command
 *  INPUT : string of cmd
 *  OUTPUT : pid of the created child process 
 *  */
pid_t executeSingleCommand(char* sgl_cmd) {

  int status = 0;
  char*sgl_cmd_argv[512];
  parseSingleCommand(sgl_cmd, sgl_cmd_argv);

  pid_t child_pid = fork();
  if ( child_pid==-1 ) {
    /* fork error */
    perror("Cannot create a child process\n");
    exit(EXIT_FAILURE);
  }
  else if ( child_pid==0 ) {
    /* execute the command line by calling execvp */
    sleep(5);
    execvp(sgl_cmd_argv[0], sgl_cmd_argv);
    exit(EXIT_SUCCESS);
  }
  else {
    /*  wait for the child process to terminate in parent process*/
    fprintf(stdout, "waiting for child process to terminate : %s\n", sgl_cmd);
    return waitpid(child_pid,&status,0);
  }
}


int main(int argc, char *argv[]) {

  char  cmd_line[512];
  char  cwd[512];
  char  *mysh_argv[512];
//  char  *sgl_cmd_argv[512];
  int   mode_code;  //  indicate the mode of multiple commands
  int   cmd_num = 0;
//  int   status = 0;
  pid_t child_pid=1;

  while (1) {

    /* print the shell prompt */
    echoPrompt();

    /* read the command */
    fgets(cmd_line, 512, stdin);

    /* remove the trailing newline char from fget() input */
    strtok( cmd_line, "\n");

    /* check the command line mode */
    mode_code = getCommandMode(cmd_line);

    if(mode_code==3) {
      /* sequantial and parallel mode cannot exist in one line */
      printErrorMsg();
    }

    if(mode_code==1) {
      /* split sequantial commands */
      parseSequentialCommands(cmd_line, mysh_argv);
      while(mysh_argv[cmd_num])  {

        child_pid = executeSingleCommand(*(mysh_argv+(cmd_num++)));
        fprintf(stdout, "%d\n", child_pid);

      }
    }

    if(mode_code==2) {
      /* split parallel commands */
      parseParallelCommands(cmd_line, mysh_argv);
//      while(mysh_argv[cmd_num]) 
//        executeSingleCommand(*(mysh_argv+(cmd_num++)));
    }

    /* parse the command */
    parseSingleCommand(cmd_line, mysh_argv);

    exit(EXIT_SUCCESS);

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
    else {
      child_pid = fork();
      if ( child_pid==-1 ) {
        /* fork error */
        perror("Cannot create a child process\n");
        exit(EXIT_FAILURE);
      }
      else if ( child_pid==0 ) {
        /* execute the command line by calling execvp */
        execvp(mysh_argv[0], mysh_argv);
        exit(EXIT_SUCCESS);
      }
      else {
      }
    }
  }

  return  0;
}
