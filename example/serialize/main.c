#include "haclog/haclog.h"
#include <stdio.h>
#include <string.h>

static haclog_thread_local haclog_bytes_buffer_t *s_haclog_bytes_buffer = NULL;

#define SERIALIZE(level, fmt, ...)                                          \
	do {                                                                    \
		haclog_printf_loc_t loc = {                                         \
			.file = __FILE__,                                               \
			.func = __FUNCTION__,                                           \
			.line = __LINE__,                                               \
			.level = level,                                                 \
		};                                                                  \
		static haclog_printf_primitive_t *primitive = NULL;                 \
		static haclog_spinlock_t spinlock = HACLOG_SPINLOCK_STATUS_UNLOCK;  \
		if (primitive == NULL) {                                            \
			haclog_spinlock_lock(&spinlock);                                \
			primitive = haclog_printf_primitive_gen(fmt, &loc);             \
			haclog_spinlock_unlock(&spinlock);                              \
		}                                                                   \
		haclog_printf_primitive_serialize(s_haclog_bytes_buffer, primitive, \
										  ##__VA_ARGS__);                   \
	} while (0)

void analysis_bytes_buf(haclog_bytes_buffer_t *bytes_buf)
{
	haclog_atomic_int w =
		haclog_atomic_load(&bytes_buf->w, haclog_memory_order_acquire);
	if (bytes_buf->r == w) {
		return;
	}

	if (bytes_buf->capacity - bytes_buf->r <
		(int)sizeof(haclog_serialize_hdr_t)) {
		bytes_buf->r = 0;
	}
	haclog_serialize_hdr_t *hdr =
		(haclog_serialize_hdr_t *)haclog_bytes_buffer_get(bytes_buf,
														  bytes_buf->r);

	unsigned int pos = 0;
	char buf[32];
	haclog_printf_primitive_t *primitive = hdr->primitive;
	for (unsigned int i = 0; i < primitive->num_params; i++) {
		haclog_printf_spec_t *spec = primitive->specs + i;
		if (spec->pos_begin > pos) {
			int n = spec->pos_begin - pos;
			fwrite(primitive->fmt + pos, 1, n, stdout);
			fflush(stdout);
		}

		int len = spec->pos_end - spec->pos_begin;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, primitive->fmt + spec->pos_begin, len);
		if (spec->width == -1) {
		}

		pos = spec->pos_end;
	}
	if (pos != primitive->fmt_len) {
		int n = primitive->fmt_len - pos;
		fwrite(primitive->fmt + pos, 1, n, stdout);
	}
}

int main()
{
	double d = 3.14f;
	float f = 3.14f;
	const char *s = "0123456789abcdef";

	if (s_haclog_bytes_buffer == NULL) {
		s_haclog_bytes_buffer = haclog_bytes_buffer_new(1024 * 1024 * 8);
	}

	int level = 0;
	for (int i = 0; i < 1; i++) {
		SERIALIZE(level, "double=%f, float=%f, s=%s, s=%.8s|end\n", d, f, s, s);
		analysis_bytes_buf(s_haclog_bytes_buffer);
	}

	haclog_bytes_buffer_free(s_haclog_bytes_buffer);

	return 0;
}
