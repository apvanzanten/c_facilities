#include "list.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"

#define OK STAT_OK

// ==================================
// == helper function declarations ==

static void     connect(LST_Node * first, LST_Node * second);
static size_t   get_node_size(size_t element_size);
static STAT_Val create_node(LST_Node ** node_pp, size_t element_size);
static STAT_Val create_sentinel(LST_Node ** sentinel_pp, size_t element_size);
static void     destroy_chain_of_nodes(LST_Node * first_node);
static STAT_Val create_chain_of_nodes(const void * data_arr,
                                      size_t       n,
                                      size_t       element_size,
                                      LST_Node **  o_first_node,
                                      LST_Node **  o_last_nod);

// ==========================
// == creation/destruction ==

STAT_Val LST_create_on_heap(LST_List ** this_p, size_t element_size) {
  if(this_p == NULL) return LOG_STAT(STAT_ERR_ARGS, "this_p is NULL");
  if(*this_p != NULL) return LOG_STAT(STAT_ERR_ARGS, "this_p points to non-NULL");
  if(element_size == 0) return LOG_STAT(STAT_ERR_ARGS, "element size is 0");

  LST_List * this = (LST_List *)malloc(sizeof(LST_List));
  if(this == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for LST_List");

  if(!STAT_is_OK(LST_create_in_place(this, element_size))) {
    free(this);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create LST_List");
  }

  *this_p = this;

  return OK;
}

STAT_Val LST_create_in_place(LST_List * this, size_t element_size) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(element_size == 0) return LOG_STAT(STAT_ERR_ARGS, "element size is 0");

  if(!STAT_is_OK(create_sentinel(&this->sentinel, element_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create sentinel");
  }

  this->element_size = element_size;

  return OK;
}

STAT_Val LST_destroy_on_heap(LST_List ** this_p) {
  if(this_p == NULL) return LOG_STAT(STAT_ERR_ARGS, "this_p is NULL");
  if(*this_p == NULL) return OK;

  if(!STAT_is_OK(LST_destroy_in_place(*this_p))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy LST_List");
  }

  free(*this_p);

  return OK;
}

STAT_Val LST_destroy_in_place(LST_List * this) {
  if(this == NULL) return OK;

  if(this->sentinel != NULL) {
    if(!STAT_is_OK(LST_clear(this))) return LOG_STAT(STAT_ERR_INTERNAL, "failed to clear list");
    free(this->sentinel);
  }

  *this = (LST_List){0};

  return OK;
}

// ==================
// == modification ==

STAT_Val LST_insert(LST_List * this,
                    LST_Node *   successor,
                    const void * data,
                    LST_Node **  o_inserted_node) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(successor == NULL) return LOG_STAT(STAT_ERR_ARGS, "successor is NULL");
  if(data == NULL) return LOG_STAT(STAT_ERR_ARGS, "data is NULL");

  LST_Node * predecessor = successor->prev;

  LST_Node * new_node = NULL;
  if(!STAT_is_OK(create_node(&new_node, this->element_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create new node for insertion");
  }

  memcpy(new_node->data, data, this->element_size);

  connect(predecessor, new_node);
  connect(new_node, successor);

  if(o_inserted_node != NULL) *o_inserted_node = new_node;

  return OK;
}

STAT_Val LST_insert_from_array(LST_List * this,
                               LST_Node *   successor,
                               const void * arr,
                               size_t       n,
                               LST_Node **  o_first_inserted_node) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(successor == NULL) return LOG_STAT(STAT_ERR_ARGS, "successor is NULL");
  if(arr == NULL) return LOG_STAT(STAT_ERR_ARGS, "arr is NULL");

  if(n == 0) return OK;

  // we first create a chain of connected nodes, then when finished inject it whole into the list

  LST_Node * first_new_node = NULL;
  LST_Node * last_new_node  = NULL;
  if(!STAT_is_OK(
         create_chain_of_nodes(arr, n, this->element_size, &first_new_node, &last_new_node))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create chain of new nodes");
  }

  connect(successor->prev, first_new_node);
  connect(last_new_node, successor);

  if(o_first_inserted_node != NULL) *o_first_inserted_node = first_new_node;

  return OK;
}

STAT_Val LST_clear(LST_List * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  LST_Node * curr = this->sentinel->next;
  while(curr != this->sentinel) {
    LST_Node * tmp = curr;
    curr           = curr->next;
    free(tmp);
  }

  this->sentinel->next = this->sentinel;
  this->sentinel->prev = this->sentinel;

  return OK;
}

STAT_Val LST_remove(LST_Node * to_be_removed) {
  if(to_be_removed == NULL) return LOG_STAT(STAT_ERR_ARGS, "to-be-removed node pointer is NULL");

  to_be_removed->prev->next = to_be_removed->next;
  to_be_removed->next->prev = to_be_removed->prev;

  free(to_be_removed);

  return OK;
}

// =============
// == queries ==

size_t LST_get_len(const LST_List * this) {
  if(this == NULL || this->sentinel == NULL) return 0;
  size_t len = 0;

  const LST_Node * curr = LST_first(this);
  while(curr != LST_end(this)) {
    len++;
    curr = curr->next;
  }

  return len;
}

// =====================================
// == helper function implementations ==

static void connect(LST_Node * first, LST_Node * second) {
  first->next  = second;
  second->prev = first;
}

static size_t get_node_size(size_t element_size) {
  // we add to sizeof(LST_Node) and round up to nearest multiple of align_max_t.
  // Having this be a multiple of sizeof(align_max_t) is required to be able to allocate memory
  // aligned to max_align_t, such that we can store any type directly in the data flexible array
  // member without breaking things.
  const size_t base_size = sizeof(LST_Node) + element_size;

  return ((base_size / sizeof(max_align_t)) + 1) * sizeof(max_align_t);
}

static STAT_Val create_node(LST_Node ** node_pp, size_t element_size) {
  LST_Node * node = (LST_Node *)aligned_alloc(sizeof(max_align_t), get_node_size(element_size));
  if(node == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for LST_Node");

  *node_pp = node;

  return OK;
}

static STAT_Val create_sentinel(LST_Node ** sentinel_pp, size_t element_size) {
  // NOTE sentinel does not contain any data so we do not need to allocate space for the data array
  LST_Node * sentinel = (LST_Node *)aligned_alloc(sizeof(max_align_t), get_node_size(element_size));
  if(sentinel == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for sentinel");

  sentinel->next = sentinel;
  sentinel->prev = sentinel;

  *sentinel_pp = sentinel;

  return OK;
}

static void destroy_chain_of_nodes(LST_Node * first_node) {
  LST_Node * curr = first_node;

  while(curr != NULL) {
    LST_Node * to_be_removed = curr;
    curr                     = curr->next;
    free(to_be_removed);
  }
}

static STAT_Val create_chain_of_nodes(const void * data_arr,
                                      size_t       n,
                                      size_t       element_size,
                                      LST_Node **  o_first_node,
                                      LST_Node **  o_last_node) {
  LST_Node * first_node = NULL;
  LST_Node * prev_node  = NULL;

  for(size_t i = 0; i < n; i++) {
    LST_Node *   new_node = NULL;
    const void * data     = (const void *)(((const uint8_t *)data_arr) + (element_size * i));

    if(!STAT_is_OK(create_node(&new_node, element_size))) {
      destroy_chain_of_nodes(first_node);
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to create new node #%zu", i);
    }

    memcpy(new_node->data, data, element_size);

    if(i == 0) {
      first_node = new_node;
    } else {
      connect(prev_node, new_node);
    }

    prev_node = new_node;
  }

  if(o_first_node != NULL) *o_first_node = first_node;
  if(o_last_node != NULL) *o_last_node = prev_node;

  return OK;
}

// ===============
// == test-only ==

static bool is_circular_and_has_bidirectional_integrity(const LST_List * this) {
  const LST_Node * curr = this->sentinel;

  do {
    if(curr == NULL) return false;
    if(curr->next == NULL) return false;
    if(curr->prev == NULL) return false;
    if(curr->next->prev != curr) return false;
    curr = curr->next;
    // NOTE as we start on the sentinel, and check that the next node connects back to curr before
    // moving on, it is impossible to end up in a cycle that does not include the sentinel.
  } while(curr != this->sentinel);

  return true;
}

bool LST_IMPL_is_valid(const LST_List * this) {
  if(this == NULL) return false;
  if(this->element_size == 0) return false;
  if(this->sentinel == NULL) return false;
  if(!is_circular_and_has_bidirectional_integrity(this)) return false;

  return true;
}