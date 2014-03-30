/*
 * =====================================================================================
 *
 *       Filename:  spin.c
 *
 *    Description:  implement the functions declared in header file
 *
 *        Created:  03/28/2014 08:16:39 PM
 *       Compiler:  gcc
 *
 *         Author:  Daohang
 *
 * =====================================================================================
 */

#include "spin.h"

void  spinlock_init (_spinlock_t *lock) {
  lock->flag = 0;
}

void  spinlock_acquire (_spinlock_t *lock) {
  while ( xchg(&lock->flag, 1)==1 )
    ;
}

void  spinlock_release (_spinlock_t *lock) {
  lock->flag = 0;
}
