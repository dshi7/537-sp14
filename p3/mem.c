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
#define HEADER_FREE_SIZE 8

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

  *(int*)ptr_header = -1;
  *(int*)((char*)ptr_header+4) = total_size-HEADER_FREE_SIZE;

  //  debug mode setup starts
  if (debug==1) {
    debug_enable = 1;
    int *tmp;
    for ( tmp = (int*)((char*)ptr_header + HEADER_FREE_SIZE); tmp < (int*)((char*)ptr_header + sizeOfRegion); tmp ++ )
      *tmp = FREE_PATTERN;
  }
  //  debug mode setup ends

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

  void *ret_ptr;  //  hold the return value if debug is enabled
  int found = 0;

  //  traverse the free list
  void *walking_ptr = ptr_header;

  while (walking_ptr!=NULL) {

    //  sufficient space for allocation
    if ( *(int*)((char*)walking_ptr+4) > size + PADDING_SIZE*2 + HEADER_ALLOCATED_SIZE ) {
      //  mark found
      found = 1;
      //  update the available size of the current free chunk
      *(int*)((char*)walking_ptr+4) -= ( size + PADDING_SIZE*2 + HEADER_ALLOCATED_SIZE );
      //  write the size of allocated chunk
      *(int*)((char*)walking_ptr + HEADER_FREE_SIZE + *(int*)((char*)walking_ptr+4)) = size;
      //  let ret_ptr be the returned value
      ret_ptr = (void*)((char*)walking_ptr + HEADER_FREE_SIZE + *(int*)((char*)walking_ptr+4) + HEADER_ALLOCATED_SIZE + PADDING_SIZE);
      //  check debug mode
      if ( debug_enable==0 )
        return ret_ptr;
      else
        break;
    }
    else
      walking_ptr = (*(int*)walking_ptr!=-1) ? (void*)((char*)walking_ptr + *(int*)walking_ptr) : NULL;
  };

  if (found==0) {
    m_error = E_NO_SPACE;
    return NULL;
  }

  if (debug_enable==1) {
    //  set the top padding
    int *tmp;
    for ( tmp = (int*)((char*)ret_ptr - PADDING_SIZE); tmp < (int*)ret_ptr; tmp++ )
      *tmp = PADDING_PATTERN;
    //  set the bottom padding
    for ( tmp = (int*)((char*)ret_ptr + size); tmp < (int*)((char*)ret_ptr + size + PADDING_SIZE); tmp++ )
      *tmp = PADDING_PATTERN;

    //  check the filling values 
    walking_ptr = ptr_header;

    while ( walking_ptr != NULL ) {
      for ( tmp = (int*)((char*)walking_ptr+HEADER_FREE_SIZE); tmp < (int*)((char*)walking_ptr+HEADER_FREE_SIZE + *(int*)((char*)walking_ptr+4)); tmp++ )
        if (*tmp != FREE_PATTERN) {
          m_error = E_CORRUPT_FREESPACE;
          return NULL;
        }
      walking_ptr = (*(int*)walking_ptr!=-1) ? (void*)((char*)walking_ptr + *(int*)walking_ptr) : NULL;
    };

    return ret_ptr;
  }

  return NULL;
}

