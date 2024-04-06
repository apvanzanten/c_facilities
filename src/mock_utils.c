// MIT License
//
// Copyright (c) 2024 Arjen P. van Zanten
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

#include "mock_utils.h"

#include "darray.h"
#include "log.h"

#include <stdio.h>
#include <string.h>

#define OK STAT_OK

struct MOC_Expectation {
  const char * func_name;
  const char * src_func;
  int          src_line;

  DAR_DArray matchers; // type MOC_IMPL_Matcher
  DAR_DArray setters;  // type MOC_IMPL_SetArgPointee

  MOC_IMPL_SetReturn return_setter;

  MOC_IMPL_Multiplicity multiplicity;

  size_t actual_num_calls;
};

typedef struct MOC_Registry {
  DAR_DArray expectations;         // calls that are still expected to occur
  size_t     num_unexpected_calls; // counts number of unexpected calls
} MOC_Registry;

static MOC_Registry g_registry;

DAR_DArray * expectations() { return &g_registry.expectations; }

bool MOC_is_registry_initialized(void) { return DAR_is_initialized(expectations()); }

STAT_Val MOC_init_registry(void) {
  if(MOC_is_registry_initialized()) {
    if(!STAT_is_OK(MOC_destroy_registry())) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy registry before new init");
    }
  }

  g_registry = (MOC_Registry){0};

  return LOG_STAT_IF_ERR(DAR_create(expectations(), sizeof(MOC_Expectation)),
                         "failed to create expectations array");
}

STAT_Val MOC_reinit_registry(void) {
  return LOG_STAT_IF_ERR(MOC_init_registry(), "failed to init registry");
}

static STAT_Val destroy_expectation(MOC_Expectation * exp) {
  if(exp == NULL) return LOG_STAT(STAT_ERR_ARGS, "exp is NULL");

  if(DAR_is_initialized(&exp->matchers)) {
    if(!STAT_is_OK(DAR_destroy(&exp->matchers))) {
      return LOG_STAT(STAT_ERR_INTERNAL,
                      "failed to destroy matchers for expectation of '%s'",
                      exp->func_name);
    }
  }
  if(DAR_is_initialized(&exp->setters)) {
    if(!STAT_is_OK(DAR_destroy(&exp->setters))) {
      return LOG_STAT(STAT_ERR_INTERNAL,
                      "failed to destroy matchers for expectation of '%s'",
                      exp->func_name);
    }
  }

  return OK;
}

STAT_Val MOC_clear_registry(void) {
  if(!MOC_is_registry_initialized()) {
    return LOG_STAT(STAT_ERR_PRECONDITION, "registry uninitialized");
  }

  for(MOC_Expectation * exp = DAR_first(expectations()); exp != DAR_end(expectations()); exp++) {
    if(!STAT_is_OK(destroy_expectation(exp))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy expectation in registry");
    }
  }

  if(!STAT_is_OK(DAR_clear_and_shrink(expectations()))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to clear expectations array");
  }

  g_registry.num_unexpected_calls = 0;

  return OK;
}

STAT_Val MOC_destroy_registry(void) {
  if(MOC_is_registry_initialized()) {
    if(!STAT_is_OK(MOC_clear_registry())) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to clear registry");
    }
    if(!STAT_is_OK(DAR_destroy(&g_registry.expectations))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy expectations array in registry");
    }
  }

  g_registry = (MOC_Registry){0};

  return OK;
}

static bool is_valid_num_occurences(const MOC_IMPL_Multiplicity * mult, size_t num_occurrences) {
  if(mult == NULL) return false;
  switch(mult->type) {
  case MOC_IMPL_MULT_EXACTLY: return (num_occurrences == mult->number);
  case MOC_IMPL_MULT_AT_LEAST: return (num_occurrences >= mult->number);
  case MOC_IMPL_MULT_AT_MOST: return (num_occurrences <= mult->number);
  case MOC_IMPL_MULT_ANY_NUMBER: return true;
  }
  return false; // should never get here, but return something anyway.
}

bool MOC_is_all_registered_expectations_matched(void) {
  if(!MOC_is_registry_initialized()) return false;
  if(g_registry.num_unexpected_calls != 0) return false;

  for(const MOC_Expectation * exp = DAR_first(expectations()); exp != DAR_end(expectations());
      exp++) {
    if(!is_valid_num_occurences(&exp->multiplicity, exp->actual_num_calls)) return false;
  }

  return true;
}

