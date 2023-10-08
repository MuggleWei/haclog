#include "haclog_thread_context.h"
#include "haclog/haclog_context.h"
#include "haclog/haclog_err.h"
#include "haclog/haclog_thread.h"
#include <stdlib.h>
#include <string.h>

static haclog_thread_local haclog_thread_context_t *s_haclog_thread_ctx = NULL;

haclog_thread_context_t *haclog_thread_context_init()
{
	if (s_haclog_thread_ctx) {
		return s_haclog_thread_ctx;
	}

	s_haclog_thread_ctx =
		(haclog_thread_context_t *)malloc(sizeof(haclog_thread_context_t));
	if (s_haclog_thread_ctx == NULL) {
		haclog_set_error(HACLOG_ERR_ALLOC_MEM);
		return NULL;
	}
	memset(s_haclog_thread_ctx, 0, sizeof(*s_haclog_thread_ctx));

	haclog_context_t *ctx = haclog_context_get();
	s_haclog_thread_ctx->bytes_buf =
		haclog_bytes_buffer_new(ctx->bytes_buf_size);
	if (s_haclog_thread_ctx->bytes_buf == NULL) {
		haclog_thread_context_cleanup();
		haclog_set_error(HACLOG_ERR_ALLOC_MEM);
		return NULL;
	}

	int ret = haclog_context_insert_thread_context(s_haclog_thread_ctx);
	if (ret != 0) {
		haclog_thread_context_cleanup();
		haclog_set_error(ret);
		return NULL;
	}

	s_haclog_thread_ctx->tid = haclog_thread_readable_id();

	return s_haclog_thread_ctx;
}

void haclog_thread_context_cleanup()
{
	if (s_haclog_thread_ctx) {
		if (s_haclog_thread_ctx->bytes_buf) {
			haclog_bytes_buffer_join(s_haclog_thread_ctx->bytes_buf);
		}

		haclog_context_remove_thread_context(s_haclog_thread_ctx);

		s_haclog_thread_ctx->tid = 0;

		if (s_haclog_thread_ctx->bytes_buf) {
			haclog_bytes_buffer_free(s_haclog_thread_ctx->bytes_buf);
			s_haclog_thread_ctx->bytes_buf = NULL;
		}

		free(s_haclog_thread_ctx);
		s_haclog_thread_ctx = NULL;
	}
}

haclog_thread_context_t *haclog_thread_context_get()
{
	if (s_haclog_thread_ctx == NULL) {
		return haclog_thread_context_init();
	}
	return s_haclog_thread_ctx;
}
