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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stat.h"
#include "test_utils.h"

#include "bitdarray.h"

#include "span.h"

#define OK STAT_OK

static uint8_t make_byte(bool b_msb,
                         bool b6,
                         bool b5,
                         bool b4,
                         bool b3,
                         bool b2,
                         bool b1,
                         bool b_lsb) {
  return (b_msb << 7) | (b6 << 6) | (b5 << 5) | (b4 << 4) | (b3 << 3) | (b2 << 2) | (b1 << 1) |
         b_lsb;
}

static Result setup(void ** env_p);
static Result teardown(void ** env_p);

static Result tst_create_destroy(void) {
  Result r = PASS;

  BDAR_BitDArray arr = {0};
  EXPECT_EQ(&r, OK, BDAR_create(&arr));
  EXPECT_NE(&r, NULL, arr.data);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, arr.size);
  EXPECT_NE(&r, 0, arr.capacity_in_bytes);

  EXPECT_EQ(&r, OK, BDAR_destroy(&arr));
  EXPECT_EQ(&r, NULL, arr.data);

  return r;
}

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);
  if(HAS_FAILED(&r)) return r;

  BDAR_BitDArray * arr = env;

  EXPECT_NE(&r, NULL, arr->data);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, arr->size);
  EXPECT_NE(&r, 0, arr->capacity_in_bytes);

  return r;
}

static Result tst_reserve(void * env) {
  Result           r   = PASS;
  BDAR_BitDArray * arr = env;

  EXPECT_OK(&r, BDAR_reserve(arr, 10));
  EXPECT_GE(&r, arr->capacity_in_bytes, (10 / 8) + 1);
  EXPECT_EQ(&r, 0, arr->size);
  if(HAS_FAILED(&r)) printf("arr->capacity_in_bytes == %zu\n", arr->capacity_in_bytes);

  EXPECT_OK(&r, BDAR_reserve(arr, 100));
  EXPECT_GE(&r, arr->capacity_in_bytes, (100 / 8) + 1);
  EXPECT_EQ(&r, 0, arr->size);
  if(HAS_FAILED(&r)) printf("arr->capacity_in_bytes == %zu\n", arr->capacity_in_bytes);

  EXPECT_OK(&r, BDAR_reserve(arr, 1000));
  EXPECT_GE(&r, arr->capacity_in_bytes, (1000 / 8) + 1);
  EXPECT_EQ(&r, 0, arr->size);
  if(HAS_FAILED(&r)) printf("arr->capacity_in_bytes == %zu\n", arr->capacity_in_bytes);

  return r;
}

static Result tst_push_pop_back(void * env) {
  Result           r   = PASS;
  BDAR_BitDArray * arr = env;

  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_EQ(&r, 1, arr->size);
  EXPECT_EQ(&r, 1, (arr->data[0] & ((1 << 1) - 1)));

  EXPECT_OK(&r, BDAR_push_back(arr, false));
  EXPECT_EQ(&r, 2, arr->size);
  EXPECT_EQ(&r, 1, (arr->data[0] & ((1 << 2) - 1)));

  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_EQ(&r, 3, arr->size);
  EXPECT_EQ(&r, (1 << 2) | 1, (arr->data[0] & ((1 << 3) - 1)));

  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_EQ(&r, 4, arr->size);
  EXPECT_EQ(&r, (1 << 3) | (1 << 2) | 1, (arr->data[0] & ((1 << 4) - 1)));

  EXPECT_OK(&r, BDAR_push_back(arr, false));
  EXPECT_EQ(&r, 5, arr->size);
  EXPECT_EQ(&r, (1 << 3) | (1 << 2) | 1, (arr->data[0] & ((1 << 5) - 1)));

  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_EQ(&r, 6, arr->size);
  EXPECT_EQ(&r, (1 << 5) | (1 << 3) | (1 << 2) | 1, (arr->data[0] & ((1 << 6) - 1)));

  EXPECT_OK(&r, BDAR_push_back(arr, false));
  EXPECT_EQ(&r, 7, arr->size);
  EXPECT_EQ(&r, (1 << 5) | (1 << 3) | (1 << 2) | 1, (arr->data[0] & ((1 << 7) - 1)));

  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_EQ(&r, 8, arr->size);
  EXPECT_EQ(&r, (1 << 7) | (1 << 5) | (1 << 3) | (1 << 2) | 1, arr->data[0]);

  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_EQ(&r, 9, arr->size);
  EXPECT_EQ(&r, (1 << 7) | (1 << 5) | (1 << 3) | (1 << 2) | 1, arr->data[0]);
  EXPECT_EQ(&r, 1, arr->data[1] & 1);

  EXPECT_OK(&r, BDAR_pop_back(arr));
  EXPECT_EQ(&r, 8, arr->size);
  EXPECT_EQ(&r, (1 << 7) | (1 << 5) | (1 << 3) | (1 << 2) | 1, arr->data[0]);
  EXPECT_OK(&r, BDAR_pop_back(arr));
  EXPECT_EQ(&r, 7, arr->size);
  EXPECT_EQ(&r, (1 << 5) | (1 << 3) | (1 << 2) | 1, (arr->data[0] & ((1 << 7) - 1)));
  EXPECT_OK(&r, BDAR_pop_back(arr));
  EXPECT_EQ(&r, 6, arr->size);
  EXPECT_EQ(&r, (1 << 5) | (1 << 3) | (1 << 2) | 1, (arr->data[0] & ((1 << 6) - 1)));

  return r;
}

