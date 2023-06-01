#ifndef CFAC_HASHTABLE_H
#define CFAC_HASHTABLE_H

#include <stdbool.h>
#include <stdint.h>

#include "darray.h"
#include "span.h"
#include "stat.h"

typedef struct {
  DAR_DArray store;
  size_t     count;
  size_t     tombstone_count;
  size_t     value_size;
  uint16_t   key_size; // if this is 0, that means the key size is variable (e.g. a string)
} HT_HashTable;

STAT_Val HT_create(HT_HashTable * this, uint16_t key_size, size_t value_size);
STAT_Val HT_destroy(HT_HashTable * this);

STAT_Val HT_set(HT_HashTable * this, const void * key, const void * value);
STAT_Val HT_get(const HT_HashTable * this, const void * key, const void ** o_value);
STAT_Val HT_remove(HT_HashTable * this, const void * key);

static inline uint32_t HT_get_capacity(const HT_HashTable * this) {
  if(this == NULL) return 0;
  return this->store.size;
}

#endif
