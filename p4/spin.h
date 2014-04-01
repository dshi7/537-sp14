#ifndef _SPIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define _SPIN_H_

typedef struct spinlock {
  volatile unsigned int flag; //  0 for free and 1 for held
} spinlock ;

void  spinlock_init (spinlock *lock);

void  spinlock_acquire (spinlock *lock);

void  spinlock_release (spinlock *lock);


#endif
