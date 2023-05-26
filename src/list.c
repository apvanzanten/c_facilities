#include "list.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"

#define OK STAT_OK

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

STAT_Val LST_insert(LST_List * this,
                    LST_Node *   successor,
                    const void * data,
                    LST_Node **  o_inserted_node) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(successor == NULL) return LOG_STAT(STAT_ERR_ARGS, "successor is NULL");
  if(data == NULL) return LOG_STAT(STAT_ERR_ARGS, "data is NULL");

  LST_Node * new_node = NULL;
  if(!STAT_is_OK(create_node(&new_node, this->element_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create new node for insertion");
  }

  memcpy(new_node->data, data, this->element_size);

  new_node->next       = successor;
  new_node->prev       = successor->prev;
  new_node->next->prev = new_node;
  new_node->prev->next = new_node;

  if(o_inserted_node != NULL) *o_inserted_node = new_node;

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
