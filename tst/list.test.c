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

int main() {
  Test tests[] = {
      tst_create_destroy_on_heap,
      tst_create_destroy_in_place,
      tst_memory_alignment_of_nodes,
      tst_memory_alignment_of_nodes_in_list,
  };

  TestWithFixture tests_with_fixture[] = {
      tst_insert,
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

  return r;
}

static Result teardown(void ** env_pp) {
  Result      r       = PASS;
  LST_List ** list_pp = (LST_List **)env_pp;

  EXPECT_EQ(&r, OK, LST_destroy_on_heap(list_pp));

  return r;
}