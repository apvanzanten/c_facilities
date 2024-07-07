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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "stat.h"
#include "test_utils.h"

#include "log.h"

#define LOG_BUFFER_SIZE_MAX (LOG_MAX_MSG_BODY_SIZE * 8)

typedef struct {
  char   data[LOG_BUFFER_SIZE_MAX + 1];
  size_t size;
} LogBuffer;

static LogBuffer g_log_buff;

static void init_log_buff(void) { g_log_buff = (LogBuffer){0}; }

static void log_func(const char * msg, size_t n) {
  printf("logging: %.*s", (int)n, msg);

  if(g_log_buff.size + n <= LOG_BUFFER_SIZE_MAX) {

    for(size_t i = 0; i < n; i++) {
      g_log_buff.data[g_log_buff.size + i] = msg[i];
    }

    g_log_buff.size += n;
  }
}

static void setup_log_buffer_and_func(void) {
  init_log_buff();
  LOG_set_log_func(log_func);
}

static Result tst_LOG_STAT(void) {
  Result r = PASS;

  setup_log_buffer_and_func();

  EXPECT_EQ(&r, STAT_OK, LOG_STAT(STAT_OK, "test LOG_STAT, also %d", 42));

  EXPECT_NE(&r, 0, g_log_buff.size);
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "STAT_OK"));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, __func__));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "\"test LOG_STAT, also 42\""));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "\n"));

  init_log_buff(); // clear buff

  EXPECT_EQ(&r,
            STAT_ERR_FATAL,
            LOG_STAT(STAT_ERR_FATAL, "test LOG_STAT some %s, also %d", "more", 9001));

  EXPECT_NE(&r, 0, g_log_buff.size);
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "STAT_ERR_FATAL"));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, __func__));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "\"test LOG_STAT some more, also 9001\""));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "\n"));

  return r;
}

static Result tst_LOG_STAT_IF(void) {
  Result r = PASS;

  setup_log_buffer_and_func();

  EXPECT_EQ(&r, STAT_OK, LOG_STAT_IF(true, STAT_OK, "test LOG_STAT_IF, also %d", 42));

  EXPECT_NE(&r, 0, g_log_buff.size);
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "STAT_OK"));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, __func__));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "\"test LOG_STAT_IF, also 42\""));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "\n"));

  init_log_buff(); // clear buff

  EXPECT_EQ(&r, STAT_OK, LOG_STAT_IF(false, STAT_OK, ""));
  EXPECT_EQ(&r, STAT_ERR_ALLOC, LOG_STAT_IF(false, STAT_ERR_ALLOC, ""));
  EXPECT_EQ(&r, STAT_ERR_FATAL, LOG_STAT_IF(false, STAT_ERR_FATAL, ""));

  EXPECT_EQ(&r, 0, g_log_buff.size);

  return r;
}

static Result tst_LOG_STAT_IF_ERR(void) {
  Result r = PASS;

  setup_log_buffer_and_func();

  EXPECT_EQ(&r, STAT_OK, LOG_STAT_IF_ERR(STAT_OK, "test LOG_STAT_IF_ERR"));
  EXPECT_EQ(&r, STAT_OK_BUSY, LOG_STAT_IF_ERR(STAT_OK_BUSY, "test LOG_STAT_IF_ERR"));
  EXPECT_EQ(&r, STAT_OK_FINISHED, LOG_STAT_IF_ERR(STAT_OK_FINISHED, "test LOG_STAT_IF_ERR"));

  EXPECT_EQ(&r, 0, g_log_buff.size);

  init_log_buff(); // clear buff

  EXPECT_EQ(&r,
            STAT_ERR_ALLOC,
            LOG_STAT_IF_ERR(STAT_ERR_ALLOC,
                            "test LOG_STAT_IF_ERR with STAT_ERR_ALLOC, also %s",
                            "I cook socks"));

  EXPECT_NE(&r, 0, g_log_buff.size);
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "STAT_ERR_ALLOC"));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, __func__));
  EXPECT_NE(&r,
            NULL,
            strstr(g_log_buff.data,
                   "\"test LOG_STAT_IF_ERR with STAT_ERR_ALLOC, also I cook socks\""));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "\n"));

  return r;
}

static Result tst_LOG_STAT_IF_NOK(void) {

  Result r = PASS;

  setup_log_buffer_and_func();

  EXPECT_EQ(&r, STAT_OK, LOG_STAT_IF_NOK(STAT_OK, "test LOG_STAT_IF_NOK"));
  EXPECT_EQ(&r, STAT_OK_BUSY, LOG_STAT_IF_NOK(STAT_OK_BUSY, "test LOG_STAT_IF_NOK"));
  EXPECT_EQ(&r, STAT_OK_FINISHED, LOG_STAT_IF_NOK(STAT_OK_FINISHED, "test LOG_STAT_IF_NOK"));

  EXPECT_EQ(&r, 0, g_log_buff.size);

  init_log_buff(); // clear buff

  EXPECT_EQ(&r,
            STAT_WRN_OVERWRITTEN,
            LOG_STAT_IF_NOK(STAT_WRN_OVERWRITTEN,
                            "test LOG_STAT_IF_NOK with STAT_WRN_OVERWRITTEN, also %d %s",
                            7,
                            "is a cool number"));

  EXPECT_NE(&r, 0, g_log_buff.size);
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "STAT_WRN_OVERWRITTEN"));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, __func__));
  EXPECT_NE(&r,
            NULL,
            strstr(g_log_buff.data,
                   "\"test LOG_STAT_IF_NOK with STAT_WRN_OVERWRITTEN, also 7 is a cool number\""));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "\n"));

  return r;
}

static Result tst_log_report_settings(void) {
  Result r = PASS;

  setup_log_buffer_and_func();

  LOG_report_settings();

  return r;
}

static Result tst_max_out_log_msg_size(void) {
  Result r = PASS;

  setup_log_buffer_and_func();

  const size_t buff_size = 100000;

  char * buff = malloc(buff_size);
  EXPECT_NE(&r, NULL, buff);
  if(HAS_FAILED(&r)) return r;

  char c = 'a';
  for(size_t i = 0; i < buff_size - 1; i++) {
    buff[i] = c;

    if(c == 'z') {
      c = 'a';
    } else {
      c++;
    }
  }

  buff[buff_size - 1] = '\0';

  LOG_STAT(STAT_OK_INFO, "big msg!: %s", buff);

  free(buff);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_LOG_STAT,
      tst_LOG_STAT_IF,
      tst_LOG_STAT_IF_ERR,
      tst_LOG_STAT_IF_NOK,
      tst_log_report_settings,
      tst_max_out_log_msg_size,
  };

  return (run_tests(tests, sizeof(tests) / sizeof(Test)) == PASS) ? 0 : 1;
}
