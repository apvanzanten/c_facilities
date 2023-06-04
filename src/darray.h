#ifndef CFAC_DARRAY_H
#define CFAC_DARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stat.h"

#include "span.h"

// ===========
// == types ==

typedef struct {
  void *  data;
  size_t  element_size;       // size of each element in bytes
  size_t  size;               // size of array in elements
  uint8_t capacity_magnitude; // magnitude of capacity in elements

  // NOTE size of data memory should always be: element_size * (2^capacity_magnitude)
  // NOTE and always (2^capacity_magnitude) >= size
} DAR_DArray;

// ==============================
// == creation and destruction ==

STAT_Val DAR_create(DAR_DArray * this, size_t element_size);
STAT_Val DAR_create_from(DAR_DArray * this, const DAR_DArray * src);
STAT_Val DAR_create_from_cstr(DAR_DArray * this, const char * str);

STAT_Val DAR_destroy(DAR_DArray * this);

// ==================
// == modification ==
// TODO add something akin to erase-remove idiom

STAT_Val DAR_push_back(DAR_DArray * this, const void * element);
STAT_Val DAR_pop_back(DAR_DArray * this);

STAT_Val DAR_shrink_to_fit(DAR_DArray * this);

STAT_Val DAR_resize(DAR_DArray * this, size_t new_size);
STAT_Val DAR_resize_zeroed(DAR_DArray * this, size_t new_size);
STAT_Val DAR_resize_with_value(DAR_DArray * this, size_t new_size, const void * value);

STAT_Val DAR_reserve(DAR_DArray * this, size_t num_elements);

STAT_Val DAR_clear(DAR_DArray * this);
STAT_Val DAR_clear_and_shrink(DAR_DArray * this);

static inline void DAR_set(DAR_DArray * this, size_t idx, const void * value);
STAT_Val           DAR_set_checked(DAR_DArray * this, size_t idx, const void * value);

STAT_Val DAR_push_back_array(DAR_DArray * this, const void * arr, size_t n);
STAT_Val DAR_push_back_span(DAR_DArray * this, SPN_Span span);
STAT_Val DAR_push_back_darray(DAR_DArray * this, const DAR_DArray * other);

// =============
// == queries ==

bool DAR_equals(const DAR_DArray * lhs, const DAR_DArray * rhs);

size_t DAR_get_capacity(const DAR_DArray * this);
size_t DAR_get_capacity_in_bytes(const DAR_DArray * this);
size_t DAR_get_size_in_bytes(const DAR_DArray * this);

static inline size_t DAR_get_byte_idx(const DAR_DArray * this, size_t element_idx);
static inline bool   DAR_is_initialized(const DAR_DArray * this);
static inline bool   DAR_is_empty(const DAR_DArray * this);

// ==================
// == span interop ==

SPN_Span DAR_to_span(const DAR_DArray * this);
STAT_Val DAR_create_from_span(DAR_DArray * this, SPN_Span span);

// ===============
// == accessors ==

//  [const] void * DAR_get([const] DAR_DArray * this, size_t idx)
#define DAR_get(this, idx)                                                                         \
  _Generic((this),                                                                                 \
      const DAR_DArray *: DAR_IMPL_get_const,                                                      \
      DAR_DArray *: DAR_IMPL_get_nonconst)(this, idx)
static inline void *       DAR_IMPL_get_nonconst(DAR_DArray * this, size_t idx);
static inline const void * DAR_IMPL_get_const(const DAR_DArray * this, size_t idx);

//  STAT_Val DAR_get_checked([const] DAR_DArray *this, size_t idx, [const] void ** out)
#define DAR_get_checked(this, idx, out)                                                            \
  _Generic((this),                                                                                 \
      const DAR_DArray *: DAR_IMPL_get_checked_const,                                              \
      DAR_DArray *: DAR_IMPL_get_checked_nonconst)(this, idx, out)
STAT_Val DAR_IMPL_get_checked_nonconst(DAR_DArray * this, size_t idx, void ** out);
STAT_Val DAR_IMPL_get_checked_const(const DAR_DArray * this, size_t idx, const void ** out);

//  [const] void * DAR_first([const] DAR_DArray *this)
#define DAR_first(this)                                                                            \
  _Generic((this),                                                                                 \
      const DAR_DArray *: DAR_IMPL_first_const,                                                    \
      DAR_DArray *: DAR_IMPL_first_nonconst)(this)
static inline void *       DAR_IMPL_first_nonconst(DAR_DArray * this);
static inline const void * DAR_IMPL_first_const(const DAR_DArray * this);

//  [const] void * DAR_last([const] DAR_DArray *this)
#define DAR_last(this)                                                                             \
  _Generic((this), const DAR_DArray *: DAR_IMPL_last_const, DAR_DArray *: DAR_IMPL_last_nonconst)( \
      this)
static inline void *       DAR_IMPL_last_nonconst(DAR_DArray * this);
static inline const void * DAR_IMPL_last_const(const DAR_DArray * this);

//  [const] void * DAR_end([const] DAR_DArray *this)
#define DAR_end(this)                                                                              \
  _Generic((this), const DAR_DArray *: DAR_IMPL_end_const, DAR_DArray *: DAR_IMPL_end_nonconst)(   \
      this)
static inline void *       DAR_IMPL_end_nonconst(DAR_DArray * this);
static inline const void * DAR_IMPL_end_const(const DAR_DArray * this);

// =================================
// == inline function implementations ==

static inline size_t DAR_get_byte_idx(const DAR_DArray * this, size_t element_idx) {
  return this->element_size * element_idx;
}

static inline bool DAR_is_initialized(const DAR_DArray * this) { return (this->data != NULL); }

static inline bool DAR_is_empty(const DAR_DArray * this) {
  return (this == NULL || (this->size == 0));
}

static inline void * DAR_IMPL_get_nonconst(DAR_DArray * this, size_t idx) {
  return &(((uint8_t *)this->data)[DAR_get_byte_idx(this, idx)]);
}
static inline const void * DAR_IMPL_get_const(const DAR_DArray * this, size_t idx) {
  return &(((const uint8_t *)this->data)[DAR_get_byte_idx(this, idx)]);
}
static inline void DAR_set(DAR_DArray * this, size_t idx, const void * value) {
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
  for(size_t i = 0; i < this->element_size; i++) {
    dst_bytes[i] = src_bytes[i];
  }
}

static inline void *       DAR_IMPL_first_nonconst(DAR_DArray * this) { return this->data; }
static inline const void * DAR_IMPL_first_const(const DAR_DArray * this) { return this->data; }

static inline void * DAR_IMPL_last_nonconst(DAR_DArray * this) {
  return DAR_get(this, this->size - 1);
}
static inline const void * DAR_IMPL_last_const(const DAR_DArray * this) {
  return DAR_get(this, this->size - 1);
}

static inline void * DAR_IMPL_end_nonconst(DAR_DArray * this) { return DAR_get(this, this->size); }
static inline const void * DAR_IMPL_end_const(const DAR_DArray * this) {
  return DAR_get(this, this->size);
}

#endif
