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

  int status = Mem_Init(4096, 0);

  void *p = Mem_Alloc(100);
//  printf("%08x\n", (long)p);
  
  return 0;

}
