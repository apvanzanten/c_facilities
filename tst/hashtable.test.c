#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stat.h"
#include "test_utils.h"

#include "darray.h"
#include "hashtable.h"
#include "span.h"

#define OK STAT_OK

static Result setup_fixed_size_key_fixture(void ** env_p);
static Result teardown(void ** env_p);

static Result tst_create_destroy(void) {
  Result       r     = PASS;
  HT_HashTable table = {0};

  EXPECT_EQ(&r, OK, HT_create(&table, sizeof(int), sizeof(int64_t)));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, table.count);
  EXPECT_EQ(&r, sizeof(int), table.key_size);
  EXPECT_EQ(&r, sizeof(int64_t), table.value_size);
  EXPECT_NE(&r, NULL, table.store.data);

  EXPECT_EQ(&r, OK, HT_destroy(&table));

  return r;
}

static void make_rand_val(void * val, size_t size) {
  for(size_t i = 0; i < size; i++) {
    ((uint8_t *)val)[i] = (rand() & 0xff);
  }
}

static Result tst_many_random_sets_gets_removes(void) {
  Result       r      = PASS;
  HT_HashTable table  = {0};
  DAR_DArray   keys   = {0};
  DAR_DArray   values = {0};

  srand(time(NULL) + clock());

  for(uint16_t key_size = 1; key_size <= 8; key_size *= 2) {
    for(size_t value_size = 1; value_size <= 8; value_size *= 2) {
      EXPECT_EQ(&r, OK, HT_create(&table, key_size, value_size));
      EXPECT_EQ(&r, OK, DAR_create_in_place(&keys, key_size));
      EXPECT_EQ(&r, OK, DAR_create_in_place(&values, value_size));
      if(HAS_FAILED(&r)) return r;

      void * key   = malloc(key_size);
      void * value = malloc(value_size);
      EXPECT_NE(&r, NULL, key);
      EXPECT_NE(&r, NULL, value);
      if(HAS_FAILED(&r)) return r;

      for(size_t iteration = 0; iteration < 1000; iteration++) {
        if((iteration % 256) == 0) {
          printf("key_size=%u, value_size=%zu, iteration=%zu, count=%zu, tombstone_count=%zu\n",
                 key_size,
                 value_size,
                 iteration,
                 table.count,
                 table.tombstone_count);
        }

        const bool do_remove = ((table.count > 0) && (rand() < (RAND_MAX / 4)));

        if(do_remove) {
          const uint32_t item_idx = (rand() % keys.size);

          EXPECT_EQ(&r, OK, HT_remove(&table, DAR_get(&keys, item_idx)));
          if(HAS_FAILED(&r)) return r;

          memcpy(DAR_get(&keys, item_idx), DAR_last(&keys), key_size);
          EXPECT_EQ(&r, OK, DAR_resize(&keys, keys.size - 1));
          memcpy(DAR_get(&values, item_idx), DAR_last(&values), value_size);
          EXPECT_EQ(&r, OK, DAR_resize(&values, values.size - 1));
        } else {
          make_rand_val(key, key_size);
          make_rand_val(value, value_size);

          EXPECT_EQ(&r, OK, HT_set(&table, key, value));
          if(HAS_FAILED(&r)) return r;

          uint32_t item_idx = 0;
          if(SPN_find(DAR_to_span(&keys), key, &item_idx) == OK) {
            // item already existed, overwrite its value
            memcpy(DAR_get(&values, item_idx), value, value_size);
          } else { // item did not yet exist, add to local arrays
            EXPECT_EQ(&r, OK, DAR_push_back(&keys, key));
            EXPECT_EQ(&r, OK, DAR_push_back(&values, value));
          }
        }

        // check all items in table
        for(uint32_t item_idx = 0; item_idx < keys.size; item_idx++) {
          const void *   expected_value  = DAR_get(&values, item_idx);
          const void *   retrieved_value = NULL;
          const STAT_Val st = HT_get(&table, DAR_get(&keys, item_idx), &retrieved_value);
          EXPECT_EQ(&r, OK, st);
          EXPECT_NE(&r, NULL, retrieved_value);
          if(HAS_FAILED(&r)) return r;

          EXPECT_ARREQ(&r, uint8_t, expected_value, retrieved_value, value_size);

          if(HAS_FAILED(&r)) return r;
        }

        if(HAS_FAILED(&r)) return r;
      }

      free(key);
      free(value);
      EXPECT_EQ(&r, OK, HT_destroy(&table));
      EXPECT_EQ(&r, OK, DAR_destroy_in_place(&keys));
      EXPECT_EQ(&r, OK, DAR_destroy_in_place(&values));
      if(HAS_FAILED(&r)) return r;
    }
  }

  return r;
}

