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

#include "span.h"

#include "log.h"
#include "stat.h"

#include <string.h>

#define OK STAT_OK

static bool is_valid(SPN_Span span) { return (span.begin != NULL && span.element_size != 0); }

SPN_Span SPN_from_cstr(const char * cstr) {
  if(cstr == NULL) return (SPN_Span){0};
  return (SPN_Span){.begin = (const void *)cstr, .len = strlen(cstr), .element_size = 1};
}

SPN_MutSpan SPN_mut_span_from_cstr(char * cstr) {
  if(cstr == NULL) return (SPN_MutSpan){0};
  return (SPN_MutSpan){.begin = (void *)cstr, .len = strlen(cstr), .element_size = 1};
}

SPN_Span SPN_subspan(SPN_Span src, size_t begin_idx, size_t len) {
  if(begin_idx > src.len) begin_idx = src.len;
  if(begin_idx + len > src.len) len = (src.len - begin_idx);
  return (SPN_Span){.begin = SPN_get(src, begin_idx), .len = len, .element_size = src.element_size};
}

bool SPN_equals(SPN_Span lhs, SPN_Span rhs) {
  if(lhs.len != rhs.len) return false;
  if(lhs.element_size != rhs.element_size) return false;
  if(lhs.begin == NULL || rhs.begin == NULL) return false;
  if(lhs.begin == rhs.begin) return true;
  return (memcmp(lhs.begin, rhs.begin, SPN_get_size_in_bytes(lhs)) == 0);
}

bool SPN_contains_subspan(SPN_Span span, SPN_Span subspan) {
  if(!is_valid(span) || !is_valid(subspan)) return false;
  if(span.element_size != subspan.element_size) return false;
  if(span.len < subspan.len) return false;

  // NOTE this could probably be faster if we did our own comparisons
  const size_t subspan_size_bytes = SPN_get_size_in_bytes(subspan);

  for(size_t i = 0; i <= (span.len - subspan.len); i++) {
    if(memcmp(SPN_get(span, i), subspan.begin, subspan_size_bytes) == 0) return true;
  }

  return false;
}

STAT_Val SPN_find(SPN_Span span, const void * element, size_t * o_idx) {
  return SPN_find_at(span, element, 0, o_idx);
}

STAT_Val SPN_find_at(SPN_Span span, const void * element, size_t at_idx, size_t * o_idx) {
  if(!is_valid(span)) return LOG_STAT(STAT_ERR_ARGS, "span not valid");
  if(element == NULL) return LOG_STAT(STAT_ERR_ARGS, "element is NULL");

  for(size_t i = at_idx; i < span.len; i++) {
    if(memcmp(SPN_get(span, i), element, span.element_size) == 0) {
      if(o_idx != NULL) *o_idx = i;
      return OK;
    };
  }

  return STAT_OK_NOT_FOUND;
}

STAT_Val SPN_find_reverse(SPN_Span span, const void * element, size_t * o_idx) {
  if(span.len == 0) return STAT_OK_NOT_FOUND;
  return SPN_find_reverse_at(span, element, span.len - 1, o_idx);
}

STAT_Val SPN_find_reverse_at(SPN_Span span, const void * element, size_t at_idx, size_t * o_idx) {
  if(!is_valid(span)) return LOG_STAT(STAT_ERR_ARGS, "span not valid");
  if(element == NULL) return LOG_STAT(STAT_ERR_ARGS, "element is NULL");
  if(span.len == 0) return STAT_OK_NOT_FOUND;
  if(at_idx >= span.len) at_idx = (span.len - 1);

  // we start at +1 and then decrement at the start of the iteration
  size_t i = at_idx + 1;
  do {
    i--;
    if(memcmp(SPN_get(span, i), element, span.element_size) == 0) {
      if(o_idx != NULL) *o_idx = i;
      return OK;
    }
  } while(i != 0);

  return STAT_OK_NOT_FOUND;
}

STAT_Val SPN_find_subspan(SPN_Span span, SPN_Span subspan, size_t * o_idx) {
  return SPN_find_subspan_at(span, subspan, 0, o_idx);
}

