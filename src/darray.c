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

#include "darray.h"

#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "log.h"

#define OK STAT_OK

#define MAX_SIZE               UINT32_MAX
#define MIN_CAPACITY_MAGNITUDE 3
#define MAX_CAPACITY_MAGNITUDE 63
#define MAX_CAPACITY           (1LL << MAX_CAPACITY_MAGNITUDE)

static size_t get_capacity(const DAR_DArray * this);
static size_t get_capacity_from_magnitude(uint8_t magnitude);
static size_t get_capacity_in_bytes_from_magnitude(size_t element_size, uint8_t magnitude);

static STAT_Val grow_capacity_as_needed(DAR_DArray * this, size_t num_elements_to_fit);
static uint8_t get_required_capacity_magnitude(uint8_t current_cap_mag, size_t num_elements_to_fit);

STAT_Val DAR_create(DAR_DArray * this, size_t element_size) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this arg is NULL");

  this->size               = 0;
  this->element_size       = element_size;
  this->capacity_magnitude = MIN_CAPACITY_MAGNITUDE;

  const size_t capacity_in_bytes =
      get_capacity_in_bytes_from_magnitude(element_size, MIN_CAPACITY_MAGNITUDE);

  this->data = malloc(capacity_in_bytes);
  if(this->data == NULL) {
    return LOG_STAT(STAT_ERR_ALLOC,
                    "failed to allocate data array with size %zu, errno: %d (\'%s\')",
                    capacity_in_bytes,
                    errno,
                    strerror(errno));
  }

  return OK;
}

