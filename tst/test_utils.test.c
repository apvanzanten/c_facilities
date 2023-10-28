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

#include "test_utils.h"

static Result tst_expects_boolean(void) {
  Result r = PASS;

  const bool truthy = true;
  const bool falsey = false;

  // pointer indirection to shut up compiler/cppcheck about conditions always being true/false
  const bool * truthy_p = &truthy;
  const bool * falsey_p = &falsey;

  EXPECT_TRUE(&r, *truthy_p);
  if(r == FAIL) return FAIL;

  EXPECT_FALSE(&r, *truthy_p);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_FALSE(&r, *falsey_p);
  if(r == FAIL) return FAIL;

  EXPECT_TRUE(&r, *falsey_p);
  if(r != FAIL) return FAIL;
  r = PASS;

  return PASS;
}

static Result tst_expects_equality(void) {
  Result r = PASS;

  const int five = 5;

  // pointer indirection to shut up compiler/cppcheck about conditions always being true/false
  const int * five_p = &five;

  EXPECT_EQ(&r, *five_p, 5);
  if(r == FAIL) return FAIL;

  EXPECT_EQ(&r, *five_p, 4);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_NE(&r, *five_p, 4);
  if(r == FAIL) return FAIL;

  EXPECT_NE(&r, *five_p, 5);
  if(r != FAIL) return FAIL;
  r = PASS;

  return PASS;
}

static Result tst_expects_comparison(void) {
  Result r = PASS;

  const int five = 5;

  // pointer indirection to shut up compiler/cppcheck about conditions always being true/false
  const int * five_p = &five;

  EXPECT_LT(&r, *five_p, 6);
  if(r == FAIL) return FAIL;

  EXPECT_LT(&r, *five_p, 5);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_LT(&r, *five_p, 4);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_LE(&r, *five_p, 5);
  EXPECT_LE(&r, *five_p, 6);
  if(r == FAIL) return FAIL;

  EXPECT_LE(&r, *five_p, 4);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_LE(&r, *five_p, 0);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_GT(&r, *five_p, 4);
  if(r == FAIL) return FAIL;

  EXPECT_GT(&r, *five_p, 5);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_GT(&r, *five_p, 6);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_GE(&r, *five_p, 5);
  EXPECT_GE(&r, *five_p, 4);
  if(r == FAIL) return FAIL;

  EXPECT_GE(&r, *five_p, 6);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_GE(&r, *five_p, 100);
  if(r != FAIL) return FAIL;
  r = PASS;

  return PASS;
}

static Result tst_expects_float_equality(void) {
  Result r = PASS;

  const double five = 5.0;

  // pointer indirection to shut up compiler/cppcheck about conditions always being true/false
  const double * five_p = &five;

  EXPECT_FLOAT_EQ(&r, *five_p, 5.0, 0.01);
  EXPECT_FLOAT_EQ(&r, *five_p, 4.98, 0.02);
  EXPECT_FLOAT_EQ(&r, *five_p, 6.0, 1.0);
  if(r == FAIL) return FAIL;

  EXPECT_FLOAT_EQ(&r, *five_p, 5.05, 0.04);
  if(r != FAIL) return FAIL;
  r = PASS;
  EXPECT_FLOAT_EQ(&r, *five_p, 4.98, 0.01);
  if(r != FAIL) return FAIL;
  r = PASS;
  EXPECT_FLOAT_EQ(&r, *five_p, 2.00, 2.99);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_FLOAT_NE(&r, *five_p, 5.1, 0.01);
  EXPECT_FLOAT_NE(&r, *five_p, 4.97, 0.02);
  EXPECT_FLOAT_NE(&r, *five_p, 6.1, 1.0);
  if(r == FAIL) return FAIL;

  EXPECT_FLOAT_NE(&r, *five_p, 5.01, 0.01);
  if(r != FAIL) return FAIL;
  r = PASS;
  EXPECT_FLOAT_NE(&r, *five_p, 4.98, 0.02);
  if(r != FAIL) return FAIL;
  r = PASS;
  EXPECT_FLOAT_NE(&r, *five_p, 4.0, 1.0);
  if(r != FAIL) return FAIL;
  r = PASS;

  return r;
}

