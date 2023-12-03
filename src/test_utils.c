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
