
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "stat.h"
#include "test_utils.h"

#include "darray.h"

#define OK STAT_OK

static Result setup(void ** env_p);
static Result teardown(void ** env_p);

Result tst_create_destroy_on_heap() {
  Result r = PASS;

  DAR_DArray * arr = NULL;
  EXPECT_EQ(&r, OK, DAR_create_on_heap(&arr, sizeof(int)));

  EXPECT_NE(&r, NULL, arr);
  if(HAS_FAILED(&r)) return r;

  EXPECT_NE(&r, NULL, arr->data);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, sizeof(int), arr->element_size);
  EXPECT_EQ(&r, 0, arr->size);

  EXPECT_EQ(&r, OK, DAR_destroy_on_heap(&arr));
  EXPECT_EQ(&r, NULL, arr);

  return r;
}

Result tst_create_destroy_in_place() {
  Result r = PASS;

  DAR_DArray arr = {0};
  EXPECT_EQ(&r, OK, DAR_create_in_place(&arr, sizeof(int)));

  EXPECT_NE(&r, NULL, arr.data);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, sizeof(int), arr.element_size);
  EXPECT_EQ(&r, 0, arr.size);

  EXPECT_EQ(&r, OK, DAR_destroy_in_place(&arr));
  EXPECT_EQ(&r, NULL, arr.data);

  return r;
}

Result tst_fixture(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  EXPECT_NE(&r, arr, NULL);
  if(HAS_FAILED(&r)) return r;

  EXPECT_NE(&r, NULL, arr->data);
  EXPECT_EQ(&r, 0, arr->size);
  EXPECT_NE(&r, 0, arr->capacity_magnitude);
  EXPECT_EQ(&r, 8, arr->element_size);

  return r;
}

Result tst_push_back(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  double vals[] = {1.0, 2.0, 3.0, 4.0};

  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[0]));
  EXPECT_EQ(&r, 1, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, &vals[0], sizeof(double)));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[1]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[2]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[3]));
  EXPECT_EQ(&r, 4, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, vals, sizeof(vals)));
  if(HAS_FAILED(&r)) return r;

  return r;
}

Result tst_pop_back(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  double vals[] = {1.0, 2.0, 3.0, 4.0};

  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[0]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[1]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[2]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[3]));
  EXPECT_EQ(&r, 4, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, vals, sizeof(vals)));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_pop_back(arr));
  EXPECT_EQ(&r, 3, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, vals, sizeof(double) * 3));

  EXPECT_EQ(&r, OK, DAR_pop_back(arr));
  EXPECT_EQ(&r, 2, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, vals, sizeof(double) * 2));

  EXPECT_EQ(&r, OK, DAR_pop_back(arr));
  EXPECT_EQ(&r, 1, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, vals, sizeof(double) * 1));

  EXPECT_EQ(&r, OK, DAR_pop_back(arr));
  EXPECT_EQ(&r, 0, arr->size);

  return r;
}

Result tst_capacity(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const size_t num_elements = 1024;

  size_t capacity = DAR_get_capacity(arr);
  size_t size     = 0;

  while(size < num_elements) {
    const double val = 1.234;

    EXPECT_EQ(&r, OK, DAR_push_back(arr, &val));
    size++;

    if(capacity >= size) {
      EXPECT_EQ(&r, capacity, DAR_get_capacity(arr));
    } else {
      const size_t new_capacity = DAR_get_capacity(arr);
      EXPECT_TRUE(&r, capacity < new_capacity);
      capacity = new_capacity;
    }

    if(HAS_FAILED(&r)) return r;
  }

  do {
    EXPECT_EQ(&r, OK, DAR_pop_back(arr));
    size--;

    EXPECT_EQ(&r, OK, DAR_shrink_to_fit(arr));

    const size_t minimum_capacity = 8;

    if(((size * 2) > capacity) || (capacity <= minimum_capacity)) {
      EXPECT_EQ(&r, capacity, DAR_get_capacity(arr));
    } else {
      const size_t new_capacity = DAR_get_capacity(arr);
      EXPECT_TRUE(&r, capacity > new_capacity);
      capacity = new_capacity;
    }

    if(HAS_FAILED(&r)) return r;
  } while(size > 0);

  return r;
}

Result tst_reserve(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const size_t initial_capacity = DAR_get_capacity(arr);

  for(size_t num_elements = 16; num_elements < 10000; num_elements *= 1.2) {
    EXPECT_EQ(&r, OK, DAR_reserve(arr, num_elements));

    const size_t reserved_capacity = DAR_get_capacity(arr);

    for(size_t i = 1; i <= num_elements; i++) {
      const double val = (double)i;

      EXPECT_EQ(&r, OK, DAR_push_back(arr, &val));

      EXPECT_EQ(&r, i, arr->size);
      EXPECT_EQ(&r, reserved_capacity, DAR_get_capacity(arr));

      if(HAS_FAILED(&r)) return r;
    }

    while(arr->size != 0) {
      EXPECT_EQ(&r, OK, DAR_pop_back(arr));
      if(HAS_FAILED(&r)) return r;
    }

    EXPECT_EQ(&r, OK, DAR_shrink_to_fit(arr));
    EXPECT_EQ(&r, initial_capacity, DAR_get_capacity(arr));
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

int main() {
  Test tests[] = {
      tst_create_destroy_on_heap,
      tst_create_destroy_in_place,
  };

  TestWithFixture tests_with_fixture[] = {
      tst_fixture,
      tst_push_back,
      tst_pop_back,
      tst_capacity,
      tst_reserve,
  };

  const Result test_res = run_tests(tests, sizeof(tests) / sizeof(Test));
  const Result test_with_fixture_res =
      run_tests_with_fixture(tests_with_fixture,
                             sizeof(tests_with_fixture) / sizeof(TestWithFixture),
                             setup,
                             teardown);

  return ((test_res == PASS) && (test_with_fixture_res == PASS)) ? 0 : 1;
}

static Result setup(void ** env_p) {
  Result        r     = PASS;
  DAR_DArray ** arr_p = (DAR_DArray **)env_p;

  EXPECT_EQ(&r, OK, DAR_create_on_heap(arr_p, sizeof(double)));

  return r;
}

static Result teardown(void ** env_p) {
  Result        r     = PASS;
  DAR_DArray ** arr_p = (DAR_DArray **)env_p;

  EXPECT_EQ(&r, OK, DAR_destroy_on_heap(arr_p));

  return r;
}