#ifndef CFAC_SPAN_H
#define CFAC_SPAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  const void * begin;        // NOTE non-owning
  uint32_t     len;          // len in number of elements
  uint8_t      element_size; // size of an element in bytes
} SPN_Span;

SPN_Span SPN_from_cstr(const char * cstr);

SPN_Span SPN_subspan(SPN_Span src, uint32_t begin_idx, uint32_t len);

bool SPN_equals(SPN_Span lhs, SPN_Span rhs);

bool SPN_contains_subspan(SPN_Span span, SPN_Span subspan);

inline static size_t SPN_get_byte_idx(SPN_Span src, uint32_t idx) {
  return ((size_t)src.element_size) * ((size_t)idx);
}

inline static const void * SPN_get(SPN_Span src, uint32_t idx) {
  return (const void *)(&(((const uint8_t *)src.begin)[SPN_get_byte_idx(src, idx)]));
}

inline static size_t SPN_get_size_in_bytes(SPN_Span span) {
  return ((size_t)span.len * (size_t)span.element_size);
}

#endif
