
#include "hash.h"

void  Hash_Init (hash_t *hash, int buckets) {
  hash->size = buckets;
  hash->hash_table = calloc(buckets, sizeof(list_t));
}

void  Hash_Insert (hash_t *hash, void *element, unsigned int key) {
  int hash_key = key % hash->size;
  List_Insert ( hash->hash_table + hash_key, element, key );
}

void  Hash_Delete (hash_t *hash, unsigned int key) {
  int hash_key = key % hash->size;
  List_Delete ( hash->hash_table + hash_key, key );
}

void  *Hash_Lookup (hash_t *hash, unsigned int key) {
  int hash_key = key % hash->size;
  return List_Lookup ( hash->hash_table + hash_key, key );
}
