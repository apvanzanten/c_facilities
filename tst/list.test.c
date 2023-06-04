#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stat.h"
#include "test_utils.h"

#include "list.h"

#define OK STAT_OK

static Result setup(void ** env_p);
static Result teardown(void ** env_p);

static Result tst_create_destroy_in_place(void) {
  Result r = PASS;

  LST_List list = {0};
  EXPECT_EQ(&r, OK, LST_create(&list, sizeof(int)));
  EXPECT_TRUE(&r, LST_INT_is_valid(&list));

  EXPECT_EQ(&r, sizeof(int), list.element_size);

  EXPECT_NE(&r, NULL, list.sentinel);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, list.sentinel, list.sentinel->next);
  EXPECT_EQ(&r, list.sentinel, list.sentinel->prev);

  EXPECT_EQ(&r, OK, LST_destroy(&list));

  return r;
}

static bool is_aligned(size_t alignment_bytes, const void * ptr) {
  return ((((uintptr_t)ptr) % alignment_bytes) == 0);
}

static Result tst_memory_alignment_of_nodes(void) {
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

static Result tst_memory_alignment_of_nodes_in_list(void) {
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
    EXPECT_EQ(&r, OK, LST_create(&list, element_size));
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

    EXPECT_EQ(&r, OK, LST_destroy(&list));
    if(HAS_FAILED(&r)) return r;
  }

  free(data);

  return r;
}

