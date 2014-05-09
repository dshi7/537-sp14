#ifndef __MFS_H__
#define __MFS_H__

#define MFS_DIRECTORY     (0)
#define MFS_REGULAR_FILE  (1)

#define MFS_BLOCK_SIZE    (4096)

typedef struct  __MFS_Stat_t  {
  int type;         //  MFS_DIRECTORY or MFS_REGULAR_FILE
  int size;         //  bytes
  int blocks;       //  number of blocks allocated to file

} MFS_Stat_t;

typedef struct  __MFS_DirEnt_t  {
  int inum;         //  inode number of entry (-1 means entry not used)
  char name[252];   //  up to 252 bytes of name in directory (including \0)
} MFS_DirEnt_t;

int MFS_Init (char *hostname, int port);

#endif