#include "haclog_sleep.h"
#include "haclog/haclog_err.h"
#include <time.h>

#if HACLOG_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
	#include <time.h>
	#include <errno.h>
#endif

int haclog_nsleep(unsigned long ns)
{
#if HACLOG_PLATFORM_WINDOWS
	static_assert(0, "to be continued...");
#else
	#if _POSIX_C_SOURCE >= 199309L
	struct timespec ts = { .tv_sec = 0, .tv_nsec = ns };

	int res = 0;
	do {
		res = nanosleep(&ts, &ts);
	} while (res && errno == EINTR);

	return 0;
	#else
	static_assert(0, "_POSIX_C_SOURCE >= 199309L is required");
	return HACLOG_ERR_SYSCALL;
	#endif /* _POSIX_C_SOURCE >= 199309L */
#endif
}
