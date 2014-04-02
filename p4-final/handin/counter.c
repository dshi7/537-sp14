/*
 * =====================================================================================
 *
 *       Filename:  counter.c
 *
 *    Description:  implement functions declared in header file
 *
 *        Created:  03/29/2014 01:49:57 PM
 *       Compiler:  gcc
 *
 *         Author:  Daohang
 *
 * =====================================================================================
 */

#include "counter.h"

void  Counter_Init (counter_t *c, int value) {
  c->val = value;
#ifdef  PTH
  int rc = pthread_mutex_init(&c->plock, NULL);
  if (rc!=0)
    fprintf(stderr, "Error : init mutex.\n");
#else
  spinlock_init(&(c->spinlock));
#endif
}

int Counter_GetValue (counter_t *c) {
  return  c->val;
}

void  Counter_Increment (counter_t *c) {
#ifdef  PTH
  int rc = pthread_mutex_lock(&c->plock);
  if (rc!=0)
    fprintf(stderr, "Error : mutex lock.\n");
#else
  spinlock_acquire(&(c->spinlock));
#endif

  ++ (c->val);

#ifdef  PTH
  rc = pthread_mutex_unlock(&c->plock);
  if (rc!=0)
    fprintf(stderr, "Error : mutex unlock.\n");
#else
  spinlock_release(&(c->spinlock));
#endif
}

void  Counter_Decrement (counter_t *c) {
  spinlock_acquire(&(c->spinlock));
  -- (c->val);
  spinlock_release(&(c->spinlock));
}

