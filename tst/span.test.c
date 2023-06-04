#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "stat.h"
#include "test_utils.h"

#include "span.h"

#define OK STAT_OK

static Result tst_create_from_cstr(void) {
  Result r = PASS;

  const char str[] = "As I drove away sadly on my motorbike";
  const int  len   = strlen(str);

  const SPN_Span span = SPN_from_cstr(str);
  EXPECT_EQ(&r, str, span.begin);
  EXPECT_EQ(&r, len, span.len);
  EXPECT_EQ(&r, 1, span.element_size);

  return r;
}

static Result tst_get_size_in_bytes(void) {
  Result r = PASS;

  const char data[] = "this will do as a stand-in for some data";

  EXPECT_EQ(&r, 0, SPN_get_size_in_bytes((SPN_Span){0}));
  EXPECT_EQ(&r, 1, SPN_get_size_in_bytes((SPN_Span){data, .len = 1, .element_size = 1}));
  EXPECT_EQ(&r, 10, SPN_get_size_in_bytes((SPN_Span){data, .len = 10, .element_size = 1}));
  EXPECT_EQ(&r, 40, SPN_get_size_in_bytes((SPN_Span){data, .len = 10, .element_size = 4}));

  return r;
}

static Result tst_get_char(void) {
  Result r = PASS;

  const char     str[] = "But this one's jucky on the inside!";
  const SPN_Span span  = SPN_from_cstr(str);

  for(size_t i = 0; i < span.len; i++) {
    EXPECT_EQ(&r, &str[i], SPN_get(span, i));
  }

  return r;
}

static Result tst_get_int(void) {
  Result r = PASS;

  const int      vals[] = {1, 2, 3, 4, 5, 6, 7, 8};
  const SPN_Span span   = {.begin        = vals,
                           .len          = sizeof(vals) / sizeof(int),
                           .element_size = sizeof(int)};

  for(size_t i = 0; i < span.len; i++) {
    EXPECT_EQ(&r, &vals[i], SPN_get(span, i));
  }

  return r;
}

static Result tst_equals(void) {
  Result r = PASS;

  const char str[]       = "we were to understand there would be punch and pie?";
  const char str_copy[]  = "we were to understand there would be punch and pie?";
  const char other_str[] = "Hey, you can't have that! That snorkel's been like a snorkel to me!";

  const double   numbers[]          = {1.0, 1.0, 2.0, 3.0, 5.0, 8.0, 13.0, 21.0};
  const double   numbers_copy[]     = {1.0, 1.0, 2.0, 3.0, 5.0, 8.0, 13.0, 21.0};
  const double   other_numbers[]    = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0};
  const SPN_Span span_numbers       = {.begin        = numbers,
                                       .len          = sizeof(numbers) / sizeof(double),
                                       .element_size = sizeof(double)};
  const SPN_Span span_numbers_copy  = {.begin        = numbers_copy,
                                       .len          = sizeof(numbers_copy) / sizeof(double),
                                       .element_size = sizeof(double)};
  const SPN_Span span_other_numbers = {.begin        = other_numbers,
                                       .len          = sizeof(other_numbers) / sizeof(double),
                                       .element_size = sizeof(double)};

  const bool     flags[]          = {true, false, false, true};
  const bool     flags_copy[]     = {true, false, false, true};
  const bool     other_flags[]    = {true, false, true, true};
  const SPN_Span span_flags       = {.begin        = flags,
                                     .len          = sizeof(flags) / sizeof(bool),
                                     .element_size = sizeof(bool)};
  const SPN_Span span_flags_copy  = {.begin        = flags_copy,
                                     .len          = sizeof(flags_copy) / sizeof(bool),
                                     .element_size = sizeof(bool)};
  const SPN_Span span_other_flags = {.begin        = other_flags,
                                     .len          = sizeof(other_flags) / sizeof(bool),
                                     .element_size = sizeof(bool)};

  EXPECT_TRUE(&r, SPN_equals(SPN_from_cstr(str), SPN_from_cstr(str)));
  EXPECT_TRUE(&r, SPN_equals(SPN_from_cstr(str), SPN_from_cstr(str_copy)));
  EXPECT_FALSE(&r, SPN_equals(SPN_from_cstr(str), SPN_from_cstr(other_str)));
  EXPECT_FALSE(&r, SPN_equals(SPN_from_cstr(str_copy), SPN_from_cstr(other_str)));

  EXPECT_TRUE(&r, SPN_equals(span_numbers, span_numbers));
  EXPECT_TRUE(&r, SPN_equals(span_numbers_copy, span_numbers_copy));
  EXPECT_TRUE(&r, SPN_equals(span_numbers, span_numbers_copy));
  EXPECT_FALSE(&r, SPN_equals(span_numbers, span_other_numbers));
  EXPECT_FALSE(&r, SPN_equals(span_numbers_copy, span_other_numbers));

  EXPECT_TRUE(&r, SPN_equals(span_flags, span_flags));
  EXPECT_TRUE(&r, SPN_equals(span_flags_copy, span_flags_copy));
  EXPECT_TRUE(&r, SPN_equals(span_flags, span_flags_copy));
  EXPECT_FALSE(&r, SPN_equals(span_flags, span_other_flags));
  EXPECT_FALSE(&r, SPN_equals(span_flags_copy, span_other_flags));

  return r;
}

