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

#ifndef CFAC_BENCH_UTILS_H
#define CFAC_BENCH_UTILS_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "stat.h"

#include "darray.h"

typedef int BNC_Witness;

typedef STAT_Val (*BNC_SetupFn)(void **);
typedef STAT_Val (*BNC_TeardownFn)(void **);

typedef BNC_Witness (*BNC_BaselineFn)(void * env);
typedef BNC_Witness (*BNC_BenchFn)(void * env);

typedef double (*BNC_GetTimeFn)();

typedef struct BNC_PassResult {
  double      baseline_time;
  double      pass_time;
  BNC_Witness witness;
} BNC_PassResult;

typedef struct BNC_Benchmark {
  const char * name;

  BNC_SetupFn    setup_fn;
  BNC_TeardownFn teardown_fn;

  BNC_BenchFn    bench_fn;
  BNC_BaselineFn baseline_fn;
  BNC_GetTimeFn  get_time_fn;

  size_t num_iterations_per_pass;

  size_t min_num_passes;
  size_t max_num_passes;
  double max_run_time;
  double desired_std_dev_percent; // std dev in percentage of mean

  void * environment;

  double start_time;

  DAR_DArray pass_results; // holds BNC_PassResult
} BNC_Benchmark;

STAT_Val BNC_run_benchmark(BNC_Benchmark * benchmark);
STAT_Val BNC_print_benchmark_results(const BNC_Benchmark * benchmark);
STAT_Val BNC_destroy_benchmark(BNC_Benchmark * benchmark);

double BNC_get_total_pass_time(const BNC_Benchmark * benchmark);
double BNC_get_mean_pass_time(const BNC_Benchmark * benchmark);
double BNC_get_mean_iteration_time(const BNC_Benchmark * benchmark);
double BNC_get_min_pass_time(const BNC_Benchmark * benchmark);
double BNC_get_max_pass_time(const BNC_Benchmark * benchmark);
double BNC_get_pass_time_std_dev_percent(const BNC_Benchmark * benchmark);

double BNC_get_total_baseline_time(const BNC_Benchmark * benchmark);
double BNC_get_mean_baseline_time(const BNC_Benchmark * benchmark);
double BNC_get_min_baseline_time(const BNC_Benchmark * benchmark);
double BNC_get_max_baseline_time(const BNC_Benchmark * benchmark);
double BNC_get_baseline_time_std_dev_percent(const BNC_Benchmark * benchmark);

#endif
