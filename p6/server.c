#include <stdio.h>
#include <errno.h>
#include "udp.h"
#include "mfs.h"


#define BUFFER_SIZE (4096)
#define FS_SIZE 128
#define BLK_SIZE 64 
#define STR_LENTH 512
#define INODE_SIZE  25

#define DEBUG 1

int INODE_HEAD = 0;
int BLOCK_HEAD = 0;
int INODE_BMAP_HEAD = 0;
int BLOCK_BMAP_HEAD = 0;

int fd;

int mfs_argc;
char *mfs_argv[512];
char buffer[BUFFER_SIZE];

//  Parse the buffer[BUFFER_SIZE] into mfs_argv[512]
void  parseSocketMessage (char *buffer, char **mfs_argv, int *mfs_argc, int show_detail) 
{

  char *token;
  int i=0;

  token = strtok (buffer, ";");

  *mfs_argc = 0;
  mfs_argv[*mfs_argc] = token;

  while (token != NULL) {
    token = strtok (NULL, ";");
    mfs_argv[++ (*mfs_argc)] = token;
  }

  if (show_detail)
    while (i < *mfs_argc)
      puts (mfs_argv[i++]);
}

//  Initialize the file system in the server side
//  Para :  image_name
//  Return  : file descriptor
int SFS_Init (char *image_name) 
{

  fd = open (image_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); 
  //  printf ("fd = %d\n", fd);
  if (fd==-1) {
    //    printf ("ERROR : %s\n", strerror(errno));
    return -1;
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
      write (fd, (void*)zero, 1);  
      //  inode size field
      write (fd, (void*)zero, 2);  
      //  inode block field
      write (fd, (void*)zero, 2);  
      for (j=0; j<10; j++) {
        //  inode data block ptr
        write (fd, (void*)zero, 2);  
      }
    }

    BLOCK_HEAD = (int)lseek (fd, 0, SEEK_END);
    //  Write FS_SIZE data blocks in the file
    for (i=0; i<FS_SIZE; i++) {
      //  Write a single data block
      for (j=0; j<BLK_SIZE; j++)
        //  Each data block is 4096 bytes
        write (fd, (void*)zero, 1);
    }

    INODE_BMAP_HEAD = (int)lseek (fd, 0, SEEK_END);
    //  Write FS_SIZE inode bitmap in the file
    for (i=0; i<FS_SIZE; i++) {
      write (fd, (void*)zero, 1);
    }

    BLOCK_BMAP_HEAD = (int)lseek (fd, 0, SEEK_END);
    //  Write FS_SIZE block bitmap in the file
    for (i=0; i<FS_SIZE; i++) {
      write (fd, (void*)zero, 1);
    }

#ifdef  DEBUG
    printf ("SEEK %d\n", INODE_HEAD);
    printf ("SEEK %d\n", BLOCK_HEAD-INODE_HEAD);
    printf ("SEEK %d\n", INODE_BMAP_HEAD-BLOCK_HEAD);
    printf ("SEEK %d\n", BLOCK_BMAP_HEAD-INODE_BMAP_HEAD);
#endif  

    //  Initialize the root directory

    //  Update the inode bitmap
    char  inode_bitmap[FS_SIZE];    //  Inode bitmap
    lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
    read (fd, (void*)inode_bitmap, FS_SIZE);
    inode_bitmap[0] = 1;

    lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
    write (fd, (void*)inode_bitmap, FS_SIZE);

    //  Update the block bitmap
    char  block_bitmap[FS_SIZE];    //  Block bitmap
    lseek (fd, BLOCK_BMAP_HEAD, SEEK_SET);
    read (fd, (void*)block_bitmap, FS_SIZE);
    block_bitmap[0] = 1;

    lseek (fd, BLOCK_BMAP_HEAD, SEEK_SET);
    write (fd, (void*)block_bitmap, FS_SIZE);

    //  Update the inode
    char  inode[INODE_SIZE];
    lseek (fd, INODE_HEAD, SEEK_SET);
    read (fd, (void*)inode, INODE_SIZE);
    inode[0] = 0; //  type
    inode[2] = 4; //  bytes
    inode[4] = 1; //  block size : take at least 1 block

    lseek (fd, INODE_HEAD, SEEK_SET);
    write (fd, (void*)inode, INODE_SIZE);

  }
  return  fd;
}

