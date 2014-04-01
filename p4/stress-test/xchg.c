
#include "xchg.h"

unsigned int  xchg(volatile unsigned int *addr, unsigned int newval) {
  unsigned int result;
//  printf("xchg.newval=%d\n", newval);
  asm volatile ("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
//  printf("xchg.result=%d\n", result);
  return  result;
}

