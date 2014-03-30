#ifdef  _LIST_H_
#define _LIST_H_

#define "spin.h"

typedef struct list {
  void  *val;
  struct list *next;
} list_t;

void  List_Init(list_t *list);

void  List_Insert(list_t *list, void *element, unsigned int key);

void  List_Delete(list_t *list, unsigned int key);

void  *List_Lookup(list_t *list, unsigned int key);

#endif
