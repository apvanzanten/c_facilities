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

#include "ringbuffer.h"

#include "log.h"

#define OK STAT_OK

static void advance_begin_idx(RBUF_RingBuffer * this) {
  this->begin_idx++;
  if(this->begin_idx == this->buffer.size) this->begin_idx = 0;

  this->is_empty = (this->begin_idx == this->end_idx);
}
static void advance_end_idx(RBUF_RingBuffer * this) {
  this->end_idx++;
  if(this->end_idx == this->buffer.size) this->end_idx = 0;

  this->is_empty = false;
}

STAT_Val RBUF_create(RBUF_RingBuffer * this, size_t element_size, size_t capacity) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(element_size == 0) return LOG_STAT(STAT_ERR_ARGS, "element_size not allowed to be zero");
  if(capacity == 0) return LOG_STAT(STAT_ERR_ARGS, "capacity not allowed to be zero");

  *this          = (RBUF_RingBuffer){0};
  this->is_empty = true;

  if(STAT_is_ERR(DAR_create(&this->buffer, element_size)) ||
     STAT_is_ERR(DAR_resize_zeroed(&this->buffer, capacity))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to create and size buffer");
  }

  return OK;
}

STAT_Val RBUF_destroy(RBUF_RingBuffer * this) {
  if(this == NULL) return OK;

  if(STAT_is_ERR(DAR_destroy(&this->buffer))) {
    return LOG_STAT(STAT_ERR_INTERNAL, "failed to destroy buffer");
  }

  *this = (RBUF_RingBuffer){0};

  return OK;
}

STAT_Val RBUF_try_push_back(RBUF_RingBuffer * this, const void * val_p) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(!RBUF_is_initialized(this)) return LOG_STAT(STAT_ERR_PRECONDITION, "not initialized");

  if(!RBUF_has_space(this)) return STAT_OK_FULL;

  return LOG_STAT_IF_ERR(RBUF_push_back(this, val_p), "failed to push to ringbuffer");
}

STAT_Val RBUF_push_back(RBUF_RingBuffer * this, const void * val_p) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(!RBUF_is_initialized(this)) return LOG_STAT(STAT_ERR_PRECONDITION, "not initialized");

  if(!RBUF_has_space(this)) advance_begin_idx(this); // overwriting element, move begin

  DAR_set(&this->buffer, this->end_idx, val_p);
  advance_end_idx(this);

  return OK;
}

STAT_Val RBUF_pop_front(RBUF_RingBuffer * this) {
  if(this == NULL) return LOG_STAT(STAT_ERR_ARGS, "this is NULL");
  if(!RBUF_is_initialized(this)) return LOG_STAT(STAT_ERR_PRECONDITION, "not initialized");

  if(this->is_empty) return STAT_OK_EMPTY;

  advance_begin_idx(this);

  return OK;
}