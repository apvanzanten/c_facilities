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

#include "log.h"
#include "test_utils.h"

#include "mock_utils.h"

static Result tst_init_verify_no_calls(void) {
  Result r = PASS;

  EXPECT_OK(&r, MOC_init_registry());

  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, MOC_destroy_registry());

  return r;
}

/* fake functions */
static void take_int_return_void(int i) { REGISTER_MADE_CALL(NULL, __func__, &i); }

static bool take_int_and_string_return_bool(int i, const char * str) {
  const MOC_Expectation * exp = NULL;

  if(!STAT_is_OK(REGISTER_MADE_CALL(&exp, __func__, &i, &str))) {
    LOG_STAT(STAT_ERR_INTERNAL, "failed to register made call");
    return false;
  }

  bool ret = true;
  if(exp != NULL) MOC_set_return_val_from_expectation(exp, &ret);

  return ret;
}

/* comp functions */
static int comp_int(const void * a, const void * b) {
  int v_a = *(const int *)a;
  int v_b = *(const int *)b;
  return (v_a == v_b) ? 0 : (v_a < v_b) ? -1 : 1;
}
static int comp_str(const void * a, const void * b) { return strcmp(a, b); }

/* set functions */
static void set_bool(void * dst, const void * src) { *((bool *)dst) = *((const bool *)src); }

static Result tst_match_int_arg(void) {
  Result r = PASS;
  EXPECT_OK(&r, MOC_init_registry());

  int i = 9;

  take_int_return_void(i);

  printf("call made before expectation, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());
  if(HAS_FAILED(&r)) MOC_print_expectations_report(MOC_REPORT_VERBOSITY_ALL);
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int)));

  printf("call not yet made (after expectation), verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());
  // do not clear, as we want to keep the existing expectation

  take_int_return_void(i);

  printf("call made after expectation, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_destroy_registry());
  return r;
}

static Result tst_two_args_both_matched(void) {
  Result r = PASS;
  EXPECT_OK(&r, MOC_init_registry());

  int          i           = 9;
  int          j           = 8;
  const char * hello       = "hello!";
  const char * other_hello = "hello!";
  const char * bye         = "bye";

  take_int_and_string_return_bool(i, hello);

  printf("call made before expectation, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r,
            EXPECT_CALL(take_int_and_string_return_bool,
                        MATCH_ARG(0, &i, comp_int),
                        MATCH_ARG(1, &hello, comp_str)));

  take_int_and_string_return_bool(i, other_hello);

  printf("call made after expectation, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r,
            EXPECT_CALL(take_int_and_string_return_bool,
                        MATCH_ARG(0, &i, comp_int),
                        MATCH_ARG(1, &hello, comp_str)));

  take_int_and_string_return_bool(i, bye); // call has wrong arg at index 1, cause fail

  printf("call made to expected function but with wrong arg at index 1, should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r,
            EXPECT_CALL(take_int_and_string_return_bool,
                        MATCH_ARG(0, &i, comp_int),
                        MATCH_ARG(1, &hello, comp_str)));

  take_int_and_string_return_bool(j, hello); // call has wrong arg at index 0, cause fail

  printf("call made to expected function but with wrong arg at index 0, should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, MOC_destroy_registry());
  return r;
}

static Result tst_two_args_one_matched(void) {
  Result r = PASS;
  EXPECT_OK(&r, MOC_init_registry());

  int          i     = 9;
  int          j     = 8;
  const char * hello = "hello!";
  const char * bye   = "bye";

  EXPECT_OK(&r, EXPECT_CALL(take_int_and_string_return_bool, MATCH_ARG(1, &hello, comp_str)));

  take_int_and_string_return_bool(j, hello);

  printf("call made to expected function with matcher only for arg 1, should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, EXPECT_CALL(take_int_and_string_return_bool, MATCH_ARG(0, &i, comp_int)));

  take_int_and_string_return_bool(i, bye);

  printf("call made to expected function with matcher only for arg 0, should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, EXPECT_CALL(take_int_and_string_return_bool, MATCH_ARG(0, &i, comp_int)));

  take_int_and_string_return_bool(j, bye);

  printf("call made to expected function with non-match for arg 0, should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());
  EXPECT_OK(&r, MOC_clear_registry());

  take_int_and_string_return_bool(i, bye);

  printf("call made to expected function with non-match for arg 1, should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, MOC_destroy_registry());
  return r;
}

static Result tst_multiplicity_EXACTLY(void) {
  Result r = PASS;
  EXPECT_OK(&r, MOC_init_registry());

  int i = 9;

  /* EXACTLY(0) */
  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int), TIMES(EXACTLY(0))));

  printf("EXACTLY(0) calls expected, 0 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("EXACTLY(0) calls expected, 1 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  /* EXACTLY(1) */
  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int), TIMES(EXACTLY(1))));

  printf("EXACTLY(1) calls expected, 0 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("EXACTLY(1) calls expected, 1 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("EXACTLY(1) calls expected, 2 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  /* EXACTLY(2) */
  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int), TIMES(EXACTLY(2))));

  printf("EXACTLY(2) calls expected, 0 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("EXACTLY(2) calls expected, 1 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("EXACTLY(2) calls expected, 2 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("EXACTLY(2) calls expected, 3 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, MOC_destroy_registry());
  return r;
}

static Result tst_multiplicity_AT_LEAST(void) {
  Result r = PASS;
  EXPECT_OK(&r, MOC_init_registry());

  int i = 9;

  /* AT_LEAST(0) */
  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int), TIMES(AT_LEAST(0))));

  printf("AT_LEAST(0) calls expected, 0 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_LEAST(0) calls expected, 1 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  /* AT_LEAST(1) */
  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int), TIMES(AT_LEAST(1))));

  printf("AT_LEAST(1) calls expected, 0 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_LEAST(1) calls expected, 1 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_LEAST(1) calls expected, 2 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  /* AT_LEAST(2) */
  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int), TIMES(AT_LEAST(2))));

  printf("AT_LEAST(2) calls expected, 0 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_LEAST(2) calls expected, 1 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_LEAST(2) calls expected, 2 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_LEAST(2) calls expected, 3 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, MOC_destroy_registry());
  return r;
}