static Result tst_insert(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  for(size_t i = 0; i < num_vals; i++) {
    LST_Node * predecessor = list->sentinel->prev;
    LST_Node * new_node    = NULL;

    EXPECT_EQ(&r, OK, LST_insert(list, list->sentinel, &vals[i], &new_node));
    EXPECT_NE(&r, NULL, list->sentinel);
    EXPECT_TRUE(&r, LST_INT_is_valid(list));
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

static Result tst_get_len(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  for(size_t i = 0; i < num_vals; i++) {
    LST_Node * new_node = NULL;

    EXPECT_EQ(&r, OK, LST_insert(list, list->sentinel, &vals[i], &new_node));
    EXPECT_TRUE(&r, LST_INT_is_valid(list));

    EXPECT_EQ(&r, i + 1, LST_get_len(list));

    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_first_last_end(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  LST_Node * first_node = NULL;

  for(size_t i = 0; i < num_vals; i++) {
    LST_Node * new_node = NULL;

    EXPECT_EQ(&r, OK, LST_insert(list, list->sentinel, &vals[i], &new_node));
    EXPECT_TRUE(&r, LST_INT_is_valid(list));
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

static Result tst_next_prev(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  for(size_t i = 0; i < num_vals; i++) {
    LST_Node * new_node = NULL;

    EXPECT_EQ(&r, OK, LST_insert(list, list->sentinel, &vals[i], &new_node));
    EXPECT_TRUE(&r, LST_INT_is_valid(list));
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

static Result tst_contains_and_find(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  for(size_t i = 0; i < num_vals; i++) {
    LST_Node * new_node = NULL;

    EXPECT_EQ(&r, OK, LST_insert(list, list->sentinel, &vals[i], &new_node));
    EXPECT_TRUE(&r, LST_INT_is_valid(list));
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

static Result tst_insert_from_array(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  LST_Node * first_node = NULL;

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, &first_node));
  EXPECT_TRUE(&r, LST_INT_is_valid(list));
  EXPECT_EQ(&r, num_vals, LST_get_len(list));
  EXPECT_EQ(&r, first_node, LST_first(list));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, vals[0], *((double *)LST_data(first_node)));
  EXPECT_EQ(&r, vals[num_vals - 1], *((double *)LST_data(LST_last(list))));

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, first_node, vals, num_vals, &first_node));
  EXPECT_TRUE(&r, LST_INT_is_valid(list));
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

static Result tst_remove(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, NULL));
  EXPECT_TRUE(&r, LST_INT_is_valid(list));
  EXPECT_EQ(&r, num_vals, LST_get_len(list));

  EXPECT_EQ(&r, OK, LST_remove(LST_first(list)));
  EXPECT_TRUE(&r, LST_INT_is_valid(list));
  EXPECT_EQ(&r, num_vals - 1, LST_get_len(list));

  EXPECT_EQ(&r, vals[1], *((double *)LST_data(LST_first(list))));

  EXPECT_EQ(&r, OK, LST_remove(LST_first(list)->next));
  EXPECT_TRUE(&r, LST_INT_is_valid(list));
  EXPECT_EQ(&r, num_vals - 2, LST_get_len(list));

  EXPECT_EQ(&r, vals[1], *((double *)LST_data(LST_first(list))));
  EXPECT_EQ(&r, vals[3], *((double *)LST_data(LST_first(list)->next)));

  return r;
}

static Result tst_remove_sequence(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, NULL));
  EXPECT_TRUE(&r, LST_INT_is_valid(list));
  EXPECT_EQ(&r, num_vals, LST_get_len(list));

  // remove sequence {0.0, 1.0, 2.0}
  // after: {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
  EXPECT_EQ(&r, OK, LST_remove_sequence(LST_first(list), LST_next(LST_first(list), 3)));
  EXPECT_TRUE(&r, LST_INT_is_valid(list));
  EXPECT_EQ(&r, num_vals - 3, LST_get_len(list));
  EXPECT_EQ(&r, 3.0, *((double *)LST_data(LST_first(list))));
  EXPECT_EQ(&r, 9.0, *((double *)LST_data(LST_last(list))));

  // remove sequence {5.0, 6.0, 7.0, 8.0}
  // after: {3.0, 4.0, 9.0};
  EXPECT_EQ(&r,
            OK,
            LST_remove_sequence(LST_next(LST_first(list), 2), LST_next(LST_first(list), 6)));
  EXPECT_TRUE(&r, LST_INT_is_valid(list));
  EXPECT_EQ(&r, num_vals - 7, LST_get_len(list));
  EXPECT_EQ(&r, 3.0, *((double *)LST_data(LST_first(list))));
  EXPECT_EQ(&r, 4.0, *((double *)LST_data(LST_next(LST_first(list), 1))));
  EXPECT_EQ(&r, 9.0, *((double *)LST_data(LST_last(list))));

  return r;
}

static Result tst_extract_and_inject(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, NULL));

  { // extract from back and inject at start
    LST_Node * node = LST_last(list);

    EXPECT_EQ(&r, OK, LST_extract(node));
    EXPECT_TRUE(&r, LST_INT_is_valid(list));
    EXPECT_EQ(&r, (num_vals - 1), LST_get_len(list));
    EXPECT_NE(&r, NULL, node);
    EXPECT_NE(&r, node, LST_last(list));
    EXPECT_FALSE(&r, LST_contains(list, LST_data(node)));
    if(HAS_FAILED(&r)) return r;

    EXPECT_EQ(&r, OK, LST_inject(node, LST_first(list)));
    EXPECT_TRUE(&r, LST_INT_is_valid(list));
    EXPECT_EQ(&r, num_vals, LST_get_len(list));
    EXPECT_EQ(&r, node, LST_first(list));
    if(HAS_FAILED(&r)) return r;
  }

  { // extract from front and inject at back
    LST_Node * node = LST_first(list);

    EXPECT_EQ(&r, OK, LST_extract(node));
    EXPECT_TRUE(&r, LST_INT_is_valid(list));
    EXPECT_EQ(&r, (num_vals - 1), LST_get_len(list));
    EXPECT_NE(&r, NULL, node);
    EXPECT_NE(&r, node, LST_first(list));
    EXPECT_FALSE(&r, LST_contains(list, LST_data(node)));
    if(HAS_FAILED(&r)) return r;

    EXPECT_EQ(&r, OK, LST_inject(node, LST_end(list)));
    EXPECT_TRUE(&r, LST_INT_is_valid(list));
    EXPECT_EQ(&r, num_vals, LST_get_len(list));
    EXPECT_EQ(&r, node, LST_last(list));
    if(HAS_FAILED(&r)) return r;
  }

  { // extract from [2] and inject before [4]
    LST_Node * node = LST_next(LST_first(list), 2);

    EXPECT_EQ(&r, OK, LST_extract(node));
    EXPECT_TRUE(&r, LST_INT_is_valid(list));
    EXPECT_EQ(&r, (num_vals - 1), LST_get_len(list));
    EXPECT_NE(&r, NULL, node);
    EXPECT_FALSE(&r, LST_contains(list, LST_data(node)));
    if(HAS_FAILED(&r)) return r;

    EXPECT_EQ(&r, OK, LST_inject(node, LST_next(LST_first(list), 4)));
    EXPECT_TRUE(&r, LST_INT_is_valid(list));
    EXPECT_EQ(&r, num_vals, LST_get_len(list));
    EXPECT_EQ(&r, node, LST_next(LST_first(list), 4));
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_extract_and_inject_sequence_front_to_back(void * env_p) {
  Result     r    = PASS;
  LST_List * list = (LST_List *)env_p;

  const double vals[]   = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, LST_insert_from_array(list, LST_end(list), vals, num_vals, NULL));

  // extract first 3 nodes, inject at end
  LST_Node * first = LST_first(list);
  LST_Node * last  = LST_next(LST_first(list), 2);

  EXPECT_EQ(&r, OK, LST_extract_sequence(first, LST_next(last, 1)));

  EXPECT_TRUE(&r, LST_INT_is_valid(list));
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

  EXPECT_TRUE(&r, LST_INT_is_valid(list));
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

static Result tst_extract_and_inject_sequence_middle(void * env_p) {
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

  EXPECT_TRUE(&r, LST_INT_is_valid(list));
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

  EXPECT_TRUE(&r, LST_INT_is_valid(list));
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

static Result tst_many_random_actions(void) {
  // randomly generate sequence of actions with sequence of parameters:
  // * insert node
  //    - position to insert
  // * insert from array
  //    - size of array
  //    - position to insert
  // * remove node, only if list is not empty
  //    - position to remove
  // * remove sequence, only if list is not empty
  //    - first node
  //    - successor node
  // * extract and inject node, only if list is not empty
  //    - position of node to extract
  //    - position to inject
  // * extract and inject sequence, only if list is not empty
  //    - position of first node to extract
  //    - position of successor
  //    - position to inject
  //
  // check along the way:
  // * validity
  // * length
  // * sum of values

  Result r = PASS;

  srand(time(NULL) + clock());

  const size_t num_iterations       = 5000;
  const size_t num_possible_actions = 6; // see list above
  const size_t element_size         = sizeof(uint32_t);
  const size_t max_sequence_size    = 32;

  LST_List list = {0};
  EXPECT_EQ(&r, OK, LST_create(&list, element_size));
  if(HAS_FAILED(&r)) return r;

  size_t list_len         = 0;
  size_t list_element_sum = 0;

  size_t action_histogram[6] = {0};
  EXPECT_EQ(&r, num_possible_actions, sizeof(action_histogram) / sizeof(size_t));
  if(HAS_FAILED(&r)) return r;

  for(size_t iteration = 0; iteration < num_iterations; iteration++) {
    // pick one of six actions
    const size_t  num_possible_actions_adjusted = (list_len == 0 ? 2 : num_possible_actions);
    const uint8_t action_nr                     = ((size_t)rand() % num_possible_actions_adjusted);
    EXPECT_TRUE(&r, action_nr < num_possible_actions);
    if(HAS_FAILED(&r)) return r;

    switch(action_nr) {
    case 0: { // insert node
      const uint32_t val_to_insert = (iteration & 0xfff);
      const int      pos_to_insert = (rand() % (list_len + 1));
      LST_Node *     successor     = LST_next(LST_first(&list), pos_to_insert);
      LST_Node *     inserted_node = NULL;

      EXPECT_EQ(&r, OK, LST_insert(&list, successor, &val_to_insert, &inserted_node));
      EXPECT_EQ(&r, val_to_insert, *((uint32_t *)LST_data(inserted_node)));

      list_len++;
      list_element_sum += val_to_insert;
      action_histogram[0]++;
      break;
    }
    case 1: { // insert from array
      const size_t array_size    = (rand() % max_sequence_size) + 1;
      const int    pos_to_insert = (rand() % (list_len + 1));
      LST_Node *   successor     = LST_next(LST_first(&list), pos_to_insert);
      LST_Node *   first_node    = NULL;

      uint32_t * array = malloc(array_size * element_size);
      EXPECT_NE(&r, NULL, array);
      if(HAS_FAILED(&r)) return r;

      for(uint32_t i = 0; i < array_size; i++) {
        array[i] = i;
        list_element_sum += i;
      }

      EXPECT_EQ(&r, OK, LST_insert_from_array(&list, successor, array, array_size, &first_node));
      EXPECT_EQ(&r, array[0], *((uint32_t *)LST_data(first_node)));

      free(array);

      list_len += array_size;
      action_histogram[1]++;
      break;
    }
    case 2: { // remove node
      if(list_len > 0) {
        const int  pos_to_remove  = (rand() % list_len);
        LST_Node * node_to_remove = LST_next(LST_first(&list), pos_to_remove);

        list_element_sum -= *((uint32_t *)LST_data(node_to_remove));

        EXPECT_EQ(&r, OK, LST_remove(node_to_remove));

        list_len--;
        action_histogram[2]++;
      }
      break;
    }
    case 3: { // remove sequence
      if(list_len > 0) {
        const size_t max_to_remove = (max_sequence_size > list_len) ? list_len : max_sequence_size;
        const size_t num_to_remove = (rand() % max_to_remove) + 1;
        const size_t max_first_pos_to_remove = (list_len - num_to_remove);
        const int    first_pos_to_remove =
            (max_first_pos_to_remove == 0 ? max_first_pos_to_remove
                                          : (rand() % max_first_pos_to_remove));
        LST_Node * first_node_to_remove = LST_next(LST_first(&list), first_pos_to_remove);
        LST_Node * successor            = LST_next(first_node_to_remove, num_to_remove);

        for(LST_Node * node = first_node_to_remove; node != successor; node = node->next) {
          list_element_sum -= *((uint32_t *)LST_data(node));
        }

        EXPECT_EQ(&r, OK, LST_remove_sequence(first_node_to_remove, successor));

        list_len -= num_to_remove;
        action_histogram[3]++;
      }
      break;
    }
    case 4: { // extract and inject node
      if(list_len > 0) {
        const int    pos_to_extract   = (rand() % list_len);
        const size_t interim_list_len = (list_len - 1);
        const int    pos_to_inject    = (interim_list_len == 0 ? 0 : (rand() % (interim_list_len)));
        LST_Node *   node             = LST_next(LST_first(&list), pos_to_extract);

        EXPECT_EQ(&r, OK, LST_extract(node));
        EXPECT_EQ(&r, interim_list_len, LST_get_len(&list));
        if(HAS_FAILED(&r)) return r;

        LST_Node * successor = LST_next(LST_first(&list), pos_to_inject);
        EXPECT_EQ(&r, OK, LST_inject(node, successor));
        EXPECT_EQ(&r, list_len, LST_get_len(&list));

        action_histogram[4]++;
      }
      break;
    }
    case 5: { // extract and inject sequence
      if(list_len > 0) {
        const size_t max_to_extract = (max_sequence_size > list_len) ? list_len : max_sequence_size;
        const size_t num_to_extract = (rand() % max_to_extract) + 1;
        const size_t max_pos_to_extract = (list_len - num_to_extract);
        const int    pos_to_extract =
            (max_pos_to_extract == 0 ? max_pos_to_extract : (rand() % max_pos_to_extract));
        const size_t interim_list_len = (list_len - num_to_extract);
        const size_t pos_to_inject    = (interim_list_len == 0 ? 0 : (rand() % interim_list_len));

        LST_Node * first_node        = LST_next(LST_first(&list), pos_to_extract);
        LST_Node * extract_successor = LST_next(first_node, num_to_extract);
        LST_Node * last_node         = extract_successor->prev;

        EXPECT_EQ(&r, OK, LST_extract_sequence(first_node, extract_successor));
        EXPECT_EQ(&r, interim_list_len, LST_get_len(&list));
        if(HAS_FAILED(&r)) return r;

        LST_Node * inject_successor = LST_next(LST_first(&list), pos_to_inject);
        EXPECT_EQ(&r, OK, LST_inject_sequence(first_node, last_node, inject_successor));
        EXPECT_EQ(&r, list_len, LST_get_len(&list));

        action_histogram[5]++;
      }
      break;
    }
    default: EXPECT_FALSE(&r, true); return r;
    }

    EXPECT_TRUE(&r, LST_INT_is_valid(&list));
    EXPECT_EQ(&r, list_len, LST_get_len(&list));
    if(HAS_FAILED(&r)) return r;

    size_t sum = 0;
    for(LST_Node * node = LST_first(&list); node != LST_end(&list); node = node->next) {
      sum += *((uint32_t *)LST_data(node));
    }
    EXPECT_EQ(&r, list_element_sum, sum);

    if(HAS_FAILED(&r)) return r;

    if((iteration & 0xff) == 0) {
      printf("it %zu, list_len: %zu, list_element_sum: %zu, ",
             iteration,
             list_len,
             list_element_sum);
      printf("action hist: [ ");
      for(size_t i = 0; i < num_possible_actions; i++) {
        printf("%zu ", action_histogram[i]);
      }
      printf("]\n");
    }
  }

  LST_destroy(&list);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_create_destroy_in_place,
      tst_memory_alignment_of_nodes,
      tst_memory_alignment_of_nodes_in_list,

      // run this test a couple times, it gets a new seed every time
      tst_many_random_actions,
      tst_many_random_actions,
      tst_many_random_actions,
      tst_many_random_actions,
      tst_many_random_actions,
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

  EXPECT_NE(&r, NULL, list_pp);
  if(HAS_FAILED(&r)) return r;

  *list_pp = malloc(sizeof(LST_List));
  EXPECT_NE(&r, NULL, *list_pp);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, LST_create(*list_pp, sizeof(double)));
  EXPECT_TRUE(&r, LST_INT_is_valid(*list_pp));

  return r;
}

static Result teardown(void ** env_pp) {
  Result      r       = PASS;
  LST_List ** list_pp = (LST_List **)env_pp;

  EXPECT_NE(&r, NULL, list_pp);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, LST_destroy(*list_pp));

  free(*list_pp);
  *list_pp = NULL;

  return r;
}