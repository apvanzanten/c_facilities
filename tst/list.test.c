#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "stat.h"
#include "test_utils.h"

#include "list.h"

#define OK STAT_OK

static Result setup(void ** env_p);
static Result teardown(void ** env_p);

Result tst_create_destroy_on_heap() {
  Result r = PASS;

  LST_List * list = NULL;
  EXPECT_EQ(&r, OK, LST_create_on_heap(&list, sizeof(int)));
  EXPECT_NE(&r, NULL, list);
  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, sizeof(int), list->element_size);

  EXPECT_NE(&r, NULL, list->sentinel);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, list->sentinel, list->sentinel->next);
  EXPECT_EQ(&r, list->sentinel, list->sentinel->prev);

  EXPECT_EQ(&r, OK, LST_destroy_on_heap(&list));

  return r;
}

Result tst_create_destroy_in_place() {
  Result r = PASS;

  LST_List list = {0};
  EXPECT_EQ(&r, OK, LST_create_in_place(&list, sizeof(int)));
  EXPECT_TRUE(&r, LST_IMPL_is_valid(&list));

  EXPECT_EQ(&r, sizeof(int), list.element_size);

  EXPECT_NE(&r, NULL, list.sentinel);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, list.sentinel, list.sentinel->next);
  EXPECT_EQ(&r, list.sentinel, list.sentinel->prev);

  EXPECT_EQ(&r, OK, LST_destroy_in_place(&list));

  return r;
}

static bool is_aligned(size_t alignment_bytes, const void * ptr) {
  return ((((uintptr_t)ptr) % alignment_bytes) == 0);
}

