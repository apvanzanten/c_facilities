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
} HT_HashTable;

typedef struct {
  DAR_DArray key;
  DAR_DArray value;
  uint32_t   hash;
  bool       is_tombstone;
} HT_Entry;

STAT_Val HT_create(HT_HashTable * this);
STAT_Val HT_destroy(HT_HashTable * this);

STAT_Val HT_set(HT_HashTable * this, SPN_Span key, SPN_Span value);
STAT_Val HT_get(const HT_HashTable * this, SPN_Span key, SPN_Span * o_value);
STAT_Val HT_remove(HT_HashTable * this, SPN_Span key);

static inline bool HT_contains(const HT_HashTable * this, SPN_Span key) {
  if(this == NULL || SPN_is_empty(key)) return false;
  return (HT_get(this, key, NULL) == STAT_OK);
}

static inline size_t HT_get_capacity(const HT_HashTable * this) {
  if(this == NULL) return 0;
  return this->store.size;
}

#endif
