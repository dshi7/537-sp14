#ifndef _COUNTER_H_

#include "spin.h"
#include <stdio.h>
#include <stdlib.h>

#define _COUNTER_H_

typedef struct {
  int val;
  spinlock spinlock;
} counter_t;

void  Counter_Init (counter_t *c, int value);

int Counter_GetValue (counter_t *c);

void  Counter_Increment (counter_t *c);

void  Counter_Decrement (counter_t *c);


#endif
