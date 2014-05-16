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

#define MSG_SIZE  (8192)
#include "mfs.h"
#include "udp.h"


#define DEBUG 1
#undef  DEBUG

int sd = -1;
struct sockaddr_in addr, addr2;

//  Each of the following functions implements :
//    Send message to server
//    Read the returned value from server
int MFS_Init (char *hostname, int port) 
{

  //  Initialize a lock

  //  Initialize a specific port for client
  //    ** Not used further
  sd = UDP_Open(port+2014); 
  assert(sd > -1);

  //  Initialize a socket address to contact server at specified port
  //    Using : hostname and port
  int rc = UDP_FillSockAddr (&addr, hostname, port); 
  assert (rc == 0);

  //  Write a message including the function and parameters
  char  message[MSG_SIZE];
  
//  printf ("CLIENT sends : init\n");
  sprintf (message, "init");
  rc = UDP_Write (sd, &addr, message, MSG_SIZE);

  rc = UDP_Read (sd, &addr2, message, MSG_SIZE);
//  printf("CLIENT:: read %d bytes (message: '%s')\n", rc, message);

  return 0;

}

int MFS_Lookup(int pinum, char *name) 
{

  int rc;

#ifdef  DEBUG
  printf ("LOOKUP %d %s\n", pinum, name, 0);
#endif

  char  message[MSG_SIZE];
  sprintf (message, "l;%d;%s", pinum, name);

  rc = UDP_Write (sd, &addr, message, MSG_SIZE);

  char  message2[MSG_SIZE];

  struct sockaddr_in addr2;

  rc = UDP_Read (sd, &addr2, message2, MSG_SIZE);

  int val = atoi(message2);

#ifdef  DEBUG
  printf ("Returned value = %d\n\n", val);
#endif

  return val;
}

int MFS_Stat(int inum, MFS_Stat_t *m)
{

  int rc ;

#ifdef  DEBUG
  printf ("STAT %d\n", inum);
#endif

  char  message[MSG_SIZE];
  sprintf (message, "s;%d", inum);

  rc = UDP_Write (sd, &addr, message, MSG_SIZE);

  char  message2[MSG_SIZE];

  struct sockaddr_in addr2;

  rc = UDP_Read (sd, &addr2, message2, MSG_SIZE);

#ifdef  DEBUG
  printf ("Returned message = %s\n", message2);
#endif

  if (strcmp(message2, "error")==0) {
#ifdef DEBUG 
    printf ("Returned value = -1\n\n");
#endif
    return -1;
  }
  //  Token the received message
  char *token = strtok (message2, " ");
  
  m->type = atoi (token);
  
  token = strtok (NULL, " ");

  m->size = (m->type==MFS_REGULAR_FILE) ? atoi(token) : atoi(token) /2 * sizeof(MFS_DirEnt_t);

  token = strtok (NULL, " ");
  m->blocks = atoi (token);

#ifdef DEBUG 
    printf ("Returned value = 0\n\n");
#endif
  return 0;
}

int MFS_Write(int inum, char *buffer, int block)
{

  int rc ;
  char  buf[MFS_BLOCK_SIZE+1];

  int  i;
  for (i=0; i<MFS_BLOCK_SIZE; i++)
    buf[i] = buffer[i];

  buf[MFS_BLOCK_SIZE] = '\0';

#ifdef  DEBUG
  printf ("WRITE %d %s %d\n", inum, buf, block);

  printf ("%d\n", strlen(buf));
#endif

  char  message[MSG_SIZE];
  sprintf (message, "w;%d;%d;%s", inum, block, buf);

  rc = UDP_Write (sd, &addr, message, MSG_SIZE);

  char  message2[MSG_SIZE];

  struct sockaddr_in addr2;

  rc = UDP_Read (sd, &addr2, message2, MSG_SIZE);

  int val = atoi(message2);

  printf ("FIRST CHAR = %d\n", buffer[0]);

#ifdef  DEBUG
  printf ("Returned value = %d\n\n", val);
#endif

  return val;
}

int MFS_Read(int inum, char *buffer, int block)
{

  int rc ;

#ifdef  DEBUG
  printf ("READ %d %d\n", inum, block);
#endif

  char  message[MSG_SIZE];
  sprintf (message, "r;%d;%d", inum, block);

  rc = UDP_Write (sd, &addr, message, MSG_SIZE);

  char  message2[MSG_SIZE];

  struct sockaddr_in addr2;

  rc = UDP_Read (sd, &addr2, message2, MSG_SIZE);

#ifdef  DEBUG
  printf ("Returned message = %s\n\n", message2);
#endif

  if (strcmp (message2, "error")==0)
    return -1;

  memcpy (buffer, message2, MFS_BLOCK_SIZE);

  return 0;
}

int MFS_Creat(int pinum, int type, char *name) 
{

  int rc ;

#ifdef  DEBUG
  printf ("CREATE %d %d %s\n", pinum, type, name);
#endif

  char  message[MSG_SIZE];
  sprintf (message, "c;%d;%d;%s", pinum, type, name);

  rc = UDP_Write (sd, &addr, message, MSG_SIZE);

  char  message2[MSG_SIZE];

  struct sockaddr_in addr2;

  rc = UDP_Read (sd, &addr2, message2, MSG_SIZE);

  int val = atoi(message2);

#ifdef  DEBUG
  printf ("Returned value = %d\n\n", val);
#endif

  return val;
}

int MFS_Unlink(int pinum, char *name) 
{
  char  message[MSG_SIZE];

  sprintf (message, "u;%d;%s", pinum, name);
}
