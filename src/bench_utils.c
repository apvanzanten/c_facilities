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

#include "bench_utils.h"

#include "log.h"

#include <math.h>
#include <stdio.h>

#define OK STAT_OK

void BNC_destroy_benchmark(BNC_Benchmark * benchmark) {
  if(benchmark != NULL) *benchmark = (BNC_Benchmark){0};
}

static void init_im_result(BNC_IntermediateResult * result) {
  *result     = (BNC_IntermediateResult){0};
  result->max = -INFINITY;
  result->min = INFINITY;
}

static STAT_Val init_benchmark(BNC_Benchmark * benchmark) {
  if(benchmark->bench_fn == NULL || benchmark->get_time_fn == NULL ||
     benchmark->num_iterations_per_pass == 0 ||
     benchmark->min_num_passes > benchmark->max_num_passes ||
     benchmark->desired_std_dev_percent < 0.0) {
    return LOG_STAT(STAT_ERR_ARGS, "benchmark in invalid state");
  }

  init_im_result(&benchmark->bench_im_result);
  init_im_result(&benchmark->baseline_im_result);

  return OK;
}

static void update_intermediate_result(BNC_IntermediateResult * result, double pass_time) {
  // to keep variance and mean we use basically Welford's algorithm, see
  // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
  result->num_passes++;

  const double delta     = pass_time - result->mean;
  const double new_mean  = result->mean + (delta / result->num_passes);
  const double new_delta = pass_time - new_mean;

  result->variance_aggregate += (delta * new_delta);
  result->mean = new_mean;

  result->total_time += pass_time;
  result->max = (pass_time > result->max) ? pass_time : result->max;
  result->min = (pass_time < result->min) ? pass_time : result->min;
}

static double get_std_dev_percent(double mean, double variance_aggregate, size_t num_passes) {
  const double variance = variance_aggregate / num_passes;

  return (sqrt(variance) / mean) * 100.0;
}

static BNC_Result finalize_result(BNC_IntermediateResult intermediate_result) {
  return (BNC_Result){
      .num_passes      = intermediate_result.num_passes,
      .total_time      = intermediate_result.total_time,
      .mean            = intermediate_result.mean,
      .min             = intermediate_result.min,
      .max             = intermediate_result.max,
      .std_dev_percent = get_std_dev_percent(intermediate_result.mean,
                                             intermediate_result.variance_aggregate,
                                             intermediate_result.num_passes),
      .witness         = intermediate_result.witness,
  };
}

static STAT_Val run_next_pass(BNC_Benchmark * benchmark) {
  BNC_IntermediateResult * baseline = &benchmark->baseline_im_result;
  BNC_IntermediateResult * bench    = &benchmark->bench_im_result;

  if(benchmark->setup_fn != NULL) {
    if(!STAT_is_OK(benchmark->setup_fn(&benchmark->environment))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to setup bench baseline environment");
    }
  }

  if(benchmark->baseline_fn != NULL) {
    const double baseline_start = benchmark->get_time_fn();
    for(size_t i = 0; i < benchmark->num_iterations_per_pass; i++) {
      baseline->witness += benchmark->baseline_fn(benchmark->environment);
    }
    const double baseline_end = benchmark->get_time_fn();

    update_intermediate_result(baseline, (baseline_end - baseline_start));
  }

  if(benchmark->teardown_fn != NULL) {
    baseline->witness += benchmark->teardown_fn(&benchmark->environment);
  }

  if(benchmark->setup_fn != NULL) {
    if(!STAT_is_OK(benchmark->setup_fn(&benchmark->environment))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to setup bench pass environment");
    }
  }

  const double pass_start = benchmark->get_time_fn();
  for(size_t i = 0; i < benchmark->num_iterations_per_pass; i++) {
    bench->witness += benchmark->bench_fn(benchmark->environment);
  }
  const double pass_end = benchmark->get_time_fn();

  update_intermediate_result(bench, (pass_end - pass_start));

  if(benchmark->teardown_fn != NULL) {
    bench->witness += benchmark->teardown_fn(&benchmark->environment);
  }

  return OK;
}

static bool is_benchmark_run_finished(const BNC_Benchmark * benchmark) {
  const size_t curr_num_passes = benchmark->bench_im_result.num_passes;

  return (curr_num_passes >= benchmark->max_num_passes) ||
         ((benchmark->get_time_fn() - benchmark->start_time) > benchmark->max_run_time) ||
         (curr_num_passes >= benchmark->min_num_passes &&
          (BNC_get_pass_time_std_dev_percent(benchmark) <= benchmark->desired_std_dev_percent));
}

STAT_Val BNC_run_benchmark(BNC_Benchmark * benchmark) {
  if(benchmark == NULL) return LOG_STAT(STAT_ERR_ARGS, "benchmark is NULL");

  if(!STAT_is_OK(init_benchmark(benchmark))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "fail to init benchmark");
  }

  benchmark->start_time = benchmark->get_time_fn();

  do {
    if(!STAT_is_OK(run_next_pass(benchmark))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to run benchmark pass");
    }
  } while(!is_benchmark_run_finished(benchmark));

  benchmark->bench_result = finalize_result(benchmark->bench_im_result);
  if(benchmark->baseline_im_result.num_passes > 0) {
    benchmark->baseline_result = finalize_result(benchmark->baseline_im_result);
  }

  return OK;
}

