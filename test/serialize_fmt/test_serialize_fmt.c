#include "haclog/haclog.h"
#include "unity.h"

void setUp()
{
	haclog_thread_context_t *th_ctx = haclog_thread_context_init();
	TEST_ASSERT_NOT_NULL(th_ctx);
}

void tearDown()
{
	haclog_thread_context_cleanup();
}

#define TEST_SERIALIZE_FMT(fmt_str, ...)                                 \
	haclog_thread_context_t *th_ctx = haclog_thread_context_get();       \
	HACLOG_SERIALIZE(th_ctx->bytes_buf, HACLOG_LEVEL_INFO, fmt_str,  \
					 ##__VA_ARGS__);                                     \
	char buf[2048];                                                      \
	int n = haclog_printf_primitive_format(th_ctx->bytes_buf, NULL, buf, \
										   sizeof(buf));                 \
	char buf2[2048];                                                     \
	int n2 = snprintf(buf2, sizeof(buf2), fmt_str, ##__VA_ARGS__);       \
	TEST_ASSERT_EQUAL(n, n2);                                            \
	TEST_ASSERT_EQUAL_STRING(buf, buf2);

void test_serialize_fmt_empty()
{
	TEST_SERIALIZE_FMT("");
}

void test_serialize_fmt_int()
{
	int i = 100;
	{
		const char *fmt_str = "int=%d";
		TEST_SERIALIZE_FMT(fmt_str, i);
	}
	{
		const char *fmt_str = "int=%6d";
		TEST_SERIALIZE_FMT(fmt_str, i);
	}
	{
		const char *fmt_str = "int=%06d";
		TEST_SERIALIZE_FMT(fmt_str, i);
	}
	{
		const char *fmt_str = "int=% 6d";
		TEST_SERIALIZE_FMT(fmt_str, i);
	}
	{
		const char *fmt_str = "int=%-6d";
		TEST_SERIALIZE_FMT(fmt_str, i);
	}
	{
		const char *fmt_str = "int=%.6d";
		TEST_SERIALIZE_FMT(fmt_str, i);
	}
	{
		const char *fmt_str = "int=%.*d";
		TEST_SERIALIZE_FMT(fmt_str, 6, i);
	}
	{
		const char *fmt_str = "int=%x";
		TEST_SERIALIZE_FMT(fmt_str, i);
	}
	{
		const char *fmt_str = "int=%06x";
		TEST_SERIALIZE_FMT(fmt_str, i);
	}
	{
		const char *fmt_str = "int=% 6x";
		TEST_SERIALIZE_FMT(fmt_str, i);
	}
}

void test_serialize_fmt_uint()
{
	unsigned int u = 100;
	{
		const char *fmt_str = "uint=%u";
		TEST_SERIALIZE_FMT(fmt_str, u);
	}
	{
		const char *fmt_str = "uint=%6u";
		TEST_SERIALIZE_FMT(fmt_str, u);
	}
	{
		const char *fmt_str = "uint=%06u";
		TEST_SERIALIZE_FMT(fmt_str, u);
	}
	{
		const char *fmt_str = "uint=% 6u";
		TEST_SERIALIZE_FMT(fmt_str, u);
	}
	{
		const char *fmt_str = "uint=%-6u";
		TEST_SERIALIZE_FMT(fmt_str, u);
	}
	{
		const char *fmt_str = "uint=%.6u";
		TEST_SERIALIZE_FMT(fmt_str, u);
	}
	{
		const char *fmt_str = "uint=%.*u";
		TEST_SERIALIZE_FMT(fmt_str, 6, u);
	}
	{
		const char *fmt_str = "uint=%x";
		TEST_SERIALIZE_FMT(fmt_str, u);
	}
	{
		const char *fmt_str = "uint=%06x";
		TEST_SERIALIZE_FMT(fmt_str, u);
	}
	{
		const char *fmt_str = "uint=% 6x";
		TEST_SERIALIZE_FMT(fmt_str, u);
	}
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_serialize_fmt_empty);
	RUN_TEST(test_serialize_fmt_int);
	RUN_TEST(test_serialize_fmt_uint);

	// TODO:

	return UNITY_END();
}
