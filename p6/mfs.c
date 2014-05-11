/*
 * =====================================================================================
 *
 *       Filename:  mfs.c
 *
 *    Description:  implement the client library functions
 *
 *        Version:  1.0
 *        Created:  05/09/2014 02:28:31 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Daohang Shi
 *
 * =====================================================================================
 */

#define MSG_SIZE  (4096)
#include "mfs.h"
#include "udp.h"

//  Each of the following functions implements :
//    Send message to server
//    Read the returned value from server
int MFS_Init (char *hostname, int port) {

  //  Initialize a specific port for client
  //    ** Not used further
  int sd = UDP_Open(port+2014); 
  assert(sd > -1);

  struct sockaddr_in addr, addr2;

  //  Initialize a socket address to contact server at specified port
  //    Using : hostname and port
  int rc = UDP_FillSockAddr (&addr, hostname, port); 
  assert (rc == 0);

  //  Write a message including the function and parameters
  char  message[MSG_SIZE];
  sprintf (message, "init");
  rc = UDP_Write (sd, &addr, message, MSG_SIZE);

  rc = UDP_Read (sd, &addr2, message, MSG_SIZE);
  printf("CLIENT:: read %d bytes (message: '%s')\n", rc, message);

  return 0;

}

int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);
int MFS_Read(int inum, char *buffer, int block);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
