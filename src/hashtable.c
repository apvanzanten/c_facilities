#include "hashtable.h"

#include <stdlib.h>

#include "log.h"

#define OK STAT_OK

#define INIT_CAPACITY 8

STAT_Val HT_create_in_place(HT_HashTable * this, uint32_t key_size, uint32_t value_size) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(key_size == 0) return LOG_STAT(STAT_ERR_ARGS, "key size can't be 0");

  *this = (HT_HashTable){
      .count            = 0,
      .capacity         = INIT_CAPACITY,
      .key_size         = key_size,
      .value_size       = value_size,
      .has_indirect_key = false,
      .key_store        = {0},
      .value_store      = {0},
  };

  if(!STAT_is_OK(DAR_create_in_place(&this->key_store, key_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create hash table key store");
  }
  if(!STAT_is_OK(DAR_create_in_place(&this->value_store, value_size))) {
    DAR_destroy_in_place(&this->key_store);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create hash table value store");
  }
  if(!STAT_is_OK(DAR_resize(&this->key_store, this->capacity)) ||
     !STAT_is_OK(DAR_resize(&this->value_store, this->capacity))) {
    DAR_destroy_in_place(&this->value_store);
    DAR_destroy_in_place(&this->key_store);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize stores to meet minimum capacity");
  }

  return OK;
}

STAT_Val HT_create_on_heap(HT_HashTable ** this_p, uint32_t key_size, uint32_t value_size) {
  if(this_p == NULL) return LOG_STAT(STAT_ERR_ARGS, "this_p is NULL");
  if(key_size == 0) return LOG_STAT(STAT_ERR_ARGS, "key size can't be 0");

  HT_HashTable * this = (HT_HashTable *)malloc(sizeof(HT_HashTable));
  if(this == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for HT_HashTable");

  if(!STAT_is_OK(HT_create_in_place(this, key_size, value_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create hash table");
  }

  *this_p = this;

  return OK;
}

STAT_Val HT_destroy_in_place(HT_HashTable * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  if(!STAT_is_OK(DAR_destroy_in_place(&(this->key_store)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy hash table key store");
  }
  if(!STAT_is_OK(DAR_destroy_in_place(&(this->value_store)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy hash table value store");
  }

  *this = (HT_HashTable){0};

  return OK;
}
STAT_Val HT_destroy_on_heap(HT_HashTable ** this_p) {
  if(this_p == NULL) return LOG_STAT(STAT_ERR_ARGS, "this_p is NULL");

  if(!STAT_is_OK(HT_destroy_in_place(*this_p))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy hash table");
  }

  free(*this_p);
  *this_p = NULL;

  return OK;
}