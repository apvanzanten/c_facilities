#ifndef CFAC_SPAN_H
#define CFAC_SPAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stat.h"

typedef struct {
  const void * begin;
  size_t       len;          // len in number of elements
  size_t       element_size; // size of an element in bytes
} SPN_Span;

typedef struct {
  void * begin;
  size_t len;          // len in number of elements
  size_t element_size; // size of an element in bytes
} SPN_MutSpan;

SPN_Span    SPN_from_cstr(const char * cstr);
SPN_MutSpan SPN_mut_span_from_cstr(char * cstr);

SPN_Span SPN_subspan(SPN_Span src, size_t begin_idx, size_t len);

bool SPN_equals(SPN_Span lhs, SPN_Span rhs);

bool SPN_contains_subspan(SPN_Span span, SPN_Span subspan);

STAT_Val SPN_find(SPN_Span span, const void * element, size_t * o_idx);
STAT_Val SPN_find_at(SPN_Span span, const void * element, size_t at_idx, size_t * o_idx);

STAT_Val SPN_find_reverse(SPN_Span span, const void * element, size_t * o_idx);
STAT_Val SPN_find_reverse_at(SPN_Span span, const void * element, size_t at_idx, size_t * o_idx);

STAT_Val SPN_find_subspan(SPN_Span span, SPN_Span subspan, size_t * o_idx);
STAT_Val SPN_find_subspan_at(SPN_Span span, SPN_Span subspan, size_t at_idx, size_t * o_idx);
STAT_Val SPN_find_subspan_reverse(SPN_Span span, SPN_Span subspan, size_t * o_idx);
STAT_Val SPN_find_subspan_reverse_at(SPN_Span span,
                                     SPN_Span subspan,
                                     size_t   at_idx,
                                     size_t * o_idx);

void     SPN_swap(SPN_MutSpan span, size_t idx_a, size_t idx_b);
STAT_Val SPN_swap_checked(SPN_MutSpan span, size_t idx_a, size_t idx_b);

static inline SPN_Span SPN_mut_to_const(SPN_MutSpan span) {
  return (SPN_Span){.begin = span.begin, .len = span.len, .element_size = span.element_size};
}

inline static size_t SPN_get_byte_idx(SPN_Span sp, size_t idx) { return (sp.element_size * idx); }

#define SPN_get(sp, i) _Generic((sp), SPN_Span: SPN_INT_get, SPN_MutSpan: SPN_INT_get_mut)(sp, i)
inline static const void * SPN_INT_get(SPN_Span sp, size_t idx) {
  return &(((const uint8_t *)sp.begin)[sp.element_size * idx]);
}
inline static void * SPN_INT_get_mut(SPN_MutSpan sp, size_t idx) {
  return &(((uint8_t *)sp.begin)[sp.element_size * idx]);
}

#define SPN_first(sp) _Generic((sp), SPN_Span: SPN_INT_first, SPN_MutSpan: SPN_INT_first_mut)(sp)
inline static const void * SPN_INT_first(SPN_Span sp) { return SPN_get(sp, 0); }
inline static void *       SPN_INT_first_mut(SPN_MutSpan sp) { return SPN_get(sp, 0); }

#define SPN_last(sp) _Generic((sp), SPN_Span: SPN_INT_last, SPN_MutSpan: SPN_INT_last_mut)(sp)
inline static const void * SPN_INT_last(SPN_Span sp) { return SPN_get(sp, (sp.len - 1)); }
inline static void *       SPN_INT_last_mut(SPN_MutSpan sp) { return SPN_get(sp, (sp.len - 1)); }

#define SPN_end(sp) _Generic((sp), SPN_Span: SPN_INT_end, SPN_MutSpan: SPN_INT_end_mut)(sp)
inline static const void * SPN_INT_end(SPN_Span sp) { return SPN_get(sp, sp.len); }
inline static void *       SPN_INT_end_mut(SPN_MutSpan sp) { return SPN_get(sp, sp.len); }

inline static size_t SPN_get_size_in_bytes(SPN_Span sp) { return (sp.len * sp.element_size); }

inline static bool SPN_is_empty(SPN_Span sp) {
  return ((sp.begin == NULL) || (SPN_get_size_in_bytes(sp) == 0));
}

#endif
