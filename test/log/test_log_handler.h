#ifndef TEST_LOG_HANDLER_H_
#define TEST_LOG_HANDLER_H_

#include "haclog/haclog.h"

#define TEST_LOG_BUF_SIZE 2048

typedef struct test_log_handler {
	haclog_handler_t base; //!< base log handler
	char *buf_cache[32];
	char bufs[32 * TEST_LOG_BUF_SIZE];
	haclog_atomic_int n;
} test_log_handler_t;

int test_log_handler_init(test_log_handler_t *handler);

#endif // !TEST_LOG_HANDLER_H_
