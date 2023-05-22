#include "test_utils.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

void print_failure(const char * file, const char * func, int line, const char * fmt, ...) {
  const char * file_basename = strrchr(file, '/');                          // find last '/'
  if(file_basename != NULL) file_basename++;                                // skip actual '/'
  if(file_basename == NULL || *file_basename == '\0') file_basename = file; // fallback, plain file

  va_list args;
  va_start(args, fmt);

  printf("--FAIL %s:%i in %s: ", file_basename, line, func);
  vprintf(fmt, args);
  putc('\n', stdout);

  va_end(args);
}

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