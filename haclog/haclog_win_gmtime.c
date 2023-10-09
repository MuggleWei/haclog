#include "haclog_win_gmtime.h"

#if HACLOG_PLATFORM_WINDOWS

struct tm* gmtime_r(const time_t *timep, struct tm *result)
{
	errno_t err = gmtime_s(result, timep);
	if (err == 0)
	{
		return result;
	}

	return NULL;
}

struct tm* localtime_r(const time_t *timep, struct tm *result)
{
	errno_t err = localtime_s(result, timep);
	if (err == 0)
	{
		return result;
	}

	return NULL;
}

time_t timegm(struct tm *p_tm)
{
	return _mkgmtime(p_tm);
}

#endif
