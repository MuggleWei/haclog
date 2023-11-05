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

void test_log_format_type_di_none()
{
	test_log_handler_t *handler = get_test_log_handler();

	LOG_DEBUG("%d", 5);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%i", 5);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%d%i", 6, 8);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("68", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("prefix|%d", 1024);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("prefix|1024", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%d|suffix", 1024);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("1024|suffix", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%2d", 5);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING(" 5", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%-2d", 5);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5 ", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%02d", 5);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("05", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%*d", 4, 5);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("   5", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%-*d", 4, 5);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5   ", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%0*d", 4, 5);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("0005", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_di_hh()
{
	test_log_handler_t *handler = get_test_log_handler();

	char c = 'a';
	LOG_DEBUG("%hhd", c);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("97", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%3hhd", c);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING(" 97", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%03hhd", c);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("097", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("prefix|%hhd", c);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("prefix|97", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%hhd|suffix", c);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("97|suffix", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_di_h()
{
	test_log_handler_t *handler = get_test_log_handler();

	if (sizeof(short int) == 2) {
		short int s = 32767;
		LOG_DEBUG("%hd", s);
		wait_test_log_n(handler, 1);
		TEST_ASSERT_EQUAL_STRING("32767", handler->buf_cache[0]);
		reset_test_log_n(handler);
	} else if (sizeof(short int) == 1) {
		short int s = 127;
		LOG_DEBUG("%hd", s);
		wait_test_log_n(handler, 1);
		TEST_ASSERT_EQUAL_STRING("127", handler->buf_cache[0]);
		reset_test_log_n(handler);
	} else {
		// do nothing
	}
}

void test_log_format_type_di_l()
{
	test_log_handler_t *handler = get_test_log_handler();

	if (sizeof(long int) == 4) {
		long int v = 2147483647;
		LOG_DEBUG("%ld", v);
		wait_test_log_n(handler, 1);
		TEST_ASSERT_EQUAL_STRING("2147483647", handler->buf_cache[0]);
		reset_test_log_n(handler);
	} else if (sizeof(long int) == 8) {
		long int v = 9223372036854775807;
		LOG_DEBUG("%ld", v);
		wait_test_log_n(handler, 1);
		TEST_ASSERT_EQUAL_STRING("9223372036854775807", handler->buf_cache[0]);
		reset_test_log_n(handler);
	} else {
		// do nothing
	}
}

void test_log_format_type_di_ll()
{
	test_log_handler_t *handler = get_test_log_handler();

	if (sizeof(long long int) == 4) {
		long long int v = 2147483647;
		LOG_DEBUG("%lld", v);
		wait_test_log_n(handler, 1);
		TEST_ASSERT_EQUAL_STRING("2147483647", handler->buf_cache[0]);
		reset_test_log_n(handler);
	} else if (sizeof(long long int) == 8) {
		long long int v = 9223372036854775807;
		LOG_DEBUG("%lld", v);
		wait_test_log_n(handler, 1);
		TEST_ASSERT_EQUAL_STRING("9223372036854775807", handler->buf_cache[0]);
		reset_test_log_n(handler);
	} else {
		// do nothing
	}
}

void test_log_format_type_di_j()
{
	test_log_handler_t *handler = get_test_log_handler();

	if (sizeof(intmax_t) >= 8) {
		intmax_t v = 9223372036854775807;
		LOG_DEBUG("%jd", v);
		wait_test_log_n(handler, 1);
		TEST_ASSERT_EQUAL_STRING("9223372036854775807", handler->buf_cache[0]);
		reset_test_log_n(handler);
	} else {
		// do nothing
	}
}

void test_log_format_type_di_z()
{
	test_log_handler_t *handler = get_test_log_handler();

	size_t v = 1024;
	LOG_DEBUG("%zd", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("1024", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_di_t()
{
	test_log_handler_t *handler = get_test_log_handler();

	ptrdiff_t v = 1024;
	LOG_DEBUG("%td", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("1024", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_uoxX_none()
{
	test_log_handler_t *handler = get_test_log_handler();

	unsigned int v = 65535;

	LOG_DEBUG("%u", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("65535", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%o", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("177777", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%x", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("ffff", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%X", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("FFFF", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_uoxX_hh()
{
	test_log_handler_t *handler = get_test_log_handler();

	unsigned int v = 255;

	LOG_DEBUG("%hhu", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("255", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%hho", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("377", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%hhx", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("ff", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%hhX", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("FF", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_uoxX_h()
{
	test_log_handler_t *handler = get_test_log_handler();

	unsigned short v = 65535;

	LOG_DEBUG("%hu", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("65535", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%ho", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("177777", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%hx", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("ffff", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%hX", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("FFFF", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_uoxX_l()
{
	test_log_handler_t *handler = get_test_log_handler();

	unsigned long v = 65535;

	LOG_DEBUG("%lu", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("65535", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%lo", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("177777", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%lx", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("ffff", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%lX", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("FFFF", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_uoxX_ll()
{
	test_log_handler_t *handler = get_test_log_handler();

	unsigned long long v = 65535;

	LOG_DEBUG("%llu", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("65535", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%llo", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("177777", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%llx", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("ffff", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%llX", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("FFFF", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_uoxX_j()
{
	test_log_handler_t *handler = get_test_log_handler();

	uintmax_t v = 65535;

	LOG_DEBUG("%ju", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("65535", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%jo", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("177777", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%jx", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("ffff", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%jX", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("FFFF", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_uoxX_z()
{
	test_log_handler_t *handler = get_test_log_handler();

	size_t v = 65535;

	LOG_DEBUG("%zu", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("65535", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%zo", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("177777", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%zx", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("ffff", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%zX", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("FFFF", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_uoxX_t()
{
	test_log_handler_t *handler = get_test_log_handler();

	ptrdiff_t v = 65535;

	LOG_DEBUG("%tu", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("65535", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%to", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("177777", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%tx", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("ffff", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%tX", v);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("FFFF", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_fFeEgGaA_none()
{
	test_log_handler_t *handler = get_test_log_handler();

	float f = 5.0;

	LOG_DEBUG("%.1f", f);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5.0", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%.*f", 2, f);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5.00", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%.1F", f);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5.0", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%.1e", f);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5.0e+00", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%.1E", f);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5.0E+00", handler->buf_cache[0]);
	reset_test_log_n(handler);

	double d = 5.0;

	LOG_DEBUG("%.1f", d);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5.0", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%.*f", 2, d);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5.00", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_fFeEgGaA_L()
{
	test_log_handler_t *handler = get_test_log_handler();

	long double ld = 5.0;

	LOG_DEBUG("%.1Lf", ld);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5.0", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%.*Lf", 2, ld);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("5.00", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_c_none()
{
	test_log_handler_t *handler = get_test_log_handler();

	LOG_DEBUG("%c", 97);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("a", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_s_none()
{
	test_log_handler_t *handler = get_test_log_handler();

	LOG_DEBUG("%s", "hello world");
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("hello world", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%.5s", "hello world");
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("hello", handler->buf_cache[0]);
	reset_test_log_n(handler);

	LOG_DEBUG("%.*s", 5, "hello world");
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("hello", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_format_type_p_none()
{
	test_log_handler_t *handler = get_test_log_handler();

	void *p = handler;

	char buf[64];
#if HACLOG_PLATFORM_WINDOWS
	snprintf(buf, sizeof(buf), "%016llX", (unsigned long long)p);
#else
	snprintf(buf, sizeof(buf), "0x%llx", (unsigned long long)p);
#endif
	LOG_DEBUG("%p", p);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING(buf, handler->buf_cache[0]);
	reset_test_log_n(handler);
}

void test_log_other_case1()
{
	test_log_handler_t *handler = get_test_log_handler();

	LOG_DEBUG("%%%d", 5);
	wait_test_log_n(handler, 1);
	TEST_ASSERT_EQUAL_STRING("%%5", handler->buf_cache[0]);
	reset_test_log_n(handler);
}

int main()
{
	UNITY_BEGIN();

	add_test_log_handler();
	haclog_backend_run();

	haclog_thread_context_init();

	RUN_TEST(test_log_level);

	RUN_TEST(test_log_format_type_di_none);
	RUN_TEST(test_log_format_type_di_hh);
	RUN_TEST(test_log_format_type_di_h);
	RUN_TEST(test_log_format_type_di_l);
	RUN_TEST(test_log_format_type_di_ll);
	RUN_TEST(test_log_format_type_di_j);
	RUN_TEST(test_log_format_type_di_z);
	RUN_TEST(test_log_format_type_di_t);

	RUN_TEST(test_log_format_type_uoxX_none);
	RUN_TEST(test_log_format_type_uoxX_hh);
	RUN_TEST(test_log_format_type_uoxX_h);
	RUN_TEST(test_log_format_type_uoxX_l);
	RUN_TEST(test_log_format_type_uoxX_ll);
	RUN_TEST(test_log_format_type_uoxX_j);
	RUN_TEST(test_log_format_type_uoxX_z);
	RUN_TEST(test_log_format_type_uoxX_t);

	RUN_TEST(test_log_format_type_fFeEgGaA_none);
	RUN_TEST(test_log_format_type_fFeEgGaA_L);

	RUN_TEST(test_log_format_type_c_none);

	RUN_TEST(test_log_format_type_s_none);

	RUN_TEST(test_log_format_type_p_none);

	RUN_TEST(test_log_other_case1);

	haclog_thread_context_cleanup();

	return UNITY_END();
}
