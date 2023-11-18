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

#include "bench_utils.h"

#include "log.h"
#include "stat.h"

#define OK STAT_OK

static double get_time() {
  struct timeval tv = {0};
  gettimeofday(&tv, NULL);
  return ((double)tv.tv_sec + ((double)tv.tv_usec / (1000.0 * 1000.0)));
}

static STAT_Val setup_int_arr(void ** env) {
  if(env == NULL) return LOG_STAT(STAT_ERR_ARGS, "env is NULL");

  DAR_DArray * arr = calloc(1, sizeof(DAR_DArray));
  if(arr == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for arr");

  *env = arr;

  return LOG_STAT_IF_ERR(DAR_create(arr, sizeof(int)), "failed to create array");
}

static STAT_Val setup_int_arr_and_reserve_1_million(void ** env) {
  if(env == NULL) return LOG_STAT(STAT_ERR_ARGS, "env is NULL");

  DAR_DArray * arr = calloc(1, sizeof(DAR_DArray));
  if(arr == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for arr");

  *env = arr;

  if(!STAT_is_OK(DAR_create(arr, sizeof(int)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create array");
  }

  if(!STAT_is_OK(DAR_reserve(arr, 1000 * 1000))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to reserve elements on array");
  }

  return OK;
}

static BNC_Witness teardown_int_arr(void ** env) {
  if(env == NULL) return 0;

  DAR_DArray * arr = *env;

  BNC_Witness witness = 0;

  for(int * p = DAR_first(arr); p != DAR_end(arr); p++) {
    witness += *p;
  }

  DAR_destroy(arr);

  free(arr);

  return witness;
}

static BNC_Witness sum_1000_ints(void * env) {
  if(env == NULL) return 0;

  BNC_Witness witness = 0;

  for(int i = 0; i < 1000; i++) {
    witness += i;
  }

  return witness;
}

static BNC_Witness push_back_1000_ints(void * env) {
  if(env == NULL) return 0;

  DAR_DArray * arr = env;

  BNC_Witness witness = 0;

  for(int i = 0; i < 1000; i++) {
    DAR_push_back(arr, &i);
    witness += i;
  }

  return witness;
}

int main(void) {
  BNC_Benchmark benches[] = {
      {
          .name        = "push back 1000 ints, unreserved",
          .setup_fn    = setup_int_arr,
          .teardown_fn = teardown_int_arr,
          .baseline_fn = sum_1000_ints,
          .bench_fn    = push_back_1000_ints,
          .get_time_fn = get_time,

          .num_iterations_per_pass = 1000, // NOTE 1000 ints per iteration go to same array
          .min_num_passes          = 100,
          .max_num_passes          = 10000,
          .max_run_time            = 10.0,
          .desired_std_dev_percent = 1.5,
      },
      {
          .name        = "push back 1000 ints, reserved",
          .setup_fn    = setup_int_arr_and_reserve_1_million,
          .teardown_fn = teardown_int_arr,
          .baseline_fn = sum_1000_ints,
          .bench_fn    = push_back_1000_ints,
          .get_time_fn = get_time,

          .num_iterations_per_pass = 1000, // NOTE 1000 ints per iteration go to same array
          .min_num_passes          = 100,
          .max_num_passes          = 10000,
          .max_run_time            = 10.0,
          .desired_std_dev_percent = 1.5,
      },
  };

  if(!STAT_is_OK(BNC_run_benchmarks(benches, sizeof(benches) / sizeof(benches[0])))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to run benchmarks");
  }

  if(!STAT_is_OK(BNC_print_benchmarks_results(benches, sizeof(benches) / sizeof(benches[0])))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to print results");
  }

  if(!STAT_is_OK(BNC_destroy_benchmarks(benches, sizeof(benches) / sizeof(benches[0])))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy benchmarks");
  }

  return OK;
}