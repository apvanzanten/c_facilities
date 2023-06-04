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

static Result setup(void ** env_p);
static Result teardown(void ** env_p);

static Result tst_create_destroy(void) {
  Result       r     = PASS;
  HT_HashTable table = {0};

  EXPECT_EQ(&r, OK, HT_create(&table));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, table.count);
  EXPECT_EQ(&r, 0, table.tombstone_count);
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

  for(size_t key_size = 1; key_size <= 8; key_size *= 2) {
    for(size_t value_size = 1; value_size <= 8; value_size *= 2) {
      EXPECT_EQ(&r, OK, HT_create(&table));
      EXPECT_EQ(&r, OK, DAR_create(&keys, key_size));
      EXPECT_EQ(&r, OK, DAR_create(&values, value_size));
      if(HAS_FAILED(&r)) return r;

      SPN_Span key   = {.begin = NULL, .element_size = 1, .len = key_size};
      SPN_Span value = {.begin = NULL, .element_size = 1, .len = value_size};

      void * tmp_key_data = malloc(key_size);

      for(size_t iteration = 0; iteration < 500; iteration++) {
        if((iteration % 256) == 0) {
          printf("key_size=%zu, value_size=%zu, iteration=%zu, count=%zu, tombstone_count=%zu\n",
                 key_size,
                 value_size,
                 iteration,
                 table.count,
                 table.tombstone_count);
        }

        const bool do_remove = ((table.count > 0) && (rand() < (RAND_MAX / 4)));

        if(do_remove) {
          const size_t item_idx = (rand() % keys.size);
          key.begin             = DAR_get(&keys, item_idx);

          EXPECT_EQ(&r, OK, HT_remove(&table, key));
          if(HAS_FAILED(&r)) return r;

          memcpy(DAR_get(&keys, item_idx), DAR_last(&keys), key_size);
          EXPECT_EQ(&r, OK, DAR_resize(&keys, keys.size - 1));
          memcpy(DAR_get(&values, item_idx), DAR_last(&values), value_size);
          EXPECT_EQ(&r, OK, DAR_resize(&values, values.size - 1));
        } else {
          make_rand_val(tmp_key_data, key_size);

          size_t     item_idx = 0;
          const bool is_pre_existing_key =
              (SPN_find(DAR_to_span(&keys), tmp_key_data, &item_idx) == OK);

          if(!is_pre_existing_key) {
            EXPECT_EQ(&r, OK, DAR_push_back(&keys, tmp_key_data));
            EXPECT_EQ(&r, OK, DAR_resize(&values, keys.size));
            item_idx = (keys.size - 1);
          }

          key.begin   = DAR_get(&keys, item_idx);
          value.begin = DAR_get(&values, item_idx);

          make_rand_val((void *)value.begin, value_size);

          EXPECT_EQ(&r, OK, HT_set(&table, key, value));
          if(HAS_FAILED(&r)) return r;
        }

        // check all items in table
        for(size_t item_idx = 0; item_idx < keys.size; item_idx++) {
          key.begin = DAR_get(&keys, item_idx);

          EXPECT_TRUE(&r, HT_contains(&table, key));
          if(HAS_FAILED(&r)) return r;

          SPN_Span retrieved_value = {0};
          EXPECT_EQ(&r, OK, HT_get(&table, key, &retrieved_value));
          EXPECT_FALSE(&r, SPN_is_empty(retrieved_value));
          if(HAS_FAILED(&r)) return r;

          const void * expected_value = DAR_get(&values, item_idx);
          EXPECT_ARREQ(&r, uint8_t, expected_value, retrieved_value.begin, value_size);

          if(HAS_FAILED(&r)) return r;
        }

        if(HAS_FAILED(&r)) return r;
      }

      free(tmp_key_data);

      EXPECT_EQ(&r, OK, HT_destroy(&table));
      EXPECT_EQ(&r, OK, DAR_destroy(&keys));
      EXPECT_EQ(&r, OK, DAR_destroy(&values));
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
    const int      key        = i;
    const int64_t  value      = ((int64_t)i * 2);
    const SPN_Span key_span   = {.begin = &key, .element_size = 1, .len = sizeof(key)};
    const SPN_Span value_span = {.begin = &value, .element_size = 1, .len = sizeof(value)};

    EXPECT_EQ(&r, OK, HT_set(table, key_span, value_span));
    EXPECT_EQ(&r, (size_t)i, table->count);
    EXPECT_TRUE(&r, table->count < (HT_get_capacity(table) * max_load_factor));

    SPN_Span retrieved_value_span = {0};
    EXPECT_EQ(&r, OK, HT_get(table, key_span, &retrieved_value_span));

    EXPECT_FALSE(&r, SPN_is_empty(retrieved_value_span));
    if(HAS_FAILED(&r)) return r;

    EXPECT_TRUE(&r, SPN_equals(value_span, retrieved_value_span));
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_get_set_strings(void * env) {
  Result         r     = PASS;
  HT_HashTable * table = env;

  EXPECT_EQ(&r, OK, HT_set(table, SPN_from_cstr(".^."), SPN_from_cstr("happy upside-down")));
  EXPECT_EQ(&r, OK, HT_set(table, SPN_from_cstr(".v."), SPN_from_cstr("sad upside-down")));
  EXPECT_EQ(&r, OK, HT_set(table, SPN_from_cstr("O_O"), SPN_from_cstr("shocked")));
  EXPECT_EQ(&r, OK, HT_set(table, SPN_from_cstr("-_-"), SPN_from_cstr("tired")));
  EXPECT_EQ(&r, OK, HT_set(table, SPN_from_cstr("d._.b"), SPN_from_cstr("listening to music")));
  EXPECT_EQ(&r, OK, HT_set(table, SPN_from_cstr("T_T"), SPN_from_cstr("crying twin waterfalls")));

  SPN_Span val = {0};
  EXPECT_EQ(&r, OK, HT_get(table, SPN_from_cstr(".^."), &val));
  EXPECT_TRUE(&r, SPN_equals(SPN_from_cstr("happy upside-down"), val));
  EXPECT_EQ(&r, OK, HT_get(table, SPN_from_cstr(".v."), &val));
  EXPECT_TRUE(&r, SPN_equals(SPN_from_cstr("sad upside-down"), val));
  EXPECT_EQ(&r, OK, HT_get(table, SPN_from_cstr("O_O"), &val));
  EXPECT_TRUE(&r, SPN_equals(SPN_from_cstr("shocked"), val));
  EXPECT_EQ(&r, OK, HT_get(table, SPN_from_cstr("-_-"), &val));
  EXPECT_TRUE(&r, SPN_equals(SPN_from_cstr("tired"), val));
  EXPECT_EQ(&r, OK, HT_get(table, SPN_from_cstr("d._.b"), &val));
  EXPECT_TRUE(&r, SPN_equals(SPN_from_cstr("listening to music"), val));
  EXPECT_EQ(&r, OK, HT_get(table, SPN_from_cstr("T_T"), &val));
  EXPECT_TRUE(&r, SPN_equals(SPN_from_cstr("crying twin waterfalls"), val));

  return r;
}

static Result tst_set_get_empty_values(void * env) {
  Result         r     = PASS;
  HT_HashTable * table = env;

  for(int i = 1; i <= 1000; i++) {
    const int      key      = i;
    const SPN_Span key_span = {.begin = &key, .element_size = 1, .len = sizeof(key)};

    EXPECT_EQ(&r, OK, HT_set(table, key_span, (SPN_Span){0}));
    EXPECT_EQ(&r, (size_t)i, table->count);

    EXPECT_TRUE(&r, HT_contains(table, key_span));
    if(HAS_FAILED(&r)) return r;

    SPN_Span retrieved_value_span = {0};
    EXPECT_EQ(&r, OK, HT_get(table, key_span, &retrieved_value_span));

    EXPECT_TRUE(&r, SPN_is_empty(retrieved_value_span));
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_remove(void * env) {
  Result         r     = PASS;
  HT_HashTable * table = env;

  size_t expect_count = 0;

  for(int i = 1; i <= 1000; i++) {
    const int      key        = i;
    const int64_t  value      = ((int64_t)i * 2);
    const SPN_Span key_span   = {.begin = &key, .element_size = 1, .len = sizeof(key)};
    const SPN_Span value_span = {.begin = &value, .element_size = 1, .len = sizeof(value)};

    EXPECT_EQ(&r, OK, HT_set(table, key_span, value_span));
    expect_count++;

    EXPECT_EQ(&r, expect_count, table->count);
    if(HAS_FAILED(&r)) return r;
  }

  for(int i = 1; i <= 1000; i++) {
    const int      key      = i;
    const SPN_Span key_span = {.begin = &key, .element_size = 1, .len = sizeof(key)};

    EXPECT_EQ(&r, OK, HT_remove(table, key_span));
    expect_count--;

    EXPECT_EQ(&r, expect_count, table->count);
    EXPECT_EQ(&r, STAT_OK_NOT_FOUND, HT_get(table, key_span, NULL));
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

int main(void) {
  Test tests[] = {
      tst_create_destroy,
      tst_many_random_sets_gets_removes,
  };

  TestWithFixture tests_with_fixture[] = {
      tst_set_get,
      tst_get_set_strings,
      tst_set_get_empty_values,
      tst_remove,
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

  HT_HashTable * table = malloc(sizeof(HT_HashTable));
  EXPECT_NE(&r, NULL, table);
  EXPECT_NE(&r, NULL, env_p);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, HT_create(table));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, table->count);
  EXPECT_EQ(&r, 0, table->tombstone_count);
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