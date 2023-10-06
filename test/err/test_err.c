#include "haclog/haclog.h"
#include "unity.h"

void setUp()
{
}

void tearDown()
{
}

haclog_thread_ret_t set_and_check_last_err(void *args)
{
	haclog_thread_t th;

	unsigned int num_thread = (unsigned int)(intptr_t)args;
	if (num_thread == 0) {
		return 0;
	}
	haclog_set_error(num_thread);

	unsigned int next_num_thread = num_thread - 1;
	haclog_thread_create(&th, set_and_check_last_err,
				(void *)(intptr_t)next_num_thread);
	haclog_thread_join(&th);

	unsigned int err_code = haclog_last_error();
	TEST_ASSERT_EQUAL(err_code, num_thread);

	return 0;
}

void test_err()
{
	unsigned int num_thread = 4;
	set_and_check_last_err((void *)(intptr_t)num_thread);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_err);

	return UNITY_END();
}
