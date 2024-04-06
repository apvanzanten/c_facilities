// MIT License
//
// Copyright (c) 2024 Arjen P. van Zanten
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

#ifndef CFAC_MOCK_UTILS_H
#define CFAC_MOCK_UTILS_H

#include <stdbool.h>
#include <stddef.h>

#include "stat.h"

STAT_Val MOC_init_registry(void);
bool     MOC_is_registry_initialized(void);
STAT_Val MOC_reinit_registry(void);
STAT_Val MOC_clear_registry(void);
STAT_Val MOC_destroy_registry(void);
bool     MOC_is_all_registered_expectations_matched(void);

typedef enum MOC_ReportVerbosity {
  MOC_REPORT_VERBOSITY_FAIL_ONLY = 0,
  MOC_REPORT_VERBOSITY_ALL,
} MOC_ReportVerbosity;

STAT_Val MOC_print_expectations_report(MOC_ReportVerbosity verbosity);

typedef struct MOC_Expectation MOC_Expectation;

typedef struct MOC_IMPL_SetReturn {
  const void * value_p;
  void (*set_fn)(void * return_val_p, const void * value_p);
} MOC_IMPL_SetReturn;

typedef struct MOC_IMPL_SetArgPointee {
  size_t       arg_idx;
  const void * value_p;
  void (*set_fn)(void * arg_p, const void * value_p);
} MOC_IMPL_SetArgPointee;

typedef struct MOC_IMPL_Matcher {
  size_t       arg_idx;
  const void * expected_val;
  int (*comp_fn)(const void * expected_val, const void * val);

  // source annotations
  const char * src_func;
  int          src_line;
  const char * src_str;
} MOC_IMPL_Matcher;

typedef struct MOC_IMPL_Multiplicity {
  enum {
    MOC_IMPL_MULT_EXACTLY = 0,
    MOC_IMPL_MULT_AT_LEAST,
    MOC_IMPL_MULT_AT_MOST,
    MOC_IMPL_MULT_ANY_NUMBER,
  } type;
  size_t number;
} MOC_IMPL_Multiplicity;

typedef struct MOC_IMPL_ExpectationModifier {
  enum {
    MOC_IMPL_EXP_MOD_TYPE_MATCHER,
    MOC_IMPL_EXP_MOD_MULTIPLICITY,
    MOC_IMPL_EXP_MOD_SET_RETURN,
    MOC_IMPL_EXP_MOD_SET_ARG_POINTEE,
  } type;

  union {
    MOC_IMPL_Matcher       matcher;
    MOC_IMPL_Multiplicity  multiplicity;
    MOC_IMPL_SetReturn     set_return;
    MOC_IMPL_SetArgPointee set_arg_pointee;
  } as;
} MOC_IMPL_ExpectationModifier;

STAT_Val MOC_IMPL_expect_call(const char *                       func_name,
                              const char *                       expect_statement_func_name,
                              int                                expect_statement_line,
                              size_t                             num_modifiers,
                              const MOC_IMPL_ExpectationModifier modifiers[]);

#define TIMES(mult /* type MOC_IMPL_Multiplicity */)                                               \
  (MOC_IMPL_ExpectationModifier) {                                                                 \
    .type = MOC_IMPL_EXP_MOD_MULTIPLICITY, .as.multiplicity = (mult)                               \
  }

#define EXACTLY(n)                                                                                 \
  (MOC_IMPL_Multiplicity) { .type = MOC_IMPL_MULT_EXACTLY, .number = (n) }
#define AT_MOST(n)                                                                                 \
  (MOC_IMPL_Multiplicity) { .type = MOC_IMPL_MULT_AT_MOST, .number = (n) }
#define ANY_NUMBER()                                                                               \
  (MOC_IMPL_Multiplicity) { .type = MOC_IMPL_MULT_ANY_NUMBER, .number = 0 }
#define AT_LEAST(n)                                                                                \
  (MOC_IMPL_Multiplicity) { .type = MOC_IMPL_MULT_AT_LEAST, .number = (n) }

#define EXPECT_CALL(expected_func, ... /* type MOC_IMPL_ExpectationModifier */)                    \
  MOC_IMPL_expect_call(#expected_func,                                                             \
                       __func__,                                                                   \
                       __LINE__,                                                                   \
                       (sizeof((MOC_IMPL_ExpectationModifier[]){__VA_ARGS__}) /                    \
                        sizeof(MOC_IMPL_ExpectationModifier)),                                     \
                       (MOC_IMPL_ExpectationModifier[]){__VA_ARGS__})

#define MATCH_ARG(in_arg_idx, in_expected_val, in_comp_fn)                                         \
  (MOC_IMPL_ExpectationModifier) {                                                                 \
    .type       = MOC_IMPL_EXP_MOD_TYPE_MATCHER,                                                   \
    .as.matcher = {.arg_idx      = (in_arg_idx),                                                   \
                   .expected_val = (in_expected_val),                                              \
                   .comp_fn      = (in_comp_fn),                                                   \
                   .src_func     = __func__,                                                       \
                   .src_line     = __LINE__,                                                       \
                   .src_str =                                                                      \
                       "MATCH_ARG(" #in_arg_idx ", " #in_expected_val ", " #in_comp_fn ")"},       \
  }

#define SET_RETURN(in_value_p, in_set_fn)                                                          \
  (MOC_IMPL_ExpectationModifier) {                                                                 \
    .type          = MOC_IMPL_EXP_MOD_SET_RETURN,                                                  \
    .as.set_return = {.value_p = (in_value_p), .set_fn = (in_set_fn)},                             \
  }

#define SET_ARG(in_arg_idx, in_value_p, in_set_fn)                                                 \
  (MOC_IMPL_SetArgPointee) {                                                                       \
    .arg_idx = (in_arg_idx), .value_p = (in_value_p), .set_fn = (in_set_fn)                        \
  }

STAT_Val MOC_IMPL_register_made_call(const MOC_Expectation ** matched_exp,
                                     const char *             func_name,
                                     size_t                   num_args,
                                     void *                   arg_ptrs[]);

#define REGISTER_MADE_CALL(exp_out_p /* const MOC_Expectation ** */,                               \
                           func_name /* const char * */,                                           \
                           ...)                                                                    \
  MOC_IMPL_register_made_call((exp_out_p),                                                         \
                              (func_name),                                                         \
                              (sizeof((void *[]){__VA_ARGS__}) / sizeof(void *)),                  \
                              (void *[]){__VA_ARGS__})

STAT_Val MOC_set_return_val_from_expectation(const MOC_Expectation * exp, void * return_val_p);

// TODO return val

#endif
