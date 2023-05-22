#ifndef CFAC_TST_UTILS_H
#define CFAC_TST_UTILS_H

#include <libgen.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef enum { FAIL, PASS } Result;

typedef Result (*Test)();

typedef Result (*SetupFn)(void **);
typedef Result (*TestWithFixture)(void *);
typedef Result (*TeardownFn)(void **);

Result run_tests_with_fixture(const TestWithFixture tests[],
                              size_t                n,
                              SetupFn               setup,
                              TeardownFn            teardown);

Result run_tests(const Test tests[], size_t n);

Result run_all_tests(const Test            tests[],
                     size_t                n,
                     const TestWithFixture tests_with_fixture[],
                     size_t                n_with_fixture,
                     SetupFn               setup,
                     TeardownFn            teardown);

// TODO stop using basename
#define PRINT_FAIL(...)                                                                            \
  printf("--FAIL %s:%i in %s: ", basename(__FILE__), __LINE__, __func__);                          \
  printf(__VA_ARGS__);                                                                             \
  putc('\n', stdout);

#define EXPECT_EQ(rPtr, a, b)                                                                      \
  if((a) != (b)) {                                                                                 \
    *(rPtr) = FAIL;                                                                                \
    PRINT_FAIL("(%s != %s)", #a, #b);                                                              \
  }

#define EXPECT_NE(rPtr, a, b)                                                                      \
  if((a) == (b)) {                                                                                 \
    *(rPtr) = FAIL;                                                                                \
    PRINT_FAIL("(%s == %s)", #a, #b);                                                              \
  }

#define EXPECT_TRUE(rPtr, expr)  EXPECT_EQ(rPtr, expr, true)
#define EXPECT_FALSE(rPtr, expr) EXPECT_EQ(rPtr, expr, false)

#define EXPECT_STREQ(rPtr, a, b)                                                                   \
  if(strcmp((a), (b)) != 0) {                                                                      \
    *(rPtr) = FAIL;                                                                                \
    PRINT_FAIL("(%s != %s) <=> (\"%s\" != \"%s\")", #a, #b, (a), (b));                             \
  }

#define EXPECT_STRNE(rPtr, a, b)                                                                   \
  if(strcmp((a), (b)) == 0) {                                                                      \
    *(rPtr) = FAIL;                                                                                \
    PRINT_FAIL("(%s == %s) == \"%s\"", #a, #b, (a));                                               \
  }

#define EXPECT_ARREQ(rPtr, a, b, n)                                                                \
  for(size_t i = 0; i < (size_t)n; i++) {                                                          \
    if((a)[i] != (b)[i]) {                                                                         \
      *(rPtr) = FAIL;                                                                              \
      PRINT_FAIL("(%s[%zu] != %s[%zu])", #a, i, #b, i);                                            \
      break;                                                                                       \
    }                                                                                              \
  }

#define EXPECT_ARRNE(rPtr, a, b, n)                                                                \
  {                                                                                                \
    size_t i = 0;                                                                                  \
    for(; i < n; i++) {                                                                            \
      if((a)[i] != (b)[i]) {                                                                       \
        break;                                                                                     \
      }                                                                                            \
    }                                                                                              \
    if(i == n) {                                                                                   \
      *(rPtr) = FAIL;                                                                              \
      PRINT_FAIL("%s == %s", #a, #b);                                                              \
    }                                                                                              \
  }

#define HAS_FAILED(r_ptr) (*(r_ptr) == FAIL)

#endif
