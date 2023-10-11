#include "haclog_file_time_rot_handler.h"
#include "haclog/haclog_err.h"
#include "haclog/haclog_os.h"
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

static bool
haclog_file_time_rot_handler_detect(haclog_file_time_rot_handler_t *handler,
									const haclog_meta_info_t *meta)
{
	time_t sec = meta->ts.tv_sec;
	if (sec == 0) {
		sec = time(NULL);
	}

	if (handler->last_sec >= sec) {
		return false;
	}

	struct tm curr_tm;
	if (handler->use_local_time) {
		localtime_r(&sec, &curr_tm);
	} else {
		gmtime_r(&sec, &curr_tm);
	}

	bool need_rot = false;
	switch (handler->rotate_unit) {
	case HACLOG_TIME_ROTATE_UNIT_SEC: {
		int curr_sec_mod = curr_tm.tm_sec / handler->rotate_mod;
		int last_sec_mod = handler->last_tm.tm_sec / handler->rotate_mod;
		if (curr_sec_mod != last_sec_mod ||
			curr_tm.tm_min != handler->last_tm.tm_min ||
			curr_tm.tm_hour != handler->last_tm.tm_hour ||
			curr_tm.tm_mday != handler->last_tm.tm_mday ||
			curr_tm.tm_mon != handler->last_tm.tm_mon ||
			curr_tm.tm_year != handler->last_tm.tm_year) {
			need_rot = true;
		}
	} break;
	case HACLOG_TIME_ROTATE_UNIT_MIN: {
		int curr_min_mod = curr_tm.tm_min / handler->rotate_mod;
		int last_min_mod = handler->last_tm.tm_min / handler->rotate_mod;
		if (curr_min_mod != last_min_mod ||
			curr_tm.tm_hour != handler->last_tm.tm_hour ||
			curr_tm.tm_mday != handler->last_tm.tm_mday ||
			curr_tm.tm_mon != handler->last_tm.tm_mon ||
			curr_tm.tm_year != handler->last_tm.tm_year) {
			need_rot = true;
		}
	} break;
	case HACLOG_TIME_ROTATE_UNIT_HOUR: {
		int curr_hour_mod = curr_tm.tm_hour / handler->rotate_mod;
		int last_hour_mod = handler->last_tm.tm_hour / handler->rotate_mod;
		if (curr_hour_mod != last_hour_mod ||
			curr_tm.tm_mday != handler->last_tm.tm_mday ||
			curr_tm.tm_mon != handler->last_tm.tm_mon ||
			curr_tm.tm_year != handler->last_tm.tm_year) {
			need_rot = true;
		}
	} break;
	case HACLOG_TIME_ROTATE_UNIT_DAY: {
		int curr_day_mod = curr_tm.tm_mday / handler->rotate_mod;
		int last_day_mod = handler->last_tm.tm_mday / handler->rotate_mod;
		if (curr_day_mod != last_day_mod ||
			curr_tm.tm_mon != handler->last_tm.tm_mon ||
			curr_tm.tm_year != handler->last_tm.tm_year) {
			need_rot = true;
		}
	} break;
	}

	handler->last_sec = sec;
	memcpy(&handler->last_tm, &curr_tm, sizeof(handler->last_tm));

	return need_rot;
}

static int
haclog_file_time_rot_handler_rotate(haclog_file_time_rot_handler_t *handler)
{
	if (handler->fp) {
		fclose(handler->fp);
		handler->fp = NULL;
	}

	int ret = 0;
	char buf[HACLOG_MAX_PATH];
	switch (handler->rotate_unit) {
	case HACLOG_TIME_ROTATE_UNIT_SEC: {
		ret = snprintf(buf, sizeof(buf), "%s.%d%02d%02dT%02d%02d%02d",
					   handler->filepath, handler->last_tm.tm_year + 1900,
					   handler->last_tm.tm_mon + 1, handler->last_tm.tm_mday,
					   handler->last_tm.tm_hour, handler->last_tm.tm_min,
					   handler->last_tm.tm_sec);

	} break;
	case HACLOG_TIME_ROTATE_UNIT_MIN: {
		ret = snprintf(buf, sizeof(buf), "%s.%d%02d%02dT%02d%02d",
					   handler->filepath, handler->last_tm.tm_year + 1900,
					   handler->last_tm.tm_mon + 1, handler->last_tm.tm_mday,
					   handler->last_tm.tm_hour, handler->last_tm.tm_min);
	} break;
	case HACLOG_TIME_ROTATE_UNIT_HOUR: {
		ret = snprintf(buf, sizeof(buf), "%s.%d%02d%02dT%02d",
					   handler->filepath, handler->last_tm.tm_year + 1900,
					   handler->last_tm.tm_mon + 1, handler->last_tm.tm_mday,
					   handler->last_tm.tm_hour);
	} break;
	case HACLOG_TIME_ROTATE_UNIT_DAY: {
		ret = snprintf(buf, sizeof(buf), "%s.%d%02d%02d", handler->filepath,
					   handler->last_tm.tm_year + 1900,
					   handler->last_tm.tm_mon + 1, handler->last_tm.tm_mday);
	} break;
	}
	if (ret < 0) {
		return HACLOG_ERR_ARGUMENTS;
	}

	handler->fp = fopen(buf, "ab+");
	if (handler->fp == NULL) {
		return HACLOG_ERR_SYS_CALL;
	}

	return 0;
}

