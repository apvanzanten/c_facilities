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

#include "stat.h"
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

static Result tst_expect_ok_nok(void) {
  Result r = PASS;

  const STAT_Val ok          = STAT_OK;
  const STAT_Val ok_finished = STAT_OK_FINISHED;
  const STAT_Val err_alloc   = STAT_ERR_ALLOC;
  const STAT_Val err_fatal   = STAT_ERR_FATAL;
  const STAT_Val wrn_runtime = STAT_WRN_RUNTIME;

  // pointer indirection to shut up compiler/cppcheck about conditions always being true/false
  const STAT_Val * p_ok          = &ok;
  const STAT_Val * p_ok_finished = &ok_finished;
  const STAT_Val * p_err_alloc   = &err_alloc;
  const STAT_Val * p_err_fatal   = &err_fatal;
  const STAT_Val * p_wrn_runtime = &wrn_runtime;

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
  EXPECT_OK(&r, *p_wrn_runtime);
  if(r != FAIL) return FAIL;
  r = PASS;

  EXPECT_NOK(&r, *p_err_alloc);
  if(r != PASS) return FAIL;
  EXPECT_NOK(&r, *p_err_fatal);
  if(r != PASS) return FAIL;
  EXPECT_NOK(&r, *p_wrn_runtime);
  if(r != PASS) return FAIL;

  EXPECT_NOK(&r, *p_ok);
  if(r != FAIL) return FAIL;
  r = PASS;
  EXPECT_NOK(&r, *p_ok_finished);
  if(r != FAIL) return FAIL;
  r = PASS;

  return r;
}

static Result tst_expect_pass(void) {
  Result r = PASS;

  const Result pass = PASS;
  const Result fail = fail;

  // pointer indirection to shut up compiler/cppcheck about conditions always being true/false
  const Result * p_pass = &pass;
  const Result * p_fail = &fail;

  EXPECT_PASS(&r, *p_pass);
  if(r != PASS) return FAIL;

  EXPECT_PASS(&r, *p_fail);
  if(r != FAIL) return FAIL;
  r = PASS;

  return r;
}

static size_t g_passing_test_counter = 0;
static size_t g_failing_test_counter = 0;

static Result passing_test(void) {
  g_passing_test_counter++;
  return PASS;
}
static Result failing_test(void) {
  g_failing_test_counter++;
  return FAIL;
}

static Result tst_run_tests(void) {
  Result r = PASS;

  g_passing_test_counter = 0;
  g_failing_test_counter = 0;
  EXPECT_EQ(&r, PASS, run_tests((Test[]){passing_test, passing_test}, 2));
  EXPECT_EQ(&r, 2, g_passing_test_counter);
  EXPECT_EQ(&r, 0, g_failing_test_counter);

  g_passing_test_counter = 0;
  g_failing_test_counter = 0;
  EXPECT_EQ(&r,
            FAIL,
            run_tests(
                (Test[]){
                    passing_test,
                    passing_test,
                    failing_test,
                    failing_test,
                    passing_test,
                },
                5));
  EXPECT_EQ(&r, 3, g_passing_test_counter);
  EXPECT_EQ(&r, 2, g_failing_test_counter);

  // with --stop-on-failure
  g_passing_test_counter = 0;
  g_failing_test_counter = 0;
  EXPECT_EQ(&r,
            FAIL,
            run_tests_with_args(
                (Test[]){
                    passing_test,
                    passing_test,
                    failing_test, // will stop executing after this one
                    failing_test,
                    passing_test,
                },
                5,
                2,
                (const char *[]){__func__, "--stop-on-failure"}));
  EXPECT_EQ(&r, 2, g_passing_test_counter);
  EXPECT_EQ(&r, 1, g_failing_test_counter);

  return r;
}

typedef enum Flag {
  FLAG_NONE = 0,
  FLAG_BLUE,
  FLAG_RED,
} Flag;

static const char * flag_to_str(Flag flag) {
  switch(flag) {
  case FLAG_NONE: return "FLAG_NONE";
  case FLAG_BLUE: return "FLAG_BLUE";
  case FLAG_RED: return "FLAG_RED";
  }
  return "UNKNOWN";
}

typedef struct Fixture {
  Flag flag;
} Fixture;

#define FLAG_ARRAY_SIZE 16

static Flag   g_input_flag_array[FLAG_ARRAY_SIZE]  = {0};
static Flag   g_output_flag_array[FLAG_ARRAY_SIZE] = {0};
static size_t g_input_flag_idx                     = 0;
static size_t g_output_flag_idx                    = 0;