static Result tst_create_from_bool_arr(void) {
  Result         r   = PASS;
  BDAR_BitDArray arr = {0};

  const bool bool_arr[] = {
      // NOTE backwards from what you might expect, [0] is least significant bit!
      // clang-format off
    true, false, false, true, false, true, false, true,   // 1010 1001
    false, false, true, true, false, false, true, false,  // 0100 1100
    true, true, false, true, false, // 01011
      // clang-format on
  };
  const size_t size = (sizeof(bool_arr) / sizeof(bool_arr[0]));

  EXPECT_OK(&r, BDAR_create_from_bool_arr(&arr, bool_arr, size));
  EXPECT_EQ(&r, size, arr.size);

  EXPECT_EQ(&r, make_byte(1, 0, 1, 0, 1, 0, 0, 1), arr.data[0]);
  EXPECT_EQ(&r, make_byte(0, 1, 0, 0, 1, 1, 0, 0), arr.data[1]);
  EXPECT_EQ(&r, make_byte(0, 0, 0, 0, 1, 0, 1, 1), (arr.data[2] & ((1 << 5) - 1)));

  EXPECT_OK(&r, BDAR_destroy(&arr));

  return r;
}

static Result tst_fill(void * env) {
  Result           r   = PASS;
  BDAR_BitDArray * arr = env;

  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_OK(&r, BDAR_push_back(arr, false));
  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_OK(&r, BDAR_push_back(arr, false));
  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_OK(&r, BDAR_push_back(arr, false));
  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_OK(&r, BDAR_push_back(arr, true));
  EXPECT_OK(&r, BDAR_push_back(arr, false));
  EXPECT_OK(&r, BDAR_push_back(arr, false));
  EXPECT_OK(&r, BDAR_push_back(arr, false));
  EXPECT_EQ(&r, 13, arr->size);

  EXPECT_OK(&r, BDAR_fill(arr, false));
  EXPECT_EQ(&r, 0, arr->data[0]);
  EXPECT_EQ(&r, 0, (arr->data[0] & ((1 << 5) - 1)));

  EXPECT_OK(&r, BDAR_fill(arr, true));
  EXPECT_EQ(&r, 0xff, arr->data[0]);
  EXPECT_EQ(&r, 0xff & ((1 << 5) - 1), (arr->data[0] & ((1 << 5) - 1)));

  return r;
}

static Result tst_fill_range(void * env) {
  Result           r   = PASS;
  BDAR_BitDArray * arr = env;

  EXPECT_OK(&r, BDAR_resize(arr, 32));
  EXPECT_EQ(&r, 32, arr->size);

  // all zeroes
  EXPECT_OK(&r, BDAR_fill_range(arr, 0, 32, false));
  EXPECT_ARREQ(&r, uint8_t, ((uint8_t[]){0x00, 0x00, 0x00, 0x00}), arr->data, 4);

  // set bytes 0xff, 0x00, 0xff, 0x00
  EXPECT_OK(&r, BDAR_fill_range(arr, 0, 8, true));
  EXPECT_OK(&r, BDAR_fill_range(arr, 8, 8, false));
  EXPECT_OK(&r, BDAR_fill_range(arr, 16, 8, true));
  EXPECT_OK(&r, BDAR_fill_range(arr, 24, 8, false));
  EXPECT_ARREQ(&r, uint8_t, ((uint8_t[]){0xff, 0x00, 0xff, 0x00}), arr->data, 4);

  // set bytes 0xff, 0xff, 0x00, 0x00
  EXPECT_OK(&r, BDAR_fill_range(arr, 0, 16, true));
  EXPECT_OK(&r, BDAR_fill_range(arr, 16, 16, false));
  EXPECT_ARREQ(&r, uint8_t, ((uint8_t[]){0xff, 0xff, 0x00, 0x00}), arr->data, 4);

  // set bytes 0x0f, 0xf0, 0xf0, 0x0f
  EXPECT_OK(&r, BDAR_fill_range(arr, 4, 8, false));
  EXPECT_OK(&r, BDAR_fill_range(arr, 20, 8, true));
  EXPECT_ARREQ(&r, uint8_t, ((uint8_t[]){0x0f, 0xf0, 0xf0, 0x0f}), arr->data, 4);

  // set bytes 0x00, 0xf0, 0xff, 0x0f
  EXPECT_OK(&r, BDAR_fill_range(arr, 0, 32, false));
  EXPECT_ARREQ(&r, uint8_t, ((uint8_t[]){0x00, 0x00, 0x00, 0x00}), arr->data, 4);
  EXPECT_OK(&r, BDAR_fill_range(arr, 12, 16, true));
  EXPECT_ARREQ(&r, uint8_t, ((uint8_t[]){0x00, 0xf0, 0xff, 0x0f}), arr->data, 4);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_create_destroy,
      tst_create_from_bool_arr,
  };

  TestWithFixture tests_with_fixture[] = {
      tst_fixture,
      tst_reserve,
      tst_push_pop_back,
      tst_fill,
      tst_fill_range,
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
  Result            r     = PASS;
  BDAR_BitDArray ** arr_p = (BDAR_BitDArray **)env_p;

  // use both time and clock so we get a different seed even if we call this many times per second
  srand(time(NULL) + clock());

  EXPECT_NE(&r, NULL, arr_p);
  if(HAS_FAILED(&r)) return r;

  *arr_p = malloc(sizeof(BDAR_BitDArray));
  EXPECT_NE(&r, NULL, *arr_p);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, BDAR_create(*arr_p));

  return r;
}

static Result teardown(void ** env_p) {
  Result            r     = PASS;
  BDAR_BitDArray ** arr_p = (BDAR_BitDArray **)env_p;

  EXPECT_NE(&r, NULL, arr_p);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, BDAR_destroy(*arr_p));

  free(*arr_p);
  *arr_p = NULL;

  return r;
}