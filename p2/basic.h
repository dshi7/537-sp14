#ifndef BASIC_H
#define BASIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

/* get the mode code */
int getCommandMode(char* cmd_line); 


/* check if this is a built-in command  */
int isBuiltInCommand(char**mysh_argv); 


/* execute a single command */ 
void  executeSingleCommand(pid_t *child_pid, char* sgl_cmd); 


#endif