static Result tst_subspan_char(void) {
  Result r = PASS;

  const char str[] = "Don't draw on my planet.";

  const SPN_Span span = SPN_from_cstr(str);

  {
    const SPN_Span subsp = SPN_subspan(span, 0, span.len);
    EXPECT_EQ(&r, span.begin, subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, span.len, subsp.len);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, 0, 0);
    EXPECT_EQ(&r, span.begin, subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 0, subsp.len);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, 5, 0);
    EXPECT_EQ(&r, SPN_get(span, 5), subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 0, subsp.len);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, 6, 4);
    EXPECT_EQ(&r, SPN_get(span, 6), subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 4, subsp.len);
    EXPECT_ARREQ(&r, char, "draw", subsp.begin, 4);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, 11, 2);
    EXPECT_EQ(&r, SPN_get(span, 11), subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 2, subsp.len);
    EXPECT_ARREQ(&r, char, "on", subsp.begin, 2);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, strlen(str) - 2, 2);
    EXPECT_EQ(&r, SPN_get(span, strlen(str) - 2), subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 2, subsp.len);
    EXPECT_ARREQ(&r, char, "t.", subsp.begin, 2);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, strlen(str) - 2, 3);
    EXPECT_EQ(&r, SPN_get(span, strlen(str) - 2), subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 2, subsp.len);
    EXPECT_ARREQ(&r, char, "t.", subsp.begin, 2);
  }

  return r;
}

static Result tst_subspan_double(void) {
  Result r = PASS;

  const double   vals[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
  const SPN_Span span   = {.begin        = vals,
                           .len          = sizeof(vals) / sizeof(double),
                           .element_size = sizeof(double)};

  {
    const SPN_Span subsp = SPN_subspan(span, 0, span.len);
    EXPECT_EQ(&r, span.begin, subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, span.len, subsp.len);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, 0, 0);
    EXPECT_EQ(&r, span.begin, subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 0, subsp.len);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, 5, 0);
    EXPECT_EQ(&r, SPN_get(span, 5), subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 0, subsp.len);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, 6, 3);
    EXPECT_EQ(&r, SPN_get(span, 6), subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 3, subsp.len);
    EXPECT_ARREQ(&r, double, &vals[6], subsp.begin, 3);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, 8, 2);
    EXPECT_EQ(&r, SPN_get(span, 8), subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 2, subsp.len);
    EXPECT_ARREQ(&r, double, &vals[8], subsp.begin, 2);
  }
  {
    const SPN_Span subsp = SPN_subspan(span, 8, 3);
    EXPECT_EQ(&r, SPN_get(span, 8), subsp.begin);
    EXPECT_EQ(&r, span.element_size, subsp.element_size);
    EXPECT_EQ(&r, 2, subsp.len);
    EXPECT_ARREQ(&r, double, &vals[8], subsp.begin, 2);
  }

  return r;
}

