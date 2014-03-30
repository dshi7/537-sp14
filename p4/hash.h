#ifndef _HASH_H_

#define _HASH_H_

#include "list.h"

typedef struct hash {

  int bucket_num;
  list_t **hash_table;

} hash_t;

void  Hash_Init (hash_t *hash, int buckets);

void  Hash_Insert (hash_t *hash, void *element, unsigned int key);

void  Hash_Delete (hash_t *hash, unsigned int key);

void  *Hash_Lookup (hash_t *hash, unsigned int key);

#endif
