#include "test_utils.h"

#include <stddef.h>
#include <stdio.h>

Result run_tests_with_fixture(const TestWithFixture tests[],
                              size_t                n,
                              SetupFn               setup,
                              TeardownFn            teardown) {
  size_t num_passed = 0;

  for(size_t i = 0; i < n; i++) {
    void * env = NULL;

    Result setup_r = (*setup)(&env);
    if(setup_r == PASS) {
      if(tests[i](env) == PASS) num_passed++;
      (*teardown)(&env);
    }
  }

  printf("%zu out of %zu tests with env passed\n", num_passed, n);

  return (num_passed == n ? PASS : FAIL);
}

Result run_tests(const Test tests[], size_t n) {
  size_t num_passed = 0;

  for(size_t i = 0; i < n; i++) {
    const Result tr = tests[i]();

    if(tr == PASS) num_passed++;
  }

  printf("%zu out of %zu tests passed\n", num_passed, n);

  return (num_passed == n ? PASS : FAIL);
}

Result run_all_tests(const Test            tests[],
                     size_t                n,
                     const TestWithFixture tests_with_fixture[],
                     size_t                n_with_fixture,
                     SetupFn               setup,
                     TeardownFn            teardown) {
  return (run_tests(tests, n) == PASS &&
          run_tests_with_fixture(tests_with_fixture, n_with_fixture, setup, teardown) == PASS)
             ? PASS
             : FAIL;
}
