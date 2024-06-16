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

#include "test_utils.h"

#include "stat.h"

static Result tst_ranges_are_positive(void) {
  Result r = PASS;

  EXPECT_TRUE(&r, STAT_INT_OK_RANGE_FIRST <= STAT_INT_OK_RANGE_LAST);
  EXPECT_TRUE(&r, STAT_INT_WRN_RANGE_FIRST <= STAT_INT_WRN_RANGE_LAST);
  EXPECT_TRUE(&r, STAT_INT_ERR_RANGE_FIRST <= STAT_INT_ERR_RANGE_LAST);

  return r;
}

static Result tst_ranges_are_disjoint(void) {
  Result r = PASS;

  EXPECT_TRUE(&r, STAT_INT_OK_RANGE_LAST < STAT_INT_WRN_RANGE_FIRST);
  EXPECT_TRUE(&r, STAT_INT_WRN_RANGE_LAST < STAT_INT_ERR_RANGE_FIRST);

  return r;
}

static Result tst_enum_storage_size(void) {
  Result r = PASS;

  EXPECT_TRUE(&r, sizeof(STAT_Val) <= sizeof(int));

  return r;
}

static Result tst_is_OK_is_WRN_is_ERR(void) {
  Result r = PASS;

  EXPECT_TRUE(&r, STAT_is_OK(STAT_OK));
  EXPECT_TRUE(&r, STAT_is_OK(STAT_OK_BUSY));
  EXPECT_TRUE(&r, STAT_is_OK(STAT_OK_FALSE));
  EXPECT_FALSE(&r, STAT_is_OK(STAT_WRN_OVERWRITTEN));
  EXPECT_FALSE(&r, STAT_is_OK(STAT_ERR_ALLOC));
  EXPECT_FALSE(&r, STAT_is_OK(STAT_ERR_ARGS));

  EXPECT_TRUE(&r, STAT_is_WRN(STAT_WRN_OVERWRITTEN));
  EXPECT_FALSE(&r, STAT_is_WRN(STAT_OK));
  EXPECT_FALSE(&r, STAT_is_WRN(STAT_ERR_COMPILE));

  EXPECT_TRUE(&r, STAT_is_ERR(STAT_ERR_ALLOC));
  EXPECT_TRUE(&r, STAT_is_ERR(STAT_ERR_ARGS));
  EXPECT_TRUE(&r, STAT_is_ERR(STAT_ERR_INTERNAL));
  EXPECT_FALSE(&r, STAT_is_ERR(STAT_OK));
  EXPECT_FALSE(&r, STAT_is_ERR(STAT_OK_FINISHED));
  EXPECT_FALSE(&r, STAT_is_ERR(STAT_WRN_OVERWRITTEN));

  return r;
}

static Result tst_is_valid(void) {
  Result r = PASS;

  EXPECT_TRUE(&r, STAT_is_valid(STAT_OK));
  EXPECT_TRUE(&r, STAT_is_valid(STAT_OK_BUSY));
  EXPECT_TRUE(&r, STAT_is_valid(STAT_WRN_OVERWRITTEN));
  EXPECT_TRUE(&r, STAT_is_valid(STAT_ERR_COMPILE));
  EXPECT_TRUE(&r, STAT_is_valid(STAT_ERR_NOT_FOUND));

  EXPECT_FALSE(&r, STAT_is_valid(0));
  EXPECT_FALSE(&r, STAT_is_valid(12));
  EXPECT_FALSE(&r, STAT_is_valid(23));
  EXPECT_FALSE(&r, STAT_is_valid((int)STAT_OK - 1));

  return r;
}

static Result tst_to_str(void) {
  Result r = PASS;

#define CHECK(val) EXPECT_STREQ(&r, #val, STAT_to_str(val))

  CHECK(STAT_OK);
  CHECK(STAT_OK_INFO);
  CHECK(STAT_OK_BUSY);
  CHECK(STAT_OK_FINISHED);
  CHECK(STAT_OK_TRUE);
  CHECK(STAT_OK_FALSE);
  CHECK(STAT_OK_NOT_FOUND);
  CHECK(STAT_OK_FULL);
  CHECK(STAT_OK_EMPTY);
  CHECK(STAT_WRN_OVERWRITTEN);
  CHECK(STAT_WRN_NOTHING_TO_DO);
  CHECK(STAT_ERR_ARGS);
  CHECK(STAT_ERR_ASSERTION);
  CHECK(STAT_ERR_USAGE);
  CHECK(STAT_ERR_UNIMPLEMENTED);
  CHECK(STAT_ERR_PRECONDITION);
  CHECK(STAT_ERR_RANGE);
  CHECK(STAT_ERR_EMPTY);
  CHECK(STAT_ERR_FULL);
  CHECK(STAT_ERR_INTERNAL);
  CHECK(STAT_ERR_FATAL);
  CHECK(STAT_ERR_IO);
  CHECK(STAT_ERR_READ);
  CHECK(STAT_ERR_WRITE);
  CHECK(STAT_ERR_ALLOC);
  CHECK(STAT_ERR_NOT_FOUND);
  CHECK(STAT_ERR_DUPLICATE);
  CHECK(STAT_ERR_PARSE);
  CHECK(STAT_ERR_COMPILE);
  CHECK(STAT_ERR_RUNTIME);

#undef CHECK

  return r;
}

int main(void) {
  Test tests[] = {
      tst_ranges_are_positive,
      tst_ranges_are_disjoint,
      tst_enum_storage_size,
      tst_is_OK_is_WRN_is_ERR,
      tst_is_valid,
      tst_to_str,
  };

  return (run_tests(tests, sizeof(tests) / sizeof(Test)) == PASS) ? 0 : 1;
}
