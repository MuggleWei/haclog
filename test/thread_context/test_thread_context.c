#include "haclog/haclog.h"
#include "unity.h"
#include <stdlib.h>

void setUp()
{
}

void tearDown()
{
}

static void s_haclog_handle_new_thread_ctx(haclog_context_t *ctx)
{
	haclog_spinlock_lock(&ctx->spinlock);

	while (ctx->th_ctx_add_list.next) {
		// remove from add list
		haclog_thread_context_list_t *node = ctx->th_ctx_add_list.next;
		ctx->th_ctx_add_list.next = node->next;
		node->next = NULL;

		// add into list
		node->next = ctx->th_ctx_head.next;
		ctx->th_ctx_head.next = node;
	}

	haclog_spinlock_unlock(&ctx->spinlock);
}

static void s_haclog_handle_remove_thread_ctx(haclog_context_t *ctx)
{
	haclog_thread_context_list_t *prev = &ctx->th_ctx_head;
	haclog_thread_context_list_t *node = prev->next;
	while (node) {
		haclog_atomic_int status = haclog_atomic_load(
			&node->th_ctx->status, haclog_memory_order_relaxed);
		if (status == HACLOG_THREAD_CONTEXT_STATUS_WAIT_REMOVE) {
			haclog_atomic_store(&node->th_ctx->status,
								HACLOG_THREAD_CONTEXT_STATUS_DONE,
								haclog_memory_order_relaxed);

			prev->next = node->next;
			node->next = NULL;

			free(node);

			node = prev->next;
		} else {
			prev = node;
			node = node->next;
		}
	}
}

#if HACLOG_PLATFORM_WINDOWS
static haclog_thread_ret_t __stdcall s_free_thread_ctx_func(void *args)
#else
static haclog_thread_ret_t s_free_thread_ctx_func(void *args)
#endif
{
	haclog_atomic_int *p_sig = (haclog_atomic_int *)args;

	haclog_context_t *ctx = haclog_context_get();
	while (1) {
		s_haclog_handle_new_thread_ctx(ctx);
		s_haclog_handle_remove_thread_ctx(ctx);

		if (haclog_atomic_load(p_sig, haclog_memory_order_relaxed) == 1) {
			break;
		}
	}

	return 0;
}

void test_ctx_init()
{
	haclog_context_t *ctx = haclog_context_get();
	TEST_ASSERT_EQUAL(ctx->spinlock, HACLOG_SPINLOCK_STATUS_UNLOCK);
	TEST_ASSERT_NULL(ctx->th_ctx_add_list.next);
	TEST_ASSERT_NULL(ctx->th_ctx_add_list.th_ctx);
	TEST_ASSERT_NULL(ctx->th_ctx_head.next);
	TEST_ASSERT_NULL(ctx->th_ctx_head.th_ctx);
	TEST_ASSERT_EQUAL(ctx->n_handler, 0);
}

void test_thread_ctx_case1()
{
	haclog_thread_context_t *thread_ctx = haclog_thread_context_init();
	TEST_ASSERT_NOT_NULL(thread_ctx);

	TEST_ASSERT_EQUAL(thread_ctx, haclog_thread_context_get());

	haclog_atomic_int sig = 0;
	haclog_thread_t th;
	haclog_thread_create(&th, s_free_thread_ctx_func, &sig);

	haclog_thread_context_cleanup();

	haclog_atomic_store(&sig, 1, haclog_memory_order_relaxed);
	haclog_thread_join(&th);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_ctx_init);
	RUN_TEST(test_thread_ctx_case1);

	return UNITY_END();
}
