#include <stdio.h>
#include <errno.h>
#include "udp.h"
#include "mfs.h"


#define BUFFER_SIZE (8192)
#define FS_SIZE 4096
#define BLK_SIZE  4096 
#define STR_LENTH 8192
#define INODE_SIZE  25

#define DEBUG 1
//#undef DEBUG

//  default timeout

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

int getFreeDataBlock (void)
{
  int i;
  char block_bitmap[FS_SIZE];
  lseek (fd, BLOCK_BMAP_HEAD, SEEK_SET);
  read (fd, block_bitmap, FS_SIZE);
  for (i=0; i<FS_SIZE; i++)
    if (!block_bitmap[i])
      return i;
  return -1;
}

//  Initialize the file system in the server side
//  Para :  image_name
//  Return  : file descriptor
int SFS_Init (char *image_name) 
{

  fd = open (image_name, O_RDONLY, S_IRUSR | S_IWUSR);

  INODE_HEAD = FS_SIZE * STR_LENTH;
  BLOCK_HEAD = INODE_HEAD + FS_SIZE * INODE_SIZE;
  INODE_BMAP_HEAD = BLOCK_HEAD + FS_SIZE * BLK_SIZE;
  BLOCK_BMAP_HEAD = INODE_BMAP_HEAD + FS_SIZE;
  if (fd!=-1) {
    fd = open (image_name, O_RDWR, S_IRUSR | S_IWUSR);
    return fd;
  }
  else
    fd = open (image_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

  int i, j;

  //  Write inode table in the file
  char line[STR_LENTH] = "\n";
  char  zero[2];  
  zero[0] = 0; zero[1] = 0;

  for (i=0; i<FS_SIZE; i++) 
    write (fd, line, STR_LENTH);

  int file_length = FS_SIZE * (STR_LENTH + INODE_HEAD + BLK_SIZE + 1 + 1);
  ftruncate (fd, file_length);
//  INODE_HEAD = (int)lseek (fd, 0, SEEK_END);
  //  Write FS_SIZE inodes in the file
//  for (i=0; i<FS_SIZE; i++) {
//    //  inode type field
//    write (fd, (void*)zero, 1);  
//    //  inode size field
//    write (fd, (void*)zero, 2);  
//    //  inode block field
//    write (fd, (void*)zero, 2);  
//    for (j=0; j<10; j++) {
//      //  inode data block ptr
//      write (fd, (void*)zero, 2);  
//    }
//  }

//  BLOCK_HEAD = (int)lseek (fd, 0, SEEK_END);
  //  Write FS_SIZE data blocks in the file
//  for (i=0; i<FS_SIZE; i++) {
//    //  Write a single data block
//    for (j=0; j<BLK_SIZE; j++)
//      //  Each data block is 4096 bytes
//      write (fd, (void*)zero, 1);
//  }

//  INODE_BMAP_HEAD = (int)lseek (fd, 0, SEEK_END);
  //  Write FS_SIZE inode bitmap in the file
  for (i=0; i<FS_SIZE; i++) {
    write (fd, (void*)zero, 1);
  }

//  BLOCK_BMAP_HEAD = (int)lseek (fd, 0, SEEK_END);
  //  Write FS_SIZE block bitmap in the file
//  for (i=0; i<FS_SIZE; i++) {
//    write (fd, (void*)zero, 1);
//  }

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

#ifdef  DEBUG
  printf ("init finished.\n");
#endif

  return  fd;
}

int SFS_Lookup (int pinum, char *name) 
{

  int i, j;
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

    int byte_num = (inode[1]<<8) | inode[2];
    int blk_num = (inode[3]<<8) | inode[4];
#ifdef  DEBUG
    printf ("parent inode : %d\nbyte number : %d\nblock number : %d\n", pinum, byte_num, blk_num);
#endif

    //  . and .. should use at least one block
    assert (blk_num>0); 

    //  all the contaned inodes starts from the second block
    char  blk_chunk[BLK_SIZE];

    int blk_cnt = 0;
    int byte_cnt = 0;
    int blk_id;
    int data;

    while (blk_cnt < blk_num) {
      buf[0] = inode[5+2*blk_cnt];
      buf[1] = inode[6+2*blk_cnt];
      blk_id = (buf[0] << 8) | buf[1];

      //  jump to the data block
      lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
      read (fd, (void*)blk_chunk, BLK_SIZE);

#ifdef  DEBUG
      printf ("Block id = %d\n", blk_id);
      for (j=0; j< BLK_SIZE; j++)
        printf ("%d ", blk_chunk[j]);
      printf ("\n");
#endif

      for ( j = ( (blk_cnt==0)?2:0 ); (j < BLK_SIZE) && (byte_cnt<byte_num); j=j+2 ) {
        data = (blk_chunk[j]<<8) | blk_chunk[j+1];

        if (data==0)
          continue;
        byte_cnt++;

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
        if (strcmp(file_name, name)==0) {
#ifdef  DEBUG
          printf ("FOUND !!! inode [%d] : %s\n", data, file_name);
#endif
          return data;
        }
      }
      if (byte_cnt==byte_num)
        break;
      blk_cnt ++;
    }
    //    for (i=0; i< blk_num; i++) {
    //
    //      buf[0] = inode[5 + 2*i + 1];
    //      buf[1] = inode[5 + 2*i];
    //      int blk_id = buf[0] | (buf[1]<<8);
    //
    //      //  jump to the data block
    //      lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
    //      read (fd, (void*)blk_chunk, BLK_SIZE);
    //
    //#ifdef  DEBUG
    //      printf ("Block id = %d\n", blk_id);
    //      for (j=0; j< BLK_SIZE; j++)
    //        printf ("%d ", blk_chunk[j]);
    //      printf ("\n");
    //#endif
    //
    //      for ( j=0; j < (i==(blk_num-1) ? (byte_num % BLK_SIZE) : BLK_SIZE); j=j+2 ) {
    //
    //        if (i==0 && ( j==0 || j==2 ) ) {
    //          continue;
    //        }
    //
    //
    //        int data = (blk_chunk[j]<<8) | blk_chunk[j+1];
    //
    //        lseek (fd, data * STR_LENTH, SEEK_SET);
    //        char  file_name[STR_LENTH];
    //        read (fd, (void*)file_name, STR_LENTH);
    //
    //        //  force the last "\n" to be '\0'
    //        file_name[strlen(file_name)-1] = '\0';
    //
    //#ifdef  DEBUG
    //        printf ("Look up file inode : %d\n", data);
    //        printf ("The file name : %s\n", file_name);
    //        printf ("Input name : %s\n", name);
    //#endif 
    //
    //        //  return the inode if the required file is found
    //        if (strcmp(file_name, name)==0) {
    //#ifdef  DEBUG
    //          printf ("FOUND !!! inode [%d] : %s\n", data, file_name);
    //#endif
    //          return data;
    //        }
    //      }
    //    }

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

  if (!inode_bitmap[pinum])
    return -1;

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
  }

  //  Read the number of allocated data blocks.
  int byte_num = (pinode[1]<<8) | pinode[2];
  int blk_num = (pinode[3]<<8) | pinode[4];
#ifdef  DEBUG
  printf ("pinode[3]=%d\tpinode[4]=%d\tblk_num=%d\n", pinode[3], pinode[4], blk_num);
#endif

  //  Determining whether to use one allocated data block or a new one before writing an inode number
  if ( byte_num + 2 <= blk_num * BLK_SIZE ) {

    //  Write to the last data block
    int blk_id = ( pinode[5 + 2*(blk_num-1)] << 8 ) | pinode[5 + 2*blk_num-1];

    //  Jump to the data block
    lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
    read (fd, (void*)blk_chunk, BLK_SIZE);

    int inblk_offset = byte_num % BLK_SIZE;

    blk_chunk[inblk_offset] = ( inode_id >> 8 ) & 255;
    blk_chunk[inblk_offset+1] = inode_id & 255;

    lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
    write (fd, (void*)blk_chunk, BLK_SIZE);

  }
  else {

    //  Look for a new data block (not happen a lot)
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

    blk_num ++;
    pinode[3] = (blk_num>>8) & 255;
    pinode[4] = blk_num & 255;
    pinode[4+2*blk_num-1] = (pblock_id>>8) & 255;
    pinode[4+2*blk_num] = pblock_id & 255;
  }

  block_bitmap[block_id] = 1;
  lseek (fd, BLOCK_BMAP_HEAD, SEEK_SET);
  write (fd, (void*)block_bitmap, FS_SIZE);

  //  Update the parent inode
  byte_num = byte_num + 2;  //  one more integer (2 bytes) is added
#ifdef  DEBUG
  printf ("After : blk_num = %d\n", blk_num);
#endif
  pinode[1] = (byte_num>>8) & 255;
  pinode[2] = byte_num & 255;

#ifdef  DEBUG
  printf ("UPDATED PARENT INODE : %d\n", pinum);
  for (i=0; i<INODE_SIZE; i++)
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
  int  i;
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

#ifdef DEBUG
  printf ("required inode : %d\n", inum);
  for (i=0; i<INODE_SIZE; i++)
    printf ("%d ", inode[i]);
  printf ("\n");
#endif

  int type = (int)inode[0];

  int size = (inode[1]<<8) | inode[2];
  int blk = (inode[3]<<8) | inode[4];

  sprintf (reply, "%d %d %d", type, size, blk);
  return;
}

