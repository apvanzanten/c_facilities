#include "darray.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"

#define OK STAT_OK

#define MAX_SIZE               UINT32_MAX
#define MIN_CAPACITY_MAGNITUDE 3
#define MAX_CAPACITY_MAGNITUDE 32

static size_t get_capacity(const DAR_DArray * this);
static size_t get_capacity_from_magnitude(uint8_t magnitude);
static size_t get_capacity_in_bytes_from_magnitude(uint8_t element_size, uint8_t magnitude);

static size_t get_byte_idx(const DAR_DArray * this, uint32_t element_idx);
static void * at(DAR_DArray * this, uint32_t idx);

static uint8_t get_minimum_required_capacity_magnitude(uint32_t size);

static STAT_Val grow_capacity(DAR_DArray * this);

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

STAT_Val DAR_create_in_place(DAR_DArray * this, uint8_t element_size) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this arg is NULL");

  this->element_size       = element_size;
  this->capacity_magnitude = MIN_CAPACITY_MAGNITUDE;

  this->data = malloc(get_capacity_in_bytes_from_magnitude(element_size, MIN_CAPACITY_MAGNITUDE));
  if(this->data == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate data array");

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

STAT_Val DAR_push_back(DAR_DArray * this, void * element) {
  if(this == NULL || element == NULL) return LOG_STAT(STAT_ERR_ARGS, "this or element is NULL");
  if(this->size == UINT32_MAX) return LOG_STAT(STAT_ERR_FULL, "DAR_Array at maximum size");

  if(this->size + 1 > get_capacity(this)) {
    if(!STAT_is_OK(grow_capacity(this))) {
      return LOG_STAT(STAT_ERR_INTERNAL, "failed to grow capacity for DAR_Array");
    }
  }

  memcpy(at(this, this->size++), element, this->element_size);

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
  if(new_data == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to reallocate to shrink array");

  this->capacity_magnitude = new_capacity_magnitude;
  this->data               = new_data;

  return OK;
}

static size_t get_capacity_from_magnitude(uint8_t magnitude) { return 1 << magnitude; }

static size_t get_capacity(const DAR_DArray * this) {
  return get_capacity_from_magnitude(this->capacity_magnitude);
}

static size_t get_capacity_in_bytes_from_magnitude(uint8_t element_size, uint8_t magnitude) {
  return element_size * get_capacity_from_magnitude(magnitude);
}

static STAT_Val grow_capacity(DAR_DArray * this) {
  const uint8_t new_capacity_magnitude = this->capacity_magnitude + 1;
  const size_t  new_capacity_in_bytes =
      get_capacity_in_bytes_from_magnitude(this->element_size, new_capacity_magnitude);

  void * new_data = realloc(this->data, new_capacity_in_bytes);
  if(new_data == NULL) return LOG_STAT(STAT_ERR_ALLOC, "failed to allocate for growing capacity");

  this->data               = new_data;
  this->capacity_magnitude = new_capacity_magnitude;

  return OK;
}

static size_t get_byte_idx(const DAR_DArray * this, uint32_t element_idx) {
  return this->element_size * element_idx;
}

static void * at(DAR_DArray * this, uint32_t idx) {
  return &(((char *)this->data)[get_byte_idx(this, idx)]);
}

static uint8_t get_minimum_required_capacity_magnitude(uint32_t size) {
  // NOTE this can probably be done more efficiently with some intrinsics (or bit twiddling)
  //      this is probably good enough, I may optimize it later when I have benchmarks set up
  for(uint8_t magnitude = 1; magnitude < MAX_CAPACITY_MAGNITUDE; magnitude++) {
    const size_t capacity = (1LL < magnitude);
    if(capacity > size) return magnitude;
  }
  return MAX_CAPACITY_MAGNITUDE;
}