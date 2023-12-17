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

#include "bitdarray.h"

#include "log.h"

#include <stdlib.h>
#include <string.h>

#define OK STAT_OK

#define INIT_CAPACITY 8

static size_t   calc_size_in_bytes(size_t size_in_bits);
static size_t   determine_needed_capacity(size_t size_in_bits, size_t start_capacity);
static STAT_Val grow_capacity_as_needed(BDAR_BitDArray * this, size_t num_bits_to_fit);

STAT_Val BDAR_create(BDAR_BitDArray * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  *this = (BDAR_BitDArray){0};

  return LOG_STAT_IF_ERR(grow_capacity_as_needed(this, 0), "failed to grow initial array memory");
}

STAT_Val BDAR_create_from_bool_arr(BDAR_BitDArray * this, const bool * bool_arr, size_t n) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  *this = (BDAR_BitDArray){0};

  if(BDAR_resize(this, n) != OK) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize to fit bool arr");
  }

  for(size_t i = 0; i < n; i++) {
    if(BDAR_set_bit_val(this, i, bool_arr[i]) != OK) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to set bit val from bool arr at idx %zu", i);
    }
  }

  return OK;
}

STAT_Val BDAR_create_from_bool_darr(BDAR_BitDArray * this, const DAR_DArray * bool_darr) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(bool_darr == NULL) return LOG_STAT(STAT_ERR_ARGS, "bool_darr is NULL");

  return LOG_STAT_IF_ERR(BDAR_create_from_bool_arr(this, bool_darr->data, bool_darr->size),
                         "failed to create from bool darr");
}

STAT_Val BDAR_reserve(BDAR_BitDArray * this, size_t num_bits) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(num_bits < this->size) return OK;

  return LOG_STAT_IF_ERR(grow_capacity_as_needed(this, num_bits),
                         "failed to reserve for %zu bits",
                         num_bits);
}

STAT_Val BDAR_resize(BDAR_BitDArray * this, size_t new_size) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(new_size == this->size) return OK;

  if(grow_capacity_as_needed(this, new_size) != OK) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to grow capacity");
  }

  this->size = new_size;

  return OK;
}

STAT_Val BDAR_resize_with_value(BDAR_BitDArray * this, size_t new_size, bool fill_val) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(new_size == this->size) return OK;

  const size_t old_size = this->size;

  if(BDAR_resize(this, new_size) != OK) return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize");

  if(new_size > old_size) {
    if(BDAR_fill_range(this, old_size, (new_size - old_size), fill_val) != OK) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to fill newly created range after resize");
    }
  }

  return OK;
}

STAT_Val BDAR_push_back(BDAR_BitDArray * this, bool val) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this == NULL");

  const size_t new_size = this->size + 1;
  if(grow_capacity_as_needed(this, new_size) != OK) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to grow capacity for push back");
  }

  this->size = new_size;

  return LOG_STAT_IF_ERR(BDAR_set_bit_val(this, (this->size - 1), val),
                         "failed to set new bit val");
}
STAT_Val BDAR_pop_back(BDAR_BitDArray * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this == NULL");
  if(this->size == 0) return LOG_STAT(STAT_ERR_EMPTY, "no element to pop");

  this->size--;

  return OK;
}

// STAT_Val BDAR_push_front(BDAR_BitDArray * this, bool val) {
//   if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this == NULL");

//   // TODO

//   return OK;
// }
// STAT_Val BDAR_pop_front(BDAR_BitDArray * this) {
//   if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this == NULL");

//   // TODO

//   return OK;
// }

STAT_Val BDAR_set_bit_val(BDAR_BitDArray * this, size_t idx, bool val) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this == NULL");
  if(idx >= this->size) return LOG_STAT(STAT_ERR_RANGE, "idx %zu out of range", idx);

  const size_t byte_idx    = (idx / 8);
  const size_t idx_in_byte = (idx % 8);

  const uint8_t val_mask    = 1 << idx_in_byte;
  const uint8_t val_shifted = (val ? 1 : 0) << idx_in_byte;

  this->data[byte_idx] &= ~val_mask;   // unset old value
  this->data[byte_idx] |= val_shifted; // set to new value

  return OK;
}

STAT_Val BDAR_fill_range(BDAR_BitDArray * this, size_t start_idx, size_t n, bool fill_val) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this == NULL");
  if(n == 0) return OK;
  if((start_idx + n) > this->size) {
    return LOG_STAT(STAT_ERR_RANGE, "idx %zu with size %zu out of range", start_idx, n);
  }

  const uint8_t fill_byte = (fill_val ? 0xff : 0);

  const size_t start_idx_in_byte = (start_idx % 8);
  const size_t end_idx_in_byte   = ((start_idx + n) % 8);

  const bool starts_at_start_of_byte = (start_idx_in_byte == 0);
  const bool ends_at_end_of_byte     = (end_idx_in_byte == 0);

  const size_t first_byte_idx = start_idx / 8;
  const size_t last_byte_idx  = ((start_idx + n) / 8) - (ends_at_end_of_byte ? 1 : 0);
  const size_t num_bytes      = (last_byte_idx - first_byte_idx) + 1;

  const size_t num_full_bytes =
      (num_bytes == 1)
          ? ((starts_at_start_of_byte && ends_at_end_of_byte) ? 1 : 0)
          : (num_bytes - (starts_at_start_of_byte ? 0 : 1) - (ends_at_end_of_byte ? 0 : 1));

  if(num_full_bytes > 0) {
    const size_t first_full_byte_idx =
        (starts_at_start_of_byte ? first_byte_idx : (first_byte_idx + 1));
    memset(&this->data[first_full_byte_idx], fill_byte, num_full_bytes);
  }

  if(num_bytes > num_full_bytes) {
    // there are partial bytes to deal with
    const uint8_t last_byte_mask  = (ends_at_end_of_byte ? 0xff : ((1 << end_idx_in_byte) - 1));
    const uint8_t first_byte_mask = ~((1 << start_idx_in_byte) - 1);

    if(first_byte_idx == last_byte_idx) {
      this->data[first_byte_idx] &= ~(first_byte_mask & last_byte_mask);            // unset
      this->data[first_byte_idx] |= (fill_byte & first_byte_mask & last_byte_mask); // set new value
    } else {
      this->data[first_byte_idx] &= ~first_byte_mask;              // unset
      this->data[first_byte_idx] |= (fill_byte & first_byte_mask); // set new value
      this->data[last_byte_idx] &= ~last_byte_mask;                // unset
      this->data[last_byte_idx] |= (fill_byte & last_byte_mask);   // set new value
    }
  }

  return OK;
}