static Result tst_constains_subspan_cstr(void) {
  Result r = PASS;

  EXPECT_TRUE(&r, SPN_contains_subspan(SPN_from_cstr("hi"), SPN_from_cstr("hi")));
  EXPECT_TRUE(&r, SPN_contains_subspan(SPN_from_cstr("hi"), SPN_from_cstr("h")));
  EXPECT_FALSE(&r, SPN_contains_subspan(SPN_from_cstr("hey"), SPN_from_cstr("hi")));
  EXPECT_TRUE(&r, SPN_contains_subspan(SPN_from_cstr("hey"), SPN_from_cstr("h")));
  EXPECT_FALSE(&r, SPN_contains_subspan(SPN_from_cstr("hey"), SPN_from_cstr("i")));

  EXPECT_TRUE(&r,
              SPN_contains_subspan(SPN_from_cstr("a somewhat longer string"),
                                   SPN_from_cstr("a somewhat longer string")));
  EXPECT_TRUE(&r,
              SPN_contains_subspan(SPN_from_cstr("a somewhat longer string"),
                                   SPN_from_cstr("string")));
  EXPECT_TRUE(&r,
              SPN_contains_subspan(SPN_from_cstr("a somewhat longer string"),
                                   SPN_from_cstr("longer")));
  EXPECT_TRUE(&r,
              SPN_contains_subspan(SPN_from_cstr("a somewhat longer string"),
                                   SPN_from_cstr("somewhat longer string")));

  EXPECT_FALSE(&r,
               SPN_contains_subspan(SPN_from_cstr("a somewhat longer string"),
                                    SPN_from_cstr("a sumwot lahnger strung")));
  EXPECT_FALSE(&r,
               SPN_contains_subspan(SPN_from_cstr("a somewhat longer string"),
                                    SPN_from_cstr("Slartibartfast")));
  EXPECT_FALSE(&r,
               SPN_contains_subspan(SPN_from_cstr("a somewhat longer string"),
                                    SPN_from_cstr("96")));

  return r;
}

static Result tst_constains_subspan_int(void) {
  Result r = PASS;

  const int      span_vals[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  const SPN_Span span        = {.begin        = span_vals,
                                .len          = sizeof(span_vals) / sizeof(int),
                                .element_size = sizeof(int)};

  {
    const int      subspan_vals[] = {0, 1, 2};
    const SPN_Span subspan        = {.begin        = subspan_vals,
                                     .len          = sizeof(subspan_vals) / sizeof(int),
                                     .element_size = sizeof(int)};
    EXPECT_TRUE(&r, SPN_contains_subspan(span, subspan));
  }
  {
    const int      subspan_vals[] = {4, 5, 6, 7, 8};
    const SPN_Span subspan        = {.begin        = subspan_vals,
                                     .len          = sizeof(subspan_vals) / sizeof(int),
                                     .element_size = sizeof(int)};
    EXPECT_TRUE(&r, SPN_contains_subspan(span, subspan));
  }
  {
    const SPN_Span subspan = {.begin = span_vals, .len = 0, .element_size = sizeof(int)};
    EXPECT_TRUE(&r, SPN_contains_subspan(span, subspan));
  }
  {
    const int      subspan_vals[] = {9, 10, 11, 12, 13, 14, 15};
    const SPN_Span subspan        = {.begin        = subspan_vals,
                                     .len          = sizeof(subspan_vals) / sizeof(int),
                                     .element_size = sizeof(int)};
    EXPECT_TRUE(&r, SPN_contains_subspan(span, subspan));
  }
  {
    const int      subspan_vals[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    const SPN_Span subspan        = {.begin        = subspan_vals,
                                     .len          = sizeof(subspan_vals) / sizeof(int),
                                     .element_size = sizeof(int)};
    EXPECT_TRUE(&r, SPN_contains_subspan(span, subspan));
  }
  {
    const int      subspan_vals[] = {0, 1, 3};
    const SPN_Span subspan        = {.begin        = subspan_vals,
                                     .len          = sizeof(subspan_vals) / sizeof(int),
                                     .element_size = sizeof(int)};
    EXPECT_FALSE(&r, SPN_contains_subspan(span, subspan));
  }
  {
    const int      subspan_vals[] = {0, 1, 2, 3, -1};
    const SPN_Span subspan        = {.begin        = subspan_vals,
                                     .len          = sizeof(subspan_vals) / sizeof(int),
                                     .element_size = sizeof(int)};
    EXPECT_FALSE(&r, SPN_contains_subspan(span, subspan));
  }
  {
    const int      subspan_vals[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    const SPN_Span subspan        = {.begin        = subspan_vals,
                                     .len          = sizeof(subspan_vals) / sizeof(int),
                                     .element_size = sizeof(int)};
    EXPECT_FALSE(&r, SPN_contains_subspan(span, subspan));
  }
  {
    const uint64_t subspan_vals[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    const SPN_Span subspan        = {.begin        = subspan_vals,
                                     .len          = sizeof(subspan_vals) / sizeof(uint64_t),
                                     .element_size = sizeof(uint64_t)};
    EXPECT_FALSE(&r, SPN_contains_subspan(span, subspan));
  }

  return r;
}

static Result tst_find_char(void) {
  Result r = PASS;

  const char   str[] = "0123456789abcdefghijklmnopqrstuvwxyz"; // should have unique letters only
  const size_t len   = strlen(str);

  for(size_t idx = 0; idx < len; idx++) {
    {
      size_t tmp = 9999;
      EXPECT_EQ(&r, OK, SPN_find(SPN_from_cstr(str), &(str[idx]), &tmp));
      EXPECT_EQ(&r, idx, tmp);
    }
    {
      size_t tmp = 9999;
      EXPECT_EQ(&r, OK, SPN_find_reverse(SPN_from_cstr(str), &(str[idx]), &tmp));
      EXPECT_EQ(&r, idx, tmp);
    }
    if(HAS_FAILED(&r)) return r;
  }

  {
    size_t tmp = 9999;
    EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find(SPN_from_cstr(str), "A", &tmp));
    EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find(SPN_from_cstr(str), ";", &tmp));
    EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find(SPN_from_cstr(str), "&", &tmp));
    EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find(SPN_from_cstr(str), "*", &tmp));
    EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find_reverse(SPN_from_cstr(str), "A", &tmp));
    EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find_reverse(SPN_from_cstr(str), ";", &tmp));
    EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find_reverse(SPN_from_cstr(str), "&", &tmp));
    EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find_reverse(SPN_from_cstr(str), "*", &tmp));
  }

  for(size_t at_idx = 0; at_idx < len; at_idx++) {
    for(size_t idx = 0; idx < len; idx++) {
      {
        size_t         tmp = 9999;
        const STAT_Val st  = SPN_find_at(SPN_from_cstr(str), &(str[idx]), at_idx, &tmp);
        if(idx >= at_idx) {
          EXPECT_EQ(&r, OK, st);
          EXPECT_EQ(&r, idx, tmp);
        } else {
          EXPECT_EQ(&r, STAT_OK_NOT_FOUND, st);
        }
      }
      {
        size_t         tmp = 9999;
        const STAT_Val st  = SPN_find_reverse_at(SPN_from_cstr(str), &(str[idx]), at_idx, &tmp);
        if(idx <= at_idx) {
          EXPECT_EQ(&r, OK, st);
          EXPECT_EQ(&r, idx, tmp);
        } else {
          EXPECT_EQ(&r, STAT_OK_NOT_FOUND, st);
        }
      }
      if(HAS_FAILED(&r)) return r;
    }
  }

  return r;
}

