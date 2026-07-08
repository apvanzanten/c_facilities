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

#define MSG_TERMINATOR      ((const char[]){"\"\n"})
#define MSG_TERMINATOR_SIZE sizeof(MSG_TERMINATOR)
#define MAX_MSG_SIZE        (LOG_MAX_MSG_BODY_SIZE + MSG_TERMINATOR_SIZE)

static void (*g_log_func)(const char *, size_t) = NULL;

void LOG_set_log_func(void (*func)(const char *, size_t)) { g_log_func = func; }

static void write_to_log(STAT_Val         stat,
                         const char *     stat_str,
                         LOG_INT_Location location,
                         const char *     fmt,
                         va_list          args);

static size_t write_location_to_msg(LOG_INT_Location location, char * msg, size_t max_len);

static size_t vsnprintf2(char * buff, size_t buff_size, const char * fmt, va_list args);
static size_t snprintf2(char * buff, size_t buff_size, const char * fmt, ...);

STAT_Val LOG_INT_stat(STAT_Val         stat,
                      const char *     stat_str,
                      LOG_INT_Location location,
                      const char *     fmt,
                      ...) {
  va_list args;
  va_start(args, fmt);

  write_to_log(stat, stat_str, location, fmt, args);

  va_end(args);

  return stat;
}

STAT_Val LOG_INT_stat_if(bool             condition,
                         STAT_Val         stat,
                         const char *     stat_str,
                         LOG_INT_Location location,
                         const char *     fmt,
                         ...) {
  if(condition) {
    va_list args;
    va_start(args, fmt);

    write_to_log(stat, stat_str, location, fmt, args);

    va_end(args);
  }

  return stat;
}

STAT_Val LOG_INT_stat_if_err(STAT_Val         stat,
                             const char *     stat_str,
                             LOG_INT_Location location,
                             const char *     fmt,
                             ...) {
  if(STAT_is_ERR(stat)) {
    va_list args;
    va_start(args, fmt);

    write_to_log(stat, stat_str, location, fmt, args);

    va_end(args);
  }
  return stat;
}

STAT_Val LOG_INT_stat_if_nok(STAT_Val         stat,
                             const char *     stat_str,
                             LOG_INT_Location location,
                             const char *     fmt,
                             ...) {

  if(!STAT_is_OK(stat)) {
    va_list args;
    va_start(args, fmt);

    write_to_log(stat, stat_str, location, fmt, args);

    va_end(args);
  }
  return stat;
}

void LOG_report_settings(void) {
  LOG_STAT(STAT_OK_INFO,
           "LOG_MAX_MSG_BODY_SIZE=%zu, MAX_MSG_SIZE=%zu",
           LOG_MAX_MSG_BODY_SIZE,
           MAX_MSG_SIZE);

  if(g_log_func != NULL) {
    LOG_STAT(STAT_OK_INFO, "custom log func set to addr: %p", g_log_func);
  }
}

static size_t write_location_to_msg(LOG_INT_Location location, char * msg, size_t max_len) {
  const char * file_basename = strrchr(location.file, '/'); // find last '/'

  if(file_basename != NULL) file_basename++; // skip actual '/'

  if(file_basename == NULL || *file_basename == '\0') file_basename = location.file;

  const size_t num_written =
      snprintf2(msg, max_len, "%s:%d:%s", file_basename, location.line, location.func);

  return num_written;
}

static void write_to_log(STAT_Val         stat,
                         const char *     stat_str,
                         LOG_INT_Location location,
                         const char *     fmt,
                         va_list          args) {

  char   msg[MAX_MSG_SIZE] = "";
  size_t msg_len           = 0;

  const char * str_from_stat = ((stat == STAT_OK_INFO) ? "INFO" : STAT_to_str(stat));
  const char   prefix_char   = (STAT_is_OK(stat) ? '-' : STAT_is_WRN(stat) ? '~' : '!');

  msg_len += snprintf2(msg, LOG_MAX_MSG_BODY_SIZE, "%c%s", prefix_char, str_from_stat);

  if((stat != STAT_OK_INFO) && (msg_len < LOG_MAX_MSG_BODY_SIZE) &&
     (strcmp(stat_str, str_from_stat) != 0)) {
    msg_len +=
        snprintf2(&msg[msg_len], (LOG_MAX_MSG_BODY_SIZE - msg_len), " (from `%s`)", stat_str);
  }

  if(msg_len < LOG_MAX_MSG_BODY_SIZE) {
    msg_len += snprintf2(&msg[msg_len], (LOG_MAX_MSG_BODY_SIZE - msg_len), " at ");
  }

  if(msg_len < LOG_MAX_MSG_BODY_SIZE) {
    msg_len += write_location_to_msg(location, &msg[msg_len], (LOG_MAX_MSG_BODY_SIZE - msg_len));
  }

  if(msg_len < LOG_MAX_MSG_BODY_SIZE) {
    msg_len += snprintf2(&msg[msg_len], (LOG_MAX_MSG_BODY_SIZE - msg_len), ": \"");
  }

  if(msg_len < LOG_MAX_MSG_BODY_SIZE) {
    msg_len += vsnprintf2(&msg[msg_len], (LOG_MAX_MSG_BODY_SIZE - msg_len), fmt, args);
  }

  msg_len += snprintf2(&msg[msg_len], MAX_MSG_SIZE, "%s", MSG_TERMINATOR);

  if(g_log_func != NULL) {
    g_log_func(msg, msg_len);
  } else { // if there is no log functions set, we just print to stderr as fallback
    fputs(msg, stderr);
  }
}

static size_t vsnprintf2(char * buff, size_t buff_size, const char * fmt, va_list args) {
  if((buff == NULL) || (buff_size == 0)) return 0;

  // wrapper around vsnprintf that makes the interface less obtuse
  const int r = vsnprintf(buff, buff_size, fmt, args);

  // In the rare case that r is negative due to an error with printing, we just set the empty string
  // and return 0, to indicate no characters were written. Realistically, there's no graceful error
  // recovery behavior possible anyway.

  // note buff_size - 1, to exclude the null-terminator
  const size_t num_chars_printed = (r < 0)                         ? 0
                                   : ((size_t)r < (buff_size - 1)) ? (size_t)r
                                                                   : (buff_size - 1);
  if(num_chars_printed == 0) buff[0] = '\0';

  return num_chars_printed;
}
static size_t snprintf2(char * buff, size_t buff_size, const char * fmt, ...) {
  // wrapper around snprintf that makes the interface less obtuse
  va_list args;
  va_start(args, fmt);

  const size_t r = vsnprintf2(buff, buff_size, fmt, args);

  va_end(args);

  return r;
}
