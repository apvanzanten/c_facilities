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

#include "hashtable.h"

#include <stdlib.h>
#include <string.h>

#include "darray.h"
#include "log.h"
#include "span.h"

#define OK STAT_OK

#define MIN_CAPACITY    8
#define MAX_LOAD_FACTOR 0.75

static bool     is_empty(const HT_Entry * entry) { return !DAR_is_initialized(&(entry->key)); }
static bool     has_value(const HT_Entry * entry) { return DAR_is_initialized(&(entry->value)); }
static bool     is_tombstone(const HT_Entry * entry) { return entry->is_tombstone; }
static STAT_Val create_entry(HT_Entry * entry, uint32_t hash, SPN_Span key, SPN_Span value);
static STAT_Val destroy_entry(HT_Entry * entry);
static size_t   get_index_from_hash(const HT_HashTable * this, uint32_t hash);
static uint32_t get_hash_for_key(SPN_Span key);
static STAT_Val grow_capacity_as_needed(HT_HashTable * this, size_t new_count);

static const HT_Entry * successor(const HT_HashTable * this, const HT_Entry * entry);

static STAT_Val find_entry_or_spot_for_entry_const(const HT_HashTable * this,
                                                   SPN_Span          key,
                                                   uint32_t          hash,
                                                   const HT_Entry ** o_entry);
static STAT_Val find_entry_or_spot_for_entry(HT_HashTable * this,
                                             SPN_Span    key,
                                             uint32_t    hash,
                                             HT_Entry ** o_entry);

static STAT_Val find_entry_or_spot_for_entry_impl(const HT_HashTable * this,
                                                  SPN_Span key,
                                                  uint32_t hash,
                                                  size_t * o_idx);

