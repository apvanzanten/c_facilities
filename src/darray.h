#ifndef CFAC_DARRAY_H
#define CFAC_DARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stat.h"

typedef struct {
  void *   data;
  uint8_t  element_size;
  uint8_t  capacity_magnitude;
  uint32_t size;
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

void *       DAR_get(DAR_DArray * this, uint32_t idx);
const void * DAR_get_const(const DAR_DArray * this, uint32_t idx);

STAT_Val DAR_get_checked(DAR_DArray * this, uint32_t idx, void ** out);
STAT_Val DAR_get_checked_const(const DAR_DArray * this, uint32_t idx, const void ** out);

void     DAR_set(DAR_DArray * this, uint32_t idx, const void * value);
STAT_Val DAR_set_checked(DAR_DArray * this, uint32_t idx, const void * value);

STAT_Val DAR_push_back_array(DAR_DArray * this, const void * arr, uint32_t n);
STAT_Val DAR_push_back_darray(DAR_DArray * this, const DAR_DArray * other);

bool DAR_equals(const DAR_DArray * lhs, const DAR_DArray * rhs);

size_t DAR_get_capacity(const DAR_DArray * this);
size_t DAR_get_capacity_in_bytes(const DAR_DArray * this);
size_t DAR_get_size_in_bytes(const DAR_DArray * this);

void *       DAR_first(DAR_DArray * this);
void *       DAR_last(DAR_DArray * this);
const void * DAR_first_const(const DAR_DArray * this);
const void * DAR_last_const(const DAR_DArray * this);

#endif