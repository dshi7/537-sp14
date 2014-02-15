/*
 * =====================================================================================
 *
 *       Filename:  parser.c
 *
 *    Description:  implement parser-related functions defined in "parser.h"
 *
 *        Version:  1.0
 *        Created:  02/14/2014 04:52:38 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Daohang Shi, dshi7@wisc.edu
 *
 * =====================================================================================
 */

#include "basic.h"
#include "parser.h"

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


/* parse the file redirection */
void  parseFileDirection(char* cmd_line, char**file_redir_argv) {

  *file_redir_argv = strtok(cmd_line,">");
  while (*file_redir_argv!=NULL)  
    *(++file_redir_argv)=strtok(NULL,">");

}
