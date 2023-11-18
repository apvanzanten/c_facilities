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

STAT_Val BNC_destroy_benchmark(BNC_Benchmark * benchmark) {
  if(benchmark == NULL) return OK;

  if(DAR_is_initialized(&benchmark->pass_results)) {
    if(!STAT_is_OK(DAR_destroy(&benchmark->pass_results))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy pass_results");
    }
  }

  *benchmark = (BNC_Benchmark){0};

  return OK;
}

static STAT_Val init_benchmark(BNC_Benchmark * benchmark) {
  if(benchmark->bench_fn == NULL || benchmark->get_time_fn == NULL ||
     benchmark->num_iterations_per_pass == 0 ||
     benchmark->min_num_passes > benchmark->max_num_passes ||
     DAR_is_initialized(&benchmark->pass_results) || benchmark->desired_std_dev_percent < 0.0) {
    return LOG_STAT(STAT_ERR_ARGS, "benchmark in invalid state");
  }

  if(!STAT_is_OK(DAR_create(&benchmark->pass_results, sizeof(BNC_PassResult))) ||
     !STAT_is_OK(DAR_reserve(&benchmark->pass_results, benchmark->max_num_passes))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to allocate and prepare pass results array");
  }

  return OK;
}

static STAT_Val run_next_pass(BNC_Benchmark * benchmark) {
  BNC_PassResult result = {0};

  if(benchmark->setup_fn != NULL) {
    if(!STAT_is_OK(benchmark->setup_fn(&benchmark->environment))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to setup bench pass environment");
    }
  }

  if(benchmark->baseline_fn != NULL) {
    const double baseline_start = benchmark->get_time_fn();
    for(size_t i = 0; i < benchmark->num_iterations_per_pass; i++) {
      result.witness += benchmark->baseline_fn(benchmark->environment);
    }
    const double baseline_end = benchmark->get_time_fn();

    result.baseline_time = (baseline_end - baseline_start);
  }

  const double pass_start = benchmark->get_time_fn();
  for(size_t i = 0; i < benchmark->num_iterations_per_pass; i++) {
    result.witness += benchmark->bench_fn(benchmark->environment);
  }
  const double pass_end = benchmark->get_time_fn();

  result.pass_time = (pass_end - pass_start);

  if(benchmark->teardown_fn != NULL) {
    if(!STAT_is_OK(benchmark->teardown_fn(&benchmark->environment))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to teardown bench pass environment");
    }
  }

  if(!STAT_is_OK(DAR_push_back(&benchmark->pass_results, &result))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to store benchmark result");
  }

  return OK;
}

static bool is_benchmark_run_finished(const BNC_Benchmark * benchmark) {
  const size_t curr_num_passes = benchmark->pass_results.size;

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

  return OK;
}

static bool is_valid_benchmark_result(const BNC_Benchmark * benchmark) {
  return (benchmark != NULL && DAR_is_initialized(&benchmark->pass_results) &&
          !DAR_is_empty(&benchmark->pass_results));
}

STAT_Val BNC_print_benchmark_results(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return LOG_STAT(STAT_ERR_ARGS, "result not valid");

  printf("Benchmark %s:\n", (benchmark->name == NULL) ? "NO NAME" : benchmark->name);

  printf("%40s: %zu (min=%zu, max=%zu)\n",
         "num passes",
         benchmark->pass_results.size,
         benchmark->min_num_passes,
         benchmark->max_num_passes);
  printf("%40s: %zu\n", "num iterations per pass", benchmark->num_iterations_per_pass);

  printf("%40s: %g / %g\n",
         "total time (pass / baseline)",
         BNC_get_total_pass_time(benchmark),
         BNC_get_total_baseline_time(benchmark));

  printf("%40s: %g\n", "iteration mean time", BNC_get_mean_iteration_time(benchmark));

  printf("%40s: %g / %g / %g\n",
         "baseline time (min / max / mean)",
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

  const DAR_DArray * results = &benchmark->pass_results;

  double total_time = 0.0;
  for(const BNC_PassResult * result = DAR_first(results); result != DAR_end(results); result++) {
    total_time += (result->pass_time - result->baseline_time);
  }

  return total_time;
}

double BNC_get_mean_pass_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return BNC_get_total_pass_time(benchmark) / benchmark->pass_results.size;
}

double BNC_get_mean_iteration_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return BNC_get_mean_pass_time(benchmark) / benchmark->num_iterations_per_pass;
}

double BNC_get_min_pass_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  const DAR_DArray * results = &benchmark->pass_results;

  double min_time = INFINITY;
  for(const BNC_PassResult * result = DAR_first(results); result != DAR_end(results); result++) {
    const double time = result->pass_time - result->baseline_time;

    min_time = (time < min_time) ? time : min_time;
  }
  return min_time;
}

double BNC_get_max_pass_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  const DAR_DArray * results = &benchmark->pass_results;

  double max_time = -INFINITY;
  for(const BNC_PassResult * result = DAR_first(results); result != DAR_end(results); result++) {
    const double time = result->pass_time - result->baseline_time;

    max_time = (time > max_time) ? time : max_time;
  }

  return max_time;
}

double BNC_get_pass_time_std_dev_percent(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  const DAR_DArray * results = &benchmark->pass_results;

  const double mean = BNC_get_mean_pass_time(benchmark);

  double variance_sum = 0.0;
  for(const BNC_PassResult * result = DAR_first(results); result != DAR_end(results); result++) {
    const double time = result->pass_time - result->baseline_time;

    variance_sum += pow((mean - time), 2);
  }

  const double variance = variance_sum / results->size;
  const double std_dev  = sqrt(variance);

  return (std_dev / mean) * 100.0;
}

double BNC_get_total_baseline_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  const DAR_DArray * results = &benchmark->pass_results;

  double total_time = 0.0;
  for(const BNC_PassResult * result = DAR_first(results); result != DAR_end(results); result++) {
    total_time += result->baseline_time;
  }

  return total_time;
}

double BNC_get_mean_baseline_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  return BNC_get_total_baseline_time(benchmark) / benchmark->pass_results.size;
}

double BNC_get_min_baseline_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  const DAR_DArray * results = &benchmark->pass_results;

  double min_time = INFINITY;
  for(const BNC_PassResult * result = DAR_first(results); result != DAR_end(results); result++) {
    min_time = (result->baseline_time < min_time) ? result->baseline_time : min_time;
  }
  return min_time;
}

double BNC_get_max_baseline_time(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  const DAR_DArray * results = &benchmark->pass_results;

  double max_time = -INFINITY;
  for(const BNC_PassResult * result = DAR_first(results); result != DAR_end(results); result++) {
    max_time = (result->baseline_time > max_time) ? result->baseline_time : max_time;
  }
  return max_time;
}

double BNC_get_baseline_time_std_dev_percent(const BNC_Benchmark * benchmark) {
  if(!is_valid_benchmark_result(benchmark)) return 0.0;

  const DAR_DArray * results = &benchmark->pass_results;

  const double mean = BNC_get_mean_baseline_time(benchmark);

  double variance_sum = 0.0;
  for(const BNC_PassResult * result = DAR_first(results); result != DAR_end(results); result++) {
    variance_sum += pow((mean - result->baseline_time), 2);
  }

  const double variance = variance_sum / results->size;
  const double std_dev  = sqrt(variance);

  return (std_dev / mean) * 100.0;
}
