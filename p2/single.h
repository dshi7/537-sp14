#ifndef SINGLE_H
#define SINGLE_H

#include<unistd.h>

/* execute an atomic command in shell
 *  child process id, read/write file descriptors are also specified in this function
 *  */ 
void  executeAtomicCommand(pid_t* child_pid, int read_filedes, int write_filedes, char* cmd_line);


/* execute a single command */ 
void  executeSingleCommand(pid_t* child_pid, char* cmd_line);


#endif
