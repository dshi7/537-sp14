#ifndef BASIC_H
#define BASIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

/* print the shell prompt and wait for stdin */
void  echoPrompt(); 


/* print the required error message */
void  printErrorMsg(); 


/* get the mode code */
int getCommandMode(char* cmd_line); 


/* check if this is a built-in command  */
int isBuiltInCommand(char**mysh_argv); 


#endif