static int
haclog_file_time_rot_handler_before_write(haclog_handler_t *base_handler,
										  haclog_meta_info_t *meta)
{
	HACLOG_UNUSED(base_handler);
	HACLOG_UNUSED(meta);
	return 0;
}

static int
haclog_file_time_rot_handler_after_write(haclog_handler_t *base_handler,
										 haclog_meta_info_t *meta)
{
	HACLOG_UNUSED(meta);

	haclog_file_time_rot_handler_t *handler =
		(haclog_file_time_rot_handler_t *)base_handler;

	if (handler->fp) {
		fwrite("\n", 1, 1, handler->fp);
		fflush(handler->fp);

		if (haclog_file_time_rot_handler_detect(handler, meta)) {
			if (haclog_file_time_rot_handler_rotate(handler) != 0) {
				fprintf(stderr, "failed rotate log handler");
			}
		}
	}

	return 0;
}

static int
haclog_file_time_rot_handler_write(struct haclog_handler *base_handler,
								   const char *msg, int msglen)
{
	haclog_file_time_rot_handler_t *handler =
		(haclog_file_time_rot_handler_t *)base_handler;

	int ret = 0;
	if (handler->fp) {
		ret = (int)fwrite(msg, 1, msglen, handler->fp);
	}

	return ret;
}

static int haclog_file_time_rot_handler_writev(haclog_handler_t *base_handler,
											   const char *fmt_str, ...)
{
	haclog_file_time_rot_handler_t *handler =
		(haclog_file_time_rot_handler_t *)base_handler;

	int ret = 0;
	if (handler->fp) {
		va_list args;
		va_start(args, fmt_str);
		ret = vfprintf(handler->fp, fmt_str, args);
		va_end(args);
	}

	return ret;
}

static void
haclog_file_time_rot_handler_destroy(struct haclog_handler *base_handler)
{
	haclog_file_time_rot_handler_t *handler =
		(haclog_file_time_rot_handler_t *)base_handler;
	if (handler->fp) {
		fclose(handler->fp);
		handler->fp = NULL;
	}
}

int haclog_file_time_rotate_handler_init(
	haclog_file_time_rot_handler_t *handler, const char *filepath,
	unsigned int rotate_unit, unsigned int rotate_mod,
	unsigned int use_local_time)
{
	memset(handler, 0, sizeof(*handler));

	int ret = 0;
	const char *abs_filepath = NULL;
	char log_path[HACLOG_MAX_PATH];
	if (haclog_path_isabs(filepath)) {
		abs_filepath = filepath;
	} else {
		char cur_path[HACLOG_MAX_PATH];
		ret = haclog_os_curdir(cur_path, sizeof(cur_path));
		if (ret != 0) {
			return ret;
		}

		ret = haclog_path_join(cur_path, filepath, log_path, sizeof(log_path));
		if (ret != 0) {
			return ret;
		}

		char log_dir[HACLOG_MAX_PATH];
		ret = haclog_path_dirname(log_path, log_dir, sizeof(log_dir));
		if (ret != 0) {
			return ret;
		}

		if (!haclog_path_exists(log_dir)) {
			ret = haclog_os_mkdir(log_dir);
			if (ret != 0) {
				return ret;
			}
		}

		abs_filepath = log_path;
	}

	strncpy(handler->filepath, abs_filepath, sizeof(handler->filepath) - 1);
	// handler already memset, the line below just for get rid of gcc strncpy 
	// truncated warning
	handler->filepath[sizeof(handler->filepath) - 1] = '\0';

	handler->last_sec = time(NULL);
	if (handler->use_local_time) {
		localtime_r(&handler->last_sec, &handler->last_tm);
	} else {
		gmtime_r(&handler->last_sec, &handler->last_tm);
	}

	handler->rotate_mod = rotate_mod;
	handler->rotate_unit = rotate_unit;
	handler->use_local_time = use_local_time;

	ret = haclog_file_time_rot_handler_rotate(handler);
	if (ret != 0) {
		return ret;
	}

	handler->base.before_write = haclog_file_time_rot_handler_before_write;
	handler->base.write_meta = haclog_handler_default_write_meta;
	handler->base.write = haclog_file_time_rot_handler_write;
	handler->base.writev = haclog_file_time_rot_handler_writev;
	handler->base.after_write = haclog_file_time_rot_handler_after_write;
	handler->base.destroy = haclog_file_time_rot_handler_destroy;
	handler->base.level = HACLOG_LEVEL_INFO;

	return 0;
}
