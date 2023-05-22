#ifndef CFAC_DARRAY_H
#define CFAC_DARRAY_H

#include <stdbool.h>
#include <stdint.h>

#include "stat.h"

typedef struct {
  void *   data;
  uint8_t  element_size;
  uint8_t  capacity_magnitude;
  uint32_t size;
} DAR_DArray;

STAT_Val DAR_create_on_heap(DAR_DArray ** this_p, uint8_t element_size);
STAT_Val DAR_create_in_place(DAR_DArray * this, uint8_t element_size);

STAT_Val DAR_destroy_on_heap(DAR_DArray ** this_p);
STAT_Val DAR_destroy_in_place(DAR_DArray * this);

STAT_Val DAR_push_back(DAR_DArray * this, void * element);
STAT_Val DAR_pop_back(DAR_DArray * this);

STAT_Val DAR_shrink_to_fit(DAR_DArray * this);

#endif
