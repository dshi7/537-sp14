/*
 * =====================================================================================
 *
 *       Filename:  mem.c
 *
 *    Description:  implement the functions defined in "mem.h"
 *
 *        Created:  02/28/2014 07:49:56 PM
 *       Compiler:  gcc
 *
 *         Author:  Daohang Shi
 *
 * =====================================================================================
 */

#include<sys/mman.h>
#include<fcntl.h>
#include<stdio.h>
#include<unistd.h>
#include "mem.h"

#define FREE_PATTERN  0xDEADBEEF
#define PADDING_PATTERN 0xABCDDCBA

//typedef struct {
//
//  /* size of the current free chunk */
//  int size;
//  /* pointer to the next free chunk */
//  void* next_ptr;
//
//} freeChunk;

#define PADDING_SIZE 64

/* global variables of debug signal and error signal */
int debug_enable;
int m_error;

void *ptr_free_list = NULL;
int size_of_region;

long *walking_ptr = NULL;

int Mem_Init(int sizeOfRegion, int debug) {

  //  called one time by a process using your routine

  int fd = open("/dev/zero", O_RDWR);   //  file description

  ptr_free_list = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  size_of_region = sizeOfRegion;

  if (ptr_free_list==MAP_FAILED) {
    m_error = E_BAD_ARGS;
    return -1;
  }
  close(fd);

  //  debug mode setup starts
  if (debug==1) {
    debug_enable = 1;
    int *tmp = ptr_free_list;     //  use an integer pointer for its 4-byte alignment
    while (tmp<(int*)(ptr_free_list+sizeOfRegion)) {
      *tmp = FREE_PATTERN;
      tmp++;
    }
  }
  //  debug mode setup ends

  //  the 1st 8-byte is for address 
  long  *ptr_next_free_chunk = (long*)ptr_free_list;
  *ptr_next_free_chunk = (long)ptr_next_free_chunk;

  //  the 2st 4-byte is for size of current free chunk
  int *ptr_size_chunk = (int *)(ptr_next_free_chunk+1);
  *ptr_size_chunk = sizeOfRegion;

  //  initialize the walking_ptr
  walking_ptr = ptr_next_free_chunk;

  return 0;   //  success

}

void *Mem_Alloc(int size) { 

  printf("%08x\n", ptr_free_list);
  printf("%d\n", *((int*)(ptr_free_list+8)));

  /* 4-byte assigned */
  size = ((size-1)/4+1) *4;

  int *ptr_available_size;
  long next_address;

  //  traverse the free list
  void *pinned_stamp = walking_ptr;  

  do {
    //  find the pointer to the available size of the current free chunk
    ptr_available_size = (int*)(walking_ptr+8);
    if (*ptr_available_size > size+PADDING_SIZE*2) {
      //  each allocatd space should have 64-byte paddings on both sides
      *ptr_available_size = *ptr_available_size - size;
      *(int*)(walking_ptr+*ptr_available_size+PADDING_SIZE-4) = size;
      return walking_ptr+*ptr_available_size+PADDING_SIZE;
    }
    else {
      next_address = *walking_ptr;
      walking_ptr = (long*)next_address;
    }
  }
  while (walking_ptr != pinned_stamp);

  if (debug_enable==1) {
    //  set the top padding
    int *tmp = (int*)(walking_ptr+*ptr_available_size);  
    while (tmp< (int*)(walking_ptr+*ptr_available_size+PADDING_SIZE-4)) {
      *tmp = PADDING_PATTERN;
      tmp++;
    }
    //  set the bottom padding
    tmp = (int*)(walking_ptr+*ptr_available_size+PADDING_SIZE+size);
    while (tmp< (int*)(walking_ptr+*ptr_available_size+PADDING_SIZE*2+size)) {
      *tmp = PADDING_PATTERN;
      tmp++;
    }
    //  check the filling values 
    long *free_chunk = ptr_free_list;
    do {
      for (tmp = (int*)((char*)free_chunk+12); tmp < (int*)free_chunk +*(free_chunk+8); tmp++) {
        if (*tmp != FREE_PATTERN) {
          m_error = E_CORRUPT_FREESPACE;
          return NULL;
        }
      }
      //  not the last free chunk
      if (tmp<(int*)(ptr_free_list+size_of_region)) {
        //  check the top padding
        for (tmp = (int*)free_chunk+*(free_chunk+8); tmp < (int*)free_chunk+*(free_chunk+8)+PADDING_SIZE-4; tmp ++ ) {
          if (*tmp != PADDING_PATTERN) {
            m_error = E_CORRUPT_FREESPACE;
            return NULL;
          }
        }
        //  check the bot padding
        for (tmp = (int*)(*free_chunk)-PADDING_PATTERN; tmp <(int*)(*free_chunk); tmp++) {
          if (*tmp != PADDING_PATTERN) {
            m_error = E_CORRUPT_FREESPACE;
            return NULL;
          }
        }
      }
    }
    while (*free_chunk != ptr_free_list);
  }

  //  no available space for allocation
  m_error = E_NO_SPACE;

  return NULL;

}

