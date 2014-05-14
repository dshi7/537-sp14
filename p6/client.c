#include <stdio.h>
#include "mfs.h"

#define BUFFER_SIZE (4096)
char buffer[BUFFER_SIZE];

int main(int argc, char *argv[]) 
{
  int rc = MFS_Init ("mumble-18.cs.wisc.edu", 10000);

  if (rc>0) {
    printf ("ERROR : Cannot initialize MFS\n");
    return  -1;
  }

  if (MFS_Lookup(-1, "usr") != -1)
	  return -1;

  if (MFS_Lookup(-1, "usr") != -1)
	  return -1;

  if (MFS_Lookup(4096, "usr") != -1)
	  return -1;

  if (MFS_Lookup(0, "bin") != -1)
	  return -1;

//  if (MFS_Creat(0, MFS_DIRECTORY, "bin") != 0)
//	  return -1;

	rc = MFS_Lookup(0, "bin");
	
  if (rc == -1 || rc == 0)
	  return -1;

  return 0;
}


  /*
{
  if(argc<4)
  {
    printf("Usage: client server-name server-port client-port\n");
    exit(1);
  }
  int sd = UDP_Open(atoi(argv[3])); //communicate through specified port 
  assert(sd > -1);

  struct sockaddr_in addr, addr2;
  int rc = UDP_FillSockAddr(&addr, argv[1], atoi(argv[2])); //contact server at specified port
  assert(rc == 0);

  char message[BUFFER_SIZE];
  sprintf(message, "hello world");
  rc = UDP_Write(sd, &addr, message, BUFFER_SIZE); //write message to server@specified-port
  printf("CLIENT:: sent message (%d)\n", rc);
  if (rc > 0) {
    int rc = UDP_Read(sd, &addr2, buffer, BUFFER_SIZE); //read message from ...
    printf("CLIENT:: read %d bytes (message: '%s')\n", rc, buffer);
  }

  return 0;
}

  */