int SFS_Lookup (int pinum, char *name) 
{

  int i;
  //  Check if the parent inode is out of range
  if (pinum<0 || pinum>=FS_SIZE) {
#ifdef  DEBUG
    printf ("Error : Illegal parent inode number : %d.\n", pinum);
#endif
    return -1;
  }

  char  inode_bmap[FS_SIZE+1];
  lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
  read (fd, (void*)inode_bmap, FS_SIZE+1);
  char  is_valid = inode_bmap[pinum];

  //  Check if the parent inode is used 
  if (is_valid==0) {
#ifdef  DEBUG
    printf ("Error : Invalid parent inode number.\n");
#endif
    return -1;
  }
  else {
    lseek (fd, INODE_HEAD + pinum*INODE_SIZE, SEEK_SET);
    char  inode[25];
    char  buf[2];
    read (fd, (void*)inode, 25);
#ifdef  DEBUG
    printf ("Parent INODE : %d\n", pinum);
    for (i=0; i<25; i++)
      printf ("%d ", inode[i]);
    printf ("\n");
#endif
    if (inode[0] == MFS_REGULAR_FILE) {
#ifdef  DEBUG
      printf ("Error : Input inode should be MFS_DIRECTORY.\n");
#endif
      return -1;
    }

    //  Lookup . or ..
    if (strcmp (name, ".")==0)
      return  pinum;

    if (strcmp (name, "..")==0) {
      int block_id = (buf[5]<<8) | buf[6];
      lseek (fd, BLOCK_HEAD+block_id * BLK_SIZE, SEEK_SET);
      read (fd, (void*)buf, 2);
      return (buf[0]<<8) | buf[1];
    }

    buf[0] = inode[4];
    buf[1] = inode[3];
    int blk_num = buf[0] | (buf[1]<<8);
#ifdef  DEBUG
    printf ("parent inode : %d\nblock number : %d\n", pinum, blk_num);
#endif

    //  . and .. should use at least one block
    assert (blk_num>0); 

    //  all the contaned inodes starts from the second block
    for (i=1; i<blk_num; i++) {
      buf[0] = inode[5 + 2*i + 1];
      buf[1] = inode[5 + 2*i];
      int blk_id = buf[0] | (buf[1]<<8);

      //  jump to the data block
      lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
      char  blk_chunk[BLK_SIZE];
      read (fd, (void*)blk_chunk, BLK_SIZE);

#ifdef  DEBUG
      int j=0;
      printf ("Block ID : %d\n", blk_id);
      for (j=0; j<BLK_SIZE; j++)
        printf ("%d ", blk_chunk[j]);
      printf ("\n");
#endif

      int data_id = 0;

      //  generate an integer using two bytes
      int data = (blk_chunk[data_id]<<8) | blk_chunk[data_id+1];

      while (data) {

        lseek (fd, data * STR_LENTH, SEEK_SET);
        char  file_name[STR_LENTH];
        read (fd, (void*)file_name, STR_LENTH);
        //  force the last "\n" to be '\0'
        file_name[strlen(file_name)-1] = '\0';

#ifdef  DEBUG
        printf ("Look up file inode : %d\n", data);
        printf ("The file name : %s\n", file_name);
        printf ("Input name : %s\n", name);
#endif 

        //  return the inode if the required file is found
        if (strcmp(file_name, name)==0)
          return data;

        data_id = data_id+2;
        data = (blk_chunk[data_id]<<8) | blk_chunk[data_id+1];
      }

    }

    //  Not found

#ifdef  DEBUG
    printf ("Error : the name is not found.\n");
#endif
    return -1;
  }
}

