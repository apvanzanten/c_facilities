#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "stat.h"
#include "test_utils.h"

#include "hashtable.h"

#define OK STAT_OK

static Result setup(void ** env_p);
static Result teardown(void ** env_p);

static Result tst_create_destroy_on_heap(void) {
  Result r = PASS;

  HT_HashTable * table = NULL;

  EXPECT_EQ(&r, OK, HT_create_on_heap(&table, sizeof(int), sizeof(double)));
  EXPECT_NE(&r, NULL, table);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, table->count);
  EXPECT_EQ(&r, sizeof(int), table->key_size);
  EXPECT_EQ(&r, sizeof(double), table->value_size);
  EXPECT_EQ(&r, false, table->has_indirect_key);
  EXPECT_EQ(&r, table->capacity, table->key_store.size);
  EXPECT_EQ(&r, table->capacity, table->value_store.size);

  EXPECT_EQ(&r, OK, HT_destroy_on_heap(&table));

  return r;
}

static Result tst_create_destroy_in_place(void) {
  Result r = PASS;

  HT_HashTable table = {0};

  EXPECT_EQ(&r, OK, HT_create_in_place(&table, sizeof(int), sizeof(double)));

  EXPECT_EQ(&r, 0, table.count);
  EXPECT_EQ(&r, sizeof(int), table.key_size);
  EXPECT_EQ(&r, sizeof(double), table.value_size);
  EXPECT_EQ(&r, false, table.has_indirect_key);
  EXPECT_EQ(&r, table.capacity, table.key_store.size);
  EXPECT_EQ(&r, table.capacity, table.value_store.size);

  EXPECT_EQ(&r, OK, HT_destroy_in_place(&table));

  return r;
}

static Result tst_fixture(void * env) {
  Result         r   = PASS;
  HT_HashTable * arr = env;

  EXPECT_NE(&r, NULL, arr);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_create_destroy_on_heap,
      tst_create_destroy_in_place,
  };

  TestWithFixture tests_with_fixture[] = {
      tst_fixture,
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
  Result r = PASS;

  EXPECT_NE(&r, NULL, env_p);

  return r;
}

static Result teardown(void ** env_p) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env_p);

  return r;
}