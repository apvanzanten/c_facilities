#ifndef LOG_H
#define LOG_H

#include "stat.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  const char * file;
  int          line;
  const char * func;
} LOG_IMPL_Location;

STAT_Val LOG_IMPL_stat(STAT_Val          stat,
                       const char *      err_str,
                       LOG_IMPL_Location location,
                       const char *      fmt,
                       ...);

STAT_Val LOG_IMPL_stat_if(bool              condition,
                          STAT_Val          stat,
                          const char *      err_str,
                          LOG_IMPL_Location location,
                          const char *      fmt,
                          ...);

STAT_Val LOG_IMPL_stat_if_err(STAT_Val          stat,
                              const char *      err_str,
                              LOG_IMPL_Location location,
                              const char *      fmt,
                              ...);

#define LOG_STAT(stat, ...)                                                                        \
  LOG_IMPL_stat((stat), #stat, (LOG_IMPL_Location){__FILE__, __LINE__, __func__}, __VA_ARGS__)

#define LOG_STAT_IF(condition, stat, ...)                                                          \
  LOG_IMPL_stat_if((condition),                                                                    \
                   (stat),                                                                         \
                   #stat,                                                                          \
                   (LOG_IMPL_Location){__FILE__, __LINE__, __func__},                              \
                   __VA_ARGS__)

#define LOG_STAT_IF_ERR(stat, ...)                                                                 \
  LOG_IMPL_stat_if_err((stat),                                                                     \
                       #stat,                                                                      \
                       (LOG_IMPL_Location){__FILE__, __LINE__, __func__},                          \
                       __VA_ARGS__)

void LOG_set_log_func(void (*func)(const char *, size_t));

#endif
