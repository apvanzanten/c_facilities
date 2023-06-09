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

#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG_SIZE        2040
#define MSG_TERMINATOR_SIZE 8
#define MAX_MSG_BODY_SIZE   (MAX_MSG_SIZE - MSG_TERMINATOR_SIZE)

static void (*g_log_func)(const char *, size_t) = NULL;

void LOG_set_log_func(void (*func)(const char *, size_t)) { g_log_func = func; }

static void write_to_log(STAT_Val          stat,
                         const char *      stat_str,
                         LOG_INT_Location location,
                         const char *      fmt,
                         va_list           args);

static int write_location_to_msg(LOG_INT_Location location, char * msg, size_t max_len);

STAT_Val LOG_INT_stat(STAT_Val          stat,
                       const char *      stat_str,
                       LOG_INT_Location location,
                       const char *      fmt,
                       ...) {
  va_list args;
  va_start(args, fmt);

  write_to_log(stat, stat_str, location, fmt, args);

  va_end(args);

  return stat;
}

STAT_Val LOG_INT_stat_if(bool              condition,
                          STAT_Val          stat,
                          const char *      stat_str,
                          LOG_INT_Location location,
                          const char *      fmt,
                          ...) {
  if(condition) {
    va_list args;
    va_start(args, fmt);

    write_to_log(stat, stat_str, location, fmt, args);

    va_end(args);
  }

  return stat;
}

STAT_Val LOG_INT_stat_if_err(STAT_Val          stat,
                              const char *      stat_str,
                              LOG_INT_Location location,
                              const char *      fmt,
                              ...) {
  if(STAT_is_ERR(stat)) {
    va_list args;
    va_start(args, fmt);

    write_to_log(stat, stat_str, location, fmt, args);

    va_end(args);
  }
  return stat;
}

STAT_Val LOG_INT_stat_if_nok(STAT_Val          stat,
                              const char *      stat_str,
                              LOG_INT_Location location,
                              const char *      fmt,
                              ...) {

  if(!STAT_is_OK(stat)) {
    va_list args;
    va_start(args, fmt);

    write_to_log(stat, stat_str, location, fmt, args);

    va_end(args);
  }
  return stat;
}

static int write_location_to_msg(LOG_INT_Location location, char * msg, size_t max_len) {
  const char * file_basename = strrchr(location.file, '/'); // find last '/'

  if(file_basename != NULL) file_basename++; // skip actual '/'

  if(file_basename == NULL || *file_basename == '\0') file_basename = location.file;

  const int num_written =
      snprintf(msg, max_len, "%s:%d:%s", file_basename, location.line, location.func);

  return num_written;
}

static void write_to_log(STAT_Val          stat,
                         const char *      stat_str,
                         LOG_INT_Location location,
                         const char *      fmt,
                         va_list           args) {

  char msg[MAX_MSG_SIZE] = "";
  int  msg_len           = 0;

  const char * str_from_stat = STAT_to_str(stat);

  msg_len += snprintf(msg, MAX_MSG_BODY_SIZE, "! %s", str_from_stat);

  if((msg_len < MAX_MSG_BODY_SIZE) && (strcmp(stat_str, str_from_stat) != 0)) {
    msg_len += snprintf(&msg[msg_len], (MAX_MSG_BODY_SIZE - msg_len), " (from `%s`)", stat_str);
  }

  if(msg_len < MAX_MSG_BODY_SIZE) {
    msg_len += snprintf(&msg[msg_len], (MAX_MSG_BODY_SIZE - msg_len), " at ");
  }

  if(msg_len < MAX_MSG_BODY_SIZE) {
    msg_len += write_location_to_msg(location, &msg[msg_len], (MAX_MSG_BODY_SIZE - msg_len));
  }

  if(msg_len < MAX_MSG_BODY_SIZE) {
    msg_len += snprintf(&msg[msg_len], (MAX_MSG_BODY_SIZE - msg_len), ": \"");
  }

  if(msg_len < MAX_MSG_BODY_SIZE) {
    msg_len += vsnprintf(&msg[msg_len], (MAX_MSG_BODY_SIZE - msg_len), fmt, args);
  }

  msg_len += snprintf(&msg[msg_len], (MAX_MSG_SIZE - msg_len), "\"\n");

  if(g_log_func != NULL) {
    g_log_func(msg, msg_len);
  } else { // if there is no log functions set, we just print to stderr as fallback
    fputs(msg, stderr);
  }
}