int SFS_Write (int inum, int blk_offset, char *buf)
{
  int i;
  char inode_bitmap[FS_SIZE];
  char inode[INODE_SIZE];

#ifdef  DEBUG
  printf ("buf length=%d\n", strlen(buf));
#endif

  lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
  read (fd, (void*)inode_bitmap, FS_SIZE);
  if (inum<0 || inum>=FS_SIZE || !inode_bitmap[inum]) {
    return -1;
  }

  lseek (fd, INODE_HEAD + inum * INODE_SIZE, SEEK_SET);
  read (fd, (void*)inode, INODE_SIZE);

#ifdef  DEBUG
  printf ("inode = %d\n", inum);
  for (i=0; i<INODE_SIZE; i++)
    printf ("%d ", inode[i]);
  printf ("\n");
#endif

  int type = (int)inode[0];

  if (type==MFS_DIRECTORY)
    return -1;

  int byte_num = (inode[1]<<8) | inode[2];

  int blk_num = (inode[3]<<8) | inode[4];

  int blk_id;

  char  blk_chunk[BLK_SIZE];

  if (blk_offset < 0)
    return -1;
  else if (blk_offset < blk_num) {
    //  write to the last data block
    blk_id = ( inode[5 + 2*(blk_num-1)] << 8 ) | inode[5 + 2*blk_num-1];
    lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
    read (fd, (void*)blk_chunk, BLK_SIZE);
    int inblk_offset = byte_num % BLK_SIZE;

    //  DANGER : segmentation fault risk
    for (i=0; i<strlen(buf); i++) {
      if (inblk_offset+i>=BLK_SIZE)
        continue;
      blk_chunk[inblk_offset+i] = buf[i];
    }

    lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
    write (fd, (void*)blk_chunk, BLK_SIZE);
  }
  else if (blk_offset ==blk_num) {

    //  each file can take at most 10 data blocks
    if (blk_num==10)
      return -1;
    //  find a free data block
    blk_id = getFreeDataBlock();
    if (blk_id==-1)
      return blk_id;
    //  Update the data block
    lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
    read (fd, (void*)blk_chunk, BLK_SIZE);

    //  DANGER : segmentation fault risk
    for (i=0; i<strlen(buf); i++) {
      if (i>=BLK_SIZE)
        continue;
      blk_chunk[i] = buf[i];
    }

    lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
    write (fd, (void*)blk_chunk, BLK_SIZE);

    blk_num++;

    inode[3] = (blk_num>>8) & 255;
    inode[4] = blk_num & 255;

    inode[5+2*(blk_num-1)] = (blk_id>>8) & 255;
    inode[5+2*blk_num-1] = blk_id & 255;

  }
  else {
#ifdef  DEBUG
    printf ("blk_offset = %d\tblk_num = %d\n", blk_offset, blk_num);
#endif
    return -1;
  }

  //  printf ("strlen = %d\n", strlen(buf));
  byte_num = byte_num + strlen(buf);
  //  printf ("byte_num = %d\n", byte_num);

  inode[1] = (byte_num>>8) & 255;
  inode[2] = byte_num & 255;

#ifdef DEBUG
  printf ("updated inode : %d\n", inum);
  for (i=0; i<INODE_SIZE; i++)
    printf ("%d ", inode[i]);
  printf ("\n");
#endif

  lseek (fd, INODE_HEAD + inum * INODE_SIZE, SEEK_SET);
  write (fd, (void*)inode, INODE_SIZE);

  return 0;

}