static Result tst_expects_str(void) {
  Result r = PASS;

  EXPECT_STREQ(&r, "milk and cereal", "milk and cereal");
  if(r == FAIL) return FAIL;

  EXPECT_STREQ(&r, "milk and cereal", "cereal and milk");
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_STREQ(&r, "milk and cereal", "milk");
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_STRNE(&r, "milk and cereal", "cereal and milk");
  if(r == FAIL) return FAIL;

  EXPECT_STRNE(&r, "milk and cereal", "milk and cereal");
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_STRNE(&r, "milk and cereal", "milk");
  if(r == FAIL) return FAIL;

  return PASS;
}

static Result tst_expects_arr_int(void) {
  Result r = PASS;

  const int arr_10_primes[10] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
  const int arr_10_fibs[10]   = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55};

  for(size_t i = 0; i < sizeof(arr_10_primes) / sizeof(int); i++) {
    EXPECT_ARREQ(&r, int, arr_10_primes, arr_10_primes, i);
    if(r == FAIL) return FAIL;

    EXPECT_ARRNE(&r, int, arr_10_primes, arr_10_primes, i);
    if(r != FAIL) return FAIL;
    r = PASS;
  }

  for(size_t i = 0; i < sizeof(arr_10_primes) / sizeof(int); i++) {
    EXPECT_ARRNE(&r, int, arr_10_primes, arr_10_fibs, i);
    if(r != FAIL) return FAIL;
    r = PASS;
  }

  return PASS;
}

static Result tst_expects_arr_double(void) {
  Result r = PASS;

  const double arr_some_reals[] = {
      1.2345,
      3.14,
      9.999,
      1.0,
      -23.3,
      1337.0,
      9000.1,
      42.0,
  };

  for(size_t i = 0; i < sizeof(arr_some_reals) / sizeof(double); i++) {
    EXPECT_ARREQ(&r, double, arr_some_reals, arr_some_reals, i);
    if(r == FAIL) return FAIL;

    EXPECT_ARRNE(&r, double, arr_some_reals, arr_some_reals, i);
    if(r != FAIL) return FAIL;
    r = PASS;
  }

  return PASS;
}

static Result tst_has_failed(void) {
  Result r = PASS;

  if(HAS_FAILED(&r)) return FAIL;

  r = FAIL;
  if(!HAS_FAILED(&r)) return FAIL;

  return PASS;
}

static Result tst_expect_ok(void) {
  Result r = PASS;

  const STAT_Val ok          = STAT_OK;
  const STAT_Val ok_finished = STAT_OK_FINISHED;
  const STAT_Val err_alloc   = STAT_ERR_ALLOC;
  const STAT_Val err_fatal   = STAT_ERR_FATAL;

  // pointer indirection to shut up compiler/cppcheck about conditions always being true/false
  const STAT_Val * p_ok          = &ok;
  const STAT_Val * p_ok_finished = &ok_finished;
  const STAT_Val * p_err_alloc   = &err_alloc;
  const STAT_Val * p_err_fatal   = &err_fatal;

  EXPECT_OK(&r, *p_ok);
  if(r != PASS) return FAIL;
  EXPECT_OK(&r, *p_ok_finished);
  if(r != PASS) return FAIL;

  EXPECT_OK(&r, *p_err_alloc);
  if(r != FAIL) return FAIL;
  r = PASS;
  EXPECT_OK(&r, *p_err_fatal);
  if(r != FAIL) return FAIL;
  r = PASS;

  return r;
}

int main(void) {
  Test tests[] = {
      tst_expects_boolean,
      tst_expects_equality,
      tst_expects_comparison,
      tst_expects_float_equality,
      tst_expects_str,
      tst_expects_arr_int,
      tst_expects_arr_double,
      tst_has_failed,
      tst_expect_ok,
  };

  return (run_tests(tests, sizeof(tests) / sizeof(Test)) == PASS) ? 0 : 1;
}