static size_t g_fixture_setup_counter    = 0;
static size_t g_fixture_teardown_counter = 0;
static size_t g_fixture_test_counter     = 0;
static size_t g_fail_setup_at_count      = FLAG_ARRAY_SIZE;
static size_t g_fail_test_at_count       = FLAG_ARRAY_SIZE;
static size_t g_fail_teardown_at_count   = FLAG_ARRAY_SIZE;

static Result setup(void ** fixture_pp) {
  Result r = PASS;

  g_fixture_setup_counter++;

  // NOTE fail before allocate
  if(g_fixture_setup_counter == g_fail_setup_at_count) {
    FAIL_WITH_MSG(&r, "failing setup at count %zu", g_fixture_setup_counter);
    return r;
  }

  EXPECT_LT(&r, g_input_flag_idx, FLAG_ARRAY_SIZE);
  EXPECT_LT(&r, g_output_flag_idx, FLAG_ARRAY_SIZE);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, FLAG_NONE, g_output_flag_array[g_output_flag_idx]);
  if(HAS_FAILED(&r)) return r;

  EXPECT_NE(&r, NULL, fixture_pp);
  if(HAS_FAILED(&r)) return r;

  Fixture * fixture_p = malloc(sizeof(Fixture));
  EXPECT_NE(&r, NULL, fixture_p);
  if(HAS_FAILED(&r)) return r;

  *fixture_p = (Fixture){.flag = FLAG_NONE};

  *fixture_pp = fixture_p;

  return r;
}

static Result teardown(void ** fixture_pp) {
  Result r = PASS;

  g_fixture_teardown_counter++;

  EXPECT_NE(&r, NULL, fixture_pp);
  if(HAS_FAILED(&r)) return r;

  Fixture * fixture_p = *fixture_pp;
  EXPECT_NE(&r, NULL, fixture_p);
  if(HAS_FAILED(&r)) return r;

  printf("%s #%zu:", __func__, g_fixture_teardown_counter);

  if(fixture_p->flag != FLAG_NONE) {
    g_output_flag_array[g_output_flag_idx] = fixture_p->flag;
    printf(" set output %zu to %s\n", g_output_flag_idx, flag_to_str(fixture_p->flag));
    g_output_flag_idx++;
  } else {
    printf(" did not set output, as fixture flag is FLAG_NONE");
  }

  free(fixture_p);

  // NOTE fail last, so we've already freed
  if(g_fixture_teardown_counter == g_fail_teardown_at_count) {
    FAIL_WITH_MSG(&r, "failing teardown at count %zu", g_fixture_teardown_counter);
  }

  return r;
}

static Result test_with_fixture_impl(void * fixture_void_p, Flag maybe_overrule) {
  Result r = PASS;

  g_fixture_test_counter++;

  if(g_fixture_test_counter == g_fail_test_at_count) {
    FAIL_WITH_MSG(&r, "failing test at count %zu", g_fixture_test_counter);
    return r;
  }

  Fixture * fixture_p = fixture_void_p;
  EXPECT_NE(&r, NULL, fixture_p);
  if(HAS_FAILED(&r)) return r;

  printf("%s #%zu:", __func__, g_fixture_test_counter);

  if(maybe_overrule == FLAG_NONE) {
    // no overrule, take from input
    printf(" from input at idx: %zu,", g_input_flag_idx);
    fixture_p->flag = g_input_flag_array[g_input_flag_idx++];
  } else {
    fixture_p->flag = maybe_overrule;
  }
  printf("set flag to %s\n", flag_to_str(fixture_p->flag));

  return r;
}

static Result test_with_fixture_reading_input(void * fixture_void_p) {
  return test_with_fixture_impl(fixture_void_p, FLAG_NONE);
}

static Result test_with_fixture_setting_to_blue(void * fixture_void_p) {
  return test_with_fixture_impl(fixture_void_p, FLAG_BLUE);
}

static Result test_with_fixture_setting_to_red(void * fixture_void_p) {
  return test_with_fixture_impl(fixture_void_p, FLAG_RED);
}

