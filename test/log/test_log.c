#include <string.h>
#define HACLOG_HOLD_LOG_MACRO 1
#include "haclog/haclog.h"
#include "unity.h"
#include "test/log/test_log_handler.h"

void setUp()
{
}

void tearDown()
{
}

static test_log_handler_t *get_test_log_handler()
{
	static test_log_handler_t handler;
	return &handler;
}

static void add_test_log_handler()
{
	test_log_handler_t *handler = get_test_log_handler();
	memset(handler, 0, sizeof(*handler));
	test_log_handler_init(handler);
	haclog_handler_set_level((haclog_handler_t *)handler, HACLOG_LEVEL_DEBUG);
	haclog_context_add_handler((haclog_handler_t *)handler);
}

static void wait_test_log_n(test_log_handler_t *handler, haclog_atomic_int n)
{
	while (haclog_atomic_load(&handler->n, haclog_memory_order_acquire) != n) {
		haclog_nsleep(1000);
	}
}

static void reset_test_log_n(test_log_handler_t *handler)
{
	haclog_atomic_store(&handler->n, 0, haclog_memory_order_relaxed);
}

void test_log_level()
{
	test_log_handler_t *handler = get_test_log_handler();

	LOG_TRACE("trace message");
	haclog_nsleep(5 * 1000 * 1000);
	TEST_ASSERT_EQUAL(
		haclog_atomic_load(&handler->n, haclog_memory_order_relaxed), 0);

	LOG_DEBUG("debug message");
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("debug message", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_INFO("info message");
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("info message", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_WARNING("warning message");
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("warning message", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_ERROR("error message");
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("error message", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_di()
{
	test_log_handler_t *handler = get_test_log_handler();

	LOG_DEBUG("%d", 5);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

int main()
{
	UNITY_BEGIN();

	add_test_log_handler();
	haclog_backend_run();

	haclog_thread_context_init();

	RUN_TEST(test_log_level);
	RUN_TEST(test_log_format_type_di);

	haclog_thread_context_cleanup();

	return UNITY_END();
}
