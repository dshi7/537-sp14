
#include "spin.h"

static inline uint
xchg(volatile unsigned int *addr, unsigned int newval) {
  uint result;
  asm volatile ("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
  return  result;
}

void  spinlock_init (spinlock *lock) {
  lock->flag = 0;
}

void  spinlock_acquire (spinlock *lock) {
  while ( xchg(&(lock->flag), 1)==1 )
    ;
}

void  spinlock_release (spinlock *lock) {
  lock->flag = 0;
}
