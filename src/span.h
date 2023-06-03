#ifndef CFAC_SPAN_H
#define CFAC_SPAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stat.h"

typedef struct {
  const void * begin;
  uint32_t     len;          // len in number of elements
  uint32_t     element_size; // size of an element in bytes
} SPN_Span;

// TODO const span

SPN_Span SPN_from_cstr(const char * cstr);

SPN_Span SPN_subspan(SPN_Span src, uint32_t begin_idx, uint32_t len);

bool SPN_equals(SPN_Span lhs, SPN_Span rhs);

bool SPN_contains_subspan(SPN_Span span, SPN_Span subspan);

STAT_Val SPN_find(SPN_Span span, const void * element, uint32_t * o_idx);
STAT_Val SPN_find_at(SPN_Span span, const void * element, uint32_t at_idx, uint32_t * o_idx);

STAT_Val SPN_find_reverse(SPN_Span span, const void * element, uint32_t * o_idx);
STAT_Val SPN_find_reverse_at(SPN_Span     span,
                             const void * element,
                             uint32_t     at_idx,
                             uint32_t *   o_idx);

STAT_Val SPN_find_subspan(SPN_Span span, SPN_Span subspan, uint32_t * o_idx);
STAT_Val SPN_find_subspan_at(SPN_Span span, SPN_Span subspan, uint32_t at_idx, uint32_t * o_idx);
STAT_Val SPN_find_subspan_reverse(SPN_Span span, SPN_Span subspan, uint32_t * o_idx);
STAT_Val SPN_find_subspan_reverse_at(SPN_Span   span,
                                     SPN_Span   subspan,
                                     uint32_t   at_idx,
                                     uint32_t * o_idx);

inline static size_t SPN_get_byte_idx(SPN_Span src, uint32_t idx) {
  return ((size_t)src.element_size) * ((size_t)idx);
}

inline static const void * SPN_get(SPN_Span src, uint32_t idx) {
  return (const void *)(&(((const uint8_t *)src.begin)[SPN_get_byte_idx(src, idx)]));
}

inline static size_t SPN_get_size_in_bytes(SPN_Span span) {
  return ((size_t)span.len * (size_t)span.element_size);
}

inline static bool SPN_is_empty(SPN_Span span) {
  return ((span.begin == NULL) || (SPN_get_size_in_bytes(span) == 0));
}

#endif