int  Mem_Free(void *ptr) {

  //  find the last free chunk prior to allocated space
  long prev_free_chunk = (long)ptr_free_list;

  while (prev_free_chunk<(long)ptr) {
    prev_free_chunk = *(long*)prev_free_chunk;
    //  check all the free chunks
    if (prev_free_chunk==(long)ptr_free_list) {
      m_error = E_BAD_POINTER;
      return -1;
    }
  }

  //  debug mode starts
  //  check whether either side of padding is overwritten
  int *tmp;
  int allocated_size = *(int*)(ptr-4);
  for (tmp = (int*)((char*)ptr-PADDING_SIZE); tmp<(int*)(ptr-4); tmp++) 
    if (*tmp != PADDING_PATTERN) {
      m_error = E_PADDING_OVERWRITTEN;
      return -1;
    }
  for (tmp = (int*)(ptr+allocated_size); tmp < (int*)(ptr+allocated_size+PADDING_SIZE); tmp++)
    if (*tmp != PADDING_PATTERN) {
      m_error = E_PADDING_OVERWRITTEN;
      return -1;
    }
  //  debug mode ends

  //  set DEADBEEF
  for (tmp = (int*)((char*)ptr-PADDING_SIZE); tmp < (int*)(ptr+allocated_size+PADDING_SIZE); tmp++)
    *tmp = (int)FREE_PATTERN;

  //  insert new element
  *(long*)((char*)ptr-PADDING_SIZE) = *(long*)prev_free_chunk;
  *(int*)((char*)ptr-PADDING_SIZE+8) = allocated_size+2*PADDING_SIZE;
  *(long*)prev_free_chunk = (long)((char*)ptr-PADDING_SIZE);
  
  //  check if the newly freed part can merge with the next free chunk
  if ( *(long*)prev_free_chunk == (long)(ptr+allocated_size+PADDING_SIZE) ) {
    *(long*)((char*)ptr-PADDING_SIZE) = *(long*)(*(long*)prev_free_chunk);
    *(int*)((char*)ptr-PADDING_SIZE+8) += *(int*)( (void*)(*(long*)prev_free_chunk)+8 );

    tmp = (int*)(*(long*)prev_free_chunk);
    *tmp = (int)FREE_PATTERN; 
    tmp++; *tmp = (int)FREE_PATTERN; 
    tmp++; *tmp = (int)FREE_PATTERN;  //  overwrite 12-byte of the head of the next free chunk
  }
  //  check if the newly freed part can merge with the prev free chunk
  if ( (long)(prev_free_chunk+*(int*)((char*)prev_free_chunk+8)) == (long)((char*)ptr-PADDING_SIZE) ) {
    *(long*)prev_free_chunk = *(long*)((char*)ptr-PADDING_SIZE);
    *(int*)((char*)prev_free_chunk+8) += 2*PADDING_SIZE + allocated_size;

    tmp = (int*)((char*)ptr-PADDING_SIZE);
    *tmp = (int)FREE_PATTERN; 
    tmp++; *tmp = (int)FREE_PATTERN; 
    tmp++; *tmp = (int)FREE_PATTERN;  //  overwrite 12-byte of the head of the next free chunk
  }
}