void  SFS_Read (int inum, int blk_offset, char *buf)
{
  int i;
  if (inum<0 || inum>=FS_SIZE) {
    strcpy (buf, "error");
    return;
  }

  //  read the inode bitmap
  char  inode_bitmap[FS_SIZE];
  lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
  read (fd, (void*)inode_bitmap, FS_SIZE);

  if (!inode_bitmap[inum]) {
    strcpy (buf, "error");
    return;
  }

  //  read the inode
  char  inode[INODE_SIZE];

  lseek (fd, INODE_HEAD+inum*INODE_SIZE, SEEK_SET);
  read (fd, (void*)inode, INODE_SIZE);

#ifdef  DEBUG
  printf ("inode = %d\n", inum);
  for (i=0; i<INODE_SIZE; i++)
    printf ("%d ", inode[i]);
  printf ("\n");
#endif

  if (inode[0]==MFS_REGULAR_FILE) {
    //  read the specified data block
    printf ("this is a regular file\n");
    int blk_num = (inode[3]<<8) | inode[4];
    if (blk_offset<0 || blk_offset>=blk_num) {
      strcpy (buf, "error");
      return;
    }
    int blk_id = (inode[5+2*blk_offset]<<8) | inode[5+2*blk_offset+1];

    lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
    read (fd, (void*)buf, BLK_SIZE);

    return;
  }
  else {
    //  read the specified directory
    lseek (fd, inum * STR_LENTH, SEEK_SET);
    read (fd, (void*)buf, STR_LENTH);
    buf[strlen(buf)-1] = '\0';

    return;
  }
}

