/*
 * =====================================================================================
 *
 *       Filename:  hash.c
 *
 *    Description:  implement the funcs declared in "hash.h"
 *
 *        Version:  1.0
 *       Compiler:  gcc
 *
 *         Author:  D.H.
 *
 * =====================================================================================
 */

#include "hash.h"

void  Hash_Init (hash_t *hash, int buckets) {
  hash->bucket_num = buckets;
  hash->hash_table = calloc(buckets, sizeof(list*));
  for ( int i=0; i<buckets; i++ )
    List_Init( hash_table+i );
}

void  Hash_Insert (hash_t *hash, void *element, unsigned int key) {

  int hash_key = key % hash->bucket_num;
  List_Insert(hash->hash_table + hash_key, element, 0);

}

void  Hash_Delete (hash_t *hash, unsigned int key) {

  int hash_key = key % hash->bucket_num;
  list_t *list = hash->hash_table[hash_key];
  list->next = NULL;

}

void  *Hash_Lookup (hash_t *hash, unsigned int key) {

  int hash_key = key % hash->bucket_num;
  list_t *list = hash->hash_table[hash_key];
  return  list->next->val;

}
