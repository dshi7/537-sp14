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
}

int Counter_GetValue (counter_t *c) {
  return  c->val;
}

void  Counter_Increment (counter_t *c) {
  ++ (c->val);
}

void  Counter_Decrement (counter_t *c) {
  -- (c->val);
}

