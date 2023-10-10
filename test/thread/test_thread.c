#include "haclog/haclog.h"
#include "haclog/haclog_thread.h"
#include "unity.h"
#include <stdlib.h>

void setUp()
{
}

void tearDown()
{
}

typedef struct test_set_thread_id_args {
	haclog_thread_id *thread_ids;
	int idx_thread;
} test_set_thread_id_args_t;

haclog_thread_ret_t set_thread_id(void *args)
{
	test_set_thread_id_args_t *p_args = (test_set_thread_id_args_t *)args;

	p_args->thread_ids[p_args->idx_thread] = haclog_thread_readable_id();
	free(p_args);

	return 0;
}

enum {
	num_thread = 4
};

void test_thread_id()
{
	haclog_thread_id thread_ids[num_thread + 1];
	haclog_thread_t threads[num_thread];

	thread_ids[0] = haclog_thread_readable_id();

	for (int i = 0; i < num_thread; i++) {
		test_set_thread_id_args_t *args = (test_set_thread_id_args_t *)malloc(
			sizeof(test_set_thread_id_args_t) * num_thread);
		args->thread_ids = thread_ids;
		args->idx_thread = i + 1;
		haclog_thread_create(&threads[i], set_thread_id, args);
	}

	for (int i = 0; i < num_thread; i++) {
		haclog_thread_join(&threads[i]);
	}

	for (int i = 0; i < num_thread + 1; i++) {
		for (int j = i + 1; j < num_thread + 1; j++) {
			TEST_ASSERT_NOT_EQUAL(thread_ids[i], thread_ids[j]);
		}
	}
}

void test_thread_concurrent()
{
	int n = haclog_thread_hardware_concurrency();
	TEST_ASSERT_TRUE(n > 0);
}

haclog_thread_ret_t fetch_add(void *args)
{
	haclog_atomic_int *p = (haclog_atomic_int *)args;

	for (int i = 0; i < 10000; i++) {
		haclog_atomic_fetch_add(p, 1, haclog_memory_order_relaxed);
	}

	return 0;
}

void test_thread_detach()
{
	haclog_thread_t threads[num_thread];

	haclog_atomic_int v = 0;

	for (int i = 0; i < num_thread; ++i) {
		haclog_thread_create(&threads[i], fetch_add, (void *)&v);
		haclog_thread_detach(&threads[i]);
	}

	while (haclog_atomic_load(&v, haclog_memory_order_relaxed) !=
		   num_thread * 10000) {
		haclog_thread_yield();
	}
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_thread_id);
	RUN_TEST(test_thread_concurrent);
	RUN_TEST(test_thread_detach);

	return UNITY_END();
}
