#include "haclog/haclog.h"

#define VSPRINTF_PRIMITIVE_SHOW(fmt)                \
	do {                                            \
		haclog_printf_loc_t loc = {                 \
			.file = __FILE__,                       \
			.func = __FUNCTION__,                   \
			.line = __LINE__,                       \
			.level = 0,                             \
		};                                          \
		haclog_printf_primitive_t *primitive =      \
			haclog_printf_primitive_gen(fmt, &loc); \
		haclog_printf_primitive_show(primitive);    \
		haclog_printf_primitive_clean(primitive);   \
	} while (0)

int main()
{
	VSPRINTF_PRIMITIVE_SHOW("int: %d, float: %f, string: %s");
	VSPRINTF_PRIMITIVE_SHOW("int: %-d, float: %+f, s: %+s");
	VSPRINTF_PRIMITIVE_SHOW("int: %-05d, float: %+06f, s: %+08s");
	VSPRINTF_PRIMITIVE_SHOW("int: %5d, float: %5f, s: %6s");
	VSPRINTF_PRIMITIVE_SHOW("int: %.5d, float: %.5f, s: %.6s");
	VSPRINTF_PRIMITIVE_SHOW("int: %*d, float: %*f, s: %*s");
	VSPRINTF_PRIMITIVE_SHOW("int: %.5d, float: %.5f, s: %.6s");
	VSPRINTF_PRIMITIVE_SHOW("int: %.*d, float: %.*f, s: %.*s");

	return 0;
}
