#include <stdlib.h>
#include <string.h>
#define HACLOG_HOLD_LOG_MACRO 1
#include "haclog/haclog.h"

int my_write_meta(struct haclog_handler *handler, haclog_meta_info_t *meta)
{
	const char *level = haclog_level_to_str(meta->loc->level);
	return handler->writev(handler, "%s| ", level);
}

void add_console_handler()
{
	static haclog_console_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_console_handler_init(&handler, 1) != 0) {
		fprintf(stderr, "failed init console handler");
		exit(EXIT_FAILURE);
	}

	haclog_handler_set_level((haclog_handler_t *)&handler, HACLOG_LEVEL_INFO);
	haclog_handler_set_fn_write_meta((haclog_handler_t *)&handler,
									 my_write_meta);
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
	if (haclog_file_rotate_handler_init(&handler, "logs/hello.rot.log",
										128 * 1024 * 1024, 5) != 0) {
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
			&handler, "logs/hello.time_rot.log", HACLOG_TIME_ROTATE_UNIT_DAY, 2,
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
	LOG_DEBUG("debug message");
	LOG_INFO("info message");
	LOG_WARNING("warning message");
	LOG_ERROR("error message");

	// NOTE: will crash when build type is DEBUG
	LOG_FATAL("fatal message\n"
			  "NOTE: fatal log crash when build type is DEBUG");
	int actual = 0;
	int expect = 1;
	HACLOG_ASSERT(actual == expect);
	HACLOG_ASSERT_MSG(actual == expect, "actual: %d, expect: %d", actual,
					  expect);

	LOG_INFO("bye");

	haclog_thread_context_cleanup();

	return 0;
}