Result tst_memory_alignment_of_nodes() {
  Result r = PASS;

  // expect alignment of sizeof(max_align_t) for both struct and data flexible array member
  const LST_Node node_on_stack_a = {0};
  const LST_Node node_on_stack_b = {0};
  const LST_Node node_on_stack_c = {0};
  EXPECT_TRUE(&r, is_aligned(sizeof(max_align_t), &node_on_stack_a));
  EXPECT_TRUE(&r, is_aligned(sizeof(max_align_t), &node_on_stack_b));
  EXPECT_TRUE(&r, is_aligned(sizeof(max_align_t), &node_on_stack_c));
  EXPECT_TRUE(&r, is_aligned(sizeof(max_align_t), node_on_stack_a.data));
  EXPECT_TRUE(&r, is_aligned(sizeof(max_align_t), node_on_stack_b.data));
  EXPECT_TRUE(&r, is_aligned(sizeof(max_align_t), node_on_stack_c.data));

  LST_Node * node_on_heap = NULL;

  for(size_t element_size = 0; element_size < 1024; element_size++) {
    const size_t base_size  = sizeof(LST_Node) + element_size;
    const size_t alloc_size = ((base_size / sizeof(max_align_t)) + 1) * sizeof(max_align_t);

    node_on_heap = aligned_alloc(sizeof(max_align_t), alloc_size);
    EXPECT_NE(&r, NULL, node_on_heap);

    // expect alignment of sizeof(max_align_t) for both struct and data flexible array member
    EXPECT_TRUE(&r, is_aligned(sizeof(max_align_t), node_on_heap));
    EXPECT_TRUE(&r, is_aligned(sizeof(max_align_t), node_on_heap->data));

    free(node_on_heap);
    node_on_heap = NULL;

    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

Result tst_memory_alignment_of_nodes_in_list() {
  Result r = PASS;

  LST_List     list             = {0};
  const size_t min_element_size = 1;
  const size_t max_element_size = 1024;
  const size_t num_nodes        = 8;

  uint8_t * data = malloc(max_element_size);
  EXPECT_NE(&r, NULL, data);
  if(HAS_FAILED(&r)) return r;
  for(size_t i = 0; i < max_element_size; i++) {
    data[i] = (i & 0xff);
  }

  for(size_t element_size = min_element_size; element_size < max_element_size; element_size++) {
    EXPECT_EQ(&r, OK, LST_create_in_place(&list, element_size));
    if(HAS_FAILED(&r)) return r;

    for(size_t node_idx = 0; node_idx < num_nodes; node_idx++) {
      LST_Node * new_node = NULL;

      EXPECT_EQ(&r, OK, LST_insert(&list, list.sentinel, (void *)data, &new_node));
      EXPECT_NE(&r, NULL, list.sentinel);
      EXPECT_NE(&r, NULL, new_node);
      if(HAS_FAILED(&r)) return r;

      EXPECT_TRUE(&r, is_aligned(sizeof(max_align_t), new_node));
      EXPECT_TRUE(&r, is_aligned(sizeof(max_align_t), new_node->data));

      if(HAS_FAILED(&r)) return r;
    }

    EXPECT_EQ(&r, OK, LST_destroy_in_place(&list));
    if(HAS_FAILED(&r)) return r;
  }

  free(data);

  return r;
}

Result tst_insert(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  for(size_t i = 0; i < num_vals; i++) {
    LST_Node * predecessor = list->sentinel->prev;
    LST_Node * new_node    = NULL;

    EXPECT_EQ(&r, OK, LST_insert(list, list->sentinel, &vals[i], &new_node));
    EXPECT_NE(&r, NULL, list->sentinel);
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
    if(HAS_FAILED(&r)) return r;

    EXPECT_NE(&r, NULL, list->sentinel->prev);
    EXPECT_NE(&r, NULL, list->sentinel->next);
    if(HAS_FAILED(&r)) return r;

    EXPECT_NE(&r, NULL, new_node);
    EXPECT_EQ(&r, list->sentinel, new_node->next);
    EXPECT_EQ(&r, predecessor, new_node->prev);
    EXPECT_EQ(&r, new_node, predecessor->next);

    EXPECT_EQ(&r, vals[i], *((double *)((void *)new_node->data)));
    if(HAS_FAILED(&r)) return r;
  }

  LST_Node * prev = list->sentinel;
  LST_Node * node = list->sentinel->next;
  size_t     i    = 0;
  while(node != list->sentinel) {
    EXPECT_TRUE(&r, i < num_vals);
    if(HAS_FAILED(&r)) return r;

    EXPECT_EQ(&r, prev, node->prev);
    EXPECT_EQ(&r, node, prev->next);
    EXPECT_EQ(&r, vals[i], *((double *)((void *)node->data)));

    prev = node;
    node = node->next;
    i++;
  }

  return r;
}

Result tst_get_len(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  for(size_t i = 0; i < num_vals; i++) {
    LST_Node * new_node = NULL;

    EXPECT_EQ(&r, OK, LST_insert(list, list->sentinel, &vals[i], &new_node));
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));

    EXPECT_EQ(&r, i + 1, LST_get_len(list));

    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

Result tst_first_last_end(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  LST_Node * first_node = NULL;

  for(size_t i = 0; i < num_vals; i++) {
    LST_Node * new_node = NULL;

    EXPECT_EQ(&r, OK, LST_insert(list, list->sentinel, &vals[i], &new_node));
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
    if(HAS_FAILED(&r)) return r;

    if(i == 0) first_node = new_node;

    EXPECT_EQ(&r, first_node, LST_first(list));
    EXPECT_EQ(&r, new_node, LST_last(list));
    EXPECT_EQ(&r, list->sentinel, LST_end(list));

    EXPECT_EQ(&r, first_node, LST_first((const LST_List *)list));
    EXPECT_EQ(&r, new_node, LST_last((const LST_List *)list));
    EXPECT_EQ(&r, list->sentinel, LST_end((const LST_List *)list));

    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

Result tst_next_prev(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  for(size_t i = 0; i < num_vals; i++) {
    LST_Node * new_node = NULL;

    EXPECT_EQ(&r, OK, LST_insert(list, list->sentinel, &vals[i], &new_node));
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
    if(HAS_FAILED(&r)) return r;

    for(int j = 0; j <= (int)i; j++) {
      const double val             = vals[j];
      const int    dist_from_first = j;
      const int    dist_from_last  = (int)i - j;

      LST_Node *       first       = LST_first(list);
      const LST_Node * first_const = LST_first((const LST_List *)list);
      LST_Node *       last        = LST_last(list);
      const LST_Node * last_const  = LST_last((const LST_List *)list);

      EXPECT_EQ(&r, val, *(double *)LST_data(LST_next(first, dist_from_first)));
      EXPECT_EQ(&r, val, *(double *)LST_data(LST_next(last, -dist_from_last)));
      EXPECT_EQ(&r, val, *(const double *)LST_data(LST_next(first_const, dist_from_first)));
      EXPECT_EQ(&r, val, *(const double *)LST_data(LST_next(last_const, -dist_from_last)));

      EXPECT_EQ(&r, val, *(double *)LST_data(LST_prev(first, -dist_from_first)));
      EXPECT_EQ(&r, val, *(double *)LST_data(LST_prev(last, dist_from_last)));
      EXPECT_EQ(&r, val, *(const double *)LST_data(LST_prev(first_const, -dist_from_first)));
      EXPECT_EQ(&r, val, *(const double *)LST_data(LST_prev(last_const, dist_from_last)));
    }

    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

Result tst_contains_and_find(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  for(size_t i = 0; i < num_vals; i++) {
    LST_Node * new_node = NULL;

    EXPECT_EQ(&r, OK, LST_insert(list, list->sentinel, &vals[i], &new_node));
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
    if(HAS_FAILED(&r)) return r;

    for(size_t j = 0; j < num_vals; j++) {
      if(j <= i) {
        EXPECT_TRUE(&r, LST_contains(list, &vals[j]));

        LST_Node * found_node = NULL;
        EXPECT_EQ(&r, OK, LST_find(list, &vals[j], &found_node));
        EXPECT_NE(&r, NULL, found_node);
        EXPECT_EQ(&r, vals[j], *(double *)LST_data(found_node));
      } else {
        EXPECT_FALSE(&r, LST_contains(list, &vals[j]));

        LST_Node * found_node = NULL;
        EXPECT_EQ(&r, STAT_OK_NOT_FOUND, LST_find(list, &vals[j], &found_node));
        EXPECT_EQ(&r, NULL, found_node);
      }
    }

    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

Result tst_insert_from_array(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  LST_Node * first_node = NULL;

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, &first_node));
  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_EQ(&r, num_vals, LST_get_len(list));
  EXPECT_EQ(&r, first_node, LST_first(list));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, vals[0], *((double *)LST_data(first_node)));
  EXPECT_EQ(&r, vals[num_vals - 1], *((double *)LST_data(LST_last(list))));

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, first_node, vals, num_vals, &first_node));
  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_EQ(&r, num_vals * 2, LST_get_len(list));
  EXPECT_EQ(&r, first_node, LST_first(list));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, vals[0], *((double *)LST_data(first_node)));

  LST_Node * prev = list->sentinel;
  LST_Node * node = list->sentinel->next;
  size_t     i    = 0;
  while(node != list->sentinel) {
    EXPECT_TRUE(&r, i < (num_vals * 2));
    if(HAS_FAILED(&r)) return r;

    EXPECT_EQ(&r, prev, node->prev);
    EXPECT_EQ(&r, node, prev->next);
    EXPECT_EQ(&r, vals[(i % num_vals)], *((double *)LST_data(node)));

    prev = node;
    node = node->next;
    i++;
  }

  return r;
}

Result tst_remove(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, NULL));
  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_EQ(&r, num_vals, LST_get_len(list));

  EXPECT_EQ(&r, OK, LST_remove(LST_first(list)));
  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_EQ(&r, num_vals - 1, LST_get_len(list));

  EXPECT_EQ(&r, vals[1], *((double *)LST_data(LST_first(list))));

  EXPECT_EQ(&r, OK, LST_remove(LST_first(list)->next));
  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_EQ(&r, num_vals - 2, LST_get_len(list));

  EXPECT_EQ(&r, vals[1], *((double *)LST_data(LST_first(list))));
  EXPECT_EQ(&r, vals[3], *((double *)LST_data(LST_first(list)->next)));

  return r;
}

