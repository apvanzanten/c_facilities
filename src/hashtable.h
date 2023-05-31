#ifndef CFAC_HASHTABLE_H
#define CFAC_HASHTABLE_H

#include <stdbool.h>
#include <stdint.h>

#include "darray.h"
#include "stat.h"

typedef struct {
  uint32_t   count;
  uint32_t   capacity;
  uint32_t   key_size;
  uint32_t   value_size;
  bool       has_indirect_key; // indicates whether key has memory indirection (e.g. in a string)
  DAR_DArray key_store;
  DAR_DArray value_store;
} HT_HashTable;

STAT_Val HT_create_in_place(HT_HashTable * this, uint32_t key_size, uint32_t value_size);
STAT_Val HT_create_on_heap(HT_HashTable ** this_p, uint32_t key_size, uint32_t value_size);

STAT_Val HT_destroy_in_place(HT_HashTable * this);
STAT_Val HT_destroy_on_heap(HT_HashTable ** this_p);

#endif
