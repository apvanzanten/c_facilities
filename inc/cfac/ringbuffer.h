// MIT License
//
// Copyright (c) 2024 Arjen P. van Zanten
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

#ifndef CFAC_RING_BUFFER_H
#define CFAC_RING_BUFFER_H

#include <stdbool.h>
#include <stddef.h>

#include "darray.h"
#include "stat.h"

typedef struct RBUF_RingBuffer {
  DAR_DArray buffer;
  size_t     begin_idx;
  size_t     end_idx;
  bool       is_empty;
} RBUF_RingBuffer;

STAT_Val RBUF_create(RBUF_RingBuffer * this, size_t element_size, size_t capacity);
STAT_Val RBUF_destroy(RBUF_RingBuffer * this);

STAT_Val RBUF_push_back(RBUF_RingBuffer * this, const void * val_p);
STAT_Val RBUF_try_push_back(RBUF_RingBuffer * this, const void * val_p);
STAT_Val RBUF_pop_front(RBUF_RingBuffer * this);

static inline bool RBUF_is_initialized(const RBUF_RingBuffer * this) {
  return DAR_is_initialized(&this->buffer);
}

static inline bool RBUF_is_empty(const RBUF_RingBuffer * this) {
  return (this == NULL) || this->is_empty;
}

// NOTE we choose 'has space' instead of 'is full' deliberately, so that we can logically return
// false if this == NULL (an invalid buffer can be said to not have space, but it can't really be
// said to be full).
static inline bool RBUF_has_space(const RBUF_RingBuffer * this) {
  return (this != NULL) && (this->is_empty || (this->begin_idx != this->end_idx));
}

//  [const] void * RBUF_peek([const] RBUF_RingBuffer *this)
#define RBUF_peek(this)                                                                            \
  _Generic((this),                                                                                 \
      const RBUF_RingBuffer *: RBUF_INT_peek_const,                                                \
      RBUF_RingBuffer *: RBUF_INT_peek_nonconst)(this)

static inline void * RBUF_INT_peek_nonconst(RBUF_RingBuffer * this) {
  return DAR_get(&this->buffer, this->begin_idx);
}
static inline const void * RBUF_INT_peek_const(const RBUF_RingBuffer * this) {
  return DAR_get(&this->buffer, this->begin_idx);
}

static inline size_t RBUF_get_num_items_on_buffer(const RBUF_RingBuffer * this) {
  if(this == NULL || this->is_empty) return 0;
  if(this->begin_idx < this->end_idx) return (this->end_idx - this->begin_idx);
  if(this->begin_idx == this->end_idx) return this->buffer.size;
  return (this->buffer.size - (this->begin_idx - this->end_idx));
}

static inline size_t RBUF_get_space_in_num_items(const RBUF_RingBuffer * this) {
  if(this == NULL) return 0;
  return this->buffer.size - RBUF_get_num_items_on_buffer(this);
}

#endif