Result tst_remove_sequence(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, NULL));
  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_EQ(&r, num_vals, LST_get_len(list));

  // remove sequence {0.0, 1.0, 2.0}
  // after: {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
  EXPECT_EQ(&r, OK, LST_remove_sequence(LST_first(list), LST_next(LST_first(list), 3)));
  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_EQ(&r, num_vals - 3, LST_get_len(list));
  EXPECT_EQ(&r, 3.0, *((double *)LST_data(LST_first(list))));
  EXPECT_EQ(&r, 9.0, *((double *)LST_data(LST_last(list))));

  // remove sequence {5.0, 6.0, 7.0, 8.0}
  // after: {3.0, 4.0, 9.0};
  EXPECT_EQ(&r,
            OK,
            LST_remove_sequence(LST_next(LST_first(list), 2), LST_next(LST_first(list), 6)));
  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_EQ(&r, num_vals - 7, LST_get_len(list));
  EXPECT_EQ(&r, 3.0, *((double *)LST_data(LST_first(list))));
  EXPECT_EQ(&r, 4.0, *((double *)LST_data(LST_next(LST_first(list), 1))));
  EXPECT_EQ(&r, 9.0, *((double *)LST_data(LST_last(list))));

  return r;
}

Result tst_extract_and_inject(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, NULL));

  { // extract from back and inject at start
    LST_Node * node = LST_last(list);

    EXPECT_EQ(&r, OK, LST_extract(node));
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
    EXPECT_EQ(&r, (num_vals - 1), LST_get_len(list));
    EXPECT_NE(&r, NULL, node);
    EXPECT_NE(&r, node, LST_last(list));
    EXPECT_FALSE(&r, LST_contains(list, LST_data(node)));
    if(HAS_FAILED(&r)) return r;

    EXPECT_EQ(&r, OK, LST_inject(node, LST_first(list)));
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
    EXPECT_EQ(&r, num_vals, LST_get_len(list));
    EXPECT_EQ(&r, node, LST_first(list));
    if(HAS_FAILED(&r)) return r;
  }

  { // extract from front and inject at back
    LST_Node * node = LST_first(list);

    EXPECT_EQ(&r, OK, LST_extract(node));
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
    EXPECT_EQ(&r, (num_vals - 1), LST_get_len(list));
    EXPECT_NE(&r, NULL, node);
    EXPECT_NE(&r, node, LST_first(list));
    EXPECT_FALSE(&r, LST_contains(list, LST_data(node)));
    if(HAS_FAILED(&r)) return r;

    EXPECT_EQ(&r, OK, LST_inject(node, LST_end(list)));
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
    EXPECT_EQ(&r, num_vals, LST_get_len(list));
    EXPECT_EQ(&r, node, LST_last(list));
    if(HAS_FAILED(&r)) return r;
  }

  { // extract from [2] and inject before [4]
    LST_Node * node = LST_next(LST_first(list), 2);

    EXPECT_EQ(&r, OK, LST_extract(node));
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
    EXPECT_EQ(&r, (num_vals - 1), LST_get_len(list));
    EXPECT_NE(&r, NULL, node);
    EXPECT_FALSE(&r, LST_contains(list, LST_data(node)));
    if(HAS_FAILED(&r)) return r;

    EXPECT_EQ(&r, OK, LST_inject(node, LST_next(LST_first(list), 4)));
    EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
    EXPECT_EQ(&r, num_vals, LST_get_len(list));
    EXPECT_EQ(&r, node, LST_next(LST_first(list), 4));
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

Result tst_extract_and_inject_sequence_front_to_back(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, NULL));

  // extract first 3 nodes, inject at end
  LST_Node * first = LST_first(list);
  LST_Node * last  = LST_next(LST_first(list), 2);

  EXPECT_EQ(&r, OK, LST_extract_sequence(first, LST_next(last, 1)));

  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_FALSE(&r, LST_contains(list, LST_data(first)));
  EXPECT_FALSE(&r, LST_contains(list, LST_data(last)));
  EXPECT_EQ(&r, (num_vals - 3), LST_get_len(list));

  EXPECT_EQ(&r, NULL, first->prev);
  EXPECT_NE(&r, NULL, first->next);
  EXPECT_EQ(&r, NULL, last->next);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, last, LST_next(first, 2));
  EXPECT_EQ(&r, first, LST_prev(last, 2));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, LST_inject_sequence(first, last, LST_end(list)));

  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_TRUE(&r, LST_contains(list, LST_data(first)));
  EXPECT_TRUE(&r, LST_contains(list, LST_data(last)));
  EXPECT_EQ(&r, num_vals, LST_get_len(list));

  EXPECT_NE(&r, NULL, first->prev);
  EXPECT_NE(&r, NULL, first->next);
  EXPECT_NE(&r, NULL, last->next);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, last, LST_next(first, 2));
  EXPECT_EQ(&r, first, LST_prev(last, 2));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, last, LST_last(list));

  return r;
}

