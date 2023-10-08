#include "haclog_handler.h"
#include "haclog/haclog_err.h"
#include "haclog/haclog_path.h"
#include <stdio.h>
#include <string.h>

const char *haclog_level_to_str(int log_level)
{
	static const char *s_str_log_level_unknown = "UNKNOWN";
	static const char *s_str_log_level[HACLOG_LEVEL_MAX] = { "TRACE", "DEBUG",
															 "INFO",  "WARNING",
															 "ERROR", "FATAL" };
	int level = log_level >> HACLOG_LEVEL_OFFSET;
	if (level >= 0 && level < HACLOG_LEVEL_MAX) {
		return s_str_log_level[level];
	}
	return s_str_log_level_unknown;
}

void haclog_handler_set_level(haclog_handler_t *handler, int level)
{
	handler->level = level;
}

int haclog_handler_get_level(haclog_handler_t *handler)
{
	return handler->level;
}

int haclog_handler_should_write(haclog_handler_t *handler, int level)
{
	if (level < handler->level) {
		return 0;
	}
	return 1;
}

#if MUGGLE_PLATFORM_WINDOWS

static struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
	errno_t err = gmtime_s(result, timep);
	if (err == 0) {
		return result;
	}

	return NULL;
}

#endif

int haclog_handler_simple_fmt(haclog_meta_info_t *meta, const char *msg,
							  int msglen, char *buf, size_t bufsize)
{
	HACLOG_UNUSED(msglen);

	const char *level = haclog_level_to_str(meta->loc->level);

	char filename[256];
	haclog_path_basename(meta->loc->file, filename, sizeof(filename));

	return (int)snprintf(buf, bufsize, "%s|%llu.%09d|%s:%u|%s|%llu - %s\n",
						 level, (unsigned long long)meta->ts.tv_sec,
						 (int)meta->ts.tv_nsec, filename,
						 (unsigned int)meta->loc->line, meta->loc->func,
						 (unsigned long long)meta->tid, msg);
}

int haclog_handler_default_fmt(haclog_meta_info_t *meta, const char *msg,
							   int msglen, char *buf, size_t bufsize)
{
	HACLOG_UNUSED(msglen);

	const char *level = haclog_level_to_str(meta->loc->level);

	char filename[256];
	haclog_path_basename(meta->loc->file, filename, sizeof(filename));

	struct tm t;
	gmtime_r(&meta->ts.tv_sec, &t);

	return (int)snprintf(
		buf, bufsize,
		"%s|%d-%02d-%02dT%02d:%02d:%02d.%09d|%s:%u|%s|%llu - %s\n", level,
		(int)t.tm_year + 1900, (int)t.tm_mon + 1, (int)t.tm_mday,
		(int)t.tm_hour, (int)t.tm_min, (int)t.tm_sec, (int)meta->ts.tv_nsec,
		filename, (unsigned int)meta->loc->line, meta->loc->func,
		(unsigned long long)meta->tid, msg);
}
