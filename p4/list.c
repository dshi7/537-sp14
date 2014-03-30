#include "list.h"

void  List_Init (list_t* list) {
  list = (list_t*)malloc(sizeof(list_t));
  list->next = NULL;
}

void  List_Insert(list_t *list, void *element, unsigned int key) {

  list_t *inserted = malloc(sizeof(list_t));
  inserted->k = key;
  inserted->v = element;
  inserted->next = list->next;
  list->next = inserted;

}

void  List_Delete(list_t *list, unsigned int key) {

  list_t *ptr = list;
  while ( ptr->next != NULL ) {
    if ( ptr->next->k == key )
      break;
    ptr = ptr->next;
  }
  if ( ptr->next==NULL )
    return;
  else
    ptr->next = ptr->next->next;

}

void  *List_Lookup(list_t *list, unsigned int key) {

  list_t *ptr = list;
  while ( ptr->next != NULL ) {
    if ( ptr->next->k == key )
      break;
    ptr = ptr->next;
  }
  if ( ptr->next!=NULL )
    return ptr->next->v;

}
