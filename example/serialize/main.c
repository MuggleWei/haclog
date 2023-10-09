#include "haclog/haclog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	double d = 3.14f;
	float f = 3.14f;
	unsigned long long llu = 1024llu;
	const char *s = "0123456789abcdef";

	if (haclog_thread_context_init() == NULL) {
		exit(EXIT_FAILURE);
	}

	haclog_thread_context_t *th_ctx = haclog_thread_context_get();
	haclog_bytes_buffer_t *bytes_buf = th_ctx->bytes_buf;

	int level = 0;
	for (int i = 0; i < 1; i++) {
		HACLOG_SERIALIZE(
			th_ctx->bytes_buf, level,
			"double=%.5f, float=%.2f, ulonglong=%.*llu, s=\"%s\", s=\"%.8s\"",
			d, f, 6, llu, s, s);

		char buf[2048];
		haclog_atomic_int w =
			haclog_atomic_load(&bytes_buf->w, haclog_memory_order_acquire);
		int n = haclog_printf_primitive_format(bytes_buf, NULL, w, buf,
											   sizeof(buf));
		printf("total bytes: %d\n", n);
		printf("%s|end\n", buf);
	}

	haclog_thread_context_cleanup();

	return 0;
}
