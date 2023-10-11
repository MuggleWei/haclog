#include "haclog.h"
#include "haclog/haclog_context.h"
#include "haclog/haclog_stacktrace.h"
#include "haclog/haclog_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void haclog_handle_new_thread_ctx(haclog_context_t *ctx)
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

static void haclog_handle_remove_thread_ctx(haclog_context_t *ctx)
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

static int haclog_consume(haclog_context_t *ctx,
						  haclog_thread_context_t *th_ctx, char *buf,
						  unsigned long bufsize)
{
	int num_consume = 0;

	haclog_bytes_buffer_t *bytes_buf = th_ctx->bytes_buf;
	haclog_meta_info_t meta;
	memset(&meta, 0, sizeof(meta));
	haclog_atomic_int w =
		haclog_atomic_load(&bytes_buf->w, haclog_memory_order_acquire);
	int n = 0;

	while (1) {
		n = haclog_printf_primitive_format(bytes_buf, &meta, w, buf, bufsize);
		if (n < 0) {
			break;
		}
		meta.tid = th_ctx->tid;

		for (unsigned int i = 0; i < ctx->n_handler; ++i) {
			haclog_handler_t *handler = ctx->handlers[i];

			if (!haclog_handler_should_write(handler, meta.loc->level)) {
				continue;
			}

			haclog_handler_write(handler, &meta, buf, n);
		}

		++num_consume;
	}

	return num_consume;
}

static haclog_thread_ret_t s_haclog_backend_func(void *args)
{
	HACLOG_UNUSED(args);

	haclog_context_t *ctx = haclog_context_get();

	unsigned long bufsize = ctx->msg_buf_size;
	if (bufsize < 2048) {
		bufsize = 2048;
	}

	char *buf = (char *)malloc(bufsize);
	if (buf == NULL) {
		haclog_debug_break();
		return NULL;
	}

	while (1) {
		int num_consume = 0;

		haclog_handle_new_thread_ctx(ctx);

		haclog_thread_context_list_t *node = ctx->th_ctx_head.next;
		while (node) {
			num_consume += haclog_consume(ctx, node->th_ctx, buf, bufsize);
			node = node->next;
		}

		haclog_handle_remove_thread_ctx(ctx);

		// if no message readed, sleep this thread for a while
		if (num_consume == 0) {
			// haclog_thread_yield();
			haclog_nsleep(100);
		}
	}

	free(buf);

	return 0;
}

void haclog_backend_run()
{
	haclog_thread_t th_backend;
	haclog_thread_create(&th_backend, s_haclog_backend_func, NULL);
	haclog_thread_detach(&th_backend);
}