STAT_Val SPN_find_subspan_at(SPN_Span span, SPN_Span subspan, size_t at_idx, size_t * o_idx) {
  if(!is_valid(span)) return LOG_STAT(STAT_ERR_ARGS, "span not valid");
  if(!is_valid(subspan)) return LOG_STAT(STAT_ERR_ARGS, "subspan not valid");
  if(span.element_size != subspan.element_size) {
    return LOG_STAT(STAT_ERR_ARGS, "span and subspan have different element sizes");
  }
  if((at_idx + subspan.len) > span.len) return STAT_OK_NOT_FOUND;

  const size_t subsp_size_bytes = SPN_get_size_in_bytes(subspan);

  for(size_t i = at_idx; i <= (span.len - subspan.len); i++) {
    if(memcmp(SPN_get(span, i), subspan.begin, subsp_size_bytes) == 0) {
      if(o_idx != NULL) *o_idx = i;
      return OK;
    }
  }

  return STAT_OK_NOT_FOUND;
}

STAT_Val SPN_find_subspan_reverse(SPN_Span span, SPN_Span subspan, size_t * o_idx) {
  if(!is_valid(span)) return LOG_STAT(STAT_ERR_ARGS, "span not valid");
  if(!is_valid(subspan)) return LOG_STAT(STAT_ERR_ARGS, "subspan not valid");
  if(span.element_size != subspan.element_size) {
    return LOG_STAT(STAT_ERR_ARGS, "span and subspan have different element sizes");
  }

  if(span.len < subspan.len) return STAT_OK_NOT_FOUND;
  if(span.len == 0) { // implies subspan.len == 0
    if(o_idx != NULL) *o_idx = 0;
    return OK;
  }

  return SPN_find_subspan_reverse_at(span, subspan, (span.len - 1), o_idx);
}

STAT_Val SPN_find_subspan_reverse_at(SPN_Span span,
                                     SPN_Span subspan,
                                     size_t   at_idx,
                                     size_t * o_idx) {
  if(!is_valid(span)) return LOG_STAT(STAT_ERR_ARGS, "span not valid");
  if(!is_valid(subspan)) return LOG_STAT(STAT_ERR_ARGS, "subspan not valid");
  if(span.element_size != subspan.element_size) {
    return LOG_STAT(STAT_ERR_ARGS, "span and subspan have different element sizes");
  }
  if(span.len < subspan.len) return STAT_OK_NOT_FOUND;
  if(at_idx >= span.len) at_idx = (span.len - 1);
  if((at_idx + subspan.len) > span.len) at_idx = (span.len - subspan.len);

  const size_t subsp_size_bytes = SPN_get_size_in_bytes(subspan);

  // we start at +1 and then decrement at the start of the iteration
  size_t i = at_idx + 1;
  do {
    i--;
    if(memcmp(SPN_get(span, i), subspan.begin, subsp_size_bytes) == 0) {
      if(o_idx != NULL) *o_idx = i;
      return OK;
    }
  } while(i != 0);

  return STAT_OK_NOT_FOUND;
}

void SPN_swap(SPN_MutSpan span, size_t idx_a, size_t idx_b) {
  if(idx_a != idx_b) {
    // we swap byte-by-byte, so that we don't need any dynamically sized allocation
    uint8_t * a_as_bytes = (uint8_t *)SPN_get(span, idx_a);
    uint8_t * b_as_bytes = (uint8_t *)SPN_get(span, idx_b);
    for(size_t i = 0; i < span.element_size; i++) {
      const uint8_t tmp = a_as_bytes[i];
      a_as_bytes[i]     = b_as_bytes[i];
      b_as_bytes[i]     = tmp;
    }
  }
}

STAT_Val SPN_swap_checked(SPN_MutSpan span, size_t idx_a, size_t idx_b) {
  if(span.begin == NULL || span.element_size == 0)
    return LOG_STAT(STAT_ERR_ARGS, "span has no data is NULL");

  if(idx_a >= span.len) {
    return LOG_STAT(STAT_ERR_RANGE, "idx_a %zu out of range (size=%zu)", idx_a, span.len);
  }
  if(idx_b >= span.len) {
    return LOG_STAT(STAT_ERR_RANGE, "idx_b %zu out of range (size=%zu)", idx_b, span.len);
  }

  SPN_swap(span, idx_a, idx_b);

  return OK;
}