static Result tst_find_int(void) {
  Result r = PASS;

  const int      vals[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  const SPN_Span span   = {.begin        = vals,
                           .len          = sizeof(vals) / sizeof(int),
                           .element_size = sizeof(int)};

  for(size_t idx = 0; idx < span.len; idx++) {
    {
      size_t tmp = 9999;
      EXPECT_EQ(&r, OK, SPN_find(span, SPN_get(span, idx), &tmp));
      EXPECT_EQ(&r, idx, tmp);
    }
    {
      size_t tmp = 9999;
      EXPECT_EQ(&r, OK, SPN_find_reverse(span, SPN_get(span, idx), &tmp));
      EXPECT_EQ(&r, idx, tmp);
    }
    if(HAS_FAILED(&r)) return r;
  }

  {
    size_t tmp = 9999;
    {
      const int to_find = 16;
      EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find(span, &to_find, &tmp));
      EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find_reverse(span, &to_find, &tmp));
    }
    {
      const int to_find = -5;
      EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find(span, &to_find, &tmp));
      EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find_reverse(span, &to_find, &tmp));
    }
    {
      const int to_find = 9001;
      EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find(span, &to_find, &tmp));
      EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find_reverse(span, &to_find, &tmp));
    }
    {
      const int to_find = -1;
      EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find(span, &to_find, &tmp));
      EXPECT_EQ(&r, STAT_OK_NOT_FOUND, SPN_find_reverse(span, &to_find, &tmp));
    }
  }

  for(size_t at_idx = 0; at_idx < span.len; at_idx++) {
    for(size_t idx = 0; idx < span.len; idx++) {
      {
        size_t         tmp = 9999;
        const STAT_Val st  = SPN_find_at(span, SPN_get(span, idx), at_idx, &tmp);

        if(idx >= at_idx) {
          EXPECT_EQ(&r, OK, st);
          EXPECT_EQ(&r, idx, tmp);
        } else {
          EXPECT_EQ(&r, STAT_OK_NOT_FOUND, st);
        }
      }
      {
        size_t         tmp = 9999;
        const STAT_Val st  = SPN_find_reverse_at(span, SPN_get(span, idx), at_idx, &tmp);

        if(idx <= at_idx) {
          EXPECT_EQ(&r, OK, st);
          EXPECT_EQ(&r, idx, tmp);
        } else {
          EXPECT_EQ(&r, STAT_OK_NOT_FOUND, st);
        }
      }
      if(HAS_FAILED(&r)) return r;
    }
  }

  return r;
}

