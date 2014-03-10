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
    int *tmp = ptr_header+HEADER_FREE_SIZE;    
    while (tmp<(int*)(ptr_header+sizeOfRegion)) {
      *tmp = FREE_PATTERN;
      tmp++;
    }
  }
  //  debug mode setup ends

  *(long*)ptr_header = (long)NULL;
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

    if ( *(int*)(walking_ptr+8) > size + PADDING_SIZE*2 + HEADER_ALLOCATED_SIZE ) {

      found = 1;

      //  update the available size of the current free chunk
      *(int*)(walking_ptr+8) -= ( size + PADDING_SIZE*2 + HEADER_ALLOCATED_SIZE );

      //  write the size of allocated chunk
      *(int*)(walking_ptr + HEADER_FREE_SIZE + *(int*)(walking_ptr+8)) = size;

      //  let ret_ptr be the returned value
      ret_ptr = (void*)(walking_ptr + HEADER_FREE_SIZE + *(int*)(walking_ptr+8) + HEADER_ALLOCATED_SIZE + PADDING_SIZE);

      //  check debug mode
      if ( debug_enable==0 )
        return ret_ptr;
      else
        break;
    }
    else
      walking_ptr = *(long*)walking_ptr;

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
    while ( walking_ptr != (long)NULL ) {
      for ( tmp = (int*)(walking_ptr+HEADER_FREE_SIZE); tmp <(int*)(walking_ptr + HEADER_FREE_SIZE + *(int*)(walking_ptr+8)); tmp++ )
        if (*tmp != FREE_PATTERN) {
          m_error = E_CORRUPT_FREESPACE;
          return NULL;
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
  int *tmp;

  if (ptr==NULL)
    return 0;
  
  //  insert new element
  int allocated_size = *(int*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE);
  if (allocated_size < 0) {
    m_error = E_BAD_POINTER;
    return -1;
  }
  //  debug mode starts
  //  check whether either side of padding is overwritten
  if (debug_enable == 1) {
    for (tmp = (int*)((char*)ptr-PADDING_SIZE); tmp<(int*)ptr; tmp++) 
      if (*tmp != PADDING_PATTERN) {
        m_error = E_PADDING_OVERWRITTEN;
        return -1;
      }
    for (tmp = (int*)((char*)ptr + allocated_size); tmp < (int*)((char*)ptr + allocated_size + PADDING_SIZE); tmp++)
      if (*tmp != PADDING_PATTERN) {
        m_error = E_PADDING_OVERWRITTEN;
        return -1;
      }
    //  debug mode ends
  }

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


  *(long*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE) = (long)next_free_chunk;
  *(int*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE+8) = allocated_size+2*PADDING_SIZE+HEADER_ALLOCATED_SIZE-HEADER_FREE_SIZE;
  *(long*)prev_free_chunk = (long)((char*)ptr - PADDING_SIZE - HEADER_ALLOCATED_SIZE);
  for (tmp = (int*)((char*)ptr - PADDING_SIZE - HEADER_ALLOCATED_SIZE + HEADER_FREE_SIZE); tmp < (int*)((char*)ptr - PADDING_SIZE - HEADER_ALLOCATED_SIZE + HEADER_FREE_SIZE + *(int*)((char*)ptr-PADDING_SIZE - HEADER_ALLOCATED_SIZE + 8)); tmp ++ )
    *tmp = (int)FREE_PATTERN;
  
  //  check if the newly freed part can merge with the next free chunk
  if ( next_free_chunk == (long)((char*)ptr+allocated_size+PADDING_SIZE) ) {
    *(long*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE) = *(long*)next_free_chunk;
    *(int*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE+8) += (*(int*)(next_free_chunk+8)+HEADER_FREE_SIZE);
    for (tmp = (int*)((char*)ptr - PADDING_SIZE - HEADER_ALLOCATED_SIZE + HEADER_FREE_SIZE); tmp < (int*)((char*)ptr - PADDING_SIZE - HEADER_ALLOCATED_SIZE + HEADER_FREE_SIZE + *(int*)((char*)ptr-PADDING_SIZE - HEADER_ALLOCATED_SIZE + 8)); tmp ++ )
      *tmp = (int)FREE_PATTERN;
  }

  //  check if the newly freed part can merge with the prev free chunk
  if ( (long)(prev_free_chunk +HEADER_FREE_SIZE + *(int*)((char*)prev_free_chunk+8)) == (long)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE) ) {
    *(long*)prev_free_chunk = *(long*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE);
    *(int*)(prev_free_chunk+8) += (*(int*)((char*)ptr-PADDING_SIZE-HEADER_ALLOCATED_SIZE+8)+HEADER_FREE_SIZE);
    for ( tmp = (int*)(prev_free_chunk+HEADER_FREE_SIZE); tmp < (int*)(prev_free_chunk+HEADER_FREE_SIZE+*(int*)(prev_free_chunk+8)); tmp++ )
      *tmp = (int)FREE_PATTERN;
  }


  return 0;
}