STAT_Val HT_create(HT_HashTable * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  *this = (HT_HashTable){0};

  if(!STAT_is_OK(DAR_create(&this->store, sizeof(HT_Entry)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create hash table store");
  }
  if(!STAT_is_OK(DAR_resize_zeroed(&this->store, MIN_CAPACITY))) {
    DAR_destroy(&this->store);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize stores to meet minimum capacity");
  }

  return OK;
}

STAT_Val HT_destroy(HT_HashTable * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  for(HT_Entry * p = DAR_first(&this->store); p != DAR_end(&this->store); p++) {
    LOG_STAT_IF_ERR(destroy_entry(p), "failed to destroy entry. Continuing...");
  }

  if(!STAT_is_OK(DAR_destroy(&(this->store)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy hash table store");
  }

  *this = (HT_HashTable){0};

  return OK;
}

STAT_Val HT_set(HT_HashTable * this, SPN_Span key, SPN_Span value) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(SPN_is_empty(key)) return LOG_STAT(STAT_ERR_ARGS, "empty key");

  const uint32_t hash = get_hash_for_key(key);

  HT_Entry *     entry   = NULL;
  const STAT_Val find_st = find_entry_or_spot_for_entry(this, key, hash, &entry);
  if(!STAT_is_OK(find_st) || (find_st == STAT_OK_NOT_FOUND)) {
    return LOG_STAT(STAT_ERR_INTERNAL, "unable to find entry or spot for new entry");
  }

  if(is_empty(entry)) {
    // create a new entry in empty spot, and then grow capacity if needed
    if(!STAT_is_OK(create_entry(entry, hash, key, value))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to create hash table entry");
    }

    const size_t new_count = this->count + 1;
    if(!STAT_is_OK(grow_capacity_as_needed(this, new_count))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to grow table capacity after adding new entry");
    }
    this->count = new_count;
  } else {
    // this is the existing entry for this key, copy the new value over the old one
    if(has_value(entry)) {
      if(!STAT_is_OK(DAR_destroy(&(entry->value)))) {
        return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy existing entry for key");
      }
    }
    if(!SPN_is_empty(value)) {
      if(!STAT_is_OK(DAR_create_from_span(&(entry->value), value))) {
        return LOG_STAT(STAT_ERR_INTERNAL, "failed to write value to entry");
      }
    }
  }

  return OK;
}

STAT_Val HT_get(const HT_HashTable * this, SPN_Span key, SPN_Span * o_value) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(SPN_is_empty(key)) return LOG_STAT(STAT_ERR_ARGS, "empty key");

  const uint32_t hash = get_hash_for_key(key);

  const HT_Entry * entry   = NULL;
  const STAT_Val   find_st = find_entry_or_spot_for_entry_const(this, key, hash, &entry);
  if(!STAT_is_OK(find_st)) return LOG_STAT(STAT_ERR_INTERNAL, "error while trying to find entry");

  if((find_st == STAT_OK_NOT_FOUND) || is_empty(entry)) return STAT_OK_NOT_FOUND;

  if(o_value != NULL) *o_value = DAR_to_span(&(entry->value));

  return OK;
}

STAT_Val HT_remove(HT_HashTable * this, SPN_Span key) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(SPN_is_empty(key)) return LOG_STAT(STAT_ERR_ARGS, "empty key");

  const uint32_t hash = get_hash_for_key(key);

  HT_Entry *     entry   = NULL;
  const STAT_Val find_st = find_entry_or_spot_for_entry(this, key, hash, &entry);
  if(!STAT_is_OK(find_st)) return LOG_STAT(STAT_ERR_INTERNAL, "error while trying to find entry");

  if((find_st == STAT_OK_NOT_FOUND) || is_empty(entry)) return STAT_OK_NOT_FOUND;

  if(!STAT_is_OK(destroy_entry(entry))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy entry for removal");
  }

  const HT_Entry * next_entry = successor(this, entry);
  if(!is_empty(next_entry) || is_tombstone(next_entry)) {
    entry->is_tombstone = true;
    this->tombstone_count++;
  }

  this->count--;

  return OK;
}

static uint32_t get_hash_for_key(SPN_Span key) {
  // NOTE shamelessly stolen from Nystrom's Crafting Interpreters (you should read it. It's good!)

  const uint8_t * key_as_bytes = (const uint8_t *)key.begin;

  uint32_t hash = 2166136261u;
  for(size_t i = 0; i < SPN_get_size_in_bytes(key); i++) {
    hash ^= key_as_bytes[i];
    hash *= 16777619;
  }

  return hash;
}

static size_t get_index_from_hash(const HT_HashTable * this, uint32_t hash) {
  // NOTE There may be a more optimal way to do this, but I am not going to bother with it until I
  // have some benchmarks setup to see if it actually matters. We expect capacity to be a power of
  // two, so this is not as bad as it looks.
  return (hash % HT_get_capacity(this));
}

static STAT_Val grow_capacity_as_needed(HT_HashTable * this, size_t new_count) {
  const size_t net_count         = new_count + this->tombstone_count;
  const size_t required_capacity = ((1.0 / MAX_LOAD_FACTOR) * (double)net_count) + 1;

  const size_t old_capacity = HT_get_capacity(this);
  size_t       new_capacity = old_capacity;
  while(new_capacity < required_capacity) {
    new_capacity *= 2;
  }

  if(new_capacity != old_capacity) {
    DAR_DArray old_store = this->store; // NOTE deliberate shallow copy; equivalent to C++ 'move'
    this->store          = (DAR_DArray){0};

    if(!STAT_is_OK(DAR_create(&this->store, sizeof(HT_Entry)))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to create replacement hash table store");
    }
    if(!STAT_is_OK(DAR_resize_zeroed(&this->store, new_capacity))) {
      DAR_destroy(&this->store);
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize hash table store");
    }

    this->count           = 0;
    this->tombstone_count = 0;

    for(HT_Entry * entry = DAR_first(&old_store); entry != DAR_end(&old_store); entry++) {
      if(!is_empty(entry) && !is_tombstone(entry)) {
        HT_Entry * new_entry = NULL;
        if(!STAT_is_OK(find_entry_or_spot_for_entry(this,
                                                    DAR_to_span(&(entry->key)),
                                                    entry->hash,
                                                    &new_entry))) {
          return LOG_STAT(STAT_ERR_INTERNAL, "failed to move entry to new store location");
        }
        *new_entry = *entry;
        *entry     = (HT_Entry){0};
        this->count++;
      }
    }

    if(!STAT_is_OK(DAR_destroy(&old_store))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy old store");
    }
  }

  return OK;
}

static STAT_Val find_entry_or_spot_for_entry(HT_HashTable * this,
                                             SPN_Span    key,
                                             uint32_t    hash,
                                             HT_Entry ** o_entry) {
  size_t         idx = 0;
  const STAT_Val st  = find_entry_or_spot_for_entry_impl(this, key, hash, &idx);
  if(STAT_is_OK(st)) *o_entry = DAR_get(&(this->store), idx);
  return st;
}

static STAT_Val find_entry_or_spot_for_entry_const(const HT_HashTable * this,
                                                   SPN_Span          key,
                                                   uint32_t          hash,
                                                   const HT_Entry ** o_entry) {
  size_t         idx = 0;
  const STAT_Val st  = find_entry_or_spot_for_entry_impl(this, key, hash, &idx);
  if(STAT_is_OK(st)) *o_entry = DAR_get(&(this->store), idx);
  return st;
}

static STAT_Val find_entry_or_spot_for_entry_impl(const HT_HashTable * this,
                                                  SPN_Span key,
                                                  uint32_t hash,
                                                  size_t * o_idx) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(SPN_is_empty(key)) return LOG_STAT(STAT_ERR_ARGS, "empty key");
  if(o_idx == NULL) return LOG_STAT(STAT_ERR_ARGS, "o_idx is NULL");

  const size_t capacity  = HT_get_capacity(this);
  const size_t start_idx = get_index_from_hash(this, hash);

  *o_idx = start_idx;
  do {
    const HT_Entry * entry = DAR_get(&this->store, *o_idx);

    // did we find a spot for a potential new entry?
    if(is_empty(entry) && !is_tombstone(entry)) return OK;

    // or did we find the matching entry?
    if((entry->hash == hash) && SPN_equals(DAR_to_span(&(entry->key)), key)) return OK;

    (*o_idx)++;
    if(*o_idx == capacity) *o_idx = 0;
  } while(*o_idx != start_idx);

  return STAT_OK_NOT_FOUND;
}

