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
	HANDLE timer;
	LARGE_INTEGER ft;

	LONGLONG hundred_ns = (LONGLONG)(ns / 100);
	if (hundred_ns == 0) {
		hundred_ns = 1;
	}

	// NOTE: 
	//   * negative value represent relative
	//   * time interval unit is 100 ns
	ft.QuadPart = -hundred_ns;

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);

	return 0;
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
	return HACLOG_ERR_SYS_CALL;
	#endif /* _POSIX_C_SOURCE >= 199309L */
#endif
}
