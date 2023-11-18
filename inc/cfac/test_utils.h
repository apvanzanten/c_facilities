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

#ifndef CFAC_TEST_UTILS_H
#define CFAC_TEST_UTILS_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "stat.h"

typedef enum { FAIL, PASS } Result;

typedef Result (*Test)();

typedef Result (*SetupFn)(void **);
typedef Result (*TestWithFixture)(void *);
typedef Result (*TeardownFn)(void **);

Result run_tests(const Test tests[], size_t n);
Result run_tests_with_fixture(const TestWithFixture tests[],
                              size_t                n,
                              SetupFn               setup,
                              TeardownFn            teardown);

void print_failure(const char * file, const char * func, int line, const char * fmt, ...);

#define PRINT_FAIL(...) print_failure(__FILE__, __func__, __LINE__, __VA_ARGS__)

#define EXPECT_EQ(r_ptr, a, b)                                                                     \
  do {                                                                                             \
    if((a) != (b)) {                                                                               \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("(%s != %s)", #a, #b);                                                            \
    }                                                                                              \
  } while(false)

#define EXPECT_NE(r_ptr, a, b)                                                                     \
  do {                                                                                             \
    if((a) == (b)) {                                                                               \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("(%s == %s)", #a, #b);                                                            \
    }                                                                                              \
  } while(false)

#define EXPECT_TRUE(r_ptr, expr)  EXPECT_EQ(r_ptr, expr, true)
#define EXPECT_FALSE(r_ptr, expr) EXPECT_EQ(r_ptr, expr, false)

#define EXPECT_LT(r_ptr, a, b)                                                                     \
  do {                                                                                             \
    if((a) >= (b)) {                                                                               \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("!(%s < %s)", #a, #b);                                                            \
    }                                                                                              \
  } while(false)

#define EXPECT_LE(r_ptr, a, b)                                                                     \
  do {                                                                                             \
    if((a) > (b)) {                                                                                \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("!(%s <= %s)", #a, #b);                                                           \
    }                                                                                              \
  } while(false)

#define EXPECT_GT(r_ptr, a, b)                                                                     \
  do {                                                                                             \
    if((a) <= (b)) {                                                                               \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("!(%s > %s)", #a, #b);                                                            \
    }                                                                                              \
  } while(false)

#define EXPECT_GE(r_ptr, a, b)                                                                     \
  do {                                                                                             \
    if((a) < (b)) {                                                                                \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("!(%s >= %s)", #a, #b);                                                           \
    }                                                                                              \
  } while(false)

#define EXPECT_STREQ(r_ptr, a, b)                                                                  \
  do {                                                                                             \
    const char * a_copy = (a);                                                                     \
    const char * b_copy = (b);                                                                     \
                                                                                                   \
    if(strcmp(a_copy, b_copy) != 0) {                                                              \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("(%s != %s) <=> (\"%s\" != \"%s\")", #a, #b, a_copy, b_copy);                     \
    }                                                                                              \
  } while(false)

#define EXPECT_STRNE(r_ptr, a, b)                                                                  \
  do {                                                                                             \
    const char * a_copy = (a);                                                                     \
    if(strcmp(a_copy, (b)) == 0) {                                                                 \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("(%s == %s) == \"%s\"", #a, #b, a_copy);                                          \
    }                                                                                              \
  } while(false)

#define EXPECT_ARREQ(r_ptr, type, a, b, n)                                                         \
  do {                                                                                             \
    const type * a_copy = (a);                                                                     \
    const type * b_copy = (b);                                                                     \
                                                                                                   \
    for(size_t i = 0; i < (size_t)(n); i++) {                                                      \
      if(a_copy[i] != b_copy[i]) {                                                                 \
        *(r_ptr) = FAIL;                                                                           \
        PRINT_FAIL("(%s[%zu] != %s[%zu])", #a, i, #b, i);                                          \
        break;                                                                                     \
      }                                                                                            \
    }                                                                                              \
  } while(false)

#define EXPECT_ARRNE(r_ptr, type, a, b, n)                                                         \
  do {                                                                                             \
    const type * a_copy = (a);                                                                     \
    const type * b_copy = (b);                                                                     \
                                                                                                   \
    bool is_equal_so_far = true;                                                                   \
    for(size_t i = 0; (i < (size_t)(n)) && is_equal_so_far; i++) {                                 \
      if(a_copy[i] != b_copy[i]) is_equal_so_far = false;                                          \
    }                                                                                              \
    if(is_equal_so_far) {                                                                          \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("%s == %s", #a, #b);                                                              \
    }                                                                                              \
  } while(false)

#define HAS_FAILED(r_ptr) (*(r_ptr) == FAIL)

#define EXPECT_FLOAT_EQ(r_ptr, a, b, e)                                                            \
  do {                                                                                             \
    const double a_copy = (a);                                                                     \
    const double b_copy = (b);                                                                     \
    const double e_copy = (e);                                                                     \
                                                                                                   \
    if(((a_copy > b_copy) && ((a_copy - b_copy) > e_copy)) ||                                      \
       ((a_copy < b_copy) && (b_copy - a_copy) > e_copy)) {                                        \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("%s != %s; abs(%g - %g) > %g", #a, #b, a_copy, b_copy, e_copy);                   \
    }                                                                                              \
  } while(false)

#define EXPECT_FLOAT_NE(r_ptr, a, b, e)                                                            \
  do {                                                                                             \
    const double a_copy = (a);                                                                     \
    const double b_copy = (b);                                                                     \
    const double e_copy = (e);                                                                     \
                                                                                                   \
    if(((a_copy >= b_copy) && ((a_copy - b_copy) <= e_copy)) ||                                    \
       ((a_copy < b_copy) && (b_copy - a_copy) <= e_copy)) {                                       \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("%s != %s; abs(%g - %g) <= %g", #a, #b, a_copy, b_copy, e_copy);                  \
    }                                                                                              \
  } while(false)

#define EXPECT_OK(r_ptr, stat)                                                                     \
  do {                                                                                             \
    const STAT_Val stat_copy = (stat);                                                             \
    if(!STAT_is_OK(stat_copy)) {                                                                   \
      *(r_ptr) = FAIL;                                                                             \
      PRINT_FAIL("%s == %s; != OK", #stat, STAT_to_str(stat_copy));                                \
    }                                                                                              \
  } while(false)

#endif