int SFS_Unlink (int pinum, char *name)
{
  int i, j, blk_id, blk_num, byte_num;
  char  blk_chunk[BLK_SIZE];
  char  buf[2];
  char  inode[INODE_SIZE];
  if (pinum<0 || pinum>=FS_SIZE)
    return -1;

  int blk_cnt = 0;
  int byte_cnt = 0;
  int data;
  int inode_id = -1;

#ifdef  DEBUG
  printf ("retval from SFS_Lookup : inode_id = %d\n",  inode_id);
#endif
  char  inode_bitmap[FS_SIZE];
  lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
  read (fd, (void*)inode_bitmap, FS_SIZE);

  //  Return failure if the pinum doesn't exist.
  if (inode_bitmap[pinum]==0)
    return -1;

  lseek (fd, INODE_HEAD + pinum * INODE_SIZE, SEEK_SET);
  read (fd, (void*)inode, INODE_SIZE);
#ifdef  DEBUG
  printf ("inode = %d\n", pinum);
  for (i=0; i<INODE_SIZE; i++)
    printf ("%d ", inode[i]);
  printf ("\n");

  byte_num = (inode[1]<<8) | inode[2];
  blk_num = (inode[3]<<8) | inode[4];
  blk_id = (inode[5]<<8) | inode[6];

  //  jump to the data block
  lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
  read (fd, (void*)blk_chunk, BLK_SIZE);

  printf ("Block id = %d\n", blk_id);
  for (j=0; j< BLK_SIZE; j++)
    printf ("%d ", blk_chunk[j]);
  printf ("\n");
#endif

  while (blk_cnt < blk_num) {
    buf[0] = inode[5+2*blk_cnt];
    buf[1] = inode[6+2*blk_cnt];
    blk_id = (buf[0] << 8) | buf[1];

    //  jump to the data block
    lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
    read (fd, (void*)blk_chunk, BLK_SIZE);

#ifdef  DEBUG
    printf ("Block id = %d\n", blk_id);
    for (j=0; j< BLK_SIZE; j++)
      printf ("%d ", blk_chunk[j]);
    printf ("\n");
#endif

    for ( j = ( (blk_cnt==0)?2:0 ); (j < BLK_SIZE) && (byte_cnt<byte_num); j=j+2 ) {
      data = (blk_chunk[j]<<8) | blk_chunk[j+1];

      if (data==0)
        continue;
      byte_cnt++;

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
      if (strcmp(file_name, name)==0) {
#ifdef  DEBUG
        printf ("FOUND !!! inode [%d] : %s\n", data, file_name);
#endif
        inode_id = data;
        
        //  zero those byte
        blk_chunk[j] = 0;
        blk_chunk[j+1] = 0;
        lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
        write (fd, (void*)blk_chunk, BLK_SIZE);
        break;
      }
    }
    if (byte_cnt==byte_num || inode_id != -1)
      break;
    blk_cnt ++;
  }

  int type = inode[0];

  //  Return failure if the pinum is a regular file.
  if (type == MFS_REGULAR_FILE)
    return -1;

  if (inode_id==-1)
    return 0;

  char  block_bitmap[FS_SIZE];

  //  Return zero if the name is not existing.
  if (inode_id == -1)
    return 0;

  lseek (fd, INODE_HEAD + inode_id * INODE_SIZE, SEEK_SET);
  read (fd, (void*)inode, INODE_SIZE);

  type = inode[0];
  int size = (inode[1]<<8) | inode[2];
  int block_num = (inode[3]<<8) | inode[4];

  if (type==MFS_DIRECTORY) {

    //  Return failure if the directory is not empty.
    if (size>4)
      return -1;

    //  Only one data block
    assert (block_num==1);
    blk_id = (inode[5]<<8) | inode[6];

    //  Free the inode bitmap.
    lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
    read (fd, (void*)inode_bitmap, FS_SIZE);
    inode_bitmap[inode_id] = 0;
    lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
    write (fd, (void*)inode_bitmap, FS_SIZE);

    //  Free the block bitmap.
    lseek (fd, BLOCK_BMAP_HEAD, SEEK_SET);
    read (fd, (void*)block_bitmap, FS_SIZE);
    block_bitmap[blk_id] = 0;
    lseek (fd, BLOCK_BMAP_HEAD, SEEK_SET);
    write (fd, (void*)block_bitmap, FS_SIZE);

  }
  else {

    //  Free the inode bitmap.
    lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
    read (fd, (void*)inode_bitmap, FS_SIZE);
    inode_bitmap[inode_id] = 0;
    lseek (fd, INODE_BMAP_HEAD, SEEK_SET);
    write (fd, (void*)inode_bitmap, FS_SIZE);

    //  Free the block bitmap.
    lseek (fd, BLOCK_BMAP_HEAD, SEEK_SET);
    read (fd, (void*)block_bitmap, FS_SIZE);
    int i, block_id;

    for (i=0; i<block_num; i++) {
      block_id = (inode[5+2*i]<<8) | inode[6+2*i];
      block_bitmap[block_id] = 0;
    }
    lseek (fd, BLOCK_BMAP_HEAD, SEEK_SET);
    write (fd, (void*)block_bitmap, FS_SIZE);

  }

  //  Free the inode table
  char  inode_table_entry[STR_LENTH];
  lseek (fd, inode_id * STR_LENTH, SEEK_SET);
  read (fd, (void*)inode_table_entry, STR_LENTH);
#ifdef  DEBUG
  printf ("BEFORE : %s", inode_table_entry);
#endif
  sprintf (inode_table_entry, "\n");
  lseek (fd, inode_id * STR_LENTH, SEEK_SET);
  write (fd, (void*)inode_table_entry, STR_LENTH);
#ifdef  DEBUG
  printf ("AFTER  : %s", inode_table_entry);
#endif


  lseek (fd, INODE_HEAD + pinum * INODE_SIZE, SEEK_SET);
  read (fd, (void*)inode, INODE_SIZE);

  //  Update the size in parent directory
  size = (inode[1]<<8) | inode[2];
  size = size-2;
  inode[1] = (size >> 8) & 255;
  inode[2] = size & 255;

#ifdef  DEBUG
  printf ("inode = %d\n", pinum);
  for (i=0; i<INODE_SIZE; i++)
    printf ("%d ", inode[i]);
  printf ("\n");

  blk_id = (inode[5]<<8) | inode[6];

  //  jump to the data block
  lseek (fd, BLOCK_HEAD + blk_id * BLK_SIZE, SEEK_SET);
  read (fd, (void*)blk_chunk, BLK_SIZE);

  printf ("Block id = %d\n", blk_id);
  for (j=0; j< BLK_SIZE; j++)
    printf ("%d ", blk_chunk[j]);
  printf ("\n");
#endif

  lseek (fd, INODE_HEAD + pinum * INODE_SIZE, SEEK_SET);
  write (fd, (void*)inode, INODE_SIZE);

  return 0;
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
    printf ("\n**************\nSERVER RECEIVES : %s\n", buffer);

    parseSocketMessage (buffer, mfs_argv, &mfs_argc, 0);


    if (rc > 0) {

      //      printf("SERVER:: read %d bytes (message: '%s')\n", rc, buffer);

      if (strcmp(buffer, "init")==0) {

        int val = SFS_Init (argv[2]);

        char  reply[BUFFER_SIZE];
        sprintf (reply, "%d", fd);

        fsync (fd);
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
        fsync (fd);
        rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
      }

      if (buffer[0]=='s') {

        int inum = atoi (mfs_argv[1]);

        char reply[BUFFER_SIZE];

        SFS_Stat (inum, reply);

        //        printf ("SERVER : send %s\n", reply);
        rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
      }

      if (buffer[0]=='w') {

        int inum = atoi (mfs_argv[1]);
        int blk_offset = atoi (mfs_argv[2]);
        char  buf[STR_LENTH];
        strcpy (buf, mfs_argv[3]);

        int val = SFS_Write (inum, blk_offset, buf);
        char reply[BUFFER_SIZE];
        sprintf(reply, "%d", val);
        //        printf ("SERVER : send %d\n", val);
        fsync (fd);
        rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
      }

      if (buffer[0]=='r') {

        int inum = atoi (mfs_argv[1]);
        int blk_offset = atoi (mfs_argv[2]);

        //  in MFS_Read the size of read data is fixed as BLK_SIZE
        char  reply[BLK_SIZE+1];

        reply[BLK_SIZE] = '\0';
        SFS_Read (inum, blk_offset, reply);

        rc = UDP_Write(sd, &s, reply, BLK_SIZE+1); //write message buffer to port sd
      }

      if (buffer[0]=='u') {
        int pinum = atoi (mfs_argv[1]);
        char  name[512];
        strcpy (name, mfs_argv[2]);

        int val = SFS_Unlink (pinum, name);
        char reply[512];
        sprintf (reply, "%d", val);

        fsync (fd);
        rc = UDP_Write(sd, &s, reply, BUFFER_SIZE); //write message buffer to port sd
      }
    }
  }

  return 0;
}