static Result tst_find_at_and_reverse_with_duplicates(void) {
  Result r = PASS;

  const char str[] = "01234567890123456789";

  {
    size_t tmp = 9999;
    EXPECT_EQ(&r, OK, SPN_find(SPN_from_cstr(str), "0", &tmp));
    EXPECT_EQ(&r, 0, tmp); // finds only the first
    EXPECT_EQ(&r, OK, SPN_find_at(SPN_from_cstr(str), "0", 1, &tmp));
    EXPECT_EQ(&r, 10, tmp); // finds only the second because we skipped over the first
    EXPECT_EQ(&r, OK, SPN_find_reverse_at(SPN_from_cstr(str), "0", 9, &tmp));
    EXPECT_EQ(&r, 0, tmp); // finds only the first because we search in reverse and skip the back
    EXPECT_EQ(&r, OK, SPN_find_reverse(SPN_from_cstr(str), "0", &tmp));
    EXPECT_EQ(&r, 10, tmp); // finds only the second because we search from behind
  }

  {
    size_t tmp = 9999;
    EXPECT_EQ(&r, OK, SPN_find(SPN_from_cstr(str), "5", &tmp));
    EXPECT_EQ(&r, 5, tmp); // finds only the first
    EXPECT_EQ(&r, OK, SPN_find_at(SPN_from_cstr(str), "5", 6, &tmp));
    EXPECT_EQ(&r, 15, tmp); // finds only the second because we skipped over the first
    EXPECT_EQ(&r, OK, SPN_find_reverse_at(SPN_from_cstr(str), "5", 9, &tmp));
    EXPECT_EQ(&r, 5, tmp); // finds only the first because we search in reverse and skip the back
    EXPECT_EQ(&r, OK, SPN_find_reverse(SPN_from_cstr(str), "5", &tmp));
    EXPECT_EQ(&r, 15, tmp); // finds only the second because we search from behind
  }

  return r;
}

static Result tst_find_at_likely_usage(void) {
  Result r = PASS;

  // Let's face it, the most likely use case for a function like this (or indeed any function), is
  // finding (more) llamas.

  const char str[] = "Here's a llama, there's a llama, and another little llama.\n"
                     "Fuzzy llama, funny llama, llama llama, duck."; // credit to Burton Earny

  const SPN_Span span  = SPN_from_cstr(str);
  const SPN_Span llama = SPN_from_cstr("llama");

  const uint32_t expected_llamas[] = {9, 26, 52, 59 + 6, 59 + 19, 59 + 26, 59 + 32};
  const size_t   num_llamas = sizeof(expected_llamas) / sizeof(uint32_t); // llamas are 32 bit?

  {
    size_t at_idx = 0;
    for(size_t llama_idx = 0; llama_idx < num_llamas; llama_idx++) {
      size_t tmp = 9999;
      EXPECT_EQ(&r, OK, SPN_find_subspan_at(span, llama, at_idx, &tmp));
      EXPECT_EQ(&r, expected_llamas[llama_idx], tmp);
      at_idx = tmp + 1;

      if(HAS_FAILED(&r)) return r;
    }
  }

  { // same but in reverse (we count our llamas twice, as each time brings us joy)
    size_t at_idx = span.len - 1;
    for(int llama_idx = num_llamas - 1; llama_idx >= 0; llama_idx--) {
      size_t tmp = 9999;
      EXPECT_EQ(&r, OK, SPN_find_subspan_reverse_at(span, llama, at_idx, &tmp));
      EXPECT_EQ(&r, expected_llamas[llama_idx], tmp);
      at_idx = tmp - 1;

      if(HAS_FAILED(&r)) return r;
    }
  }

  return r;
}

