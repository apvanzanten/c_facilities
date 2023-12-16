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
#include <sys/time.h>

#include "test_utils.h"

#include "bench_utils.h"

#include "log.h"

#define OK STAT_OK

typedef double (*GetTimeFn)(void);
typedef struct GetTimeEnv {
  GetTimeFn fn;
} GetTimeEnv;

static double get_time(void) {
  struct timeval tv = {0};
  gettimeofday(&tv, NULL);
  return ((double)tv.tv_sec + ((double)tv.tv_usec / (1000.0 * 1000.0)));
}

static STAT_Val setup_get_time_env(void ** env) {
  if(env == NULL) return LOG_STAT(STAT_ERR_ARGS, "bad env");

  *env = malloc(sizeof(GetTimeEnv));
  if(*env == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for GetTimeEnv");

  GetTimeEnv * get_time_env = *env;
  get_time_env->fn          = get_time;

  return OK;
}

static BNC_Witness teardown_get_time_env(void ** env) {
  if(env == NULL) {
    LOG_STAT(STAT_ERR_INTERNAL, "bad teardown function call");
    return 0;
  }

  free(*env);

  return 1;
}

static BNC_Witness wait_1_millisecond_using_env_as_get_time(void * env) {
  if(env == NULL) return 0;

  GetTimeEnv * get_time_env  = env;
  GetTimeFn    get_time_func = get_time_env->fn;

  const double start       = get_time_func();
  const double desired_end = start + (1.0 / 1000.0);

  BNC_Witness witness = 0;
  while(get_time_func() <= desired_end) {
    witness++;
  };

  return witness;
}

static BNC_Witness baseline(void * env) {
  if(env == NULL) return 0;
  return 1;
}

static Result tst_bench_wait(void) {
  Result r = PASS;

  BNC_Benchmark benchmark = {
      .name        = "wait 1 millisecond",
      .setup_fn    = setup_get_time_env,
      .teardown_fn = teardown_get_time_env,
      .bench_fn    = wait_1_millisecond_using_env_as_get_time,
      .baseline_fn = baseline,
      .get_time_fn = get_time,

      .num_iterations_per_pass = 10,
      .min_num_passes          = 3,
      .max_num_passes          = 1000,
      .max_run_time            = 10.0,
      .desired_std_dev_percent = 5.0,
  };

  EXPECT_OK(&r, BNC_run_benchmark(&benchmark));

  const double expect_iteration_time = (1.0 / 1000.0);

  EXPECT_FLOAT_EQ(&r,
                  expect_iteration_time * benchmark.num_iterations_per_pass,
                  BNC_get_mean_pass_time(&benchmark),
                  0.002);
  EXPECT_FLOAT_EQ(&r, expect_iteration_time, BNC_get_mean_iteration_time(&benchmark), 0.00001);

  EXPECT_OK(&r, BNC_print_benchmark_results(&benchmark));

  BNC_destroy_benchmark(&benchmark);

  return r;
}

static Result tst_multiple_benchmarks(void) {
  Result r = PASS;

  BNC_Benchmark benches[3] = {
      {
          .name        = "wait 1 millisecond #1",
          .setup_fn    = setup_get_time_env,
          .teardown_fn = teardown_get_time_env,
          .bench_fn    = wait_1_millisecond_using_env_as_get_time,
          .baseline_fn = baseline,
          .get_time_fn = get_time,

          .num_iterations_per_pass = 50,
          .min_num_passes          = 5,
          .max_num_passes          = 1000,
          .max_run_time            = 10.0,
          .desired_std_dev_percent = 5.0,
      },
      {
          .name        = "wait 1 millisecond #2",
          .setup_fn    = setup_get_time_env,
          .teardown_fn = teardown_get_time_env,
          .bench_fn    = wait_1_millisecond_using_env_as_get_time,
          .baseline_fn = baseline,
          .get_time_fn = get_time,

          .num_iterations_per_pass = 50,
          .min_num_passes          = 10,
          .max_num_passes          = 1000,
          .max_run_time            = 10.0,
          .desired_std_dev_percent = 5.0,
      },
      {
          .name        = "wait 1 millisecond #3",
          .setup_fn    = setup_get_time_env,
          .teardown_fn = teardown_get_time_env,
          .bench_fn    = wait_1_millisecond_using_env_as_get_time,
          .baseline_fn = baseline,
          .get_time_fn = get_time,

          .num_iterations_per_pass = 50,
          .min_num_passes          = 15,
          .max_num_passes          = 1000,
          .max_run_time            = 10.0,
          .desired_std_dev_percent = 5.0,
      },
  };

  EXPECT_OK(&r, BNC_run_benchmarks(benches, sizeof(benches) / sizeof(BNC_Benchmark)));

  EXPECT_GE(&r, benches[0].bench_result.num_passes, benches[0].min_num_passes);
  EXPECT_GE(&r, benches[1].bench_result.num_passes, benches[1].min_num_passes);
  EXPECT_GE(&r, benches[2].bench_result.num_passes, benches[2].min_num_passes);

  EXPECT_OK(&r, BNC_print_benchmarks_results(benches, sizeof(benches) / sizeof(BNC_Benchmark)));

  const double expect_iteration_time = (1.0 / 1000.0);

  EXPECT_FLOAT_EQ(&r, expect_iteration_time, BNC_get_mean_iteration_time(&benches[0]), 0.00001);
  EXPECT_FLOAT_EQ(&r, expect_iteration_time, BNC_get_mean_iteration_time(&benches[1]), 0.00001);
  EXPECT_FLOAT_EQ(&r, expect_iteration_time, BNC_get_mean_iteration_time(&benches[2]), 0.00001);

  EXPECT_OK(&r, BNC_destroy_benchmarks(benches, sizeof(benches) / sizeof(BNC_Benchmark)));

  return r;
}

int main(void) {
  Test tests[] = {
      tst_bench_wait,
      tst_multiple_benchmarks,
  };

  return (run_tests(tests, sizeof(tests) / sizeof(Test)) == PASS) ? 0 : 1;
}