STAT_Val BNC_run_benchmarks(BNC_Benchmark * benchmarks_arr, size_t n) {
  if(benchmarks_arr == NULL) return LOG_STAT(STAT_ERR_ARGS, "benchmarks_arr is NULL");

  for(size_t i = 0; i < n; i++) {
    if(!STAT_is_OK(BNC_run_benchmark(&benchmarks_arr[i]))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to run benchmark %zu", i);
    }
  }

  return OK;
}

STAT_Val BNC_print_benchmarks_results(const BNC_Benchmark * benchmarks_arr, size_t n) {
  if(benchmarks_arr == NULL) return LOG_STAT(STAT_ERR_ARGS, "benchmarks_arr is NULL");

  for(size_t i = 0; i < n; i++) {
    if(!STAT_is_OK(BNC_print_benchmark_results(&benchmarks_arr[i]))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to print benchmark result %zu", i);
    }
  }

  return OK;
}

STAT_Val BNC_destroy_benchmarks(BNC_Benchmark * benchmarks_arr, size_t n) {
  if(benchmarks_arr == NULL) return LOG_STAT(STAT_ERR_ARGS, "benchmarks_arr is NULL");

  for(size_t i = 0; i < n; i++) {
    BNC_destroy_benchmark(&benchmarks_arr[i]);
  }

  return OK;
}

STAT_Val BNC_run_print_and_destroy_benchmarks(BNC_Benchmark * benchmarks_arr, size_t n) {
  if(benchmarks_arr == NULL) return LOG_STAT(STAT_ERR_ARGS, "benchmarks_arr is NULL");

  if(!STAT_is_OK(BNC_run_benchmarks(benchmarks_arr, n)) ||
     !STAT_is_OK(BNC_print_benchmarks_results(benchmarks_arr, n)) ||
     !STAT_is_OK(BNC_destroy_benchmarks(benchmarks_arr, n))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to run, print, and destroy benchmarks");
  }

  return OK;
}

static bool is_valid_benchmark_result(const BNC_Benchmark * benchmark) {
  return (benchmark != NULL && benchmark->bench_result.num_passes > 2);
}

STAT_Val BNC_print_benchmark_results(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return LOG_STAT(STAT_ERR_ARGS, "result not valid");

  printf("Benchmark %s:\n", (benchmark->name == NULL) ? "NO NAME" : benchmark->name);

  printf("%40s: %zu (min=%zu, max=%zu)\n",
         "num passes",
         benchmark->bench_result.num_passes,
         benchmark->min_num_passes,
         benchmark->max_num_passes);
  printf("%40s: %zu\n", "num iterations per pass", benchmark->num_iterations_per_pass);

  printf("%40s: %g / %g\n",
         "total time S (pass / baseline)",
         BNC_get_total_pass_time(benchmark),
         BNC_get_total_baseline_time(benchmark));

  printf("%40s: %g\n", "iteration mean time", BNC_get_mean_iteration_time(benchmark));

  printf("%40s: %g / %g / %g\n",
         "baseline time S (min / max / mean)",
         BNC_get_min_baseline_time(benchmark),
         BNC_get_max_baseline_time(benchmark),
         BNC_get_mean_baseline_time(benchmark));

  printf("%40s: %g\n",
         "baseline std. deviation (% of mean)",
         BNC_get_baseline_time_std_dev_percent(benchmark));

  printf("%40s: %g / %g / %g\n",
         "pass time (min / max / mean)",
         BNC_get_min_pass_time(benchmark),
         BNC_get_max_pass_time(benchmark),
         BNC_get_mean_pass_time(benchmark));

  printf("%40s: %g (desired=%g)\n",
         "pass std. deviation (% of mean)",
         BNC_get_pass_time_std_dev_percent(benchmark),
         benchmark->desired_std_dev_percent);

  return OK;
}

double BNC_get_total_pass_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return benchmark->bench_result.total_time;
}

double BNC_get_mean_pass_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return benchmark->bench_result.mean;
}

double BNC_get_mean_iteration_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return BNC_get_mean_pass_time(benchmark) / benchmark->num_iterations_per_pass;
}

double BNC_get_min_pass_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return benchmark->bench_result.min;
}

double BNC_get_max_pass_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return benchmark->bench_result.max;
}

double BNC_get_pass_time_std_dev_percent(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return benchmark->bench_result.std_dev_percent;
}

double BNC_get_total_baseline_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return benchmark->baseline_result.total_time;
}

double BNC_get_mean_baseline_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return benchmark->baseline_result.mean;
}

double BNC_get_min_baseline_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return benchmark->baseline_result.min;
}

double BNC_get_max_baseline_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return benchmark->baseline_result.max;
}

double BNC_get_baseline_time_std_dev_percent(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return benchmark->baseline_result.std_dev_percent;
}
