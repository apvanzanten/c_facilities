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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "test_utils.h"

#include "ringbuffer.h"

static Result tst_create(void) {
  Result          r    = PASS;
  RBUF_RingBuffer buff = {0};

  EXPECT_OK(&r, RBUF_create(&buff, 1, 16));
  EXPECT_TRUE(&r, RBUF_is_initialized(&buff));

  EXPECT_OK(&r, RBUF_destroy(&buff));
  EXPECT_FALSE(&r, RBUF_is_initialized(&buff));

  EXPECT_OK(&r, RBUF_create(&buff, 4, 4));
  EXPECT_TRUE(&r, RBUF_is_initialized(&buff));

  EXPECT_OK(&r, RBUF_destroy(&buff));
  EXPECT_FALSE(&r, RBUF_is_initialized(&buff));

  EXPECT_OK(&r, RBUF_create(&buff, 8, 4));
  EXPECT_TRUE(&r, RBUF_is_initialized(&buff));

  EXPECT_OK(&r, RBUF_destroy(&buff));
  EXPECT_FALSE(&r, RBUF_is_initialized(&buff));

  EXPECT_TRUE(&r, STAT_is_ERR(RBUF_create(&buff, 0, 4)));
  EXPECT_FALSE(&r, RBUF_is_initialized(&buff));

  EXPECT_TRUE(&r, STAT_is_ERR(RBUF_create(&buff, 4, 0)));
  EXPECT_FALSE(&r, RBUF_is_initialized(&buff));

  return r;
}

static Result tst_many_random(void) {
  Result          r    = PASS;
  RBUF_RingBuffer buff = {0};

  DAR_DArray elements = {0};

  srand(time(NULL) + clock());

  const size_t num_elements_per_test = 2500;

  for(size_t element_size = 1; element_size <= 32; element_size *= 2) {
    for(size_t buffer_size = 1; !HAS_FAILED(&r) && (buffer_size <= 64); buffer_size++) {
      printf("testing buffer of size %zu with element of size %zu\n", buffer_size, element_size);

      EXPECT_OK(&r, RBUF_create(&buff, element_size, buffer_size));
      EXPECT_TRUE(&r, RBUF_is_initialized(&buff));
      if(HAS_FAILED(&r)) break;

      EXPECT_OK(&r, DAR_create(&elements, element_size));
      EXPECT_TRUE(&r, DAR_is_initialized(&elements));
      if(HAS_FAILED(&r)) break;

      size_t in_idx  = 0;
      size_t out_idx = 0;

      for(size_t element_idx = 0; !HAS_FAILED(&r) && (element_idx < num_elements_per_test);
          element_idx++) {
        uint8_t element_bytes[element_size] __attribute__((aligned(sizeof(max_align_t))));

        for(size_t byte_idx = 0; byte_idx < element_size; byte_idx++) {
          element_bytes[byte_idx] = (uint8_t)(rand() & 0xffu);
        }

        EXPECT_OK(&r, DAR_push_back(&elements, element_bytes));
        EXPECT_EQ(&r, element_idx + 1, elements.size);
      }

      EXPECT_EQ(&r, num_elements_per_test, elements.size);

      while((out_idx != num_elements_per_test) && !HAS_FAILED(&r)) {

        if(!RBUF_has_space(&buff)) {
          const size_t prev_num_items = RBUF_get_num_items_on_buffer(&buff);

          EXPECT_EQ(&r, STAT_OK_FULL, RBUF_try_push_back(&buff, DAR_get(&elements, 0)));
          EXPECT_EQ(&r, prev_num_items, RBUF_get_num_items_on_buffer(&buff));
        }

        const bool push_new = RBUF_has_space(&buff) && (in_idx != num_elements_per_test) &&
                              ((out_idx == in_idx) || (rand() & 1u));

        if(push_new) {
          EXPECT_OK(&r, RBUF_push_back(&buff, DAR_get(&elements, in_idx)));
          in_idx++;
        } else {
          EXPECT_TRUE(&r, memcmp(DAR_get(&elements, out_idx), RBUF_peek(&buff), element_size) == 0);
          EXPECT_OK(&r, RBUF_pop_front(&buff));
          out_idx++;
        }

        EXPECT_EQ(&r, (in_idx - out_idx), RBUF_get_num_items_on_buffer(&buff));
        EXPECT_EQ(&r, (in_idx == out_idx), RBUF_is_empty(&buff));
        EXPECT_EQ(&r, ((in_idx - out_idx) < buffer_size), RBUF_has_space(&buff));
      }

      EXPECT_OK(&r, DAR_destroy(&elements));
      EXPECT_OK(&r, RBUF_destroy(&buff));
      EXPECT_FALSE(&r, RBUF_is_initialized(&buff));
    }
  }

  return r;
}

int main(void) {
  Test tests[] = {
      tst_create,
      tst_many_random,
  };

  return (run_tests(tests, sizeof(tests) / sizeof(Test)) == PASS) ? 0 : 1;
}
