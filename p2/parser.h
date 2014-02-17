#ifndef PARSER_H
#define PARSER_H

#include <string.h>

/* parse sequential multiple commands */
void  parseSequentialCommands(char* cmd_line, char**mysh_argv); 


/* parse parallel multiple commands */
void  parseParallelCommands(char* cmd_line, char**mysh_argv); 


/* parse a single command */
void  parseSingleCommand(char* cmd_line, char**mysh_argv);


/* parse the file redirection */
void  parseFileDirection(char* cmd_line, char**file_redir_argv);

/* parse pipe */
int   parsePipe(char* cmd_line, char**mysh_argv); 

#endif  