int  Mem_Free(void *ptr) {

  //  find the last free chunk prior to allocated space
  void *prev_free_chunk = ptr_header;
  void *next_free_chunk;
  int found = 0;
  int *tmp;

  if (ptr==NULL)
    return 0;
  
  //  ptr must be within the initialized space
  if ( ptr_header==NULL || (long)ptr<(long)ptr_header || (long)ptr>=(long)((char*)ptr_header+total_size) ) {
    m_error = E_BAD_POINTER;
    return -1;
  }

  //  set the newly inserted free chunk
  void *ptr_new = (void*)((char*)ptr - PADDING_SIZE - HEADER_ALLOCATED_SIZE);
  int allocated_size = *(int*)ptr_new;
  int additional_space = allocated_size + 2*PADDING_SIZE + HEADER_ALLOCATED_SIZE - HEADER_FREE_SIZE;

//  printf("\n\n# Mem_Free : \n");
//  printf("ptr to insert : %08x\n", ptr_new);
//  printf("ptr to header: %08x\n", ptr_header);
  
  if (allocated_size < 0) {
    m_error = E_BAD_POINTER;
    return -1;
  }

  //  debug mode starts
  //  check whether either side of padding is overwritten
  if (debug_enable == 1) {
    for ( tmp = (int*)((char*)ptr-PADDING_SIZE); tmp<(int*)ptr; tmp++ ) 
      if (*tmp != PADDING_PATTERN) {
        m_error = E_PADDING_OVERWRITTEN;
        return -1;
      }
    for ( tmp = (int*)((char*)ptr + allocated_size); tmp < (int*)((char*)ptr + allocated_size + PADDING_SIZE); tmp++)
      if (*tmp != PADDING_PATTERN) {
        m_error = E_PADDING_OVERWRITTEN;
        return -1;
      }
  }
  //  debug mode ends

  while (1) {
    next_free_chunk = *(int*)prev_free_chunk==-1 ? NULL : (void*)((char*)prev_free_chunk+*(int*)prev_free_chunk);
//    printf("\tin while loop : %08x\t%08x\t%08x\n", prev_free_chunk, ptr_new, next_free_chunk);
    if ( next_free_chunk == NULL || next_free_chunk > ptr_new ) {
//      printf("\tbreak the while loop ! \n");
//      printf("\tnow : %08x\t%08x\t%08x\n", prev_free_chunk, ptr_new, next_free_chunk);
      break;
    }
    prev_free_chunk = next_free_chunk;
  }
//  printf("%08x\t%08x\t%08x\n", prev_free_chunk, ptr_new, next_free_chunk);

  //  more rigorous boundary checks
  if ( (char*)ptr - PADDING_SIZE - HEADER_ALLOCATED_SIZE < (char*)prev_free_chunk + HEADER_FREE_SIZE + *(int*)((char*)prev_free_chunk+4) ) {
    m_error = E_BAD_POINTER;
    return -1;
  }

  if ( (long)((char*)ptr + allocated_size + PADDING_SIZE) > (next_free_chunk!=NULL ? (long)next_free_chunk : ((long)ptr_header + total_size)) ) {
//    printf("\n\nERROR : %08x %d %08x %d %08x %08x\n", ptr, allocated_size, ptr+allocated_size+PADDING_SIZE, (next_free_chunk!=NULL), (long)next_free_chunk, (long)ptr_header+total_size);
    m_error = E_PADDING_OVERWRITTEN;
    return -1;
  }

  //  insert new element into free list
  *(int*)ptr_new = (next_free_chunk==NULL) ? -1 : ( (char*)next_free_chunk - (char*)ptr_new );
//  printf("here : %d\n", *(int*)ptr_new);
  *(int*)((char*)ptr_new+4) = additional_space;
  for ( tmp = (int*)((char*)ptr_new + HEADER_FREE_SIZE); tmp < (int*)((char*)ptr_new + HEADER_FREE_SIZE + additional_space); tmp ++ )
    *tmp = (int)FREE_PATTERN;
  *(int*)prev_free_chunk = (char*)ptr_new - (char*)prev_free_chunk;


  //  check if the newly freed part can merge with the next free chunk
  if ( *(int*)ptr_new!=-1 && (char*)ptr_new + HEADER_FREE_SIZE + additional_space == (char*)next_free_chunk ) {
    if ( *(int*)next_free_chunk != -1 )
      *(int*)ptr_new += *(int*)next_free_chunk;
    else
      *(int*)ptr_new = -1;
    *(int*)((char*)ptr_new+4) += *(int*)((char*)next_free_chunk+4) + HEADER_FREE_SIZE;
    for ( tmp = (int*)next_free_chunk; tmp < (int*)((char*)next_free_chunk + HEADER_FREE_SIZE); tmp++ )
      *tmp = (int)FREE_PATTERN;
  }

  //  check if the newly freed part can merge with the prev free chunk
  if ( (char*)prev_free_chunk + HEADER_FREE_SIZE + *(int*)((char*)prev_free_chunk+4) == (char*)ptr_new ) {
    if ( *(int*)ptr_new != -1 )
      *(int*)prev_free_chunk += *(int*)ptr_new;
    else
      *(int*)prev_free_chunk = -1;
    *(int*)((char*)prev_free_chunk+4) += *(int*)((char*)ptr_new+4) + HEADER_FREE_SIZE;
    for ( tmp = (int*)ptr_new; tmp < (int*)((char*)ptr_new + HEADER_FREE_SIZE);  tmp++ )
      *tmp = (int)FREE_PATTERN;
  }
  return 0;
}

void Mem_Dump() {
//  printf ("\n\n%08x\t%d\n", ptr_header, total_size);

  printf("\n###\nMem_Dump() :\n\n");
  void *walking_ptr = ptr_header;

  while (walking_ptr!=NULL) {
    printf("Free List : \n\t%08x\t%d\n\t%08x\t%d\n", walking_ptr, *(int*)walking_ptr, (char*)walking_ptr+4, *(int*)((char*)walking_ptr+4));
    if (*(int*)walking_ptr != -1)
      walking_ptr = (void*)((char*)walking_ptr + *(int*)walking_ptr);
    else
      walking_ptr = NULL;
  }
}
