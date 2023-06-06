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

#ifndef CFAC_TST_UTILS_H
#define CFAC_TST_UTILS_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

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

// EXPECT_OK?

#define EXPECT_EQ(rPtr, a, b)                                                                      \
  do {                                                                                             \
    if((a) != (b)) {                                                                               \
      *(rPtr) = FAIL;                                                                              \
      PRINT_FAIL("(%s != %s)", #a, #b);                                                            \
    }                                                                                              \
  } while(false)

#define EXPECT_NE(rPtr, a, b)                                                                      \
  do {                                                                                             \
    if((a) == (b)) {                                                                               \
      *(rPtr) = FAIL;                                                                              \
      PRINT_FAIL("(%s == %s)", #a, #b);                                                            \
    }                                                                                              \
  } while(false)

#define EXPECT_TRUE(rPtr, expr)  EXPECT_EQ(rPtr, expr, true)
#define EXPECT_FALSE(rPtr, expr) EXPECT_EQ(rPtr, expr, false)

#define EXPECT_STREQ(rPtr, a, b)                                                                   \
  do {                                                                                             \
    const char * a_copy = (a);                                                                     \
    const char * b_copy = (b);                                                                     \
    if(strcmp(a_copy, b_copy) != 0) {                                                              \
      *(rPtr) = FAIL;                                                                              \
      PRINT_FAIL("(%s != %s) <=> (\"%s\" != \"%s\")", #a, #b, a_copy, b_copy);                     \
    }                                                                                              \
  } while(false)

#define EXPECT_STRNE(rPtr, a, b)                                                                   \
  do {                                                                                             \
    const char * a_copy = (a);                                                                     \
    if(strcmp(a_copy, (b)) == 0) {                                                                 \
      *(rPtr) = FAIL;                                                                              \
      PRINT_FAIL("(%s == %s) == \"%s\"", #a, #b, a_copy);                                          \
    }                                                                                              \
  } while(false)

#define EXPECT_ARREQ(rPtr, type, a, b, n)                                                          \
  do {                                                                                             \
    const type * a_copy = (a);                                                                     \
    const type * b_copy = (b);                                                                     \
    for(size_t i = 0; i < (size_t)(n); i++) {                                                      \
      if(a_copy[i] != b_copy[i]) {                                                                 \
        *(rPtr) = FAIL;                                                                            \
        PRINT_FAIL("(%s[%zu] != %s[%zu])", #a, i, #b, i);                                          \
        break;                                                                                     \
      }                                                                                            \
    }                                                                                              \
  } while(false)

#define EXPECT_ARRNE(rPtr, type, a, b, n)                                                          \
  do {                                                                                             \
    const type * a_copy = (a);                                                                     \
    const type * b_copy = (b);                                                                     \
                                                                                                   \
    bool is_equal_so_far = true;                                                                   \
    for(size_t i = 0; (i < (size_t)(n)) && is_equal_so_far; i++) {                                 \
      if(a_copy[i] != b_copy[i]) is_equal_so_far = false;                                          \
    }                                                                                              \
    if(is_equal_so_far) {                                                                          \
      *(rPtr) = FAIL;                                                                              \
      PRINT_FAIL("%s == %s", #a, #b);                                                              \
    }                                                                                              \
  } while(false)

#define HAS_FAILED(r_ptr) (*(r_ptr) == FAIL)

#endif