static Result tst_set_get(void * env) {
  Result         r     = PASS;
  HT_HashTable * table = env;

  const double max_load_factor = 0.75; // defined inside hashtable.c

  for(int i = 1; i <= 1000; i++) {
    const int     key   = i;
    const int64_t value = ((int64_t)i * 2);

    EXPECT_EQ(&r, OK, HT_set(table, &key, &value));
    EXPECT_EQ(&r, (size_t)i, table->count);
    EXPECT_TRUE(&r, table->count < (HT_get_capacity(table) * max_load_factor));

    const int64_t * value_p = NULL;
    EXPECT_EQ(&r, OK, HT_get(table, &key, ((const void **)&value_p)));

    EXPECT_NE(&r, NULL, value_p);
    if(HAS_FAILED(&r)) return r;

    EXPECT_EQ(&r, value, *value_p);
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_remove(void * env) {
  Result         r     = PASS;
  HT_HashTable * table = env;

  size_t expect_count = 0;

  for(int i = 1; i <= 1000; i++) {
    const int     key   = i;
    const int64_t value = ((int64_t)i * 2);

    EXPECT_EQ(&r, OK, HT_set(table, &key, &value));
    expect_count++;

    EXPECT_EQ(&r, expect_count, table->count);
    if(HAS_FAILED(&r)) return r;
  }

  for(int i = 1; i <= 1000; i++) {
    const int key = i;

    EXPECT_EQ(&r, OK, HT_remove(table, &key));
    expect_count--;

    EXPECT_EQ(&r, expect_count, table->count);
    EXPECT_EQ(&r, STAT_OK_NOT_FOUND, HT_get(table, &key, NULL));
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

int main(void) {
  Test tests[] = {
      tst_create_destroy,
      tst_many_random_sets_gets_removes,
  };

  TestWithFixture tests_with_fixed_size_key_fixture[] = {
      tst_set_get,
      tst_remove,
  };

  const Result test_res = run_tests(tests, sizeof(tests) / sizeof(Test));
  const Result test_with_fixed_size_key_fixture_res =
      run_tests_with_fixture(tests_with_fixed_size_key_fixture,
                             sizeof(tests_with_fixed_size_key_fixture) / sizeof(TestWithFixture),
                             setup_fixed_size_key_fixture,
                             teardown);

  return ((test_res == PASS) && (test_with_fixed_size_key_fixture_res == PASS)) ? 0 : 1;
}

static Result setup_fixed_size_key_fixture(void ** env_p) {
  Result r = PASS;

  HT_HashTable * table = malloc(sizeof(HT_HashTable));
  EXPECT_NE(&r, NULL, table);
  EXPECT_NE(&r, NULL, env_p);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, HT_create(table, sizeof(int), sizeof(int64_t)));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, table->count);
  EXPECT_EQ(&r, sizeof(int), table->key_size);
  EXPECT_EQ(&r, sizeof(int64_t), table->value_size);
  EXPECT_NE(&r, NULL, table->store.data);
  if(HAS_FAILED(&r)) return r;

  *env_p = table;

  return r;
}

static Result teardown(void ** env_p) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env_p);
  if(HAS_FAILED(&r)) return r;

  HT_HashTable * table = *((HT_HashTable **)env_p);

  EXPECT_EQ(&r, OK, HT_destroy(table));
  if(HAS_FAILED(&r)) return r;

  free(table);

  return r;
}