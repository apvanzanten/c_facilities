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

#ifndef CFAC_LIST_H
#define CFAC_LIST_H

#include <stddef.h>
#include <stdint.h>

#include "stat.h"

// ===========
// == types ==

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

// ==============================
// == creation and destruction ==

STAT_Val LST_create(LST_List * this, size_t element_size);
STAT_Val LST_destroy(LST_List * this);

// ==================
// == modification ==

STAT_Val LST_insert(LST_List * this,
                    LST_Node *   successor,
                    const void * data,
                    LST_Node **  o_inserted_node);
STAT_Val LST_insert_from_array(LST_List * this,
                               LST_Node *   successor,
                               const void * arr,
                               size_t       n,
                               LST_Node **  o_first_inserted_node);

STAT_Val LST_remove(LST_Node * to_be_removed);
STAT_Val LST_remove_sequence(LST_Node * first, LST_Node * successor);

STAT_Val LST_inject(LST_Node * to_be_injected, LST_Node * successor);
STAT_Val LST_extract(LST_Node * to_be_extracted);

STAT_Val LST_inject_sequence(LST_Node * first, LST_Node * last, LST_Node * successor);
STAT_Val LST_extract_sequence(LST_Node * first, LST_Node * successor);

STAT_Val LST_clear(LST_List * this);

// =============
// == queries ==

size_t             LST_get_len(const LST_List * this);
bool               LST_contains(const LST_List * this, const void * value);
static inline bool LST_is_empty(const LST_List * this);

// STAT_Val LST_find([const] LST_List * this, const void * value, [const] LST_Node ** o_found_node);
#define LST_find(list, value, o_found_node)                                                        \
  _Generic((list),                                                                                 \
      const LST_List *: LST_INT_find_const,                                                        \
      LST_List *: LST_INT_find_nonconst)(list, value, o_found_node)
STAT_Val LST_INT_find_nonconst(LST_List * this, const void * value, LST_Node ** o_found_node);
STAT_Val LST_INT_find_const(const LST_List * this,
                            const void *      value,
                            const LST_Node ** o_found_node);

// ===============
// == accessors ==

//  [const] LST_Node * LST_first([const] LST_List * this);
#define LST_first(list)                                                                            \
  _Generic((list), const LST_List *: LST_INT_first_const, LST_List *: LST_INT_first_nonconst)(list)
static inline LST_Node *       LST_INT_first_nonconst(LST_List * this);
static inline const LST_Node * LST_INT_first_const(const LST_List * this);

//  [const] LST_Node * LST_last([const] LST_List * this);
#define LST_last(list)                                                                             \
  _Generic((list), const LST_List *: LST_INT_last_const, LST_List *: LST_INT_last_nonconst)(list)
static inline LST_Node *       LST_INT_last_nonconst(LST_List * this);
static inline const LST_Node * LST_INT_last_const(const LST_List * this);

//  [const] LST_Node * LST_end([const] LST_List * this);
#define LST_end(list)                                                                              \
  _Generic((list), const LST_List *: LST_INT_end_const, LST_List *: LST_INT_end_nonconst)(list)
static inline LST_Node *       LST_INT_end_nonconst(LST_List * this);
static inline const LST_Node * LST_INT_end_const(const LST_List * this);

//  [const] void * LST_data([const] LST_Node * node);
#define LST_data(node)                                                                             \
  _Generic((node), const LST_Node *: LST_INT_data_const, LST_Node *: LST_INT_data_nonconst)(node)
static inline void *       LST_INT_data_nonconst(LST_Node * node);
static inline const void * LST_INT_data_const(const LST_Node * node);
// NOTE We have this LST_data accessors because our data member is not void (because arrays can't
// be members, and flexible array members must be arrays so can't be void), but casting directly
// from uint8_t to whatever is the relevant data type will likely result in warnings/errors from
// compilers and linters. It's not a perfect solution but it is slightly more convenient.

//  [const] LST_Node * LST_next([const] LST_Node * node, int n);
#define LST_next(node, n)                                                                          \
  _Generic((node), const LST_Node *: LST_INT_next_const, LST_Node *: LST_INT_next_nonconst)(node, n)
static inline LST_Node *       LST_INT_next_nonconst(LST_Node * node_pp, int n);
static inline const LST_Node * LST_INT_next_const(const LST_Node * node_pp, int n);

//  [const] LST_Node * LST_prev([const] LST_Node * node, int n);
#define LST_prev(node, n)                                                                          \
  _Generic((node), const LST_Node *: LST_INT_prev_const, LST_Node *: LST_INT_prev_nonconst)(node, n)
static inline LST_Node *       LST_INT_prev_nonconst(LST_Node * node_pp, int n);
static inline const LST_Node * LST_INT_prev_const(const LST_Node * node_pp, int n);

// =====================================
// == inline function implementations ==

static inline LST_Node * LST_INT_first_nonconst(LST_List * this) { return this->sentinel->next; }
static inline LST_Node * LST_INT_last_nonconst(LST_List * this) { return this->sentinel->prev; }
static inline LST_Node * LST_INT_end_nonconst(LST_List * this) { return this->sentinel; }
static inline const LST_Node * LST_INT_first_const(const LST_List * this) {
  return this->sentinel->next;
}
static inline const LST_Node * LST_INT_last_const(const LST_List * this) {
  return this->sentinel->prev;
}
static inline const LST_Node * LST_INT_end_const(const LST_List * this) { return this->sentinel; }
static inline void *           LST_INT_data_nonconst(LST_Node * node) { return (void *)node->data; }
static inline const void *     LST_INT_data_const(const LST_Node * node) {
  return (const void *)node->data;
}

static inline LST_Node * LST_INT_next_nonconst(LST_Node * node, int n) {
  for(int i = 0; i < n; i++) node = node->next;
  for(int i = 0; i > n; i--) node = node->prev;
  return node;
}
static inline const LST_Node * LST_INT_next_const(const LST_Node * node, int n) {
  for(int i = 0; i < n; i++) node = node->next;
  for(int i = 0; i > n; i--) node = node->prev;
  return node;
}

static inline LST_Node * LST_INT_prev_nonconst(LST_Node * node, int n) {
  return LST_INT_next_nonconst(node, -n);
}
static inline const LST_Node * LST_INT_prev_const(const LST_Node * node, int n) {
  return LST_INT_next_const(node, -n);
}

static inline bool LST_is_empty(const LST_List * this) {
  return (this == NULL || this->sentinel == this->sentinel->next);
}

// ===============
// == test-only ==

bool LST_INT_is_valid(const LST_List * this);

#endif
