/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  main ()
 *
 *        Created:  02/28/2014 07:52:10 PM
 *       Compiler:  gcc
 *
 *         Author:  Daohang Shi
 *
 * =====================================================================================
 */

#include <stdio.h>
#include "mem.h"

int main() {

  printf("helloworld\n");

  printf("%d\n", getpagesize());

  int status = Mem_Init(4096, 0);

  Mem_Alloc(8192);
  
  return 0;

}