STAT_Val DAR_create_from(DAR_DArray * this, const DAR_DArray * src) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(src == NULL) return LOG_STAT(STAT_ERR_ARGS, "src is NULL");

  if(!STAT_is_OK(DAR_create(this, src->element_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create new array");
  }

  if(!STAT_is_OK(DAR_push_back_darray(this, src))) {
    DAR_destroy(this);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to push src data into this array");
  }

  return OK;
}

STAT_Val DAR_create_from_cstr(DAR_DArray * this, const char * str) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(str == NULL) return LOG_STAT(STAT_ERR_ARGS, "str is NULL");

  if(!STAT_is_OK(DAR_create(this, sizeof(char)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create new array");
  }

  if(!STAT_is_OK(DAR_push_back_array(this, str, strlen(str) + 1))) { // +1 for null termination
    DAR_destroy(this);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to push str data into this array");
  }

  return OK;
}

STAT_Val DAR_destroy(DAR_DArray * this) {
  if(this == NULL) return OK;

  free(this->data);
  *this = (DAR_DArray){0};

  return OK;
}

STAT_Val DAR_push_back(DAR_DArray * this, const void * element) {
  if(this == NULL || element == NULL) return LOG_STAT(STAT_ERR_ARGS, "this or element is NULL");
  if(this->size == UINT32_MAX) return LOG_STAT(STAT_ERR_FULL, "DAR_Array at maximum size");

  const size_t new_size = this->size + 1;

  if(!STAT_is_OK(grow_capacity_as_needed(this, new_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to grow capacity for push back");
  }

  memcpy(DAR_get(this, this->size), element, this->element_size);

  this->size = new_size;

  return OK;
}

STAT_Val DAR_pop_back(DAR_DArray * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(this->size == 0) return LOG_STAT(STAT_ERR_EMPTY, "no element to pop");

  this->size--;

  return OK;
}

STAT_Val DAR_shrink_to_fit(DAR_DArray * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  const uint8_t new_capacity_magnitude =
      get_required_capacity_magnitude(this->capacity_magnitude, this->size);
  if(new_capacity_magnitude == this->capacity_magnitude) return OK;

  const size_t new_capacity_in_bytes =
      get_capacity_in_bytes_from_magnitude(this->element_size, new_capacity_magnitude);

  void * new_data = realloc(this->data, new_capacity_in_bytes);
  if(new_data == NULL) {
    return LOG_STAT(STAT_ERR_ALLOC,
                    "failed to reallocate for shrink to size %zu, errno: %d (\'%s\')",
                    new_capacity_in_bytes,
                    errno,
                    strerror(errno));
  }

  this->capacity_magnitude = new_capacity_magnitude;
  this->data               = new_data;

  return OK;
}

STAT_Val DAR_resize(DAR_DArray * this, size_t new_size) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  if(!STAT_is_OK(grow_capacity_as_needed(this, new_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to grow capacity for resize");
  }
  this->size = new_size;

  return OK;
}

STAT_Val DAR_resize_zeroed(DAR_DArray * this, size_t new_size) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  const size_t old_size = this->size;

  if(!STAT_is_OK(DAR_resize(this, new_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize for resize zeroed");
  }

  if(new_size > old_size) {
    const size_t grown_in_bytes = (new_size - old_size) * this->element_size;
    memset(DAR_get(this, old_size), 0, grown_in_bytes);
  }

  return OK;
}

STAT_Val DAR_resize_with_value(DAR_DArray * this, size_t new_size, const void * value) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(value == NULL) return LOG_STAT(STAT_ERR_ARGS, "value is NULL");

  const size_t old_size = this->size;

  if(!STAT_is_OK(DAR_resize(this, new_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize for resize zeroed");
  }

  for(size_t idx = old_size; idx < new_size; idx++) {
    memcpy(DAR_get(this, idx), value, this->element_size);
  }

  return OK;
}

STAT_Val DAR_reserve(DAR_DArray * this, size_t num_elements) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  if(!STAT_is_OK(grow_capacity_as_needed(this, num_elements))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to grow capacity for reserve");
  }

  return OK;
}

size_t DAR_get_capacity(const DAR_DArray * this) {
  if(this == NULL) return 0;
  return get_capacity(this);
}
size_t DAR_get_size_in_bytes(const DAR_DArray * this) {
  if(this == NULL) return 0;
  return this->size * this->element_size;
}

STAT_Val DAR_clear(DAR_DArray * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  if(!STAT_is_OK(DAR_resize(this, 0))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize for clear");
  }

  return OK;
}

STAT_Val DAR_clear_and_shrink(DAR_DArray * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  if(!STAT_is_OK(DAR_clear(this))) return LOG_STAT(STAT_ERR_INTERNAL, "failed to clear");
  if(!STAT_is_OK(DAR_shrink_to_fit(this))) return LOG_STAT(STAT_ERR_INTERNAL, "failed to shrink");

  return OK;
}

STAT_Val DAR_INT_get_checked_nonconst(DAR_DArray * this, size_t idx, void ** out) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(out == NULL) return LOG_STAT(STAT_ERR_ARGS, "out is NULL");

  if(idx >= this->size) {
    return LOG_STAT(STAT_ERR_RANGE, "idx %u out of range (size=%u)", idx, this->size);
  }

  *out = DAR_get(this, idx);

  return OK;
}

STAT_Val DAR_INT_get_checked_const(const DAR_DArray * this, size_t idx, const void ** out) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(out == NULL) return LOG_STAT(STAT_ERR_ARGS, "out is NULL");

  if(idx >= this->size) {
    return LOG_STAT(STAT_ERR_RANGE, "idx %u out of range (size=%u)", idx, this->size);
  }

  *out = DAR_get(this, idx);

  return OK;
}

STAT_Val DAR_set_checked(DAR_DArray * this, size_t idx, const void * value) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(value == NULL) return LOG_STAT(STAT_ERR_ARGS, "value is NULL");

  if(idx >= this->size) {
    return LOG_STAT(STAT_ERR_RANGE, "idx %u out of range (size=%u)", idx, this->size);
  }

  DAR_set(this, idx, value);

  return OK;
}

STAT_Val DAR_push_back_array(DAR_DArray * this, const void * arr, size_t n) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(arr == NULL) return LOG_STAT(STAT_ERR_ARGS, "arr is NULL");

  if(n == 0) return OK;

  const size_t old_size = this->size;

  if(!STAT_is_OK(DAR_resize(this, this->size + n))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize");
  }

  memcpy(DAR_get(this, old_size), arr, n * this->element_size);

  return OK;
}

STAT_Val DAR_push_back_span(DAR_DArray * this, SPN_Span span) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(this->element_size != span.element_size) {
    return LOG_STAT(STAT_ERR_ARGS,
                    "element size mismatch (%u != %u)",
                    this->element_size,
                    span.element_size);
  }

  if(SPN_is_empty(span)) return OK;

  return DAR_push_back_array(this, span.begin, span.len);
}

STAT_Val DAR_push_back_darray(DAR_DArray * this, const DAR_DArray * other) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(other == NULL) return LOG_STAT(STAT_ERR_ARGS, "other is NULL");

  return DAR_push_back_span(this, DAR_to_span(other));
}

