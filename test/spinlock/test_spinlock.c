#include "haclog/haclog.h"
#include "unity.h"

void setUp()
{
}

void tearDown()
{
}

#define DOUBLE_CHECKED(ret)                                                    \
	do {                                                                       \
		static int s_val = 0;                                                  \
		if (s_val == 0) {                                                      \
			static haclog_spinlock_t spinlock = HACLOG_SPINLOCK_STATUS_UNLOCK; \
			haclog_spinlock_lock(&spinlock);                                   \
			if (s_val == 0) {                                                  \
				++s_val;                                                       \
			}                                                                  \
			haclog_spinlock_unlock(&spinlock);                                 \
		}                                                                      \
		ret = s_val;                                                           \
	} while (0)

#if HACLOG_PLATFORM_WINDOWS
static haclog_thread_ret_t __stdcall s_double_checked_func(void *args)
#else
static haclog_thread_ret_t s_double_checked_func(void *args)
#endif
{
	haclog_atomic_int *p_sig = (haclog_atomic_int *)args;
	while (haclog_atomic_load(p_sig, haclog_memory_order_relaxed) == 0) {
	}

	int ret = 0;
	for (int i = 0; i < 32; i++) {
		DOUBLE_CHECKED(ret);
		TEST_ASSERT_EQUAL(ret, 1);
	}

	return 0;
}

void test_double_check()
{
	haclog_atomic_int sig = 0;

	haclog_thread_t threads[32];
	for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); i++) {
		haclog_thread_create(&threads[i], s_double_checked_func, &sig);
	}

	haclog_nsleep(5 * 1000 * 1000);
	haclog_atomic_store(&sig, 1, haclog_memory_order_relaxed);

	for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); i++) {
		haclog_thread_join(&threads[i]);
	}
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_double_check);

	return UNITY_END();
}