static void print_multiplicity(const MOC_IMPL_Multiplicity * mult) {
  if(mult != NULL) {
    switch(mult->type) {
    case MOC_IMPL_MULT_EXACTLY: printf("EXACTLY(%zu)", mult->number); break;
    case MOC_IMPL_MULT_AT_LEAST: printf("AT_LEAST(%zu)", mult->number); break;
    case MOC_IMPL_MULT_AT_MOST: printf("AT_MOST(%zu)", mult->number); break;
    case MOC_IMPL_MULT_ANY_NUMBER: printf("ANY_NUMBER"); break;
    default: printf("UNKNOWN(%zu)", mult->number);
    }
  }
}

STAT_Val MOC_print_expectations_report(MOC_ReportVerbosity verbosity) {
  if(!MOC_is_registry_initialized()) {
    return LOG_STAT(STAT_ERR_PRECONDITION, "registry uninitialized");
  }

  if((verbosity == MOC_REPORT_VERBOSITY_ALL) || (g_registry.num_unexpected_calls > 0)) {
    printf("num unexpected calls: %zu\n", g_registry.num_unexpected_calls);
  }

  for(const MOC_Expectation * exp = DAR_first(expectations()); exp != DAR_end(expectations());
      exp++) {
    if((verbosity == MOC_REPORT_VERBOSITY_ALL) ||
       !is_valid_num_occurences(&exp->multiplicity, exp->actual_num_calls)) {

      printf("- %s expected from %s:%d with matchers:\n",
             exp->func_name,
             exp->src_func,
             exp->src_line);

      for(const MOC_IMPL_Matcher * matcher = DAR_first(&exp->matchers);
          matcher != DAR_end(&exp->matchers);
          matcher++) {
        printf("  > %s:%d: %s\n", matcher->src_func, matcher->src_line, matcher->src_str);
      }

      printf("  expected ");
      print_multiplicity(&exp->multiplicity);
      printf(" times; occurred %zu times;\n", exp->actual_num_calls);
    }
  }

  return OK;
}

