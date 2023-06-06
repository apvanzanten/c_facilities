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

#include "test_utils.h"

#include "refcount.h"

static Result tst_basic_usecase() {
  Result r = PASS;

  RC_Ref ref = RC_allocate(sizeof(int));

  EXPECT_NE(&r, NULL, ref.block);
  EXPECT_EQ(&r, 1, RC_get_ref_count(ref));
  if(HAS_FAILED(&r)) return r;

  RC_Ref other_ref = RC_copy(ref);

  EXPECT_EQ(&r, ref.block, other_ref.block);
  EXPECT_EQ(&r, RC_get(ref), RC_get(other_ref));
  EXPECT_EQ(&r, 2, RC_get_ref_count(ref));
  EXPECT_EQ(&r, 2, RC_get_ref_count(other_ref));

  RC_release(other_ref);

  EXPECT_EQ(&r, 1, RC_get_ref_count(ref));

  RC_release(ref);

  return r;
}

static Result tst_with_const() {
  Result r = PASS;

  RC_Ref ref = RC_allocate(sizeof(int));

  EXPECT_NE(&r, NULL, ref.block);
  EXPECT_EQ(&r, 1, RC_get_ref_count(ref));
  if(HAS_FAILED(&r)) return r;

  RC_ConstRef const_ref = RC_copy(RC_as_const(ref));

  EXPECT_EQ(&r, ref.block, const_ref.block);
  EXPECT_EQ(&r, RC_get(ref), RC_get(const_ref));
  EXPECT_EQ(&r, 2, RC_get_ref_count(ref));
  EXPECT_EQ(&r, 2, RC_get_ref_count(const_ref));

  RC_release(const_ref);

  EXPECT_EQ(&r, 1, RC_get_ref_count(ref));

  RC_release(ref);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_basic_usecase,
      tst_with_const,
  };

  return (run_tests(tests, sizeof(tests) / sizeof(Test)) == PASS) ? 0 : 1;
}