static Result set_flags_and_reset_counters(const Flag * input_flags) {
  Result r = PASS;
  EXPECT_NE(&r, NULL, input_flags);
  if(HAS_FAILED(&r)) return r;

  bool is_all_set = false;
  for(size_t i = 0; i < FLAG_ARRAY_SIZE; i++) {
    if(!is_all_set && input_flags[i] == FLAG_NONE) is_all_set = true;
    g_input_flag_array[i] = (is_all_set ? FLAG_NONE : input_flags[i]);

    g_output_flag_array[i] = FLAG_NONE;
  }
  EXPECT_TRUE(&r, is_all_set);

  g_fixture_setup_counter    = 0;
  g_fixture_teardown_counter = 0;
  g_fixture_test_counter     = 0;
  g_input_flag_idx           = 0;
  g_output_flag_idx          = 0;
  g_fail_setup_at_count      = FLAG_ARRAY_SIZE;
  g_fail_test_at_count       = FLAG_ARRAY_SIZE;
  g_fail_teardown_at_count   = FLAG_ARRAY_SIZE;

  return r;
}

static Result check_flags(const Flag * expect_flags) {
  Result r = PASS;
  EXPECT_NE(&r, NULL, expect_flags);
  if(HAS_FAILED(&r)) return r;

  bool is_all_checked = false;
  for(size_t i = 0; i < FLAG_ARRAY_SIZE; i++) {
    EXPECT_EQ(&r, expect_flags[i], g_output_flag_array[i]);
    if(HAS_FAILED(&r)) {
      printf("%s: fail check on flag %zu; expected: %s, actual: %s\n",
             __func__,
             i,
             flag_to_str(expect_flags[i]),
             flag_to_str(g_output_flag_array[i]));
      break;
    }
    if(expect_flags[i] == FLAG_NONE) {
      is_all_checked = true;
      break;
    }
  }
  EXPECT_TRUE(&r, is_all_checked);

  return r;
}

static Result tst_run_tests_with_fixture_pass(void) {
  Result r = PASS;
  printf("== %s starting ==\n", __func__);

  EXPECT_PASS(&r,
              set_flags_and_reset_counters((Flag[]){
                  FLAG_BLUE,
                  FLAG_RED,
                  FLAG_RED,
                  FLAG_NONE,
              }));
  EXPECT_PASS(&r,
              run_tests_with_fixture(
                  (TestWithFixture[]){
                      test_with_fixture_reading_input,
                      test_with_fixture_setting_to_blue,
                      test_with_fixture_reading_input,
                      test_with_fixture_setting_to_blue,
                      test_with_fixture_setting_to_red,
                      test_with_fixture_reading_input,
                  },
                  6,
                  setup,
                  teardown));
  EXPECT_EQ(&r, 6, g_fixture_setup_counter);
  EXPECT_EQ(&r, 6, g_fixture_test_counter);
  EXPECT_EQ(&r, 6, g_fixture_teardown_counter);
  EXPECT_PASS(&r,
              check_flags((Flag[]){
                  FLAG_BLUE,
                  FLAG_BLUE,
                  FLAG_RED,
                  FLAG_BLUE,
                  FLAG_RED,
                  FLAG_RED,
                  FLAG_NONE,
              }));

  return r;
}

static Result tst_run_tests_with_fixture_fail(void) {
  Result r = PASS;
  printf("== %s starting ==\n", __func__);

  EXPECT_PASS(&r,
              set_flags_and_reset_counters((Flag[]){
                  FLAG_BLUE,
                  FLAG_RED,
                  FLAG_RED,
                  FLAG_NONE,
              }));
  g_fail_setup_at_count    = 2;
  g_fail_teardown_at_count = 4;
  g_fail_test_at_count     = 5;
  EXPECT_EQ(&r,
            FAIL,
            run_tests_with_fixture(
                (TestWithFixture[]){
                    test_with_fixture_reading_input,
                    test_with_fixture_setting_to_blue, // fail setup
                    test_with_fixture_reading_input,
                    test_with_fixture_setting_to_blue, // fail teardown
                    test_with_fixture_setting_to_red,  // fail test
                    test_with_fixture_reading_input,
                },
                6,
                setup,
                teardown));
  EXPECT_EQ(&r, 6, g_fixture_setup_counter); // failed setup is still counted
  EXPECT_EQ(&r, 5, g_fixture_test_counter);  // 1 less due to failed setup; failed test is counted
  EXPECT_EQ(&r, 5, g_fixture_teardown_counter); // 1 less due to failed setup
  EXPECT_PASS(&r,
              check_flags((Flag[]){
                  FLAG_BLUE,
                  // FLAG_BLUE, <- fails setup, so test is not executed
                  FLAG_RED,
                  FLAG_BLUE, // <-- fails teardown, but not before saving output
                  // FLAG_RED, // <-- fails test, so no output
                  FLAG_RED,
                  FLAG_NONE,
              }));

  return r;
}

