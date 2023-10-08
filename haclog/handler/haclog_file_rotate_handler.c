#include "haclog_file_rotate_handler.h"
#include "haclog/haclog_err.h"
#include "haclog/haclog_os.h"
#include <string.h>

static int
haclog_file_rotate_handler_rotate(haclog_file_rotate_handler_t *handler)
{
	if (handler->fp) {
		fclose(handler->fp);
		handler->fp = NULL;
	}

	char buf[HACLOG_MAX_PATH];
	snprintf(buf, sizeof(buf) - 1, "%s.%d", handler->filepath,
			 handler->backup_count);
	if (haclog_path_exists(buf)) {
		haclog_os_remove(buf);
	}

	char src[HACLOG_MAX_PATH], dst[HACLOG_MAX_PATH];
	for (int i = (int)handler->backup_count - 1; i > 0; i--) {
		snprintf(src, sizeof(src), "%s.%d", handler->filepath, i);
		snprintf(dst, sizeof(dst), "%s.%d", handler->filepath, i + 1);
		haclog_os_rename(src, dst);
	}

	snprintf(dst, sizeof(dst), "%s.1", handler->filepath);
	haclog_os_rename(handler->filepath, dst);

	handler->fp = fopen(handler->filepath, "ab+");
	if (handler->fp == NULL) {
		return HACLOG_ERR_SYS_CALL;
	}

	handler->offset = 0;

	return 0;
}

static int haclog_file_rotate_handler_write(struct haclog_handler *base_handler,
											haclog_meta_info_t *meta,
											const char *msg, int msglen)
{
	HACLOG_UNUSED(meta);

	haclog_file_rotate_handler_t *handler =
		(haclog_file_rotate_handler_t *)base_handler;

	int ret = 0;
	if (handler->fp) {
		ret = (int)fwrite(msg, 1, msglen, handler->fp);
		fflush(handler->fp);

		handler->offset += (long)ret;
		if (handler->offset >= handler->max_bytes) {
			if (haclog_file_rotate_handler_rotate(handler) != 0) {
				fprintf(stderr, "failed rotate log handler");
			}
		}
	}

	return ret;
}

static void
haclog_file_rotate_handler_destroy(struct haclog_handler *base_handler)
{
	haclog_file_rotate_handler_t *handler =
		(haclog_file_rotate_handler_t *)base_handler;
	if (handler->fp) {
		fclose(handler->fp);
		handler->fp = NULL;
	}
}

int haclog_file_rotate_handler_init(haclog_file_rotate_handler_t *handler,
									const char *filepath,
									unsigned int max_bytes,
									unsigned int backup_count)
{
	memset(handler, 0, sizeof(*handler));

	int ret = 0;
	const char* abs_filepath = NULL;
	char log_path[HACLOG_MAX_PATH];
	if (haclog_path_isabs(filepath))
	{
		abs_filepath = filepath;
	}
	else
	{
		char cur_path[HACLOG_MAX_PATH];
		ret = haclog_os_curdir(cur_path, sizeof(cur_path));
		if (ret != 0)
		{
			return ret;
		}

		ret = haclog_path_join(cur_path, filepath, log_path, sizeof(log_path));
		if (ret != 0)
		{
			return ret;
		}

		char log_dir[HACLOG_MAX_PATH];
		ret = haclog_path_dirname(log_path, log_dir, sizeof(log_dir));
		if (ret != 0)
		{
			return ret;
		}

		if (!haclog_path_exists(log_dir))
		{
			ret = haclog_os_mkdir(log_dir);
			if (ret != 0)
			{
				return ret;
			}
		}

		abs_filepath = log_path;
	}

	handler->fp = fopen(abs_filepath, "ab+");
	if (handler->fp == NULL)
	{
		return HACLOG_ERR_SYS_CALL;
	}

	strncpy(handler->filepath, abs_filepath, sizeof(handler->filepath)-1);
	handler->max_bytes = max_bytes;
	handler->backup_count = backup_count;

	fseek(handler->fp, 0, SEEK_END);
	handler->offset = ftell(handler->fp);

	if (handler->offset >= handler->max_bytes)
	{
		haclog_file_rotate_handler_rotate(handler);
	}

	handler->base.fmt = haclog_handler_default_fmt;
	handler->base.write = haclog_file_rotate_handler_write;
	handler->base.destroy = haclog_file_rotate_handler_destroy;
	handler->base.level = HACLOG_LEVEL_INFO;

	return 0;
}
