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
#include <stdlib.h>

typedef struct Settings {
  bool stop_on_failure;
} Settings;

#define DEFAULT_SETTINGS ((Settings){.stop_on_failure = false})

static Settings get_settings(int argc, const char ** argv) {
  if((argc <= 1) || (argv == NULL)) {
    return (Settings){0};
  }

  Settings settings = {0};

  for(int i = 1 /* skip over target name */; i < argc; i++) {
    const char * arg = argv[i];

    if(arg == NULL) {
      fprintf(stderr, "malformed args provided\n");
      exit(1);
    }

    if(strcmp(arg, "--stop-on-failure") == 0) {
      settings.stop_on_failure = true;
    } else {
      fprintf(stderr, "unexpected arg: %s\n", arg);
      exit(1);
    }
  }

  return settings;
}

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

static Result run_tests_with_fixture_impl(const TestWithFixture tests[],
                                          size_t                n,
                                          SetupFn               setup,
                                          TeardownFn            teardown,
                                          Settings              settings) {
  size_t num_passed   = 0;
  size_t num_executed = 0;

  for(size_t i = 0; i < n; i++) {
    void * env = NULL;

    Result setup_r = (*setup)(&env);
    if(setup_r == PASS) {
      const Result test_r     = tests[i](env);
      const Result teardown_r = teardown(&env);

      num_executed++;

      if((test_r == PASS) && (teardown_r == PASS)) {
        num_passed++;
      } else if(settings.stop_on_failure) {
        break;
      }
    }
  }

  printf("executed %zu out of %zu tests with env, %zu passed\n", num_executed, n, num_passed);

  return ((num_passed == num_executed) ? PASS : FAIL);
}

static Result run_tests_impl(const Test tests[], size_t n, Settings settings) {
  size_t num_passed   = 0;
  size_t num_executed = 0;

  for(size_t i = 0; i < n; i++) {
    const Result tr = tests[i]();

    num_executed++;

    if(tr == PASS) {
      num_passed++;
    } else if(settings.stop_on_failure) {
      break;
    }
  }

  printf("executed %zu out of %zu tests, %zu passed\n", num_executed, n, num_passed);

  return (num_passed == n ? PASS : FAIL);
}

Result run_tests_with_fixture(const TestWithFixture tests[],
                              size_t                n,
                              SetupFn               setup,
                              TeardownFn            teardown) {
  return run_tests_with_fixture_impl(tests, n, setup, teardown, DEFAULT_SETTINGS);
}

Result run_tests(const Test tests[], size_t n) {
  return run_tests_impl(tests, n, DEFAULT_SETTINGS);
}

Result run_tests_with_args(const Test tests[], size_t n, int argc, const char ** argv) {
  return run_tests_impl(tests, n, get_settings(argc, argv));
}

Result run_tests_with_fixture_and_args(const TestWithFixture tests[],
                                       size_t                n,
                                       SetupFn               setup,
                                       TeardownFn            teardown,
                                       int                   argc,
                                       const char **         argv) {
  return run_tests_with_fixture_impl(tests, n, setup, teardown, get_settings(argc, argv));
}
