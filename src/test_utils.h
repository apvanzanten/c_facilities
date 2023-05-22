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

#define PRINT_FAIL(...)                                                                            \
  do {                                                                                             \
    printf("--FAIL %s:%i in %s: ", &(strrchr(__FILE__, '/')[1]), __LINE__, __func__);              \
    printf(__VA_ARGS__);                                                                           \
    putc('\n', stdout);                                                                            \
  } while(false)

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
    const type * a_copy          = (a);                                                            \
    const type * b_copy          = (b);                                                            \
    bool         is_equal_so_far = true;                                                           \
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
