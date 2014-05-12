#include <stdio.h>
#include <errno.h>
#include "udp.h"
#include "mfs.h"


#define BUFFER_SIZE (4096)
#define FS_SIZE 4096
#define BLK_SIZE 64 
#define STR_LENTH 512
#define INODE_SIZE  25

int INODE_HEAD = 0;
int BLOCK_HEAD = 0;
int INODE_BMAP_HEAD = 0;
int BLOCK_BMAP_HEAD = 0;

int fd;


//  Initialize the file system in the server side
//  Para :  image_name
//  Return  : file descriptor
int SFS_Init (char *image_name) {

  fd = open (image_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); 
//  printf ("fd = %d\n", fd);
  if (fd==-1) {
    printf ("ERROR : %s\n", strerror(errno));
  }
  else {

    int i, j;

    //  Write inode table in the file
    char line[STR_LENTH] = "\n";
    char  zero[2];  
    zero[0] = 0; zero[1] = 0;
    for (i=0; i<FS_SIZE; i++) 
      write (fd, line, STR_LENTH);

    INODE_HEAD = (int)lseek (fd, 0, SEEK_END);
    //  Write FS_SIZE inodes in the file
    for (i=0; i<FS_SIZE; i++) {
      //  inode type field
      write (fd, zero, 1);  
      //  inode size field
      write (fd, zero, 2);  
      //  inode block field
      write (fd, zero, 2);  
      for (j=0; j<10; j++) {
        //  inode data block ptr
        write (fd, zero, 2);  
      }
    }
    
    BLOCK_HEAD = (int)lseek (fd, 0, SEEK_END);
    //  Write FS_SIZE data blocks in the file
    for (i=0; i<FS_SIZE; i++) {
      //  Write a single data block
      for (j=0; j<BLK_SIZE; j++)
        //  Each data block is 4096 bytes
        write (fd, zero, 1);
    }

    INODE_BMAP_HEAD = (int)lseek (fd, 0, SEEK_END);
    //  Write FS_SIZE inode bitmap in the file
    for (i=0; i<FS_SIZE; i++) {
      write (fd, zero, 1);
    }

    BLOCK_BMAP_HEAD = (int)lseek (fd, 0, SEEK_END);
    //  Write FS_SIZE block bitmap in the file
    for (i=0; i<FS_SIZE; i++) {
      write (fd, zero, 1);
    }

    printf ("SEEK %d\n", INODE_HEAD);
    printf ("SEEK %d\n", BLOCK_HEAD-INODE_HEAD);
    printf ("SEEK %d\n", INODE_BMAP_HEAD-BLOCK_HEAD);
    printf ("SEEK %d\n", BLOCK_BMAP_HEAD-INODE_BMAP_HEAD);
  }
  return  fd;
}

int SFS_Lookup (int pinum, char *name) {

  //  Check if the parent inode is out of range
  if (pinum<0 || pinum>=FS_SIZE) {
    printf ("Error : Illegal parent inode number : %d.\n", pinum);
    return -1;
  }

  lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
  char  inode_bmap[FS_SIZE+1];
  read (fd, (void*)inode_bmap, FS_SIZE+1);
  char  is_valid = inode_bmap[pinum];

  //  Check if the parent inode is used 
  if (is_valid==0) {
    printf ("Error : Invalid parent inode number.\n");
    return -1;
  }
  else {
    lseek (fd, INODE_HEAD + pinum*INODE_SIZE, SEEK_SET);
    char  inode[25];
    read (fd, (void*)inode, 25);
    if (inode[0] == MFS_DIRECTORY) {
      printf ("Error : Not a directory\n");
      return -1;
    }
    char  buf[2];
    buf[0] = inode[4];
    buf[1] = inode[3];
    int blk_num = buf[0] | (buf[1]<<8);

    //  . and .. should use at least one block
    assert (blk_num>0); 

    //  all the contaned inodes starts from the second block
    int i;
    for (i=1; i<blk_num; i++) {
      buf[0] = inode[5 + 2*i + 1];
      buf[1] = inode[5 + 2*i];
      int blk_id = buf[0] | (buf[1]<<8);

      //  jump to the data block
      lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
      char  blk_chunk[BLK_SIZE];
      read (fd, (void*)blk_chunk, BLK_SIZE);
      int data_id = 0;

      //  generate an integer using two bytes
      int data = (blk_chunk[data_id]<<8) | blk_chunk[data_id+1];

      while (data) {
        lseek (fd, data * STR_LENTH, SEEK_SET);
        char  file_name[STR_LENTH];
        read (fd, (void*)file_name, STR_LENTH);

        //  return the inode if the required file is found
        if (strcmp(file_name, name)==0)
          return data;

        data_id = data_id+2;
        data = (blk_chunk[data_id]<<8) | blk_chunk[data_id+1];
      }

    }

    //  Not found

    return -1;
  }
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
        
        int val = SFS_Init (argv[2]);

        char  reply[BUFFER_SIZE];
        sprintf (reply, "%d", val);

        printf ("SERVER : val = %d\n", val);
        printf ("SERVER : send %d\n", val);
        rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
        continue;

      }

      if (buffer[0]=='l') {
        char  pinum_str[20];
        char  name[512];
        int   pinum;
        int i=2;
        while (buffer[i]!=' ') {
          pinum_str[i-2] = buffer[i];
          i++;
        }
        pinum = atoi(pinum_str);
    
        i++;
        int j=0;
        while (buffer[i+j] != 0) {
          name[j] = buffer[i+j];
          j++;
        }
        name[j] = '\0';

        int val = SFS_Lookup (pinum, name);
        char reply[BUFFER_SIZE];
        sprintf(reply, "%d", val);
        printf ("SERVER : send %d\n", val);
        rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
      }

//      char reply[BUFFER_SIZE];
//      sprintf(reply, "reply");
//      rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
    }
  }

  return 0;
}


