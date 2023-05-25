#ifndef CFAC_STAT_H
#define CFAC_STAT_H

// NOTE We use only positive integers starting from 0x10000 to (try to) avoid overlapping with
// commonly used error code values.
// * Generally, error codes in the unix family are allowed to be any positive integer value that can
// fit in 'int', but then the size of 'int' is implementation defined (and at the time unix was
// developed it was not that common for it to be 32 bit, as it is now). In practice, modern linux
// and bsd systems tend to use values that stay within a 16-bit integer (whether this is deliberate,
// I don't know).
// * Win32 error codes are all in the range 0x0000-0xFFFF
// * Somewhat commonly C standard library functions use negative integers to indicate an error.

// NOTE Windows HRESULT and NTSTATUS assign a particular value to each bit of a 32-bit value, and
// thus can't be worked around within a 32-bit enum.

// NOTE pretty universally across error/status codes the value 0 indicates 'OK' or 'SUCCESS'

// NOTE we end up using a 32-bit integer value for this. I find this an acceptable size, as:
// * Most commonly developed-for architectures nowadays are 32-bit or 64-bit. Hence 32-bit integers
// are cheap to work with.
// * Of the commonly used architectures that are NOT 32-bit or 64-bit, most are 8-bit, and moving
// down to an 8-bit integer would be a big constraint. It makes more sense to adapt/rewrite this
// header to suit your needs if you are working on an 8-bit platform.

// NOTE the different kinds of values are:
// * OK: status is OK, and we may want to provide some more information, e.g.
//  - STAT_OK is simply OK/SUCCESS
//  - STAT_OK_BUSY and STAT_OK_FINISHED might be used to indicate some kind of worker is fully
//  operational and OK, and also whether or not it still has more work to do.
//  - STAT_OK_TRUE and STAT_OK_FALSE can be used to not only indicate that some operation that
//  should yield a boolean went OK, but also what that boolean was.
//  - STAT_OK_FULL might be used when writing data to some buffer, and after writing that data the
//  buffer is full (thus allowing the client to know ahead of time any further write will fail).
// * WRN: operation was succesful, but something looks like it's probably not right. e.g.
//  - STAT_WRN_OVERWRITTEN might be used to indicate that some data structure was expected to be
//  empty, but was not, and the function overwrote the data that was there with its own data.

// NOTE we distribute our types of codes over distinct ranges, so we can check whether a value is of
// a certain type through comparison
#define STAT_IMPL_OK_RANGE_FIRST  0x10000
#define STAT_IMPL_OK_RANGE_LAST   (STAT_IMPL_OK_RANGE_FIRST + 0xffff)
#define STAT_IMPL_WRN_RANGE_FIRST 0x20000
#define STAT_IMPL_WRN_RANGE_LAST  (STAT_IMPL_WRN_RANGE_FIRST + 0xffff)
#define STAT_IMPL_ERR_RANGE_FIRST 0x30000
#define STAT_IMPL_ERR_RANGE_LAST  (STAT_IMPL_ERR_RANGE_FIRST + 0xffff)

#include <stdbool.h>

typedef enum {

  // OKs
  STAT_OK = STAT_IMPL_OK_RANGE_FIRST,
  STAT_OK_BUSY,
  STAT_OK_FINISHED,
  STAT_OK_TRUE,
  STAT_OK_FALSE,
  STAT_OK_NOT_FOUND,

  // warnings
  STAT_WRN_OVERWRITTEN = STAT_IMPL_WRN_RANGE_FIRST,
  STAT_WRN_NOTHING_TO_DO,

  // errors
  STAT_ERR_ARGS = STAT_IMPL_ERR_RANGE_FIRST,
  STAT_ERR_USAGE,
  STAT_ERR_UNIMPLEMENTED,
  STAT_ERR_PRECONDITION,
  STAT_ERR_RANGE,
  STAT_ERR_EMPTY,
  STAT_ERR_FULL,
  STAT_ERR_INTERNAL,
  STAT_ERR_FATAL,
  STAT_ERR_IO,
  STAT_ERR_READ,
  STAT_ERR_WRITE,
  STAT_ERR_ALLOC,
  STAT_ERR_NOT_FOUND,
  STAT_ERR_DUPLICATE,
  STAT_ERR_PARSE,
  STAT_ERR_COMPILE,
  STAT_ERR_RUNTIME,
} STAT_Val;

static inline const char * STAT_to_str(STAT_Val code) {
  switch(code) {
  // OKs
  case STAT_OK: return "STAT_OK";
  case STAT_OK_BUSY: return "STAT_OK_BUSY";
  case STAT_OK_FINISHED: return "STAT_OK_FINISHED";
  case STAT_OK_TRUE: return "STAT_OK_TRUE";
  case STAT_OK_FALSE: return "STAT_OK_FALSE";
  case STAT_OK_NOT_FOUND: return "STAT_OK_NOT_FOUND";

  // warnings
  case STAT_WRN_OVERWRITTEN: return "STAT_WRN_OVERWRITTEN";
  case STAT_WRN_NOTHING_TO_DO: return "STAT_WRN_NOTHING_TO_DO";

  // errors
  case STAT_ERR_ARGS: return "STAT_ERR_ARGS";
  case STAT_ERR_USAGE: return "STAT_ERR_USAGE";
  case STAT_ERR_UNIMPLEMENTED: return "STAT_ERR_UNIMPLEMENTED";
  case STAT_ERR_PRECONDITION: return "STAT_ERR_PRECONDITION";
  case STAT_ERR_RANGE: return "STAT_ERR_RANGE";
  case STAT_ERR_EMPTY: return "STAT_ERR_EMPTY";
  case STAT_ERR_FULL: return "STAT_ERR_FULL";
  case STAT_ERR_INTERNAL: return "STAT_ERR_INTERNAL";
  case STAT_ERR_FATAL: return "STAT_ERR_FATAL";
  case STAT_ERR_IO: return "STAT_ERR_IO";
  case STAT_ERR_READ: return "STAT_ERR_READ";
  case STAT_ERR_WRITE: return "STAT_ERR_WRITE";
  case STAT_ERR_ALLOC: return "STAT_ERR_ALLOC";
  case STAT_ERR_NOT_FOUND: return "STAT_ERR_NOT_FOUND";
  case STAT_ERR_DUPLICATE: return "STAT_ERR_DUPLICATE";
  case STAT_ERR_PARSE: return "STAT_ERR_PARSE";
  case STAT_ERR_COMPILE: return "STAT_ERR_COMPILE";
  case STAT_ERR_RUNTIME: return "STAT_ERR_RUNTIME";
  }

  return "UNKNOWN STATUS CODE";
}

// NOTE these are NOT macros, as it would be surprisingly complicated to do this with a macro that
// doesn't evaluate the input multiple times.
static inline bool STAT_is_OK(STAT_Val val) {
  return ((int)val >= STAT_IMPL_OK_RANGE_FIRST) && ((int)val <= STAT_IMPL_OK_RANGE_LAST);
}
static inline bool STAT_is_WRN(STAT_Val val) {
  return ((int)val >= STAT_IMPL_WRN_RANGE_FIRST) && ((int)val <= STAT_IMPL_WRN_RANGE_LAST);
}
static inline bool STAT_is_ERR(STAT_Val val) {
  return ((int)val >= STAT_IMPL_ERR_RANGE_FIRST) && ((int)val <= STAT_IMPL_ERR_RANGE_LAST);
}

static inline bool STAT_is_valid(STAT_Val val) {
  return STAT_is_OK(val) || STAT_is_WRN(val) || STAT_is_ERR(val);
}

#endif
