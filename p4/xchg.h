#ifndef _XCHG_H_

#include <stdlib.h>

#define _XCHG_H_

static inline unsigned int 
xchg(volatile unsigned int *addr, unsigned int newval) {
  unsigned int result;
  asm volatile ("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
  return  result;
}

#endif
