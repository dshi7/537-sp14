#include <stdio.h>
#include <stdlib.h>
#include "list.h"

void List_Init(list_t *list)
{
    list->head = NULL;
    spinlock_init(&list->lock);
}

void List_Insert(list_t *list, void *element, unsigned int key)
{
    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL) {
        fprintf(stderr, "malloc error\n");
        return;
    }
    new_node->key = key;
    new_node->element = element;
    
    // lock
    spinlock_acquire(&list->lock);
    // simply insert at head
    new_node->next = list->head;
    list->head = new_node;
    spinlock_release(&list->lock);
}

void List_Delete(list_t *list, unsigned int key)
{
    // delete the first one found
    node_t *prev_node = NULL;
    // traverse the list
    spinlock_acquire(&list->lock);
    node_t *curr_node = list->head;
    while (curr_node) {
        if (curr_node->key == key) {
            if (list->head == curr_node) {
                list->head = curr_node->next;
            } else {
                prev_node->next = curr_node->next;
            }
            break;
        }
        prev_node = curr_node;
        curr_node = curr_node->next;
    }
    spinlock_release(&list->lock);

    // actually delete the node
    if (curr_node) { 
        curr_node->next = NULL;
        free (curr_node);
    }
}

void *List_Lookup(list_t *list, unsigned int key)
{
    // return the first match
    void *element = NULL;
    // traverse the list
    spinlock_acquire(&list->lock);
    node_t *curr_node = list->head;
    while (curr_node) {
        if (curr_node->key == key) {
            element = curr_node->element;
            break;
        }
        curr_node = curr_node->next;
    }
    spinlock_release(&list->lock);
    return element;
}