int SFS_Create (int pinum, int type, char *name) 
{

  //  Return success if NAME already exists.
  if (SFS_Lookup (pinum, name)!=-1)
    return 0;

  //  Return failure is pinum is out of scope.
  if (pinum <0 || pinum>=FS_SIZE)
    return -1;

  //  Successful SFS_Create : 
  //    pinum is valid;
  //    pinum has enough space to write;
  //    
  //    free inode is available;
  //    free block is available is type==MFS_DIRECTORY
  int is_success = 0;
  int inode_id = -1;  //  Initialzed as -1 (unallocated)
  int block_id = -1;  //  Initialzed as -1 (unallocated)
  int pblock_id = -1;  //  Initialzed as -1 (unallocated)
  int  i;

  char  pinode[25];   //  The parent inode
  lseek (fd, INODE_HEAD + pinum*INODE_SIZE, SEEK_SET);
  read (fd, (void*)pinode, 25);
#ifdef  DEBUG
  printf ("PARENT INODE : %d\n", pinum);
  for (i=0; i<25; i++)
    printf ("%d ", pinode[i]);
  printf ("\n");
#endif

  char  inode_bitmap[FS_SIZE];    //  Inode bitmap
  lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
  read (fd, (void*)inode_bitmap, FS_SIZE);

  char  block_bitmap[FS_SIZE];    //  Block bitmap
  lseek (fd, BLOCK_BMAP_HEAD, SEEK_SET);
  read (fd, (void*)block_bitmap, FS_SIZE);

  char  blk_chunk[BLK_SIZE];

  //  Parent must be directory.
  if (pinode[0]!=MFS_DIRECTORY)
    return -1;

  //  Find a free inode for allocation
  for (i=1; i<FS_SIZE; i++) 
    if (!inode_bitmap[i]) {
      inode_id = i;
      break;
    }
#ifdef  DEBUG
  printf ("allocate inode id : %d\n", inode_id);
#endif
  if (inode_id==-1)
    return -1;
  inode_bitmap[inode_id] = 1;

  //  Update the inode bitmap
  lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
  write (fd, (void*)inode_bitmap, FS_SIZE);

  char inode_table_entry[STR_LENTH];  
  lseek (fd, inode_id *STR_LENTH, SEEK_SET);
  read (fd, (void*)inode_table_entry, STR_LENTH);

  char  inode[INODE_SIZE];
  lseek (fd, INODE_HEAD + inode_id*INODE_SIZE, SEEK_SET);
  read (fd, (void*)inode, INODE_SIZE);

  //  Find a free block if to create a directory
  if (type==MFS_DIRECTORY) {
    for (i=1; i<FS_SIZE; i++)
      if (!block_bitmap[i]) {
        block_id = i;
        break;
      }
#ifdef  DEBUG
    printf ("allocate block id : %d\n", block_id);
#endif
    if (block_id==-1)
      return -1;
    block_bitmap[block_id] = 1;
  }

  //  Read the number of allocated data blocks.
  int byte_num = (pinode[1]<<8) | pinode[2];
  int blk_num = (pinode[3]<<8) | pinode[4];
#ifdef  DEBUG
  printf ("pinode[3]=%d\tpinode[4]=%d\tblk_num=%d\n", pinode[3], pinode[4], blk_num);
#endif

  //  Visit each allocated block. ( from 1 .. blk_num-1 )
  for (i=1; i<blk_num; i++) {

    if (is_success==1)
      break;
    //  Determine the allocated block id.
    int blk_id = ( pinode[5+2*i]<<8 ) | pinode[5+2*i+1];
    //  Jump to the data block.
    lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
    read (fd, (void*)blk_chunk, BLK_SIZE);

    //  Find the empty space to write.
    int data_id = 0;
    while (data_id<BLK_SIZE) {
      //  Continue if the refered two bytes are already allocated.
      if (blk_chunk[data_id] || blk_chunk[data_id+1]) {
        data_id = data_id +2;
        continue;
      }
      else {
        blk_chunk[data_id+1] = inode_id & 255;
        blk_chunk[data_id] = (inode_id >>8 ) & 255;
        //  Write the block to the image file
        lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
        write (fd, (void*)blk_chunk, BLK_SIZE);

        is_success = 1;
#ifdef  DEBUG
        printf ("To write : block_id = %d, data_id = %d\n", i, data_id);
#endif
        break;
      }
    }
  }
  //  Look for a new data block (not happen a lot)
  if (!is_success) {

#ifdef  DEBUG
    printf ("!!! Look for a new data block !!!\n");
#endif
    //  Return failure if the parent inode already used 10 blocks
    if (blk_num==10)
      return  -1;

    for (i=1; i<FS_SIZE; i++) 
      if (!block_bitmap[i]) {
        pblock_id = i;
        break;
      }
    if (pblock_id==-1)
      return -1;
    block_bitmap[pblock_id] = 1;

    //  Update the newly allocated block
    lseek (fd, BLOCK_HEAD + pblock_id * BLK_SIZE, SEEK_SET);
    read (fd, (void*)blk_chunk, BLK_SIZE);
    blk_chunk[1] = inode_id & 255;
    blk_chunk[0] = (inode_id >>8 ) & 255;

#ifdef  DEBUG
    int j=0;
    printf ("Write inode %d into block id %d\n", inode_id, pblock_id);
    for (j=0; j<BLK_SIZE; j++)
      printf ("%d ", blk_chunk[j]);
    printf ("\n");
#endif
    lseek (fd, BLOCK_HEAD + pblock_id * BLK_SIZE, SEEK_SET);
    write (fd, (void*)blk_chunk, BLK_SIZE);

#ifdef  DEBUG
    lseek (fd, BLOCK_HEAD + pblock_id * BLK_SIZE, SEEK_SET);
    read (fd, (void*)blk_chunk, BLK_SIZE);
    for (j=0; j<BLK_SIZE; j++)
      printf ("%d ", blk_chunk[j]);
    printf ("\n");
#endif


    //  Update the block bitmap
    block_bitmap[block_id] = 1;

    lseek (fd, BLOCK_BMAP_HEAD, SEEK_SET);
    write (fd, (void*)block_bitmap, FS_SIZE);

    //  Update the parent inode
#ifdef  DEBUG
    printf ("Before : blk_num = %d\n", blk_num);
#endif
    byte_num = byte_num + 2;  //  one more integer (2 bytes) is added
    blk_num ++;
#ifdef  DEBUG
    printf ("After : blk_num = %d\n", blk_num);
#endif
    pinode[1] = (byte_num>>8) & 255;
    pinode[2] = byte_num & 255;
    pinode[3] = (blk_num>>8) & 255;
    pinode[4] = blk_num & 255;
    pinode[4+2*blk_num-1] = (pblock_id>>8) & 255;
    pinode[4+2*blk_num] = pblock_id & 255;

    is_success = 1;
  }
#ifdef  DEBUG
  printf ("UPDATED PARENT INODE : %d\n", pinum);
  for (i=0; i<25; i++)
    printf ("%d ", pinode[i]);
  printf ("\n");
#endif
  lseek (fd, INODE_HEAD + pinum * INODE_SIZE, SEEK_SET);
  write (fd, (void*)pinode, INODE_SIZE);

  //  Update the inode table
  lseek (fd, inode_id *STR_LENTH, SEEK_SET);
  sprintf (inode_table_entry, "%s\n", name);
  write (fd, (void*)inode_table_entry, STR_LENTH);

  //  Update the newly allocated inode
  inode[0] = type;
  //  Allocate the data block if type==MFS_DIRECTORY
  if (inode[0]==MFS_DIRECTORY) {
    inode[2] = 4;
    inode[4] = 1;
    inode[5] = (block_id>>8) & 255;
    inode[6] = block_id & 255;

    //  Update the allocated data block
    lseek (fd, BLOCK_HEAD + block_id * BLK_SIZE, SEEK_SET);
    char  buf[4];
    read (fd, buf, 4);
    //  first two bytes are the parent directory inode
    buf[0] = (pinum>>8)&255;
    buf[1] = pinum&255;
    //  second two bytes are the current directory inode
    buf[2] = (inode_id>>8)&255;
    buf[3] = inode_id&255;
    write (fd, buf, 4);
  }
#ifdef  DEBUG
  printf ("UPDATED INODE : %d\n", inode_id);
  for (i=0; i<25; i++)
    printf ("%d ", inode[i]);
  printf ("\n");
#endif
  lseek (fd, INODE_HEAD + inode_id*INODE_SIZE, SEEK_SET);
  write (fd, (void*)inode, INODE_SIZE);


  return 0;

}