static Result tst_find_subspan_basic(void) {
  Result r = PASS;

  size_t tmp = 9999;
  EXPECT_EQ(&r, OK, SPN_find_subspan(SPN_from_cstr("012345"), SPN_from_cstr("012345"), &tmp));
  EXPECT_EQ(&r, 0, tmp);

  EXPECT_EQ(&r, OK, SPN_find_subspan(SPN_from_cstr("012345"), SPN_from_cstr("123"), &tmp));
  EXPECT_EQ(&r, 1, tmp);

  EXPECT_EQ(&r,
            OK,
            SPN_find_subspan_reverse(SPN_from_cstr("012345"), SPN_from_cstr("012345"), &tmp));
  EXPECT_EQ(&r, 0, tmp);

  EXPECT_EQ(&r, OK, SPN_find_subspan_reverse(SPN_from_cstr("012345"), SPN_from_cstr("123"), &tmp));
  EXPECT_EQ(&r, 1, tmp);

  EXPECT_EQ(&r, OK, SPN_find_subspan(SPN_from_cstr("012345"), SPN_from_cstr(""), &tmp));
  EXPECT_EQ(&r, 0, tmp);

  EXPECT_EQ(&r, OK, SPN_find_subspan_reverse(SPN_from_cstr("012345"), SPN_from_cstr(""), &tmp));
  EXPECT_EQ(&r, 5, tmp);

  EXPECT_EQ(&r,
            STAT_OK_NOT_FOUND,
            SPN_find_subspan(SPN_from_cstr("012"), SPN_from_cstr("123"), &tmp));
  EXPECT_EQ(&r,
            STAT_OK_NOT_FOUND,
            SPN_find_subspan_reverse(SPN_from_cstr("012"), SPN_from_cstr("123"), &tmp));

  return r;
}

static Result tst_find_subspan_at_basic(void) {
  Result r = PASS;

  size_t tmp = 9999;

  EXPECT_EQ(&r, OK, SPN_find_subspan_at(SPN_from_cstr("012345"), SPN_from_cstr(""), 1, &tmp));
  EXPECT_EQ(&r, 1, tmp);

  EXPECT_EQ(&r, OK, SPN_find_subspan_at(SPN_from_cstr("012345"), SPN_from_cstr("234"), 0, &tmp));
  EXPECT_EQ(&r, 2, tmp);

  EXPECT_EQ(&r, OK, SPN_find_subspan_at(SPN_from_cstr("0"), SPN_from_cstr("0"), 0, &tmp));
  EXPECT_EQ(&r, 0, tmp);

  EXPECT_EQ(&r,
            OK,
            SPN_find_subspan_reverse_at(SPN_from_cstr("012345"), SPN_from_cstr(""), 1, &tmp));
  EXPECT_EQ(&r, 1, tmp);

  EXPECT_EQ(&r,
            OK,
            SPN_find_subspan_reverse_at(SPN_from_cstr("012345"), SPN_from_cstr("234"), 5, &tmp));
  EXPECT_EQ(&r, 2, tmp);

  EXPECT_EQ(&r, OK, SPN_find_subspan_reverse_at(SPN_from_cstr("0"), SPN_from_cstr("0"), 0, &tmp));
  EXPECT_EQ(&r, 0, tmp);

  EXPECT_EQ(&r, OK, SPN_find_subspan_at(SPN_from_cstr("012012"), SPN_from_cstr("1"), 0, &tmp));
  EXPECT_EQ(&r, 1, tmp);
  EXPECT_EQ(&r, OK, SPN_find_subspan_at(SPN_from_cstr("012012"), SPN_from_cstr("1"), 2, &tmp));
  EXPECT_EQ(&r, 4, tmp);

  EXPECT_EQ(&r,
            OK,
            SPN_find_subspan_reverse_at(SPN_from_cstr("012012"), SPN_from_cstr("1"), 5, &tmp));
  EXPECT_EQ(&r, 4, tmp);
  EXPECT_EQ(&r,
            OK,
            SPN_find_subspan_reverse_at(SPN_from_cstr("012012"), SPN_from_cstr("1"), 2, &tmp));
  EXPECT_EQ(&r, 1, tmp);

  return r;
}

