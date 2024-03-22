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

typedef struct {
	int i;
	short si;
	long li;
	long long lli;

	unsigned int u;
	unsigned short su;
	unsigned long lu;
	unsigned long long llu;

	float f;
	double d;
	long double ld;
	const char *s;
} data_t;

void output_type_and_specifiers()
{
	data_t data;
	memset(&data, 0, sizeof(data));
	data.s = "hello world";

	LOG_INFO("----------------");
	LOG_INFO("types and specifiers");
	LOG_INFO("----------------");
	LOG_INFO(
		"\nSpecifiers\n"
		"| length | d i           | u o x X                | f F e E g G a A | c      | s        | p     | n              |\n"
		"------------------------------------------------------------------------------------------------------------------\n"
		"| (none) | int           | unsigned int           | double          | int    | char*    | void* | int*           |\n"
		"| hh     | signed char   | unsigned char          | signed char*    |        |          |       |                |\n"
		"| h      | short int     | unsigned short int     |                 |        |          |       | short int*     |\n"
		"| l      | long int      | unsigned long int      |                 | wint_t | wchar_t* |       | long int*      |\n"
		"| ll     | long long int | unsigned long long int |                 |        |          |       | long long int* |\n"
		"| j      | intmax_t      | uintmax_t              |                 |        |          |       | intmax_t*      |\n"
		"| z      | size_t        | size_t                 |                 |        |          |       | size_t*        |\n"
		"| t      | ptrdiff_t     | ptrdiff_t              |                 |        |          |       | ptrdiff_t*     |\n"
		"| L      |               |                        | long double     |        |          |       |                |\n");

	LOG_INFO("int(d): %d, short(hd): %hd, long(ld): %ld, long long(lld): %lld",
			 data.i, data.si, data.li, data.lli);
	LOG_INFO("unsigned int(u): %u, unsigned short(hu): %hu, "
			 "unsigned long(lu): %lu, unsigned long long(llu): %llu",
			 data.u, data.su, data.lu, data.llu);
	LOG_INFO("float(f): %f, double(f): %f, long double(Lf): %Lf", data.f,
			 data.d, data.ld);
	LOG_INFO("const char*(s): %s", data.s);
	LOG_ERROR("NOTE: not support non-standard(in c99 but not in c11) "
			  "double specifier(lf)");
	// LOG_ERROR("double(lf): %lf", data.d);
	LOG_INFO(" ");
}

void before_run()
{
	fprintf(stdout, "before haclog backend run\n"
					"user can do something interests here\n"
					"e.g.\n"
					"  * set CPU affinity\n"
					"  * wait something completed\n"
					"NOTE: do not use log in before run!!!\n"
					"\n");
}

int main()
{
	add_console_handler();
	add_file_handler();
	add_file_rotate_handler();
	add_file_time_rot_handler();
	haclog_context_set_before_run_cb(before_run);
	haclog_backend_run();

	haclog_thread_context_init();

	// output different type
	output_type_and_specifiers();

	// output different level
	LOG_INFO("----------------");
	LOG_INFO("levels");
	LOG_INFO("----------------");
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
