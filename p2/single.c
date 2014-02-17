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

void  executeAtomicCommand(pid_t* child_pid, int read_filedes, int write_filedes, char* cmd_line)  {
  //void  executeAtomicCommand(pid_t* child_pid, int* read_filedes, int* write_filedes, char* cmd_line, int prev_pipe_exist, int* prev_pipe_wr, int next_pipe_exist, int* next_pipe_rd ) {

  char* sgl_cmd_argv[512];
  /* must be a non-built-in command */

  /* parse the single command */
  parseSingleCommand(cmd_line, sgl_cmd_argv);

  *child_pid = fork();

  if(*child_pid==0) {

    /* in the child process */

    dup2(read_filedes, STDIN_FILENO);

    dup2(write_filedes, STDOUT_FILENO);


    /* if false
     *  execute the command line by calling execvp */
    execvp(sgl_cmd_argv[0], sgl_cmd_argv);


    /* kill the child process */
    _exit(EXIT_SUCCESS);
  }
  else if(*child_pid==-1) {

    /* fork error */
    perror("Cannot create a child process\n");
    _exit(EXIT_FAILURE);

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
  //  char  cwd[512];
  int   redir_fd = -1;
  /* used for pipe */
  int   pipe_num;
  int   pipe_fd[512*2];        

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
    redir_fd = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
  }



  if(strchr(cmd_line, '|')) {

    int j=0;
    /* parse the pipe and the number of pipes are returned */
    pipe_num = parsePipe(cmd_line, pipe_argv);

    /* the number of atomic commands connected with pipes = pipe_num+1
     *  denoted by pipe_argv[0] , ... , pipe_argv[pipe_num] */


    /* create (pipe_num+1) different pipes */
    for(j=0; j<pipe_num; j++) 
      if (pipe(pipe_fd+(j<<1))==-1) 
        exit(EXIT_FAILURE);

    /* execute all the atomic commands */
    for(j=0; j<=pipe_num; j++) {

      if( fork()==0 ) {

        char* sgl_cmd_argv[512];
        /* control the previous pipe */
        if(j!=0) {
          close(pipe_fd[2*j-1]);
          dup2(pipe_fd[2*j-2], STDIN_FILENO);
          close(pipe_fd[2*j-2]);
        }

        /* control the next pipe */
        if(j!=pipe_num) {
          close(pipe_fd[2*j+0]);
          dup2(pipe_fd[2*j+1], STDOUT_FILENO);
          close(pipe_fd[2*j+1]);
        }

        /* parse the single command */
        parseSingleCommand(pipe_argv[j], sgl_cmd_argv);

        /* execute the command */
        execvp(sgl_cmd_argv[0], sgl_cmd_argv);

        _exit(EXIT_FAILURE);

      }
    }

    for(j=0; j<pipe_num; j++) {
      close(pipe_fd[2*j+0]);
      close(pipe_fd[2*j+1]);
    }

    for(j=0; j<=pipe_num; j++)
      wait(NULL);


  }
  else {

    executeAtomicCommand(child_pid, STDIN_FILENO, redir_fd==-1 ? STDOUT_FILENO : redir_fd, file_redir_argv[0]);

  }
}


