#include "haclog/haclog.h"
#include "unity.h"
#include <string.h>

void setUp()
{
}

void tearDown()
{
}

void test_vsprintf_primitive_gen()
{
	haclog_printf_loc_t loc = {
		.file = __FILE__,
		.func = __FUNCTION__,
		.line = __LINE__,
		.level = 0,
	};
	haclog_printf_primitive_t *primitive = NULL;

	primitive = haclog_printf_primitive_gen("", NULL);
	TEST_ASSERT_NULL(primitive);

	primitive = haclog_printf_primitive_gen("", &loc);
	TEST_ASSERT_NOT_NULL(primitive);

	haclog_printf_primitive_clean(primitive);
}

void test_vsprintf_spec_const_di()
{
	haclog_printf_loc_t loc = {
		.file = __FILE__,
		.func = __FUNCTION__,
		.line = __LINE__,
		.level = 0,
	};
	haclog_printf_primitive_t *primitive = NULL;

	const char *format_str = "int=%d, int=%i";
	primitive = haclog_printf_primitive_gen(format_str, &loc);
	TEST_ASSERT_NOT_NULL(primitive);

	TEST_ASSERT_EQUAL(primitive->fmt, format_str);
	TEST_ASSERT_EQUAL(primitive->fmt_len, strlen(format_str));
	TEST_ASSERT_EQUAL(primitive->num_params, 2);
	TEST_ASSERT_EQUAL(primitive->num_args, 2);
	TEST_ASSERT_EQUAL(primitive->loc.file, loc.file);
	TEST_ASSERT_EQUAL(primitive->loc.func, loc.func);
	TEST_ASSERT_EQUAL(primitive->loc.line, loc.line);
	TEST_ASSERT_EQUAL(primitive->loc.level, loc.level);

	for (int i = 0; i < 2; i++) {
		haclog_printf_spec_t *spec = &primitive->specs[i];
		TEST_ASSERT_EQUAL(haclog_printf_spec_param_size(spec),
						  sizeof(haclog_serialize_placeholder));
		TEST_ASSERT_EQUAL(spec->width, 0);
		TEST_ASSERT_EQUAL(spec->precision, 0);
	}

	haclog_printf_primitive_clean(primitive);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_vsprintf_primitive_gen);
	RUN_TEST(test_vsprintf_spec_const_di);

	return UNITY_END();
}
