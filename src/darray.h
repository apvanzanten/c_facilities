#ifndef CFAC_DARRAY_H
#define CFAC_DARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stat.h"

#include "span.h"

typedef struct {
  void *   data;
  uint8_t  element_size;       // size of each element in bytes
  uint8_t  capacity_magnitude; // magnitude of capacity in elements
  uint32_t size;               // size of array in elements

  // NOTE size of data memory should always be: element_size * (2^capacity_magnitude)
  // NOTE and always (2^capacity_magnitude) >= size
} DAR_DArray;

STAT_Val DAR_create_on_heap(DAR_DArray ** this_p, uint8_t element_size);
STAT_Val DAR_create_on_heap_from(DAR_DArray ** this_p, const DAR_DArray * src);
STAT_Val DAR_destroy_on_heap(DAR_DArray ** this_p);
STAT_Val DAR_create_on_heap_from_cstr(DAR_DArray ** this_p, const char * str);

STAT_Val DAR_create_in_place(DAR_DArray * this, uint8_t element_size);
STAT_Val DAR_create_in_place_from(DAR_DArray * this, const DAR_DArray * src);
STAT_Val DAR_destroy_in_place(DAR_DArray * this);
STAT_Val DAR_create_in_place_from_cstr(DAR_DArray * this, const char * str);

STAT_Val DAR_push_back(DAR_DArray * this, const void * element);
STAT_Val DAR_pop_back(DAR_DArray * this);

STAT_Val DAR_shrink_to_fit(DAR_DArray * this);

STAT_Val DAR_resize(DAR_DArray * this, uint32_t new_size);
STAT_Val DAR_resize_zeroed(DAR_DArray * this, uint32_t new_size);
STAT_Val DAR_resize_with_value(DAR_DArray * this, uint32_t new_size, const void * value);

STAT_Val DAR_reserve(DAR_DArray * this, uint32_t num_elements);

STAT_Val DAR_clear(DAR_DArray * this);
STAT_Val DAR_clear_and_shrink(DAR_DArray * this);

static inline void *       DAR_IMPL_get_nonconst(DAR_DArray * this, uint32_t idx);
static inline const void * DAR_IMPL_get_const(const DAR_DArray * this, uint32_t idx);

static inline void DAR_set(DAR_DArray * this, uint32_t idx, const void * value);

STAT_Val DAR_IMPL_get_checked_nonconst(DAR_DArray * this, uint32_t idx, void ** out);
STAT_Val DAR_IMPL_get_checked_const(const DAR_DArray * this, uint32_t idx, const void ** out);

STAT_Val DAR_set_checked(DAR_DArray * this, uint32_t idx, const void * value);

STAT_Val DAR_push_back_array(DAR_DArray * this, const void * arr, uint32_t n);
STAT_Val DAR_push_back_darray(DAR_DArray * this, const DAR_DArray * other);

bool DAR_equals(const DAR_DArray * lhs, const DAR_DArray * rhs);

size_t DAR_get_capacity(const DAR_DArray * this);
size_t DAR_get_capacity_in_bytes(const DAR_DArray * this);
size_t DAR_get_size_in_bytes(const DAR_DArray * this);

static inline size_t DAR_get_byte_idx(const DAR_DArray * this, uint32_t element_idx);

static inline void *       DAR_IMPL_first_nonconst(DAR_DArray * this);
static inline const void * DAR_IMPL_first_const(const DAR_DArray * this);
static inline void *       DAR_IMPL_last_nonconst(DAR_DArray * this);
static inline const void * DAR_IMPL_last_const(const DAR_DArray * this);
static inline void *       DAR_IMPL_end_nonconst(DAR_DArray * this);
static inline const void * DAR_IMPL_end_const(const DAR_DArray * this);

SPN_Span DAR_to_span(const DAR_DArray * this);
STAT_Val DAR_create_on_heap_from_span(DAR_DArray ** this_p, SPN_Span span);
STAT_Val DAR_create_in_place_from_span(DAR_DArray * this, SPN_Span span);

// generic definitions

#define DAR_get(this, idx)                                                                         \
  _Generic((this),                                                                                 \
      const DAR_DArray *: DAR_IMPL_get_const,                                                      \
      DAR_DArray *: DAR_IMPL_get_nonconst)(this, idx)

#define DAR_get_checked(this, idx, out)                                                            \
  _Generic((this),                                                                                 \
      const DAR_DArray *: DAR_IMPL_get_checked_const,                                              \
      DAR_DArray *: DAR_IMPL_get_checked_nonconst)(this, idx, out)

#define DAR_first(this)                                                                            \
  _Generic((this),                                                                                 \
      const DAR_DArray *: DAR_IMPL_first_const,                                                    \
      DAR_DArray *: DAR_IMPL_first_nonconst)(this)
#define DAR_last(this)                                                                             \
  _Generic((this), const DAR_DArray *: DAR_IMPL_last_const, DAR_DArray *: DAR_IMPL_last_nonconst)( \
      this)
#define DAR_end(this)                                                                              \
  _Generic((this), const DAR_DArray *: DAR_IMPL_end_const, DAR_DArray *: DAR_IMPL_end_nonconst)(   \
      this)

// inline function definitions

static inline size_t DAR_get_byte_idx(const DAR_DArray * this, uint32_t element_idx) {
  return this->element_size * element_idx;
}

static inline void * DAR_IMPL_get_nonconst(DAR_DArray * this, uint32_t idx) {
  return &(((uint8_t *)this->data)[DAR_get_byte_idx(this, idx)]);
}
static inline const void * DAR_IMPL_get_const(const DAR_DArray * this, uint32_t idx) {
  return &(((const uint8_t *)this->data)[DAR_get_byte_idx(this, idx)]);
}
static inline void DAR_set(DAR_DArray * this, uint32_t idx, const void * value) {
  // NOTE This could be done with memcpy. Based on some quick and dirty benchmarks, I gather it
  // would be significantly slower for smaller elements, but catch up and start being significantly
  // faster at around 16 bytes per element. That said, your mileage may vary depending on where/how
  // this is called; due to inlining it may be able to fill in parts of the below loop at
  // compile-time which could lead to significant optimizations, probably making it not quite so
  // slow also for larger elements.
  // Ultimately, it's not too hard for the client to call memcpy themselves if they need the
  // performance, so I decided to stick with just the loop.

  uint8_t *       dst_bytes = (uint8_t *)DAR_get(this, idx);
  const uint8_t * src_bytes = (const uint8_t *)value;
  for(uint32_t i = 0; i < this->element_size; i++) {
    dst_bytes[i] = src_bytes[i];
  }
}

static inline void * DAR_IMPL_first_nonconst(DAR_DArray * this) { return this->data; }
static inline void * DAR_IMPL_last_nonconst(DAR_DArray * this) {
  return DAR_get(this, this->size - 1);
}
static inline const void * DAR_IMPL_first_const(const DAR_DArray * this) { return this->data; }
static inline const void * DAR_IMPL_last_const(const DAR_DArray * this) {
  return DAR_get(this, this->size - 1);
}

static inline void * DAR_IMPL_end_nonconst(DAR_DArray * this) { return DAR_get(this, this->size); }
static inline const void * DAR_IMPL_end_const(const DAR_DArray * this) {
  return DAR_get(this, this->size);
}

#endif
