#include "haclog_spinlock.h"

void haclog_spinlock_lock(haclog_spinlock_t *spinlock)
{
	while (!haclog_atomic_test_and_set(spinlock, haclog_memory_order_acquire))
	{
		haclog_thread_yield();
	}
}

void haclog_spinlock_unlock(haclog_spinlock_t *spinlock)
{
	haclog_atomic_clear(spinlock, haclog_memory_order_release);
}