static Result tst_find_subspan_monster(void) {
  Result r = PASS;

  // this monstrous test case does the following
  //  set up array of values, with a unique value on each element
  //  for all possible spans on vals array
  //    for all possible subspans on vals array
  //      try to find subspan in span, and check result
  //      try to find subspan in reverse in span, and check result
  //      for all elements in span
  //        try to find subspan in span starting at element, and check result
  //        try to find subspan in reverse in span starting at element, and check result

  // NOTE I wouldn't generally recommend writing something like this, but I thought it was fun :)
  // NOTE Obvious gap in this test: duplicates, that is, we never have multiple matching sequences
  //      and then check which one we find based on the 'at' idx and 'reverse'. This gap is covered
  //      by other tests (llamas were involved).

  uint32_t     vals[16] = {0};
  const size_t len      = sizeof(vals) / sizeof(int);
  for(size_t i = 0; i < len; i++) {
    vals[i] = (2 * i) + 1; // math just to make it distinct from indices
  }

  for(size_t span_start_idx = 0; span_start_idx < len; span_start_idx++) {
    printf("span_start_idx: %zu\n", span_start_idx);

    for(size_t span_len = 0; span_len < (len - span_start_idx); span_len++) {

      const SPN_Span span = {.begin        = &vals[span_start_idx],
                             .len          = span_len,
                             .element_size = sizeof(int)};

      for(size_t subspan_start_idx = 0; subspan_start_idx < len; subspan_start_idx++) {
        for(size_t subspan_len = 0; subspan_len < (len - subspan_start_idx); subspan_len++) {

          const SPN_Span subspan = {.begin        = &vals[subspan_start_idx],
                                    .len          = subspan_len,
                                    .element_size = sizeof(int)};

          const bool subspan_is_empty           = (subspan.len == 0);
          const bool subspan_starts_before_span = (subspan_start_idx < span_start_idx);
          const bool subspan_ends_after_span =
              (subspan_start_idx + subspan.len) > (span_start_idx + span.len);
          const bool subspan_is_in_span = (!subspan_starts_before_span && !subspan_ends_after_span);

          size_t find_idx = 9999;

          {
            const STAT_Val st = SPN_find_subspan(span, subspan, &find_idx);

            if(subspan_is_empty) {
              EXPECT_EQ(&r, OK, st);
              EXPECT_EQ(&r, 0, find_idx);
            } else if(subspan_is_in_span) {
              EXPECT_EQ(&r, OK, st);
              EXPECT_EQ(&r, (subspan_start_idx - span_start_idx), find_idx);
            } else {
              EXPECT_EQ(&r, STAT_OK_NOT_FOUND, st);
            }
          }

          if(!HAS_FAILED(&r)) {
            const STAT_Val st = SPN_find_subspan_reverse(span, subspan, &find_idx);

            if(subspan_is_empty) {
              EXPECT_EQ(&r, OK, st);
              EXPECT_EQ(&r, (span_len > 0 ? (span.len - 1) : 0), find_idx);
            } else if(subspan_is_in_span) {
              EXPECT_EQ(&r, OK, st);
              EXPECT_EQ(&r, (subspan_start_idx - span_start_idx), find_idx);
            } else {
              EXPECT_EQ(&r, STAT_OK_NOT_FOUND, st);
            }
          }

          for(size_t at_idx = 0; !HAS_FAILED(&r) && at_idx < span.len; at_idx++) {
            const STAT_Val st = SPN_find_subspan_at(span, subspan, at_idx, &find_idx);

            const bool subspan_starts_before_at_idx = subspan_start_idx < (span_start_idx + at_idx);

            if(subspan_is_empty) {
              EXPECT_EQ(&r, OK, st);
              EXPECT_EQ(&r, at_idx, find_idx);
            } else if(subspan_is_in_span && !subspan_starts_before_at_idx) {
              EXPECT_EQ(&r, OK, st);
              EXPECT_EQ(&r, (subspan_start_idx - span_start_idx), find_idx);
            } else {
              EXPECT_EQ(&r, STAT_OK_NOT_FOUND, st);
            }

            if(HAS_FAILED(&r)) {
              printf("\tat_idx: %zu\n", at_idx);
              printf("\tsubspan_starts_before_at_idx: %u\n", subspan_starts_before_at_idx);
            }
          }

          for(size_t at_idx = 0; !HAS_FAILED(&r) && at_idx < span.len; at_idx++) {
            const STAT_Val st = SPN_find_subspan_reverse_at(span, subspan, at_idx, &find_idx);

            const bool subspan_starts_after_at_idx = subspan_start_idx > (span_start_idx + at_idx);

            if(subspan_is_empty) {
              EXPECT_EQ(&r, OK, st);
              EXPECT_EQ(&r, at_idx, find_idx);
            } else if(subspan_is_in_span && !subspan_starts_after_at_idx) {
              EXPECT_EQ(&r, OK, st);
              EXPECT_EQ(&r, (subspan_start_idx - span_start_idx), find_idx);
            } else {
              EXPECT_EQ(&r, STAT_OK_NOT_FOUND, st);
            }

            if(HAS_FAILED(&r)) {
              printf("\tat_idx: %zu\n", at_idx);
              printf("\tsubspan_starts_after_at_idx: %u\n", subspan_starts_after_at_idx);
            }
          }

          if(HAS_FAILED(&r)) {
            printf("\tspan_start_idx: %zu\n", span_start_idx);
            printf("\tsubspan_start_idx: %zu\n", subspan_start_idx);
            printf("\tspan.len: %zu\n", span.len);
            printf("\tsubspan.len: %zu\n", subspan.len);
            printf("\tsubspan_is_empty: %i\n", subspan_is_empty);
            printf("\tsubspan_starts_before_span: %i\n", subspan_starts_before_span);
            printf("\tsubspan_ends_after_span: %i\n", subspan_ends_after_span);
            printf("\tsubspan_is_in_span: %i\n", subspan_is_in_span);
            printf("\tfind_idx: %zu\n", find_idx);

            return r;
          }
        }
      }
    }
  }

  return r;
}

