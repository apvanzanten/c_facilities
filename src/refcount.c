#include "refcount.h"

#include <stdlib.h>

static size_t get_entry_size(size_t element_size) {
  // we add to sizeof(RC_Entry) and round up to nearest multiple of align_max_t.
  // Having this be a multiple of sizeof(align_max_t) is required to be able to allocate memory
  // aligned to max_align_t, such that we can store any type directly in the data flexible array
  // member without breaking things.
  const size_t base_size = sizeof(RC_Entry) + element_size;

  return ((base_size / sizeof(max_align_t)) + 1) * sizeof(max_align_t);
}

RC_Ref RC_allocate(size_t element_size) {
  if(element_size == 0) return (RC_Ref){.entry = NULL};

  RC_Entry * entry = (RC_Entry *)malloc(get_entry_size(element_size));
  if(entry != NULL) entry->ref_count = 1;

  return (RC_Ref){.entry = entry};
}

void RC_IMPL_free(RC_Entry * entry) { free(entry); }