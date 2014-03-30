#ifndef _SPIN_H_

#include <stdio.h>
#include <stdlib.h>
#include "xchg.h"

#define _SPIN_H_

typedef struct  {
  int flag; //  0 for free and 1 for held
} _spinlock_t ;

void  spinlock_init (_spinlock_t *lock);

void  spinlock_acquire (_spinlock_t *lock);

void  spinlock_release (_spinlock_t *lock);


#endif
