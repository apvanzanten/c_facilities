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

#ifndef CFAC_LOG_H
#define CFAC_LOG_H

#include "stat.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  const char * file;
  int          line;
  const char * func;
} LOG_INT_Location;

STAT_Val LOG_INT_stat(STAT_Val         stat,
                      const char *     stat_str,
                      LOG_INT_Location location,
                      const char *     fmt,
                      ...);

STAT_Val LOG_INT_stat_if(bool             condition,
                         STAT_Val         stat,
                         const char *     stat_str,
                         LOG_INT_Location location,
                         const char *     fmt,
                         ...);

STAT_Val LOG_INT_stat_if_err(STAT_Val         stat,
                             const char *     stat_str,
                             LOG_INT_Location location,
                             const char *     fmt,
                             ...);

STAT_Val LOG_INT_stat_if_nok(STAT_Val         stat,
                             const char *     stat_str,
                             LOG_INT_Location location,
                             const char *     fmt,
                             ...);

#define LOG_STAT(stat, ...)                                                                        \
  LOG_INT_stat((stat), #stat, (LOG_INT_Location){__FILE__, __LINE__, __func__}, __VA_ARGS__)

#define LOG_STAT_IF(condition, stat, ...)                                                          \
  LOG_INT_stat_if((condition),                                                                     \
                  (stat),                                                                          \
                  #stat,                                                                           \
                  (LOG_INT_Location){__FILE__, __LINE__, __func__},                                \
                  __VA_ARGS__)

#define LOG_STAT_IF_ERR(stat, ...)                                                                 \
  LOG_INT_stat_if_err((stat), #stat, (LOG_INT_Location){__FILE__, __LINE__, __func__}, __VA_ARGS__)

#define LOG_STAT_IF_NOK(stat, ...)                                                                 \
  LOG_INT_stat_if_nok((stat), #stat, (LOG_INT_Location){__FILE__, __LINE__, __func__}, __VA_ARGS__)

void LOG_set_log_func(void (*func)(const char *, size_t));

#endif
