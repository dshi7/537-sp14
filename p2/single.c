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

void  executeAtomicCommand(pid_t* child_pid, int read_filedes, int write_filedes, char* cmd_line, int prev_pipe_exist, int* prev_pipe_fd, int next_pipe_exist, int* next_pipe_fd) {

  char* sgl_cmd_argv[512];

  *child_pid = fork();

  if(*child_pid==0) {

    /* in the child process */

    if(prev_pipe_exist) {

//      printf ( "%s -- prev pipe : %d %d\n", cmd_line, *(prev_pipe_fd+PIPE_RD_END), *(prev_pipe_fd+PIPE_WR_END) );

      close(*(prev_pipe_fd+PIPE_WR_END));
      dup2(*(prev_pipe_fd+PIPE_RD_END), STDIN_FILENO);
      close(*(prev_pipe_fd+PIPE_RD_END));

    }

    if(next_pipe_exist) {

//      printf ( "%s -- next pipe : %d %d\n", cmd_line, *(next_pipe_fd+PIPE_RD_END), *(next_pipe_fd+PIPE_WR_END) );

      close(*(next_pipe_fd+PIPE_RD_END));
      dup2(*(next_pipe_fd+PIPE_WR_END), STDOUT_FILENO);
      close(*(next_pipe_fd+PIPE_WR_END));

    }
    else if ( write_filedes!=-1) 
        dup2(write_filedes, STDOUT_FILENO);

    /* parse the single command */
    parseSingleCommand(cmd_line, sgl_cmd_argv);


    /* if false
     *  execute the command line by calling execvp */
    execvp(sgl_cmd_argv[0], sgl_cmd_argv);


    /* kill the child process */
    _exit(EXIT_FAILURE);

  }
  else if(*child_pid==-1) {

    /* fork error */
    perror("Cannot create a child process\n");
    _exit(EXIT_FAILURE);

  }
}


/* job here means the command batch delimited by ";" or "+"
 * each job may include pipe or file redirection */
void  executeSingleJob(pid_t* child_pid, char* cmd_line) {

  /* 
   * char *cmd_line is a single command
   *  there still might contain additional spaces
   * */

  char  output_file[512];
  char* file_redir_argv[512];
  char* pipe_argv[512];
  int   redir_fd = -1;

  /* used for pipe */
  int   status = 0;
  int   pipe_num;
  pid_t pipe_pid[512];
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
      if (pipe(pipe_fd+2*j)==-1) 
        exit(EXIT_FAILURE);

    /* execute all the atomic commands */
    for(j=0; j<=pipe_num; j++) {
      
//      printf("%d %s %d %d\n", j, pipe_argv[j], j!=0, j!=pipe_num);

      executeAtomicCommand(pipe_pid+j, STDIN_FILENO, redir_fd==-1 ? STDOUT_FILENO : redir_fd, pipe_argv[j], j!=0, (j!=0)?(pipe_fd+2*(j-1)):NULL, j!=pipe_num, (j!=pipe_num)?(pipe_fd+2*j):NULL);

      if(j!=0) {
        close(pipe_fd[2*(j-1)]);
        close(pipe_fd[2*(j-1)+1]);
      }

      if(pipe_pid[j]>0)
        waitpid(pipe_pid[j], &status, 0);

    }

  }

  else {

    executeAtomicCommand(child_pid, STDIN_FILENO, redir_fd==-1 ? STDOUT_FILENO : redir_fd, file_redir_argv[0], 0, NULL, 0, NULL);

  }

}


