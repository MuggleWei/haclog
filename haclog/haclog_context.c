#include "haclog_context.h"
#include "haclog/haclog_err.h"
#include "haclog/haclog_sleep.h"
#include "haclog/haclog_thread.h"
#include <stdlib.h>

#define HACLOG_DEFAULT_BYTES_BUF_SIZE 1024 * 1024
#define HACLOG_DEFAULT_MSG_BUF_SIZE 2048

haclog_context_t *haclog_context_get()
{
	static haclog_context_t s_ctx = {
		.spinlock = HACLOG_SPINLOCK_STATUS_UNLOCK,
		.th_ctx_add_list = { .next = NULL, .th_ctx = NULL },
		.th_ctx_head = { .next = NULL, .th_ctx = NULL },
		.n_handler = 0,
		.level = HACLOG_LEVEL_FATAL,
		.handlers = { NULL },
		.bytes_buf_size = HACLOG_DEFAULT_BYTES_BUF_SIZE,
		.msg_buf_size = HACLOG_DEFAULT_MSG_BUF_SIZE,
	};
	return &s_ctx;
}

int haclog_context_insert_thread_context(haclog_thread_context_t *th_ctx)
{
	haclog_context_t *ctx = haclog_context_get();

	haclog_thread_context_list_t *node = (haclog_thread_context_list_t *)malloc(
		sizeof(haclog_thread_context_list_t));
	if (node == NULL) {
		return HACLOG_ERR_ALLOC_MEM;
	}
	node->th_ctx = th_ctx;

	haclog_spinlock_lock(&ctx->spinlock);
	node->next = ctx->th_ctx_add_list.next;
	ctx->th_ctx_add_list.next = node;
	haclog_spinlock_unlock(&ctx->spinlock);

	return 0;
}

void haclog_context_remove_thread_context(haclog_thread_context_t *th_ctx)
{
	haclog_atomic_store(&th_ctx->status,
						HACLOG_THREAD_CONTEXT_STATUS_WAIT_REMOVE,
						haclog_memory_order_relaxed);

	do {
		haclog_atomic_int status =
			haclog_atomic_load(&th_ctx->status, haclog_memory_order_relaxed);
		if (status == HACLOG_THREAD_CONTEXT_STATUS_DONE) {
			break;
		}
		haclog_nsleep(1 * 1000 * 1000);
	} while (1);
}

int haclog_context_add_handler(haclog_handler_t *handler)
{
	haclog_context_t *ctx = haclog_context_get();
	if (ctx->n_handler == sizeof(ctx->handlers) / sizeof(ctx->handlers[0])) {
		return HACLOG_ERR_ALLOC_MEM;
	}
	ctx->handlers[ctx->n_handler++] = handler;

	if (handler->level < ctx->level) {
		ctx->level = handler->level;
	}

	return 0;
}

void haclog_context_set_bytes_buf_size(unsigned long bufsize)
{
	if (bufsize < HACLOG_DEFAULT_BYTES_BUF_SIZE) {
		bufsize = HACLOG_DEFAULT_BYTES_BUF_SIZE;
	}

	haclog_context_t *ctx = haclog_context_get();
	ctx->bytes_buf_size = bufsize;
}

unsigned long haclog_context_get_bytes_buf_size()
{
	haclog_context_t *ctx = haclog_context_get();
	return ctx->bytes_buf_size;
}

void haclog_context_set_msg_buf_size(unsigned long bufsize)
{
	if (bufsize < HACLOG_DEFAULT_MSG_BUF_SIZE) {
		bufsize = HACLOG_DEFAULT_MSG_BUF_SIZE;
	}
	haclog_context_t *ctx = haclog_context_get();
	ctx->msg_buf_size = bufsize;
}