static Result tst_multiplicity_AT_MOST(void) {
  Result r = PASS;
  EXPECT_OK(&r, MOC_init_registry());

  int i = 9;

  /* AT_MOST(0) */
  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int), TIMES(AT_MOST(0))));

  printf("AT_MOST(0) calls expected, 0 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_MOST(0) calls expected, 1 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  /* AT_MOST(1) */
  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int), TIMES(AT_MOST(1))));

  printf("AT_MOST(1) calls expected, 0 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_MOST(1) calls expected, 1 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_MOST(1) calls expected, 2 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  /* AT_MOST(2) */
  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int), TIMES(AT_MOST(2))));

  printf("AT_MOST(2) calls expected, 0 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_MOST(2) calls expected, 1 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_MOST(2) calls expected, 2 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("AT_MOST(2) calls expected, 3 calls made, verification should fail\n");
  EXPECT_FALSE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, MOC_destroy_registry());
  return r;
}

static Result tst_multiplicity_ANY_NUMBER(void) {
  Result r = PASS;
  EXPECT_OK(&r, MOC_init_registry());

  int i = 9;

  EXPECT_OK(&r, EXPECT_CALL(take_int_return_void, MATCH_ARG(0, &i, comp_int), TIMES(ANY_NUMBER())));

  printf("ANY_NUMBER() calls expected, 0 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("ANY_NUMBER() calls expected, 1 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("ANY_NUMBER() calls expected, 2 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  take_int_return_void(i);

  printf("ANY_NUMBER() calls expected, 3 calls made, verification should pass\n");
  EXPECT_TRUE(&r, MOC_is_all_registered_expectations_matched());

  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, MOC_destroy_registry());
  return r;
}

static Result tst_set_return_bool(void) {
  Result r = PASS;
  EXPECT_OK(&r, MOC_init_registry());

  int  i_for_true  = 9;
  int  i_for_false = 8;
  bool bool_true   = true;
  bool bool_false  = false;

  EXPECT_OK(&r, EXPECT_CALL(take_int_and_string_return_bool, SET_RETURN(&bool_false, set_bool)));
  EXPECT_FALSE(&r, take_int_and_string_return_bool(1, "hi"));
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, EXPECT_CALL(take_int_and_string_return_bool, SET_RETURN(&bool_true, set_bool)));
  EXPECT_TRUE(&r, take_int_and_string_return_bool(4, "bye"));
  EXPECT_OK(&r, MOC_clear_registry());

  {

    EXPECT_OK(&r,
              EXPECT_CALL(take_int_and_string_return_bool,
                          MATCH_ARG(0, &i_for_true, comp_int),
                          SET_RETURN(&bool_true, set_bool)));
    EXPECT_OK(&r,
              EXPECT_CALL(take_int_and_string_return_bool,
                          MATCH_ARG(0, &i_for_false, comp_int),
                          SET_RETURN(&bool_false, set_bool)));

    EXPECT_FALSE(&r, take_int_and_string_return_bool(i_for_false, ""));
    EXPECT_TRUE(&r, take_int_and_string_return_bool(i_for_true, ""));
  }

  EXPECT_OK(&r, MOC_destroy_registry());
  return r;
}