Result tst_extract_and_inject_sequence_middle(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, NULL));

  // extract nodes 2 through 4, inject before 3 (original position 6)
  // before extraction: 0 1 2 3 4 5 6 7
  // after extraction:  0 1 5 6 7
  // after injection:   0 1 5 2 3 4 6 7
  LST_Node * first = LST_next(LST_first(list), 2);
  LST_Node * last  = LST_next(LST_first(list), 4);

  EXPECT_EQ(&r, OK, LST_extract_sequence(first, LST_next(last, 1)));

  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_FALSE(&r, LST_contains(list, LST_data(first)));
  EXPECT_FALSE(&r, LST_contains(list, LST_data(last)));
  EXPECT_EQ(&r, (num_vals - 3), LST_get_len(list));

  EXPECT_EQ(&r, NULL, first->prev);
  EXPECT_NE(&r, NULL, first->next);
  EXPECT_EQ(&r, NULL, last->next);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, last, LST_next(first, 2));
  EXPECT_EQ(&r, first, LST_prev(last, 2));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, LST_inject_sequence(first, last, LST_next(LST_first(list), 3)));

  EXPECT_TRUE(&r, LST_IMPL_is_valid(list));
  EXPECT_TRUE(&r, LST_contains(list, LST_data(first)));
  EXPECT_TRUE(&r, LST_contains(list, LST_data(last)));
  EXPECT_EQ(&r, num_vals, LST_get_len(list));

  EXPECT_NE(&r, NULL, first->prev);
  EXPECT_NE(&r, NULL, first->next);
  EXPECT_NE(&r, NULL, last->next);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, last, LST_next(first, 2));
  EXPECT_EQ(&r, first, LST_prev(last, 2));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, first, LST_next(LST_first(list), 3));
  EXPECT_EQ(&r, last, LST_prev(LST_end(list), 3));

  return r;
}

