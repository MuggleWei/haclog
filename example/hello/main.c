#include <stdlib.h>
#define HACLOG_HOLD_LOG_MACRO 1
#include "haclog/haclog.h"

void add_file_time_rot_handler()
{
	static haclog_file_time_rot_handler_t handler = {};
	if (haclog_file_time_rotate_handler_init(
			&handler, "logs/hello.log",
			HACLOG_TIME_ROTATE_UNIT_DAY, 3, 0) != 0) {
		fprintf(stderr, "failed init file time rotate handler");
		exit(EXIT_FAILURE);
	}
	haclog_handler_set_level((haclog_handler_t *)&handler, HACLOG_LEVEL_DEBUG);
	haclog_context_add_handler((haclog_handler_t *)&handler);
}

void add_console_handler()
{
	static haclog_console_handler_t handler = {};
	if (haclog_console_handler_init(&handler, 1) != 0) {
		fprintf(stderr, "failed init console handler");
		exit(EXIT_FAILURE);
	}

	haclog_handler_set_level((haclog_handler_t *)&handler,
							 HACLOG_LEVEL_WARNING);
	haclog_context_add_handler((haclog_handler_t *)&handler);
}

int main()
{
	add_file_time_rot_handler();
	add_console_handler();
	haclog_backend_run();

	haclog_thread_context_init();

	LOG_TRACE("trace message");
	LOG_DEBUG("debug message");
	LOG_INFO("info message");
	LOG_WARNING("warning message");
	LOG_ERROR("error message");
	LOG_FATAL("fatal message");

	haclog_thread_context_cleanup();

	return 0;
}