STAT_Val BDAR_fill(BDAR_BitDArray * this, bool fill_val) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this == NULL");
  if(this->size == 0) return OK;

  return LOG_STAT_IF_ERR(BDAR_fill_range(this, 0, this->size, fill_val), "failed to fill range");
}

STAT_Val BDAR_shift_left(BDAR_BitDArray * this, size_t num_shift) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this == NULL");
  if(num_shift == 0) return OK;

  const size_t current_size_bytes = calc_size_in_bytes(this->size);

  if(num_shift >= this->size) {
    memset(this->data, 0, current_size_bytes);
    return OK;
  }

  if(num_shift > 8) {
    const size_t num_full_bytes = num_shift / 8;
    memmove(&this->data[num_full_bytes], this->data, current_size_bytes - num_full_bytes);
    memset(this->data, 0, num_full_bytes);

    num_shift -= (num_full_bytes * 8);
  }

  if(num_shift > 0) {
    for(size_t i = current_size_bytes - 1; i > 0; i--) {
      this->data[i] <<= num_shift;

      const uint8_t bits_to_copy_over = (this->data[i - 1] & ~((1 << (8 - num_shift)) - 1));
      this->data[i] |= (bits_to_copy_over >> (8 - num_shift));
    }
    this->data[0] <<= num_shift;
  }

  return OK;
}
STAT_Val BDAR_shift_right(BDAR_BitDArray * this, size_t num_shift) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this == NULL");
  if(num_shift == 0) return OK;

  const size_t current_size_bytes = calc_size_in_bytes(this->size);

  if(num_shift >= this->size) {
    memset(this->data, 0, current_size_bytes);
    return OK;
  }

  if(num_shift > 8) {
    const size_t num_full_bytes = num_shift / 8;
    memmove(this->data, &this->data[num_full_bytes], current_size_bytes - num_full_bytes);
    memset(&this->data[(current_size_bytes - num_full_bytes)], 0, num_full_bytes);

    num_shift -= (num_full_bytes * 8);
  }

  if(num_shift > 0) {
    for(size_t i = 0; i < current_size_bytes - 1; i++) {
      this->data[i] >>= num_shift;

      const uint8_t bits_to_copy_over = (this->data[i + 1] & ((1 << num_shift) - 1));
      this->data[i] |= (bits_to_copy_over << (8 - num_shift));
    }

    const size_t size_in_last_byte = (this->size % 8);
    if(size_in_last_byte <= num_shift) {
      this->data[(current_size_bytes - 1)] = 0;
    } else {
      const uint8_t last_byte_mask = ((1 << size_in_last_byte) - 1);
      const uint8_t shifted_mask   = (last_byte_mask >> num_shift) ^ last_byte_mask;

      this->data[(current_size_bytes - 1)] >>= num_shift;
      this->data[(current_size_bytes - 1)] &= ~shifted_mask;
    }
  }

  return OK;
}

STAT_Val BDAR_destroy(BDAR_BitDArray * this) {
  if(this == NULL) return OK;

  free(this->data);
  *this = (BDAR_BitDArray){0};

  return OK;
}

static size_t calc_size_in_bytes(size_t size_in_bits) {
  return (size_in_bits / 8) + ((size_in_bits % 8) ? 1 : 0);
}

static size_t determine_needed_capacity(size_t size_in_bits, size_t start_capacity) {
  const size_t min_bytes = calc_size_in_bytes(size_in_bits);

  size_t needed_capacity = (start_capacity > INIT_CAPACITY) ? start_capacity : INIT_CAPACITY;
  while(needed_capacity < min_bytes) needed_capacity *= 2;

  return needed_capacity;
}

static STAT_Val grow_capacity_as_needed(BDAR_BitDArray * this, size_t num_bits_to_fit) {
  if((this->capacity_in_bytes > INIT_CAPACITY) &&
     ((this->capacity_in_bytes * 8) >= num_bits_to_fit)) {
    return OK;
  }

  const size_t needed_capacity =
      determine_needed_capacity(num_bits_to_fit, this->capacity_in_bytes);

  if(this->capacity_in_bytes < needed_capacity) {
    uint8_t * new_data = realloc(this->data, needed_capacity);
    if(new_data == NULL) {
      return LOG_STAT(STAT_ERR_ALLOC,
                      "failed to reallocate to grow from size %zu to %zu",
                      this->capacity_in_bytes,
                      needed_capacity);
    }

    this->data              = new_data;
    this->capacity_in_bytes = needed_capacity;
  }

  return OK;
}