static STAT_Val populate_expectation(MOC_Expectation *                  exp,
                                     size_t                             num_modifiers,
                                     const MOC_IMPL_ExpectationModifier modifiers[]) {
  if(exp == NULL) return LOG_STAT(STAT_ERR_ARGS, "exp is NULL");

  if(!STAT_is_OK(DAR_create(&exp->matchers, sizeof(MOC_IMPL_Matcher)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create arg matchers array");
  }
  if(!STAT_is_OK(DAR_create(&exp->setters, sizeof(MOC_IMPL_SetArgPointee)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create arg setters array");
  }

  for(size_t mod_idx = 0; mod_idx < num_modifiers; mod_idx++) {
    const MOC_IMPL_ExpectationModifier * mod = &modifiers[mod_idx];

    switch(mod->type) {
    case MOC_IMPL_EXP_MOD_TYPE_MATCHER:
      if(!STAT_is_OK(DAR_push_back(&exp->matchers, &(mod->as.matcher)))) {
        return LOG_STAT(STAT_ERR_INTERNAL, "failed to add matcher to arg matchers array");
      }
      break;
    case MOC_IMPL_EXP_MOD_MULTIPLICITY: exp->multiplicity = mod->as.multiplicity; break;
    case MOC_IMPL_EXP_MOD_SET_RETURN: exp->return_setter = mod->as.set_return; break;
    case MOC_IMPL_EXP_MOD_SET_ARG_POINTEE:
      if(!STAT_is_OK(DAR_push_back(&exp->setters, &mod->as.set_arg_pointee))) {
        return LOG_STAT(STAT_ERR_INTERNAL,
                        "failed to add arg pointee setter to expectation setters array");
      }
      break;
    }
  }

  return OK;
}

STAT_Val MOC_IMPL_expect_call(const char *                       func_name,
                              const char *                       expect_statement_func_name,
                              int                                expect_statement_line,
                              size_t                             num_modifiers,
                              const MOC_IMPL_ExpectationModifier modifiers[]) {
  if(!MOC_is_registry_initialized()) {
    return LOG_STAT(STAT_ERR_PRECONDITION, "registry not initialized");
  }
  if(func_name == NULL) return LOG_STAT(STAT_ERR_ARGS, "func_name is NULL");
  if(expect_statement_func_name == NULL) {
    return LOG_STAT(STAT_ERR_ARGS, "expect_statement_func_name is NULL");
  }
  if(modifiers == NULL) return LOG_STAT(STAT_ERR_ARGS, "modifiers arr is NULL");

  MOC_Expectation expectation = {
      .func_name        = func_name,
      .src_func         = expect_statement_func_name,
      .src_line         = expect_statement_line,
      .matchers         = {0},
      .setters          = {0},
      .multiplicity     = EXACTLY(1), // default to exactly once, as that is typically what we want
      .actual_num_calls = 0,
      .return_setter    = {0},
  };

  if(!STAT_is_OK(populate_expectation(&expectation, num_modifiers, modifiers)) ||
     !STAT_is_OK(DAR_push_back(expectations(), &expectation))) {
    destroy_expectation(&expectation);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create new expectation in expectations array");
  }

  return OK;
}

static STAT_Val try_match_call(const MOC_Expectation * exp,
                               const char *            func_name,
                               size_t                  num_args,
                               void *                  arg_ptrs[]) {
  if(exp == NULL) return LOG_STAT(STAT_ERR_ARGS, "exp is NULL");
  if(func_name == NULL) return LOG_STAT(STAT_ERR_ARGS, "func_name is NULL");
  if(arg_ptrs == NULL) return LOG_STAT(STAT_ERR_ARGS, "arg_ptrs is NULL");

  for(const MOC_IMPL_Matcher * m = DAR_first(&exp->matchers); m != DAR_end(&exp->matchers); m++) {
    if(m->arg_idx >= num_args) return STAT_OK_FALSE;

    if(m->comp_fn == NULL) {
      return LOG_STAT(STAT_ERR_READ,
                      "comp_fn is NULL for matcher %s",
                      (m->src_str != NULL) ? m->src_str : "???");
    }

    if(m->comp_fn(m->expected_val, arg_ptrs[m->arg_idx]) != 0) return STAT_OK_FALSE;
  }

  return STAT_OK_TRUE;
}

STAT_Val MOC_IMPL_register_made_call(const MOC_Expectation ** matched_exp,
                                     const char *             func_name,
                                     size_t                   num_args,
                                     void *                   arg_ptrs[]) {
  if(!MOC_is_registry_initialized()) {
    return LOG_STAT(STAT_ERR_PRECONDITION, "registry not initialized");
  }
  if(func_name == NULL) return LOG_STAT(STAT_ERR_ARGS, "func_name is NULL");
  if(arg_ptrs == NULL) return LOG_STAT(STAT_ERR_ARGS, "arg_ptrs is NULL");

  for(MOC_Expectation * exp = DAR_first(expectations()); exp != DAR_end(expectations()); exp++) {
    const STAT_Val match_st = try_match_call(exp, func_name, num_args, arg_ptrs);

    if(match_st == STAT_OK_TRUE) {
      exp->actual_num_calls++;

      // TODO set args

      if(matched_exp != NULL) *matched_exp = exp;
      return OK;
    } else if(!STAT_is_OK(match_st)) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failure during matching of expectation");
    }
  }

  // if we get here, no matching expectation was found
  g_registry.num_unexpected_calls++;
  return STAT_OK_NOT_FOUND;
}

STAT_Val MOC_set_return_val_from_expectation(const MOC_Expectation * exp, void * return_val_p) {
  if(exp == NULL) return LOG_STAT(STAT_ERR_ARGS, "exp is NULL");
  if(return_val_p == NULL) return LOG_STAT(STAT_ERR_ARGS, "return_val_p is NULL");
  if(exp->return_setter.set_fn == NULL) return LOG_STAT(STAT_ERR_ARGS, "set_fn is NULL");
  if(exp->return_setter.value_p == NULL) return LOG_STAT(STAT_ERR_ARGS, "value_p is NULL");

  exp->return_setter.set_fn(return_val_p, exp->return_setter.value_p);

  return OK;
}