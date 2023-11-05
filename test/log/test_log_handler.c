#include "test_log_handler.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static int test_log_handler_before_write(haclog_handler_t *base_handler,
										 haclog_meta_info_t *meta)
{
	HACLOG_UNUSED(base_handler);
	HACLOG_UNUSED(meta);
	return 0;
}

static int test_log_handler_after_write(haclog_handler_t *base_handler,
										haclog_meta_info_t *meta)
{
	HACLOG_UNUSED(meta);

	test_log_handler_t *handler = (test_log_handler_t *)base_handler;
	haclog_atomic_fetch_add(&handler->n, 1, haclog_memory_order_release);

	return 0;
}

static int test_log_handler_write(struct haclog_handler *base_handler,
								  const char *msg, int msglen)
{
	test_log_handler_t *handler = (test_log_handler_t *)base_handler;

	int ret = 0;
	if ((size_t)handler->n <
		sizeof(handler->buf_cache) / sizeof(handler->buf_cache[0])) {
		if (msglen >= TEST_LOG_BUF_SIZE - 1) {
			msglen = TEST_LOG_BUF_SIZE - 1;
		}
		memcpy(handler->buf_cache[handler->n], msg, msglen);
		handler->buf_cache[handler->n][msglen] = '\0';

		ret = msglen;
	}

	return ret;
}

static int test_log_handler_writev(haclog_handler_t *base_handler,
								   const char *fmt_str, ...)
{
	test_log_handler_t *handler = (test_log_handler_t *)base_handler;

	int ret = 0;
	if ((size_t)handler->n <
		sizeof(handler->buf_cache) / sizeof(handler->buf_cache[0])) {
		va_list args;
		va_start(args, fmt_str);
		vsnprintf(handler->buf_cache[handler->n], TEST_LOG_BUF_SIZE, fmt_str,
				  args);
		va_end(args);
	}

	return ret;
}

static void test_log_handler_destroy(struct haclog_handler *base_handler)
{
	HACLOG_UNUSED(base_handler);
}

static int test_log_handler_write_meta(haclog_handler_t *handler,
									   haclog_meta_info_t *meta)
{
	HACLOG_UNUSED(handler);
	HACLOG_UNUSED(meta);
	return 0;
}

int test_log_handler_init(test_log_handler_t *handler)
{
	memset(handler, 0, sizeof(*handler));

	for (size_t i = 0;
		 i < sizeof(handler->buf_cache) / sizeof(handler->buf_cache[0]); i++) {
		handler->buf_cache[i] = handler->bufs + i * TEST_LOG_BUF_SIZE;
	}

	handler->base.before_write = test_log_handler_before_write;
	handler->base.write_meta = test_log_handler_write_meta;
	handler->base.write = test_log_handler_write;
	handler->base.writev = test_log_handler_writev;
	handler->base.after_write = test_log_handler_after_write;
	handler->base.destroy = test_log_handler_destroy;
	handler->base.level = HACLOG_LEVEL_INFO;

	return 0;
}
