#include <stdio.h>
#include <errno.h>
#include "udp.h"


#define BUFFER_SIZE (4096)
#define FS_SIZE 4
#define BLK_SIZE  8

typedef struct  __INODE_ {
  int type;     //  type field : 0 for directory and 1 for regular file
  int size;     //  size field : file size;
  int blk_num;  //  block field : number of blocks allocated to this file
  int blk[10];  //  at most 10 pointers to blocks
} inode;

inode *fs_inode;
int *fs_blk;
int *fs_inode_bitmap;
int *fs_blk_bitmap;

//  Initialize the file system in the server side
//  Para :  image_name
//  Return  : file descriptor
int SFS_Init (char *image_name) {

  int fd = open (image_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); 
  printf ("fd = %d\n", fd);
  if (fd==-1) {
    printf ("ERROR : %s\n", strerror(errno));
  }
  else {

    fs_inode = (inode*)calloc(FS_SIZE, sizeof(inode));
    fs_blk = (int*)calloc(FS_SIZE, sizeof(int));
    fs_inode_bitmap = (int*)calloc(FS_SIZE, sizeof(int));
    fs_blk_bitmap = (int*)calloc(FS_SIZE, sizeof(int));

    int i, j;

    //  Write inode table in the file
    for (i=0; i<FS_SIZE; i++) 
      write (fd, "\n", 1);
    //  Write FS_SIZE inodes in the file
    for (i=0; i<FS_SIZE; i++) {
      //  Write a single inode
      write (fd, "-1\n", 3);      //  inode type field
      write (fd, "0\n", 2);       //  inode size field
      write (fd, "0\n", 2);       //  inode block field
      for (j=0; j<10; j++)
        write (fd, "NULL\n", 5);  //  inode data block ptr
    }
    //  Write FS_SIZE data blocks in the file
    for (i=0; i<FS_SIZE; i++) {
      //  Write a single data block
      for (j=0; j<BLK_SIZE; j++)
        //  Each data block is 4096 bytes
        write (fd, "00000000", 8);
      write (fd, "\n", 1);
    }
    //  Write FS_SIZE inode bitmap in the file
    for (i=0; i<FS_SIZE; i++) {
      write (fd, "0", 1);
    }
    write (fd, "\n", 1);
    //  Write FS_SIZE block bitmap in the file
    for (i=0; i<FS_SIZE; i++) {
      write (fd, "0", 1);
    }
    write (fd, "\n", 1);

  }
  return  fd;
}

int main(int argc, char *argv[])
{

  if(argc<2)
  {
    printf("Usage: server server-port-number\n");
    exit(1);
  }

  int portid = atoi(argv[1]);
  int sd = UDP_Open(portid); //port # 
  assert(sd > -1);

  printf("waiting in loop\n");

  while (1) {
    struct sockaddr_in s;
    char buffer[BUFFER_SIZE];
    int rc = UDP_Read(sd, &s, buffer, BUFFER_SIZE); //read message buffer from port sd
    if (rc > 0) {
      printf("SERVER:: read %d bytes (message: '%s')\n", rc, buffer);

      if (strcmp(buffer, "init")==0) {
        printf ("this is INIT\n");
        SFS_Init (argv[2]);
      }

      char reply[BUFFER_SIZE];
      sprintf(reply, "reply");
      rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
    }
  }

  return 0;
}


