/*
 * =====================================================================================
 *
 *       Filename:  list.c
 *
 *    Description:  implement funcs declared in "list.h"
 *
 *        Created:  03/29/2014 03:14:31 PM
 *       Compiler:  gcc
 *
 *         Author:  Gary D.Shi
 *
 * =====================================================================================
 */

#include "list.h"

void  List_Init(list_t *list) {
  list = malloc(list_t);
  list->next = NULL;
}

void  List_Insert(list_t *list, void *element, unsigned int key) {
  unsigned  int pos = 0;
  list_t  *ptr = list;

  _spinlock_t lock;
  spinlock_acquire(&lock);

  while (pos++<key) {
    assert ( ptr->next!=NULL );   //   stupid project ; don't need to check it
    ptr = ptr->next;
  }
  list_t  *inserted_item = malloc(list_t);
  inserted_item->val = element;
  inserted_item->next=  ptr->next;
  ptr->next = inserted_item;

  spinlock_release(&lock);
}

void  List_Delete(list_t *list, unsigned int key); {
  unsigned  int pos = 0;
  list_t  *ptr = list;

  _spinlock_t lock;

  spinlock_acquire(&lock);

  while (pos++<key) {
    assert ( ptr->next!=NULL );   //   stupid project ; don't need to check it
    ptr = ptr->next;
  }
  ptr->next= ptr->next->next;

  spinlock_release(&lock);
}

void  *List_Lookup(list_t *list, unsigned int key) {
  unsigned  int pos = 0;
  list_t  *ptr = list;

  _spinlock_t lock;
  spinlock_acquire(&lock);
  
  while (pos++<key) {
    assert ( ptr->next!=NULL );   //   stupid project ; don't need to check it
    ptr = ptr->next;
  }
  return  ptr->next->val;

  spinlock_release(&lock);
}

