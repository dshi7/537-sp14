#ifndef __list_h__
#define __list_h__

#include "spin.h"

typedef struct __node_t {
    unsigned int key;
    void *element;
    struct __node_t *next;
} node_t;

typedef struct {
    node_t *head;
    spinlock lock;
} list_t;

void List_Init(list_t *list);
void List_Insert(list_t *list, void *element, unsigned int key);
void List_Delete(list_t *list, unsigned int key);
void *List_Lookup(list_t *list, unsigned int key);

#endif
