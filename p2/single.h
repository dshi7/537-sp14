#ifndef SINGLE_H
#define SINGLE_H

#include<unistd.h>

#define PIPE_RD_END 0
#define PIPE_WR_END 1

/* execute an atomic command in shell
 *  child process id, read/write file descriptors are also specified in this function
 *  */ 
void  executeAtomicCommand(pid_t* child_pid, int read_filedes, int write_filedes, char* cmd_line);
//void  executeAtomicCommand(pid_t* child_pid, int* read_filedes, int* write_filedes, char* cmd_line, int prev_pipe_exist, int* prev_pipe_wr, int next_pipe_exist, int* next_pipe_rd ); 


/* execute a single command */ 
void  executeSingleCommand(pid_t* child_pid, char* cmd_line);


#endif
