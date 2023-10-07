#include "haclog.h"
#include "haclog/haclog_thread.h"
#include <stdio.h>

extern haclog_context_t *haclog_context_get();

static haclog_thread_ret_t s_haclog_backend_func(void *args)
{
	HACLOG_UNUSED(args);

	char buf[2048];
	haclog_context_t *ctx = haclog_context_get();
	while (1) {
		haclog_spinlock_lock(&ctx->spinlock);

		haclog_thread_context_list_t *node = ctx->th_ctx_head.next;
		while (node) {
			haclog_bytes_buffer_t *bytes_buf = node->th_ctx->bytes_buf;
			int n = haclog_printf_primitive_format(bytes_buf, buf, sizeof(buf));
			if (n > 0) {
				// TODO: implement log handler
				// fprintf(stdout, "%s\n", buf);
			} else {
				node = node->next;
			}
		}

		haclog_spinlock_unlock(&ctx->spinlock);

		haclog_thread_yield();
	}

	return 0;
}

void haclog_backend_run()
{
	haclog_thread_t th_backend;
	haclog_thread_create(&th_backend, s_haclog_backend_func, NULL);
	haclog_thread_detach(&th_backend);
}
