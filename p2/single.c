/*
 * =====================================================================================
 *
 *       Filename:  single.c
 *
 *    Description:  execute a single command defined in "single.h"
 *
 *        Created:  02/15/2014 04:20:58 PM
 *       Compiler:  gcc
 *
 *         Author:  Gary D.Shi  dshi7@wisc.edu
 *
 * =====================================================================================
 */

#include "single.h"
#include "parser.h"
#include "basic.h"
#include <stdio.h>
#include <stdlib.h>

void  executeAtomicCommand(pid_t* child_pid, int read_filedes, int write_filedes, char* cmd_line) {

  char* sgl_cmd_argv[512];
  /* must be a non-built-in command */

  /* parse the single command */
  parseSingleCommand(cmd_line, sgl_cmd_argv);

  *child_pid = fork();

  if ( *child_pid==-1 ) {

    /* fork error */
    perror("Cannot create a child process\n");
    exit(EXIT_FAILURE);

  }
  else if ( *child_pid==0 ) {

    /* in the child process */

    if(read_filedes!=-1)
      dup2(read_filedes, STDIN_FILENO);

    if(write_filedes!=-1)
      dup2(write_filedes, STDOUT_FILENO);


    /* if false
     *  execute the command line by calling execvp */
    execvp(sgl_cmd_argv[0], sgl_cmd_argv);


    /* kill the child process */
    _exit(EXIT_SUCCESS);
  }
}

void  executeSingleCommand(pid_t* child_pid, char* cmd_line) {

  /* 
   * char *cmd_line is a single command
   *  there still might contain additional spaces
   * */

  char  output_file[512];
  char* file_redir_argv[512];
  char* pipe_argv[512];
  //  char* sgl_cmd_argv[512];
  //  char  cwd[512];
  int   redir_filedes = -1;


  /* parse the file redirection */
  parseFileDirection(cmd_line, file_redir_argv);

  /* return error if more than one output redirections are specified */
  if(file_redir_argv[2]!=NULL) {
    printErrorMsg();
  }

  /* if an output file is specified 
   *  then remove the spaces */
  if(file_redir_argv[1]!=NULL) {
    strcpy(output_file, file_redir_argv[1]);
    char  *token = strtok(output_file, " \t");
    strcpy(output_file, token);
    redir_filedes = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
  }



  if(strchr(cmd_line, '|')) {

    /* parse the pipe */
    parsePipe(cmd_line, pipe_argv);

    int j=0;
    while(pipe_argv[j])
      printf("%s\n", pipe_argv[j++]);

    exit(EXIT_SUCCESS);

  }
  else {

    executeAtomicCommand(child_pid, STDIN_FILENO, redir_filedes==-1 ? STDOUT_FILENO : redir_filedes, file_redir_argv[0]);

  }
  //    else if ( *child_pid==0 ) {
  //
  //      /* in the child process */
  //
  //      /* duplicate file descriptor if file redirection is required */
  //      if (file_redir_argv[1]) {
  //        file_descriptor = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
  //      }
  //
  //      if(file_descriptor!=-1)
  //        dup2(file_descriptor, STDOUT_FILENO);
  //
  //      /* check if the command is built-in */
  //      if(isBuiltInCommand(sgl_cmd_argv)) {
  //
  //        /* if true
  //         *  then execute the built-in command */
  //        if(!strcmp(sgl_cmd_argv[0],"pwd")) {
  //          if(getcwd(cwd,sizeof(cwd))) 
  //            fprintf(stdout, "%s\n", cwd);
  //          else
  //            printErrorMsg();
  //        }
  //        else if(!strcmp(sgl_cmd_argv[0],"cd")) {
  //
  //        }
  //      }
  //      else {
  //
  //        /* if false
  //         *  execute the command line by calling execvp */
  //        execvp(sgl_cmd_argv[0], sgl_cmd_argv);
  //
  //      }
  //
  //      if(file_descriptor!=-1)
  //        close(file_descriptor);
  //
  //
  //      /* kill the child process */
  //      _exit(EXIT_SUCCESS);
  //    }
}


