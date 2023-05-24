#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "stat.h"
#include "test_utils.h"

#include "span.h"

#define OK STAT_OK

Result tst_create_from_cstr() {
  Result r = PASS;

  const char str[] = "As I drove away sadly on my motorbike";
  const int  len   = strlen(str);

  const SPN_Span span = SPN_from_cstr(str);
  EXPECT_EQ(&r, str, span.begin);
  EXPECT_EQ(&r, len, span.len);
  EXPECT_EQ(&r, 1, span.element_size);

  return r;
}

Result tst_get_size_in_bytes() {
  Result r = PASS;

  const char data[] = "this will do as a stand-in for some data";

  EXPECT_EQ(&r, 0, SPN_get_size_in_bytes((SPN_Span){0}));
  EXPECT_EQ(&r, 1, SPN_get_size_in_bytes((SPN_Span){data, .len = 1, .element_size = 1}));
  EXPECT_EQ(&r, 10, SPN_get_size_in_bytes((SPN_Span){data, .len = 10, .element_size = 1}));
  EXPECT_EQ(&r, 40, SPN_get_size_in_bytes((SPN_Span){data, .len = 10, .element_size = 4}));

  return r;
}

Result tst_get_char() {
  Result r = PASS;

  const char     str[] = "But this one's jucky on the inside!";
  const SPN_Span span  = SPN_from_cstr(str);

  for(uint32_t i = 0; i < span.len; i++) {
    EXPECT_EQ(&r, &str[i], SPN_get(span, i));
  }

  return r;
}

Result tst_get_int() {
  Result r = PASS;

  const int      vals[] = {1, 2, 3, 4, 5, 6, 7, 8};
  const SPN_Span span   = {.begin        = vals,
                           .len          = sizeof(vals) / sizeof(int),
                           .element_size = sizeof(int)};

  for(uint32_t i = 0; i < span.len; i++) {
    EXPECT_EQ(&r, &vals[i], SPN_get(span, i));
  }

  return r;
}

Result tst_equals() {
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

Result tst_subspan_char() {
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

Result tst_subspan_double() {
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

Result tst_constains_subspan_cstr() {
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

Result tst_constains_subspan_int() {
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
    const SPN_Span subspan = {.begin = NULL, .len = 0, .element_size = sizeof(int)};
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

int main() {
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
  };

  return (run_tests(tests, sizeof(tests) / sizeof(Test)) == PASS) ? 0 : 1;
}