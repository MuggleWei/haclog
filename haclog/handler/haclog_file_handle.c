#include "haclog/haclog_err.h"
#include "haclog/haclog_os.h"
#include "haclog/haclog_path.h"
#include "haclog_file_handler.h"
#include <stdio.h>
#include <string.h>

static int haclog_file_handler_before_write(haclog_handler_t *base_handler,
											haclog_meta_info_t *meta)
{
	HACLOG_UNUSED(base_handler);
	HACLOG_UNUSED(meta);
	return 0;
}

static int haclog_file_handler_after_write(haclog_handler_t *base_handler,
										   haclog_meta_info_t *meta)
{
	HACLOG_UNUSED(meta);

	haclog_file_handler_t *handler = (haclog_file_handler_t *)base_handler;
	if (handler->fp) {
		fwrite("\n", 1, 1, handler->fp);
		fflush(handler->fp);
	}

	return 0;
}

static int haclog_file_handler_write(struct haclog_handler *base_handler,
									 haclog_meta_info_t *meta, const char *msg,
									 int msglen)
{
	HACLOG_UNUSED(meta);

	haclog_file_handler_t *handler = (haclog_file_handler_t *)base_handler;

	int ret = 0;
	if (handler->fp) {
		ret = (int)fwrite(msg, 1, msglen, handler->fp);
	}

	return ret;
}

static void haclog_file_handler_destroy(struct haclog_handler *base_handler)
{
	haclog_file_handler_t *handler = (haclog_file_handler_t *)base_handler;
	if (handler->fp) {
		fclose(handler->fp);
		handler->fp = NULL;
	}
}

int haclog_file_handler_init(haclog_file_handler_t *handler,
							 const char *filepath, const char *mode)
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

	handler->fp = fopen(abs_filepath, mode);
	if (handler->fp == NULL) {
		return HACLOG_ERR_SYS_CALL;
	}

	handler->base.before_write = haclog_file_handler_before_write;
	handler->base.meta_fmt = haclog_handler_default_fmt;
	handler->base.write = haclog_file_handler_write;
	handler->base.after_write = haclog_file_handler_after_write;
	handler->base.destroy = haclog_file_handler_destroy;
	handler->base.level = HACLOG_LEVEL_INFO;

	return 0;
}