static STAT_Val create_entry(HT_Entry * entry, uint32_t hash, SPN_Span key, SPN_Span value) {
  *entry = (HT_Entry){0};

  entry->hash         = hash;
  entry->is_tombstone = false;

  if(!STAT_is_OK(DAR_create_from_span(&(entry->key), key))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to write key to new entry");
  }

  if(!SPN_is_empty(value)) {
    if(!STAT_is_OK(DAR_create_from_span(&(entry->value), value))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to write value to new entry");
    }
  }

  return OK;
}

static STAT_Val destroy_entry(HT_Entry * entry) {
  if(entry == NULL) return OK;

  const STAT_Val key_st   = DAR_destroy(&(entry->key));
  const STAT_Val value_st = (has_value(entry) ? DAR_destroy(&(entry->value)) : OK);

  LOG_STAT_IF_ERR(key_st, "failed to destroy entry key data array. Continuing...");
  LOG_STAT_IF_ERR(value_st, "failed to destroy entry value data array. Continuing...");

  *entry = (HT_Entry){0};

  return ((!STAT_is_OK(key_st) || !STAT_is_OK(value_st))
              ? LOG_STAT(STAT_ERR_INTERNAL, "error destroying entry")
              : OK);
}

static const HT_Entry * successor(const HT_HashTable * this, const HT_Entry * entry) {
  if(entry == DAR_last(&(this->store))) return DAR_first(&(this->store));
  return (++entry);
}
