#include "haclog_context.h"
#include "haclog/haclog_err.h"
#include <stdlib.h>

haclog_context_t *haclog_context_get()
{
	static haclog_context_t s_ctx = {
		.spinlock = HACLOG_SPINLOCK_STATUS_UNLOCK,
		.th_ctx_head = { .next = NULL, .th_ctx = NULL },
		.n_handler = 0,
		.level = HACLOG_LEVEL_FATAL,
		.handlers = { NULL },
		.bytes_buf_size = 1024 * 1024 * 8,
		.buf_size = 4096,
	};
	return &s_ctx;
}

int haclog_context_insert_thread_context(haclog_thread_context_t *th_ctx)
{
	haclog_context_t *ctx = haclog_context_get();

	haclog_thread_context_list_t *node = (haclog_thread_context_list_t *)malloc(
		sizeof(haclog_thread_context_list_t));
	if (node == NULL) {
		haclog_spinlock_unlock(&ctx->spinlock);
		return HACLOG_ERR_ALLOC_MEM;
	}
	node->th_ctx = th_ctx;

	haclog_spinlock_lock(&ctx->spinlock);
	node->next = ctx->th_ctx_head.next;
	ctx->th_ctx_head.next = node;
	haclog_spinlock_unlock(&ctx->spinlock);

	return 0;
}

void haclog_context_remove_thread_context(haclog_thread_context_t *th_ctx)
{
	haclog_context_t *ctx = haclog_context_get();
	haclog_spinlock_lock(&ctx->spinlock);

	haclog_thread_context_list_t *node = &ctx->th_ctx_head;
	while (node->next) {
		if (node->next->th_ctx == th_ctx) {
			haclog_thread_context_list_t *next = node->next->next;
			free(node->next);
			node->next = next;
			break;
		}
		node = node->next;
	}

	haclog_spinlock_unlock(&ctx->spinlock);
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
	if (bufsize < 1024 * 8) {
		bufsize = 1024 * 8;
	}

	haclog_context_t *ctx = haclog_context_get();
	ctx->bytes_buf_size = bufsize;
}

unsigned long haclog_context_get_bytes_buf_size()
{
	haclog_context_t *ctx = haclog_context_get();
	return ctx->bytes_buf_size;
}

void haclog_context_set_buf_size(unsigned long bufsize)
{
	if (bufsize < 2048) {
		bufsize = 2048;
	}
	haclog_context_t *ctx = haclog_context_get();
	ctx->buf_size = bufsize;
}