static Result tst_run_tests_with_fixture_and_args_fail(void) {
  Result r = PASS;
  printf("== %s starting ==\n", __func__);

  EXPECT_PASS(&r, set_flags_and_reset_counters((Flag[]){FLAG_NONE}));
  g_fail_setup_at_count = 2;
  EXPECT_EQ(&r,
            FAIL,
            run_tests_with_fixture_and_args(
                (TestWithFixture[]){
                    test_with_fixture_setting_to_red,
                    test_with_fixture_setting_to_blue, // fail setup
                    test_with_fixture_reading_input,   // no longer executed, due to stop-on-failure
                },
                3,
                setup,
                teardown,
                2,
                (const char *[]){"bla", "--stop-on-failure"}));
  EXPECT_EQ(&r, 2, g_fixture_setup_counter);    // failed setup is still counted
  EXPECT_EQ(&r, 1, g_fixture_test_counter);     // only 1 due to failed setup on 2
  EXPECT_EQ(&r, 1, g_fixture_teardown_counter); // only 1 due to failed setup on 2
  EXPECT_PASS(&r,
              check_flags((Flag[]){
                  FLAG_RED,
                  FLAG_NONE, // end because failure on setup #2
              }));

  EXPECT_PASS(&r, set_flags_and_reset_counters((Flag[]){FLAG_NONE}));
  g_fail_test_at_count = 3;
  EXPECT_EQ(&r,
            FAIL,
            run_tests_with_fixture_and_args(
                (TestWithFixture[]){
                    test_with_fixture_setting_to_red,
                    test_with_fixture_setting_to_blue,
                    test_with_fixture_setting_to_red, // fail test
                    test_with_fixture_reading_input,  // no longer executed, due to stop-on-failure
                },
                4,
                setup,
                teardown,
                2,
                (const char *[]){"bla", "--stop-on-failure"}));
  EXPECT_EQ(&r, 3, g_fixture_setup_counter);
  EXPECT_EQ(&r, 3, g_fixture_test_counter);     // failed test is still counted
  EXPECT_EQ(&r, 3, g_fixture_teardown_counter); // failed test still must be torn down
  EXPECT_PASS(&r,
              check_flags((Flag[]){
                  FLAG_RED,
                  FLAG_BLUE,
                  FLAG_NONE, // end because failure on test #3
              }));

  EXPECT_PASS(&r, set_flags_and_reset_counters((Flag[]){FLAG_NONE}));
  g_fail_teardown_at_count = 4;
  EXPECT_EQ(&r,
            FAIL,
            run_tests_with_fixture_and_args(
                (TestWithFixture[]){
                    test_with_fixture_setting_to_red,
                    test_with_fixture_setting_to_blue,
                    test_with_fixture_setting_to_red,
                    test_with_fixture_setting_to_blue, // fail teardown
                    test_with_fixture_reading_input,   // no longer executed, due to stop-on-failure
                },
                5,
                setup,
                teardown,
                2,
                (const char *[]){"bla", "--stop-on-failure"}));
  EXPECT_EQ(&r, 4, g_fixture_setup_counter);
  EXPECT_EQ(&r, 4, g_fixture_test_counter);
  EXPECT_EQ(&r, 4, g_fixture_teardown_counter); // failed teardown is still counted
  EXPECT_PASS(&r,
              check_flags((Flag[]){
                  FLAG_RED,
                  FLAG_BLUE,
                  FLAG_RED,
                  FLAG_BLUE, // failed teardown still stores output
                  FLAG_NONE, // end because failure on teardown #4
              }));

  return r;
}

int main(int argc, const char ** argv) {
  Test tests[] = {
      tst_expects_boolean,
      tst_expects_equality,
      tst_expects_comparison,
      tst_expects_float_equality,
      tst_expects_str,
      tst_expects_arr_int,
      tst_expects_arr_double,
      tst_has_failed,
      tst_expect_ok_nok,
      tst_expect_pass,
      tst_run_tests,
      tst_run_tests_with_fixture_pass,
      tst_run_tests_with_fixture_fail,
      tst_run_tests_with_fixture_and_args_fail,
  };

  return (run_tests_with_args(tests, sizeof(tests) / sizeof(Test), argc, argv) == PASS) ? 0 : 1;
}
