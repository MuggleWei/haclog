#include "haclog.h"
#include "haclog/haclog_context.h"
#include "haclog/haclog_stacktrace.h"
#include "haclog/haclog_thread.h"
#include <stdio.h>
#include <stdlib.h>

static void haclog_consume(haclog_context_t *ctx,
						   haclog_thread_context_t *th_ctx, char *msg,
						   unsigned long bufsize)
{
	haclog_bytes_buffer_t *bytes_buf = th_ctx->bytes_buf;
	haclog_meta_info_t meta;
	memset(&meta, 0, sizeof(meta));
	haclog_atomic_int w =
		haclog_atomic_load(&bytes_buf->w, haclog_memory_order_acquire);
	int n = 0;

	while (1) {
		n = haclog_printf_primitive_format(bytes_buf, &meta, w, msg, bufsize);
		if (n < 0) {
			break;
		}
		meta.tid = th_ctx->tid;

		for (unsigned int i = 0; i < ctx->n_handler; ++i) {
			haclog_handler_t *handler = ctx->handlers[i];

			if (!haclog_handler_should_write(handler, meta.loc->level)) {
				continue;
			}

			haclog_handler_write(handler, &meta, msg, n);
		}
	}
}

static haclog_thread_ret_t s_haclog_backend_func(void *args)
{
	HACLOG_UNUSED(args);

	haclog_context_t *ctx = haclog_context_get();

	unsigned long bufsize = ctx->buf_size;
	if (bufsize < 2048) {
		bufsize = 2048;
	}

	char *msg = (char *)malloc(bufsize);
	if (msg == NULL) {
		haclog_debug_break();
		return NULL;
	}

	while (1) {
		haclog_spinlock_lock(&ctx->spinlock);

		haclog_thread_context_list_t *node = ctx->th_ctx_head.next;
		while (node) {
			haclog_consume(ctx, node->th_ctx, msg, bufsize);
			node = node->next;
		}

		haclog_spinlock_unlock(&ctx->spinlock);

		haclog_thread_yield();
	}

	free(msg);

	return 0;
}

void haclog_backend_run()
{
	haclog_thread_t th_backend;
	haclog_thread_create(&th_backend, s_haclog_backend_func, NULL);
	haclog_thread_detach(&th_backend);
}
