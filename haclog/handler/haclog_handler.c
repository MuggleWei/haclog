#include "haclog_handler.h"
#include "haclog/haclog_err.h"
#include "haclog/haclog_path.h"
#include "haclog/haclog_win_gmtime.h"
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

int haclog_handler_write(haclog_handler_t *handler, haclog_meta_info_t *meta,
						 const char *msg, int msglen)
{
	int n = 0;
	int total_bytes = 0;

	handler->before_write(handler, meta);

	n = handler->write_meta(handler, meta);
	if (n > 0) {
		total_bytes += n;
	}

	if (msglen > 0) {
		n = handler->write(handler, msg, msglen);
		if (n > 0) {
			total_bytes += n;
		}
	}

	handler->after_write(handler, meta);

	return total_bytes;
}

int haclog_handler_default_write_meta(haclog_handler_t *handler,
									  haclog_meta_info_t *meta)
{
	const char *level = haclog_level_to_str(meta->loc->level);

	char filename[256];
	haclog_path_basename(meta->loc->file, filename, sizeof(filename));

	struct tm t;
	gmtime_r(&meta->ts.tv_sec, &t);

	return handler->writev(
		handler, "%s|%d-%02d-%02dT%02d:%02d:%02d.%09d|%s:%u|%s|%llu - ", level,
		(int)t.tm_year + 1900, (int)t.tm_mon + 1, (int)t.tm_mday,
		(int)t.tm_hour, (int)t.tm_min, (int)t.tm_sec, (int)meta->ts.tv_nsec,
		filename, (unsigned int)meta->loc->line, meta->loc->func,
		(unsigned long long)meta->tid);
}
