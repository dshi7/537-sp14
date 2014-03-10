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
#define HEADER_ALLOCATED_SIZE 4
#define HEADER_FREE_SIZE 12

#define PADDING_SIZE 64

/* global variables of debug signal and error signal */
int debug_enable;
int m_error;

void *ptr_header = NULL;
int total_size;

int Mem_Init(int sizeOfRegion, int debug) {

  //  called only once by a process using your routine
  if(ptr_header != NULL) {
    m_error = E_BAD_ARGS;
    return -1;
  }

  int fd = open("/dev/zero", O_RDWR);   //  file description

  ptr_header = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  total_size = sizeOfRegion;

  if (ptr_header==MAP_FAILED) {
    m_error = E_BAD_ARGS;
    return -1;
  }
  close(fd);

  //  debug mode setup starts
  if (debug==1) {
    debug_enable = 1;
    int *tmp = ptr_header;     //  use an integer pointer for its 4-byte alignment
    while (tmp<(int*)(ptr_header+sizeOfRegion)) {
      *tmp = FREE_PATTERN;
      tmp++;
    }
  }
  //  debug mode setup ends

  //  the 1st 8-byte is for address 
  *(long*)ptr_header = (long)NULL;

  //  the 2st 4-byte is for size of current free chunk
  *(int*)((char*)ptr_header+8) = total_size-HEADER_FREE_SIZE;
  
  return 0;   //  success

}

void *Mem_Alloc(int size) { 

  if (ptr_header==NULL) {
    m_error = E_NO_SPACE;
    return NULL;
  }

  if (size<=0) {
    return NULL;
  }

  /* 4-byte assigned */
  size = ((size-1)/4+1) *4;

  int *ptr_available_size;
  void *ret_ptr;  //  hold the return value if debug is enabled
  int found = 0;

  //  traverse the free list
  long walking_ptr = (long)ptr_header;

  while (walking_ptr != (long)NULL) {

    //  find the pointer to the available size of the current free chunk
    ptr_available_size = (int*)(walking_ptr+8);
    if (*ptr_available_size > size+PADDING_SIZE*2+HEADER_ALLOCATED_SIZE) {
      found = 1;
      *ptr_available_size -= (size+PADDING_SIZE*2+HEADER_ALLOCATED_SIZE);

      //  each allocatd space should have 64-byte paddings on both sides
      *(int*)(walking_ptr + HEADER_FREE_SIZE + *ptr_available_size ) = size;
      ret_ptr = (void*)(walking_ptr + HEADER_FREE_SIZE + *ptr_available_size + HEADER_ALLOCATED_SIZE + PADDING_SIZE);
      if ( debug_enable==0 )
        return ret_ptr;
      else 
        break;
    }
    else {
      walking_ptr = *(long*)walking_ptr;
    }
  };

  if (found==0) {
    m_error = E_NO_SPACE;
    return NULL;
  }

  if (debug_enable==1) {
    //  set the top padding
    int *tmp;
    for ( tmp = (int*)((char*)ret_ptr - PADDING_SIZE); tmp < (int*)ret_ptr; tmp++ )
      *tmp = (int)PADDING_PATTERN;
    //  set the bottom padding
    for ( tmp = (int*)((char*)ret_ptr + size); tmp < (int*)((char*)ret_ptr + size + PADDING_SIZE); tmp++ )
      *tmp = (int)PADDING_PATTERN;

    //  check the filling values 
    walking_ptr = (long)ptr_header;
    while (walking_ptr != (long)NULL) {
      for (tmp = (int*)(walking_ptr+HEADER_FREE_SIZE); tmp<(int*)(walking_ptr+HEADER_FREE_SIZE+*(int*)(walking_ptr+8)); tmp++) {
        if (*tmp != FREE_PATTERN) {
          m_error = E_CORRUPT_FREESPACE;
          return NULL;
        }
      }
      walking_ptr = *(long*)walking_ptr;
    };

    return ret_ptr;
  }

  return NULL;

}

int  Mem_Free(void *ptr) {

  //  find the last free chunk prior to allocated space
  long prev_free_chunk = (long)ptr_header;
  long next_free_chunk = *(long*)prev_free_chunk;
  int found = 0;

  if (ptr==NULL)
    return 0;

  //  ptr must be within the initialized space
  if ( prev_free_chunk==(long)NULL || (long)ptr<(long)ptr_header || (long)ptr>=(long)ptr_header+total_size ) {
    m_error = E_BAD_POINTER;
    return -1;
  }

  while (prev_free_chunk!=(long)NULL) {
    next_free_chunk = *(long*)prev_free_chunk;
    if (next_free_chunk==(long)NULL || next_free_chunk>(long)ptr) 
      break;
    prev_free_chunk = next_free_chunk;
  }

  //  debug mode starts
  //  check whether either side of padding is overwritten
  int *tmp;
  int allocated_size = *(int*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE);
  for (tmp = (int*)((char*)ptr-PADDING_SIZE); tmp<(int*)((char*)ptr); tmp++) 
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
  for (tmp = (int*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE); tmp < (int*)(ptr+allocated_size+PADDING_SIZE); tmp++)
    *tmp = (int)FREE_PATTERN;

  //  insert new element
  *(long*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE) = next_free_chunk;
  *(int*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE+8) = allocated_size+2*PADDING_SIZE+HEADER_ALLOCATED_SIZE-HEADER_FREE_SIZE;
  *(long*)prev_free_chunk = (long)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE);
  
  //  check if the newly freed part can merge with the next free chunk
  if ( next_free_chunk == (long)(ptr+allocated_size+PADDING_SIZE*2+HEADER_ALLOCATED_SIZE) ) {
    *(long*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE) = *(long*)next_free_chunk;
    *(int*)((char*)ptr-PADDING_SIZE+8)+=(*(int*)((char*)next_free_chunk+8)+HEADER_FREE_SIZE);

    tmp = (int*)next_free_chunk;
    *tmp = (int)FREE_PATTERN; 
    tmp++; *tmp = (int)FREE_PATTERN; 
    tmp++; *tmp = (int)FREE_PATTERN;  //  overwrite 12-byte of the head of the next free chunk
  }
  //  check if the newly freed part can merge with the prev free chunk
  if ( (long)(prev_free_chunk+*(int*)((char*)prev_free_chunk+8)) == (long)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE) ) {
    *(long*)prev_free_chunk = *(long*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE);
    *(int*)((char*)prev_free_chunk+8) += (*(int*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE+8)+HEADER_FREE_SIZE);

    tmp = (int*)((char*)ptr-PADDING_SIZE-HEADER_FREE_SIZE);
    *tmp = (int)FREE_PATTERN; 
    tmp++; *tmp = (int)FREE_PATTERN; 
    tmp++; *tmp = (int)FREE_PATTERN;  //  overwrite 12-byte of the head of the next free chunk
  }

  return 0;
}
