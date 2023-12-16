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

#ifndef CFAC_BITDARRAY_H
#define CFAC_BITDARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "darray.h"

#include "stat.h"

typedef struct BDAR_BitDArray {
  uint8_t * data;
  size_t    size; // size in bits
  size_t    capacity_in_bytes;
} BDAR_BitDArray;

STAT_Val BDAR_create(BDAR_BitDArray * this);
STAT_Val BDAR_create_from_bool_arr(BDAR_BitDArray * this, const bool * bool_arr, size_t n);
STAT_Val BDAR_create_from_bool_darr(BDAR_BitDArray * this, const DAR_DArray * bool_darr);

STAT_Val BDAR_reserve(BDAR_BitDArray * this, size_t num_bits);
STAT_Val BDAR_resize(BDAR_BitDArray * this, size_t new_size);

STAT_Val BDAR_push_back(BDAR_BitDArray * this, bool val);
STAT_Val BDAR_pop_back(BDAR_BitDArray * this);

STAT_Val BDAR_push_front(BDAR_BitDArray * this, bool val);
STAT_Val BDAR_pop_front(BDAR_BitDArray * this);

STAT_Val               BDAR_set_bit_val(BDAR_BitDArray * this, size_t idx, bool val);
static inline STAT_Val BDAR_set_bit(BDAR_BitDArray * this, size_t idx);
static inline STAT_Val BDAR_clear_bit(BDAR_BitDArray * this, size_t idx);

STAT_Val BDAR_fill_range(BDAR_BitDArray * this, size_t start_idx, size_t n, bool fill_val);
STAT_Val BDAR_fill(BDAR_BitDArray * this, bool fill_val);

// STAT_Val BDAR_shift_left(BDAR_BitDArray * this, size_t num_shift);
// STAT_Val BDAR_shift_right(BDAR_BitDArray * this, size_t num_shift);

STAT_Val BDAR_destroy(BDAR_BitDArray * this);

static inline STAT_Val BDAR_set_bit(BDAR_BitDArray * this, size_t idx) {
  return BDAR_set_bit_val(this, idx, true);
}
static inline STAT_Val BDAR_clear_bit(BDAR_BitDArray * this, size_t idx) {
  return BDAR_set_bit_val(this, idx, false);
}

#endif
