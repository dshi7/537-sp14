#ifndef PARSER_H
#define PARSER_H

#include <string.h>

/* parse sequential multiple commands */
void  parseSequentialCommands(char* cmd_line, char**mysh_argv); 


/* parse parallel multiple commands */
void  parseParallelCommands(char* cmd_line, char**mysh_argv); 


/* parse a single command */
void  parseSingleCommand(char* cmd_line, char**mysh_argv);


/* execute a single command */ 
void  executeSingleCommand(pid_t *child_pid, char** sgl_cmd_argv); 


#endif  
