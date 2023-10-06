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

int main()
{
	double d = 3.14f;
	float f = 3.14f;
	unsigned long long llu = 25llu;
	const char *s = "0123456789abcdef";

	if (s_haclog_bytes_buffer == NULL) {
		s_haclog_bytes_buffer = haclog_bytes_buffer_new(1024 * 1024 * 8);
	}

	int level = 0;
	for (int i = 0; i < 1; i++) {
		SERIALIZE(level,
				  "double=%f, float=%.2f, ulonglong=%llu, s=\"%s\", s=\"%.8s\"",
				  d, f, llu, s, s);

		char buf[2048];
		int n = haclog_printf_primitive_format(s_haclog_bytes_buffer, buf,
											   sizeof(buf));
		printf("total bytes: %d\n", n);
		printf("%s|end\n", buf);
	}

	haclog_bytes_buffer_free(s_haclog_bytes_buffer);

	return 0;
}
