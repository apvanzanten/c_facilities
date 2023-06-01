#include "hashtable.h"

#include <stdlib.h>
#include <string.h>

#include "darray.h"
#include "log.h"

#define OK STAT_OK

#define MIN_CAPACITY    8
#define MAX_LOAD_FACTOR 0.75

typedef struct {
  void *   key;
  void *   value;
  uint32_t hash;
  bool     is_tombstone;
  uint16_t key_size;
} Entry;

static bool has_variable_keysize(const HT_HashTable * this) { return (this->key_size == 0); }

static bool     is_empty(Entry * entry) { return entry->key == NULL; }
static bool     is_tombstone(Entry * entry) { return entry->is_tombstone; }
static void     clear(HT_HashTable * this);
static STAT_Val create_entry(Entry *      entry,
                             uint32_t     hash,
                             const void * key,
                             uint16_t     key_size,
                             const void * value,
                             size_t       value_size);
static void     destroy_entry(Entry * entry);
static uint32_t get_index_from_hash(const HT_HashTable * this, uint32_t hash);
static uint32_t get_hash_for_key(const void * key, size_t n_bytes);
static STAT_Val grow_capacity_as_needed(HT_HashTable * this, uint32_t new_count);
static STAT_Val find_entry_or_spot_for_entry(HT_HashTable * this,
                                             const void * key,
                                             uint16_t     key_size,
                                             uint32_t     hash,
                                             Entry **     entry);

STAT_Val HT_create(HT_HashTable * this, uint16_t key_size, size_t value_size) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  *this = (HT_HashTable){
      .store           = {0},
      .count           = 0,
      .tombstone_count = 0,
      .value_size      = value_size,
      .key_size        = key_size,
  };

  if(!STAT_is_OK(DAR_create_in_place(&this->store, sizeof(Entry)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create hash table store");
  }
  if(!STAT_is_OK(DAR_resize_zeroed(&this->store, MIN_CAPACITY))) {
    DAR_destroy_in_place(&this->store);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize stores to meet minimum capacity");
  }

  return OK;
}

STAT_Val HT_destroy(HT_HashTable * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  clear(this);

  if(!STAT_is_OK(DAR_destroy_in_place(&(this->store)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy hash table store");
  }

  *this = (HT_HashTable){0};

  return OK;
}

STAT_Val HT_set(HT_HashTable * this, const void * key, const void * value) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(key == NULL) return LOG_STAT(STAT_ERR_ARGS, "key is NULL");
  if(this->value_size > 0 && value == NULL) return LOG_STAT(STAT_ERR_ARGS, "value is NULL");
  if(has_variable_keysize(this)) return LOG_STAT(STAT_ERR_ARGS, "this table has variable key size");

  const uint32_t hash  = get_hash_for_key(key, this->key_size);
  Entry *        entry = NULL;

  const STAT_Val find_st = find_entry_or_spot_for_entry(this, key, this->key_size, hash, &entry);
  if(!STAT_is_OK(find_st) || (find_st == STAT_OK_NOT_FOUND)) {
    return LOG_STAT(STAT_ERR_INTERNAL, "unable to find entry or spot for new entry");
  }

  if(is_empty(entry)) {
    // create a new entry in empty spot, and then grow capacity if needed
    if(!STAT_is_OK(create_entry(entry, hash, key, this->key_size, value, this->value_size))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to create hash table entry");
    }

    const uint32_t new_count = this->count + 1;
    if(!STAT_is_OK(grow_capacity_as_needed(this, new_count))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to grow table capacity after adding new entry");
    }
    this->count = new_count;
  } else {
    // this is the existing entry for this key, copy the new value over the old one
    memcpy(entry->value, value, this->value_size);
  }

  return OK;
}

STAT_Val HT_get(const HT_HashTable * this, const void * key, const void ** o_value) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(key == NULL) return LOG_STAT(STAT_ERR_ARGS, "key is NULL");
  if(has_variable_keysize(this)) return LOG_STAT(STAT_ERR_ARGS, "this table has variable key size");

  const uint32_t hash = get_hash_for_key(key, this->key_size);

  Entry * entry = NULL;

  // we do a const cast here to avoid having to write this function twice for const/non-cost usage
  // TODO find better way
  HT_HashTable * nc_this = (HT_HashTable *)this;

  const STAT_Val find_st = find_entry_or_spot_for_entry(nc_this, key, this->key_size, hash, &entry);
  if(!STAT_is_OK(find_st)) return LOG_STAT(STAT_ERR_INTERNAL, "error while trying to find entry");

  if((find_st == STAT_OK_NOT_FOUND) || is_empty(entry)) return STAT_OK_NOT_FOUND;

  if(o_value != NULL) *o_value = entry->value;

  return OK;
}

STAT_Val HT_remove(HT_HashTable * this, const void * key) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(key == NULL) return LOG_STAT(STAT_ERR_ARGS, "key is NULL");
  if(has_variable_keysize(this)) return LOG_STAT(STAT_ERR_ARGS, "this table has variable key size");

  const uint32_t hash = get_hash_for_key(key, this->key_size);

  Entry * entry = NULL;

  const STAT_Val find_st = find_entry_or_spot_for_entry(this, key, this->key_size, hash, &entry);
  if(!STAT_is_OK(find_st)) return LOG_STAT(STAT_ERR_INTERNAL, "error while trying to find entry");

  if((find_st == STAT_OK_NOT_FOUND) || is_empty(entry)) return STAT_OK_NOT_FOUND;

  destroy_entry(entry);
  entry->is_tombstone = true; // TODO apply only when next entry is empty

  this->tombstone_count++;
  this->count--;

  return OK;
}

