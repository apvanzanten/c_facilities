#include "refcount.h"

#include <stdlib.h>

static size_t get_entry_size(size_t element_size) {
  // we add to sizeof(RC_RefCountedBlock) and round up to nearest multiple of align_max_t.
  // Having this be a multiple of sizeof(align_max_t) is required to be able to allocate memory
  // aligned to max_align_t, such that we can store any type directly in the data flexible array
  // member without breaking things.
  const size_t base_size = sizeof(RC_RefCountedBlock) + element_size;

  return ((base_size / sizeof(max_align_t)) + 1) * sizeof(max_align_t);
}

RC_Ref RC_allocate(size_t element_size) {
  if(element_size == 0) return (RC_Ref){.block = NULL};

  RC_RefCountedBlock * block = (RC_RefCountedBlock *)malloc(get_entry_size(element_size));
  if(block != NULL) block->ref_count = 1;

  return (RC_Ref){.block = block};
}

void RC_INT_free(RC_RefCountedBlock * block) { free(block); }