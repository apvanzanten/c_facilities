#include "span.h"

#include "log.h"
#include "stat.h"

#include <string.h>

#define OK STAT_OK

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
  if(span.begin == NULL || subspan.begin == NULL) return false;
  if(span.element_size != subspan.element_size) return false;
  if(span.len < subspan.len) return false;
  if(subspan.len == 0) return true;

  return false; // TODO

  // return (memmem(span.begin,
  //                SPN_get_size_in_bytes(span),
  //                subspan.begin,
  //                SPN_get_size_in_bytes(subspan)) != NULL);
}
