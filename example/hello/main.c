#include <stdlib.h>
#include <memset.h>
#define HACLOG_HOLD_LOG_MACRO 1
#include "haclog/haclog.h"

void add_console_handler()
{
	static haclog_console_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_console_handler_init(&handler, 1) != 0) {
		fprintf(stderr, "failed init console handler");
		exit(EXIT_FAILURE);
	}

	haclog_handler_set_level((haclog_handler_t *)&handler, HACLOG_LEVEL_INFO);
	haclog_context_add_handler((haclog_handler_t *)&handler);
}

void add_file_handler()
{
	static haclog_file_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_file_handler_init(&handler, "logs/hello.log", "w") != 0) {
		fprintf(stderr, "failed init file handler");
		exit(EXIT_FAILURE);
	}
	haclog_handler_set_level((haclog_handler_t *)&handler, HACLOG_LEVEL_DEBUG);
	haclog_context_add_handler((haclog_handler_t *)&handler);
}

void add_file_rotate_handler()
{
	static haclog_file_rotate_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_file_rotate_handler_init(&handler, "logs/hello.rot.log", 16,
										5) != 0) {
		fprintf(stderr, "failed init file handler");
		exit(EXIT_FAILURE);
	}
	haclog_handler_set_level((haclog_handler_t *)&handler, HACLOG_LEVEL_DEBUG);
	haclog_context_add_handler((haclog_handler_t *)&handler);
}

void add_file_time_rot_handler()
{
	static haclog_file_time_rot_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_file_time_rotate_handler_init(
			&handler, "logs/hello.time_rot.log", HACLOG_TIME_ROTATE_UNIT_SEC, 2,
			0) != 0) {
		fprintf(stderr, "failed init file time rotate handler");
		exit(EXIT_FAILURE);
	}
	haclog_handler_set_level((haclog_handler_t *)&handler, HACLOG_LEVEL_DEBUG);
	haclog_context_add_handler((haclog_handler_t *)&handler);
}

int main()
{
	add_console_handler();
	add_file_handler();
	add_file_rotate_handler();
	add_file_time_rot_handler();
	haclog_backend_run();

	haclog_thread_context_init();

	LOG_TRACE("trace message");
	haclog_nsleep(500 * 1000 * 1000);
	LOG_DEBUG("debug message");
	haclog_nsleep(500 * 1000 * 1000);
	LOG_INFO("info message");
	haclog_nsleep(500 * 1000 * 1000);
	LOG_WARNING("warning message");
	haclog_nsleep(500 * 1000 * 1000);
	LOG_ERROR("error message");
	haclog_nsleep(500 * 1000 * 1000);
	LOG_FATAL("fatal message");
	haclog_nsleep(500 * 1000 * 1000);

	LOG_INFO("bye");

	haclog_thread_context_cleanup();

	return 0;
}
