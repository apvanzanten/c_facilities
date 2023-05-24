#include "darray.h"

#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "log.h"

#define OK STAT_OK

#define MAX_SIZE               UINT32_MAX
#define MIN_CAPACITY_MAGNITUDE 3
#define MAX_CAPACITY_MAGNITUDE 32

static size_t get_capacity(const DAR_DArray * this);
static size_t get_capacity_from_magnitude(uint8_t magnitude);
static size_t get_capacity_in_bytes_from_magnitude(uint8_t element_size, uint8_t magnitude);

static size_t get_byte_idx(const DAR_DArray * this, uint32_t element_idx);

static uint8_t get_minimum_required_capacity_magnitude(uint32_t size);

static STAT_Val grow_capacity_as_needed(DAR_DArray * this, uint32_t num_elements_to_fit);

STAT_Val DAR_create_on_heap(DAR_DArray ** this_p, uint8_t element_size) {
  if(this_p == NULL || *this_p != NULL) return LOG_STAT(STAT_ERR_ARGS, "bad arg 'this_p'");

  DAR_DArray * this = (DAR_DArray *)malloc(sizeof(DAR_DArray));
  if(this == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for DAR_DArray");

  if(!STAT_is_OK(DAR_create_in_place(this, element_size))) {
    free(this);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create DAR_DArray");
  }

  *this_p = this;

  return OK;
}

STAT_Val DAR_create_on_heap_from(DAR_DArray ** this_p, const DAR_DArray * src) {
  if(this_p == NULL || *this_p != NULL) return LOG_STAT(STAT_ERR_ARGS, "bad arg 'this_p'");
  if(src == NULL) return LOG_STAT(STAT_ERR_ARGS, "src is NULL");

  DAR_DArray * this = (DAR_DArray *)malloc(sizeof(DAR_DArray));
  if(this == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for DAR_DArray");

  if(!STAT_is_OK(DAR_create_in_place_from(this, src))) {
    free(this);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create DAR_DArray");
  }

  *this_p = this;

  return OK;
}

STAT_Val DAR_create_on_heap_from_cstr(DAR_DArray ** this_p, const char * str) {
  if(this_p == NULL || *this_p != NULL) return LOG_STAT(STAT_ERR_ARGS, "bad arg 'this_p'");
  if(str == NULL) return LOG_STAT(STAT_ERR_ARGS, "str is NULL");

  DAR_DArray * this = (DAR_DArray *)malloc(sizeof(DAR_DArray));
  if(this == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for DAR_DArray");

  if(!STAT_is_OK(DAR_create_in_place_from_cstr(this, str))) {
    free(this);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create DAR_DArray");
  }

  *this_p = this;

  return OK;
}

STAT_Val DAR_create_in_place(DAR_DArray * this, uint8_t element_size) {
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

STAT_Val DAR_create_in_place_from(DAR_DArray * this, const DAR_DArray * src) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(src == NULL) return LOG_STAT(STAT_ERR_ARGS, "src is NULL");

  if(!STAT_is_OK(DAR_create_in_place(this, src->element_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create new array");
  }

  if(!STAT_is_OK(DAR_push_back_darray(this, src))) {
    DAR_destroy_in_place(this);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to push src data into this array");
  }

  return OK;
}

STAT_Val DAR_create_in_place_from_cstr(DAR_DArray * this, const char * str) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(str == NULL) return LOG_STAT(STAT_ERR_ARGS, "str is NULL");

  if(!STAT_is_OK(DAR_create_in_place(this, sizeof(char)))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create new array");
  }

  if(!STAT_is_OK(DAR_push_back_array(this, str, strlen(str) + 1))) { // +1 for null termination
    DAR_destroy_in_place(this);
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to push str data into this array");
  }

  return OK;
}

STAT_Val DAR_destroy_on_heap(DAR_DArray ** this_p) {
  if(this_p == NULL) return LOG_STAT(STAT_ERR_ARGS, "this_p arg is NULL");

  if(*this_p == NULL) return OK;

  if(!STAT_is_OK(DAR_destroy_in_place(*this_p))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy DAR_Array");
  }

  free(*this_p);
  *this_p = NULL;

  return OK;
}

STAT_Val DAR_destroy_in_place(DAR_DArray * this) {
  if(this == NULL) return OK;

  free(this->data);
  *this = (DAR_DArray){0};

  return OK;
}

STAT_Val DAR_push_back(DAR_DArray * this, const void * element) {
  if(this == NULL || element == NULL) return LOG_STAT(STAT_ERR_ARGS, "this or element is NULL");
  if(this->size == UINT32_MAX) return LOG_STAT(STAT_ERR_FULL, "DAR_Array at maximum size");

  const uint32_t new_size = this->size + 1;

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

  uint8_t new_capacity_magnitude = get_minimum_required_capacity_magnitude(this->size);
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

STAT_Val DAR_resize(DAR_DArray * this, uint32_t new_size) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  if(!STAT_is_OK(grow_capacity_as_needed(this, new_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to grow capacity for resize");
  }
  this->size = new_size;

  return OK;
}

STAT_Val DAR_resize_zeroed(DAR_DArray * this, uint32_t new_size) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");

  const uint32_t old_size = this->size;

  if(!STAT_is_OK(DAR_resize(this, new_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize for resize zeroed");
  }

  if(new_size > old_size) {
    const size_t grown_in_bytes = (new_size - old_size) * this->element_size;
    memset(DAR_get(this, old_size), 0, grown_in_bytes);
  }

  return OK;
}

STAT_Val DAR_resize_with_value(DAR_DArray * this, uint32_t new_size, const void * value) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(value == NULL) return LOG_STAT(STAT_ERR_ARGS, "value is NULL");

  const uint32_t old_size = this->size;

  if(!STAT_is_OK(DAR_resize(this, new_size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize for resize zeroed");
  }

  for(uint32_t idx = old_size; idx < new_size; idx++) {
    memcpy(DAR_get(this, idx), value, this->element_size);
  }

  return OK;
}

STAT_Val DAR_reserve(DAR_DArray * this, uint32_t num_elements) {
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

void * DAR_get(DAR_DArray * this, uint32_t idx) {
  return &(((char *)this->data)[get_byte_idx(this, idx)]);
}

const void * DAR_get_const(const DAR_DArray * this, uint32_t idx) {
  return &(((const char *)this->data)[get_byte_idx(this, idx)]);
}

STAT_Val DAR_get_checked(DAR_DArray * this, uint32_t idx, void ** out) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(out == NULL) return LOG_STAT(STAT_ERR_ARGS, "out is NULL");

  if(idx >= this->size) {
    return LOG_STAT(STAT_ERR_RANGE, "idx %u out of range (size=%u)", idx, this->size);
  }

  *out = DAR_get(this, idx);

  return OK;
}

STAT_Val DAR_get_checked_const(const DAR_DArray * this, uint32_t idx, const void ** out) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(out == NULL) return LOG_STAT(STAT_ERR_ARGS, "out is NULL");

  if(idx >= this->size) {
    return LOG_STAT(STAT_ERR_RANGE, "idx %u out of range (size=%u)", idx, this->size);
  }

  *out = DAR_get_const(this, idx);

  return OK;
}

void DAR_set(DAR_DArray * this, uint32_t idx, const void * value) {
  memcpy(DAR_get(this, idx), value, this->element_size);
}

STAT_Val DAR_set_checked(DAR_DArray * this, uint32_t idx, const void * value) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(value == NULL) return LOG_STAT(STAT_ERR_ARGS, "value is NULL");

  if(idx >= this->size) {
    return LOG_STAT(STAT_ERR_RANGE, "idx %u out of range (size=%u)", idx, this->size);
  }

  DAR_set(this, idx, value);

  return OK;
}

STAT_Val DAR_push_back_array(DAR_DArray * this, const void * arr, uint32_t n) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(arr == NULL) return LOG_STAT(STAT_ERR_ARGS, "arr is NULL");

  if(n == 0) return OK;

  const uint32_t old_size = this->size;

  if(!STAT_is_OK(DAR_resize(this, this->size + n))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to resize");
  }

  memcpy(DAR_get(this, old_size), arr, n * this->element_size);

  return OK;
}

STAT_Val DAR_push_back_darray(DAR_DArray * this, const DAR_DArray * other) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(other == NULL) return LOG_STAT(STAT_ERR_ARGS, "other is NULL");

  if(this->element_size != other->element_size) {
    return LOG_STAT(STAT_ERR_ARGS,
                    "element size mismatch (%u != %u)",
                    this->element_size,
                    other->element_size);
  }

  if(!STAT_is_OK(DAR_push_back_array(this, other->data, other->size))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to push back data");
  }

  return OK;
}

bool DAR_equals(const DAR_DArray * lhs, const DAR_DArray * rhs) {
  if(lhs == NULL || rhs == NULL) return false;
  if(lhs->element_size != rhs->element_size) return false;
  if(lhs->size != rhs->size) return false;

  return (memcmp(lhs->data, rhs->data, DAR_get_size_in_bytes(lhs)) == 0);
}

void *       DAR_first(DAR_DArray * this) { return this->data; }
void *       DAR_last(DAR_DArray * this) { return DAR_get(this, this->size - 1); }
const void * DAR_first_const(const DAR_DArray * this) { return this->data; }
const void * DAR_last_const(const DAR_DArray * this) { return DAR_get_const(this, this->size - 1); }

static size_t get_capacity_from_magnitude(uint8_t magnitude) { return 1LL << magnitude; }

static size_t get_capacity(const DAR_DArray * this) {
  return get_capacity_from_magnitude(this->capacity_magnitude);
}

static size_t get_capacity_in_bytes_from_magnitude(uint8_t element_size, uint8_t magnitude) {
  return element_size * get_capacity_from_magnitude(magnitude);
}

static STAT_Val grow_capacity_as_needed(DAR_DArray * this, uint32_t num_elements_to_fit) {
  const uint8_t req_cap_magnitude = get_minimum_required_capacity_magnitude(num_elements_to_fit);
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

static size_t get_byte_idx(const DAR_DArray * this, uint32_t element_idx) {
  return this->element_size * element_idx;
}

static uint8_t get_minimum_required_capacity_magnitude(uint32_t size) {
  // TODO optimize to use existing magnitude, that probably saves a lot of time in many cases
  // NOTE this can probably be done more efficiently with some intrinsics (or bit twiddling)
  //      this is probably good enough, I may optimize it later when I have benchmarks set up
  for(uint8_t magnitude = MIN_CAPACITY_MAGNITUDE; magnitude < MAX_CAPACITY_MAGNITUDE; magnitude++) {
    const size_t capacity = get_capacity_from_magnitude(magnitude);
    if(capacity >= (size_t)size) return magnitude;
  }
  return MAX_CAPACITY_MAGNITUDE;
}