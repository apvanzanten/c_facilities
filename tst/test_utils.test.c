
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "test_utils.h"

static Result tst_expects_boolean(void) {
  Result r = PASS;

  bool truthy = true;
  bool falsey = false;

  // pointer indirection to shut up compiler/cppcheck about conditions always being true/false
  bool * truthy_p = &truthy;
  bool * falsey_p = &falsey;

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

int main(void) {
  Test tests[] = {
      tst_expects_boolean,
      tst_expects_equality,
      tst_expects_str,
      tst_expects_arr_int,
      tst_expects_arr_double,
      tst_has_failed,
  };

  return (run_tests(tests, sizeof(tests) / sizeof(Test)) == PASS) ? 0 : 1;
}