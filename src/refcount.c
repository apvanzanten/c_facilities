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