static uint32_t get_hash_for_key(const void * key, size_t n_bytes) {
  // NOTE shamelessly stolen from Nystrom's Crafting Interpreters (you should read it. It's good!)

  uint32_t        hash         = 2166136261u;
  const uint8_t * key_as_bytes = (const uint8_t *)key;

  for(size_t i = 0; i < n_bytes; i++) {
    hash ^= key_as_bytes[i];
    hash *= 16777619;
  }

  return hash;
}

static uint32_t get_index_from_hash(const HT_HashTable * this, uint32_t hash) {
  // NOTE There may be a more optimal way to do this, but I am not going to bother with it until I
  // have some benchmarks setup to see if it actually matters. We expect capacity to be a power of
  // two, so this is not as bad as it looks.
  return (hash % HT_get_capacity(this));
}

static STAT_Val grow_capacity_as_needed(HT_HashTable * this, uint32_t new_count) {
  const size_t net_count         = new_count + this->tombstone_count;
  const size_t required_capacity = ((1.0 / MAX_LOAD_FACTOR) * (double)net_count) + 1;

  const uint32_t old_capacity = HT_get_capacity(this);
  uint32_t       new_capacity = old_capacity;
  while(new_capacity < required_capacity) {
    new_capacity *= 2;
  }

  if(new_capacity != old_capacity) {
    DAR_DArray old_store = this->store; // NOTE deliberate shallow copy; equivalent to C++ 'move'
    this->store          = (DAR_DArray){0};

    if(!STAT_is_OK(DAR_create_in_place(&this->store, sizeof(Entry)))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to create replacement hash table store");
    }
    if(!STAT_is_OK(DAR_resize_zeroed(&this->store, new_capacity))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize hash table store");
    }

    this->count           = 0;
    this->tombstone_count = 0;

    for(Entry * entry = DAR_first(&old_store); entry != DAR_end(&old_store); entry++) {
      if(!is_empty(entry) && !is_tombstone(entry)) {
        const uint16_t key_size = (has_variable_keysize(this) ? entry->key_size : this->key_size);

        Entry * new_entry = NULL;
        if(!STAT_is_OK(
               find_entry_or_spot_for_entry(this, entry->key, key_size, entry->hash, &new_entry))) {
          return LOG_STAT(STAT_ERR_INTERNAL, "failed to move entry to new store location");
        }
        *new_entry = *entry;
        *entry     = (Entry){0};
        this->count++;
      }
    }

    if(!STAT_is_OK(DAR_destroy_in_place(&old_store))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy old store");
    }
  }

  return OK;
}

static STAT_Val find_entry_or_spot_for_entry(HT_HashTable * this,
                                             const void * key,
                                             uint16_t     key_size,
                                             uint32_t     hash,
                                             Entry **     entry) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(key == NULL) return LOG_STAT(STAT_ERR_ARGS, "key is NULL");
  if(entry == NULL) return LOG_STAT(STAT_ERR_ARGS, "entry is NULL");

  const uint32_t capacity  = HT_get_capacity(this);
  const uint32_t start_idx = get_index_from_hash(this, hash);

  uint32_t idx = start_idx;
  do {
    *entry = DAR_get(&this->store, idx);

    // did we find a spot for a potential new entry?
    if(is_empty(*entry) && !is_tombstone(*entry)) return OK;

    // or did we find the matching entry?
    if(((*entry)->hash == hash) && (memcmp(key, (*entry)->key, key_size) == 0)) return OK;

    idx++;
    if(idx == capacity) idx = 0;
  } while(idx != start_idx);

  return STAT_OK_NOT_FOUND;
}

static STAT_Val create_entry(Entry *      entry,
                             uint32_t     hash,
                             const void * key,
                             uint16_t     key_size,
                             const void * value,
                             size_t       value_size) {
  *entry = (Entry){0};

  entry->hash = hash;

  entry->key = malloc(key_size);
  if(entry->key == NULL) {
    return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate memory for key with size %u", key_size);
  }

  memcpy(entry->key, key, key_size);

  if(value_size != 0) {
    entry->value = malloc(value_size);
    if(entry->value == NULL) {
      free(entry->key);
      return LOG_STAT(STAT_ERR_ALLOC,
                      "failed to allocate memory for value with size %u",
                      value_size);
    }

    memcpy(entry->value, value, value_size);
  }

  entry->key_size = key_size;

  return OK;
}

static void destroy_entry(Entry * entry) {
  if(entry != NULL) {
    free(entry->key);
    free(entry->value);
    *entry = (Entry){0};
  }
}

static void clear(HT_HashTable * this) {
  if(this != NULL) {
    for(Entry * p = DAR_first(&this->store); p != DAR_end(&this->store); p++) {
      destroy_entry(p);
    }
    this->count = 0;
  }
}