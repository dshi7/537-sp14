#ifndef  _LIST_H_

#include "spin.h"

#define _LIST_H_

typedef struct list_t {

  unsigned int k;
  void  *v;
  struct list_t *next;

} list_t;

void  List_Init(list_t *list);

void  List_Insert(list_t *list, void *element, unsigned int key);

void  List_Delete(list_t *list, unsigned int key);

void  *List_Lookup(list_t *list, unsigned int key);

#endif