STAT_Val DAR_delete(DAR_DArray * this, size_t idx) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(idx >= this->size) {
    return LOG_STAT(STAT_ERR_RANGE, "idx %u out of range (size=%u)", idx, this->size);
  }

  if(idx < (this->size - 1)) memcpy(DAR_get(this, idx), DAR_last(this), this->element_size);

  return LOG_STAT_IF_ERR(DAR_resize(this, (this->size - 1)), "failed to reduce arr size by 1");
}

STAT_Val DAR_order_preserving_delete(DAR_DArray * this, size_t idx) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(idx >= this->size) {
    return LOG_STAT(STAT_ERR_RANGE, "idx %u out of range (size=%u)", idx, this->size);
  }

  const size_t num_elements_to_move = (this->size - 1) - idx;

  if(num_elements_to_move > 0) {
    memmove(DAR_get(this, idx),
            DAR_get(this, idx + 1),
            (this->element_size * num_elements_to_move));
  }

  return LOG_STAT_IF_ERR(DAR_resize(this, (this->size - 1)), "failed to reduce arr size by 1");
}

bool DAR_equals(const DAR_DArray * lhs, const DAR_DArray * rhs) {
  if(lhs == NULL || rhs == NULL) return false;
  if(lhs->element_size != rhs->element_size) return false;
  if(lhs->size != rhs->size) return false;

  return (memcmp(lhs->data, rhs->data, DAR_get_size_in_bytes(lhs)) == 0);
}

static size_t get_capacity_from_magnitude(uint8_t magnitude) { return 1LL << magnitude; }

static size_t get_capacity(const DAR_DArray * this) {
  return get_capacity_from_magnitude(this->capacity_magnitude);
}

static size_t get_capacity_in_bytes_from_magnitude(size_t element_size, uint8_t magnitude) {
  return element_size * get_capacity_from_magnitude(magnitude);
}

static STAT_Val grow_capacity_as_needed(DAR_DArray * this, size_t num_elements_to_fit) {
  if(this->capacity_magnitude == MAX_CAPACITY_MAGNITUDE) {
    return LOG_STAT(STAT_ERR_FULL, "array capacity at max");
  }

  const uint8_t req_cap_magnitude =
      get_required_capacity_magnitude(this->capacity_magnitude, num_elements_to_fit);

  if(this->capacity_magnitude >= req_cap_magnitude) return OK;

  const size_t new_capacity_in_bytes =
      get_capacity_in_bytes_from_magnitude(this->element_size, req_cap_magnitude);

  void * new_data = realloc(this->data, new_capacity_in_bytes);
  if(new_data == NULL) {
    return LOG_STAT(STAT_ERR_ALLOC,
                    "failed to reallocate for growing capacity to size %zu, errno: %d (\'%s\')",
                    new_capacity_in_bytes,
                    errno,
                    strerror(errno));
  }

  this->data               = new_data;
  this->capacity_magnitude = req_cap_magnitude;

  return OK;
}

static uint8_t get_required_capacity_magnitude(uint8_t current_cap_mag,
                                               size_t  num_elements_to_fit) {
  // NOTE this can possibly be done more efficiently with some intrinsics (or bit twiddling)
  //      this is probably good enough, I may optimize it later when I have benchmarks set up
  uint8_t cap_mag = current_cap_mag;
  while((cap_mag > MIN_CAPACITY_MAGNITUDE) &&
        (get_capacity_from_magnitude(cap_mag) > num_elements_to_fit)) {
    cap_mag--;
  }
  while((cap_mag < MAX_CAPACITY_MAGNITUDE) &&
        (get_capacity_from_magnitude(cap_mag) < num_elements_to_fit)) {
    cap_mag++;
  }
  return cap_mag;
}

SPN_Span DAR_to_span(const DAR_DArray * this) {
  if(this == NULL) return (SPN_Span){0};
  return (SPN_Span){.begin = this->data, .len = this->size, .element_size = this->element_size};
}

SPN_MutSpan DAR_to_mut_span(DAR_DArray * this) {
  if(this == NULL) return (SPN_MutSpan){0};
  return (SPN_MutSpan){.begin = this->data, .len = this->size, .element_size = this->element_size};
}

STAT_Val DAR_create_from_span(DAR_DArray * this, SPN_Span span) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(span.element_size == 0) return LOG_STAT(STAT_ERR_ARGS, "span has invalid element size");
  if(span.begin == NULL) return LOG_STAT(STAT_ERR_ARGS, "span has invalid data pointer");

  if(!STAT_is_OK(DAR_create(this, span.element_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create new array");
  }

  if(!STAT_is_OK(DAR_push_back_array(this, span.begin, span.len))) {
    DAR_destroy(this);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to copy span data into array");
  }

  return OK;
}