int main() {
  Test tests[] = {
      tst_create_destroy_on_heap,
      tst_create_destroy_in_place,
      tst_memory_alignment_of_nodes,
      tst_memory_alignment_of_nodes_in_list,
  };

  TestWithFixture tests_with_fixture[] = {
      tst_insert,
      tst_get_len,
      tst_first_last_end,
      tst_next_prev,
      tst_contains_and_find,
      tst_insert_from_array,
      tst_remove,
      tst_remove_sequence,
      tst_extract_and_inject,
      tst_extract_and_inject_sequence_front_to_back,
      tst_extract_and_inject_sequence_middle,
  };

  const Result test_res = run_tests(tests, sizeof(tests) / sizeof(Test));
  const Result test_with_fixture_res =
      run_tests_with_fixture(tests_with_fixture,
                             sizeof(tests_with_fixture) / sizeof(TestWithFixture),
                             setup,
                             teardown);

  return ((test_res == PASS) && (test_with_fixture_res == PASS)) ? 0 : 1;
}

static Result setup(void ** env_pp) {
  Result      r       = PASS;
  LST_List ** list_pp = (LST_List **)env_pp;

  EXPECT_EQ(&r, OK, LST_create_on_heap(list_pp, sizeof(double)));
  EXPECT_TRUE(&r, LST_IMPL_is_valid(*list_pp));

  return r;
}

static Result teardown(void ** env_pp) {
  Result      r       = PASS;
  LST_List ** list_pp = (LST_List **)env_pp;

  EXPECT_EQ(&r, OK, LST_destroy_on_heap(list_pp));

  return r;
}