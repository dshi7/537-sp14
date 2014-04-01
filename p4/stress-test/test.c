/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  main func
 *
 *        Version:  1.0
 *        Created:  03/29/2014 01:54:53 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Daohang
 *
 * =====================================================================================
 */

#include <pthread.h>
#include "spin.h"
#include "counter.h"
#include "list.h"
#include "hash.h"

int main(int argc, char** argv) {

  int element = 255;
  unsigned int key = 127;
  list_t lst;
  printf("helloworld\n");
  List_Init(&lst);
  printf("helloworld\n");
  List_Insert(&lst, (void*)&element, key);
  printf("helloworld\n");
  void *val = List_Lookup(&lst, key);
  if (val)
    return 0;
  else
    return -1;
}