typedef struct {
  double  numbers[1000];
  uint8_t bytes[10000];
} BigStruct;

static Result tst_large_elements(void) {
  Result r = PASS;

  BigStruct    elements[16] = {0};
  const size_t element_size = sizeof(BigStruct);
  const size_t len          = sizeof(elements) / sizeof(BigStruct);
  for(size_t i = 0; i < len; i++) {
    for(size_t j = 0; j < sizeof(elements[i].numbers) / sizeof(double); j++) {
      elements[i].numbers[j] = (double)i * (double)j;
    }
    for(size_t j = 0; j < sizeof(elements[i].bytes) / sizeof(uint8_t); j++) {
      elements[i].bytes[j] = i * j;
    }
  }

  const SPN_Span span = {.begin = elements, .element_size = element_size, .len = len};

  for(size_t i = 0; i < len; i++) {
    size_t idx = 0;
    EXPECT_EQ(&r, OK, SPN_find(span, &elements[i], &idx));
    EXPECT_EQ(&r, i, idx);
  }
  for(size_t i = 0; i < len; i++) {
    size_t idx = 0;
    EXPECT_EQ(&r, OK, SPN_find_reverse(span, &elements[i], &idx));
    EXPECT_EQ(&r, i, idx);
  }

  return r;
}

static Result tst_get_first_last_end_cstr(void) {
  Result r = PASS;

  const char str[] = "Wooh! What a scene, huh?";

  SPN_Span span = SPN_from_cstr(str);

  size_t i = 0;
  for(const char * p = SPN_first(span); p != SPN_end(span); p++) {
    EXPECT_EQ(&r, &str[i], p);
    EXPECT_EQ(&r, &str[i], SPN_get(span, i));
    i++;
  }
  EXPECT_EQ(&r, &str[strlen(str) - 1], (const char *)SPN_last(span));

  return r;
}

static Result tst_get_first_last_end_ints(void) {
  Result r = PASS;

  const int seq[] = {2, 3, 5, 7, 11, 13, 17, 19, 23};

  SPN_Span span = {.begin        = seq,
                   .element_size = sizeof(seq[0]),
                   .len          = sizeof(seq) / sizeof(seq[0])};

  size_t i = 0;
  for(const int * p = SPN_first(span); p != SPN_end(span); p++) {
    EXPECT_EQ(&r, &seq[i], p);
    EXPECT_EQ(&r, &seq[i], SPN_get(span, i));
    i++;
  }
  EXPECT_EQ(&r, &seq[span.len - 1], (const int *)SPN_last(span));

  return r;
}

int main(void) {
  Test tests[] = {
      tst_create_from_cstr,
      tst_get_size_in_bytes,
      tst_get_char,
      tst_get_int,
      tst_equals,
      tst_subspan_char,
      tst_subspan_double,
      tst_constains_subspan_cstr,
      tst_constains_subspan_int,
      tst_find_char,
      tst_find_int,
      tst_find_at_and_reverse_with_duplicates,
      tst_find_at_likely_usage,
      tst_find_subspan_basic,
      tst_find_subspan_at_basic,
      tst_find_subspan_monster,
      tst_large_elements,
      tst_get_first_last_end_cstr,
      tst_get_first_last_end_ints,
  };

  return (run_tests(tests, sizeof(tests) / sizeof(Test)) == PASS) ? 0 : 1;
}