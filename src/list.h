#ifndef CFAC_LIST_H
#define CFAC_LIST_H

#include <stddef.h>
#include <stdint.h>

#include "stat.h"

typedef struct LST_Node {
  // NOTE we align this struct and the data flexible array member to sizeof(max_align_t), so that we
  // can store larger things in the data flexible array member (that may need such alignment).
  // There is a memory usage to this: We end up always allocating a multiple of sizeof(max_align_t).
  struct LST_Node * prev;
  struct LST_Node * next;
  uint8_t           data[] __attribute__((aligned(sizeof(max_align_t))));
} __attribute__((aligned(sizeof(max_align_t)))) LST_Node;

typedef struct LST_List {
  LST_Node * sentinel; // this is pointer because we want to be able to move our LST_List around
                       // without invalidating pointers to the sentinel (in the nodes)
  size_t element_size;
} LST_List;

STAT_Val LST_create_on_heap(LST_List ** this_p, size_t element_size);
STAT_Val LST_create_in_place(LST_List * this, size_t element_size);

STAT_Val LST_destroy_on_heap(LST_List ** this_p);
STAT_Val LST_destroy_in_place(LST_List * this);

STAT_Val LST_insert(LST_List * this,
                    LST_Node *   successor,
                    const void * data,
                    LST_Node **  o_inserted_node);

STAT_Val LST_clear(LST_List * this);

static inline LST_Node *       LST_first(LST_List * this) { return this->sentinel->next; }
static inline LST_Node *       LST_last(LST_List * this) { return this->sentinel->prev; }
static inline LST_Node *       LST_end(LST_List * this) { return this->sentinel; }
static inline const LST_Node * LST_first_const(const LST_List * this) {
  return this->sentinel->next;
}
static inline const LST_Node * LST_last_const(const LST_List * this) {
  return this->sentinel->prev;
}
static inline const LST_Node * LST_end_const(const LST_List * this) { return this->sentinel; }

#endif
