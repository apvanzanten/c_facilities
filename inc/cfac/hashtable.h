// MIT License
//
// Copyright (c) 2023 Arjen P. van Zanten
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