static const char * return_str(void) {
  const MOC_Expectation * exp = NULL;
  if(!STAT_is_OK(REGISTER_MADE_CALL(&exp, __func__))) {
    LOG_STAT(STAT_ERR_INTERNAL, "failed to register made call");
    return "bad";
  }

  const char * str = NULL;
  if(exp != NULL) MOC_set_return_val_from_expectation(exp, &str);

  return str;
}

static void set_str(void * dst_p, const void * src) { *((const char **)dst_p) = (const char *)src; }

static Result tst_set_return_str(void) {
  Result r = PASS;
  EXPECT_OK(&r, MOC_init_registry());

  const char * hello = "hello";
  const char * hi    = "hi";

  EXPECT_OK(&r, EXPECT_CALL(return_str, SET_RETURN(hello, set_str)));
  EXPECT_STREQ(&r, "hello", return_str());
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, EXPECT_CALL(return_str, SET_RETURN(hi, set_str)));
  EXPECT_STREQ(&r, "hi", return_str());
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, EXPECT_CALL(return_str, SET_RETURN("", set_str)));
  EXPECT_STREQ(&r, "", return_str());
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, MOC_destroy_registry());
  return r;
}

static void set_arg_int(void * arg_p, const void * src) {
  int * i_arg_p = *(int **)arg_p;
  int   i_src   = *((const int *)src);
  *i_arg_p      = i_src;
}
static void set_arg_bool(void * arg_p, const void * src) {
  bool * i_arg_p = *(bool **)arg_p;
  bool   i_src   = *((const bool *)src);
  *i_arg_p       = i_src;
}

static void ouput_int_and_bool_via_arg(int * i, bool * b) {
  REGISTER_MADE_CALL(NULL, __func__, &i, &b);
}

static Result tst_set_arg(void) {
  Result r = PASS;
  EXPECT_OK(&r, MOC_init_registry());

  int  i_out = 0;
  bool b_out = false;

  int  six       = 6;
  int  nine      = 9;
  bool not_true  = false;
  bool not_false = true;

  EXPECT_OK(&r,
            EXPECT_CALL(ouput_int_and_bool_via_arg,
                        SET_ARG(0, &six, set_arg_int),
                        SET_ARG(1, &not_false, set_arg_bool)));
  ouput_int_and_bool_via_arg(&i_out, &b_out);
  EXPECT_EQ(&r, 6, i_out);
  EXPECT_TRUE(&r, b_out);
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r,
            EXPECT_CALL(ouput_int_and_bool_via_arg,
                        SET_ARG(0, &nine, set_arg_int),
                        SET_ARG(1, &not_true, set_arg_bool)));
  ouput_int_and_bool_via_arg(&i_out, &b_out);
  EXPECT_EQ(&r, 9, i_out);
  EXPECT_FALSE(&r, b_out);
  EXPECT_OK(&r, MOC_clear_registry());

  EXPECT_OK(&r, MOC_destroy_registry());
  return r;
}

int main(void) {
  Test tests[] = {
      tst_init_verify_no_calls,
      tst_match_int_arg,
      tst_two_args_both_matched,
      tst_two_args_one_matched,
      tst_multiplicity_EXACTLY,
      tst_multiplicity_AT_LEAST,
      tst_multiplicity_AT_MOST,
      tst_multiplicity_ANY_NUMBER,
      tst_set_return_bool,
      tst_set_return_str,
      tst_set_arg,
  };

  const Result test_res = run_tests(tests, sizeof(tests) / sizeof(Test));

  return (test_res == PASS) ? 0 : 1;
}