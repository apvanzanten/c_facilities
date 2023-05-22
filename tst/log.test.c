#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "stat.h"
#include "test_utils.h"

#include "log.h"

#define LOG_BUFFER_SIZE_MAX 1024

typedef struct {
  char   data[LOG_BUFFER_SIZE_MAX + 1];
  size_t size;
} LogBuffer;

static LogBuffer g_log_buff;

static void init_log_buff() { g_log_buff = (LogBuffer){0}; }

static void log_func(const char * msg, size_t n) {
  printf("logging: %.*s", (int)n, msg);

  if(g_log_buff.size + n <= LOG_BUFFER_SIZE_MAX) {

    for(size_t i = 0; i < n; i++) {
      g_log_buff.data[g_log_buff.size + i] = msg[i];
    }

    g_log_buff.size += n;
  }
}

static void setup_log_buffer_and_func() {
  init_log_buff();
  LOG_set_log_func(log_func);
}

Result tst_LOG_STAT() {
  Result r = PASS;

  setup_log_buffer_and_func();

  EXPECT_EQ(&r, STAT_OK, LOG_STAT(STAT_OK, "test LOG_STAT, also %d", 42));

  EXPECT_NE(&r, 0, g_log_buff.size);
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "STAT_OK"));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, __func__));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "\"test LOG_STAT, also 42\""));

  init_log_buff(); // clear buff

  EXPECT_EQ(&r,
            STAT_ERR_FATAL,
            LOG_STAT(STAT_ERR_FATAL, "test LOG_STAT some %s, also %d", "more", 9001));

  EXPECT_NE(&r, 0, g_log_buff.size);
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "STAT_ERR_FATAL"));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, __func__));
  EXPECT_NE(&r, NULL, strstr(g_log_buff.data, "\"test LOG_STAT some more, also 9001\""));

  return r;
}

int main() {
  Test tests[] = {
      tst_LOG_STAT,
  };

  return (run_tests(tests, sizeof(tests) / sizeof(Test)) == PASS) ? 0 : 1;
}