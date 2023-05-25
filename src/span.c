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

SPN_Span SPN_subspan(SPN_Span src, uint32_t begin_idx, uint32_t len) {
  if(begin_idx > src.len) begin_idx = src.len;
  if(begin_idx + len > src.len) len = (src.len - begin_idx);
  return (SPN_Span){.begin = SPN_get(src, begin_idx), .len = len, .element_size = src.element_size};
}

bool SPN_equals(SPN_Span lhs, SPN_Span rhs) {
  if(lhs.len != rhs.len) return false;
  if(lhs.element_size != rhs.element_size) return false;
  if(lhs.begin == rhs.begin) return true;
  return (memcmp(lhs.begin, rhs.begin, SPN_get_size_in_bytes(lhs)) == 0);
}

bool SPN_contains_subspan(SPN_Span span, SPN_Span subspan) {
  if(span.element_size != subspan.element_size) return false;
  if(subspan.len == 0) return true;
  if(span.begin == NULL || subspan.begin == NULL) return false;
  if(span.len < subspan.len) return false;

  // NOTE this could probably be faster if we did our own comparisons

  for(uint32_t i = 0; i <= (span.len - subspan.len); i++) {
    if(memcmp(SPN_get(span, i), subspan.begin, SPN_get_size_in_bytes(subspan)) == 0) return true;
  }

  return false;
}

STAT_Val SPN_find(SPN_Span span, const void * element, uint32_t * o_idx) {
  return SPN_find_at(span, element, 0, o_idx);
}

STAT_Val SPN_find_at(SPN_Span span, const void * element, uint32_t at_idx, uint32_t * o_idx) {
  if(!is_valid(span)) return LOG_STAT(STAT_ERR_ARGS, "span not valid");
  if(element == NULL) return LOG_STAT(STAT_ERR_ARGS, "element is NULL");

  for(uint32_t i = at_idx; i < span.len; i++) {
    if(memcmp(SPN_get(span, i), element, span.element_size) == 0) {
      if(o_idx != NULL) *o_idx = i;
      return OK;
    };
  }

  return STAT_OK_NOT_FOUND;
}

STAT_Val SPN_find_reverse(SPN_Span span, const void * element, uint32_t * o_idx) {
  if(span.len == 0) return STAT_OK_NOT_FOUND;
  return SPN_find_reverse_at(span, element, span.len - 1, o_idx);
}

STAT_Val SPN_find_reverse_at(SPN_Span     span,
                             const void * element,
                             uint32_t     at_idx,
                             uint32_t *   o_idx) {
  if(!is_valid(span)) return LOG_STAT(STAT_ERR_ARGS, "span not valid");
  if(element == NULL) return LOG_STAT(STAT_ERR_ARGS, "element is NULL");

  // we start at +1 and then decrement at the start of the iteration
  uint32_t i = at_idx + 1;
  do {
    i--;
    if(memcmp(SPN_get(span, i), element, span.element_size) == 0) {
      if(o_idx != NULL) *o_idx = i;
      return OK;
    }
  } while(i != 0);

  return STAT_OK_NOT_FOUND;
}