void SFS_Stat (int inum, char *reply)
{
  char inode_bitmap[FS_SIZE];
  char inode[INODE_SIZE];

  lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
  read (fd, (void*)inode_bitmap, FS_SIZE);
  if (inum<0 || inum>=FS_SIZE || !inode_bitmap[inum]) {
    sprintf (reply, "error");
    return;
  }

  lseek (fd, INODE_HEAD + inum * INODE_SIZE, SEEK_SET);
  read (fd, (void*)inode, INODE_SIZE);

  int type = (int)inode[0];

  int size = (inode[1]<<8) | inode[2];
  int blk = (inode[3]<<8) | inode[4];
  if (blk>1)
    blk --;     //  . and .. should not take a separate data block

  sprintf (reply, "%d %d %d", type, size, blk);
  return;
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

  //  printf("waiting in loop\n");

  while (1) {
    struct sockaddr_in s;
    int rc = UDP_Read(sd, &s, buffer, BUFFER_SIZE); //read message buffer from port sd
//    printf ("SERVER RECEIVES : %s\n", buffer);

    parseSocketMessage (buffer, mfs_argv, &mfs_argc, 0);


    if (rc > 0) {

      //      printf("SERVER:: read %d bytes (message: '%s')\n", rc, buffer);

      if (strcmp(buffer, "init")==0) {

        int val = SFS_Init (argv[2]);

        char  reply[BUFFER_SIZE];
        sprintf (reply, "%d", val);

        //        printf ("SERVER : val = %d\n", val);
        //        printf ("SERVER : send %d\n", val);
        rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
        continue;

      }

      if (buffer[0]=='l') {
        int pinum = atoi (mfs_argv[1]);
        char  name[512];
        strcpy (name, mfs_argv[2]);

        int val = SFS_Lookup (pinum, name);
        char reply[BUFFER_SIZE];
        sprintf(reply, "%d", val);
//        printf ("SERVER : send %d\n", val);
        rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
      }

      if (buffer[0]=='c') {

        int pinum = atoi (mfs_argv[1]);
        int type = atoi (mfs_argv[2]);
        char  name[512];
        strcpy (name, mfs_argv[3]);

        int val = SFS_Create (pinum, type, name);
        char reply[BUFFER_SIZE];
        sprintf(reply, "%d", val);
//        printf ("SERVER : send %d\n", val);
        rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
      }

      if (buffer[0]=='s') {

        int inum = atoi (mfs_argv[1]);

        char reply[BUFFER_SIZE];
        
        SFS_Stat (inum, reply);

//        printf ("SERVER : send %d\n", val);
        rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
      }
    }
  }

  return 0;
}


