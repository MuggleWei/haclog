#include "haclog/haclog.h"
#include "unity.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LINE_SIZE 8
#define FILE_PATH "logs/test_long_message.log"

void setUp()
{
}

void tearDown()
{
}

int my_write_meta(struct haclog_handler *handler, haclog_meta_info_t *meta)
{
	// do nothing
	HACLOG_UNUSED(handler);
	HACLOG_UNUSED(meta);
	return 0;
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
	if (haclog_file_handler_init(&handler, FILE_PATH, "w") != 0) {
		fprintf(stderr, "failed init file handler");
		exit(EXIT_FAILURE);
	}
	haclog_handler_set_level((haclog_handler_t *)&handler, HACLOG_LEVEL_DEBUG);
	haclog_handler_set_fn_write_meta((haclog_handler_t *)&handler,
									 my_write_meta);
	haclog_context_add_handler((haclog_handler_t *)&handler);
}

void write_log()
{
	add_console_handler();
	add_file_handler();
	haclog_backend_run();

	haclog_context_set_msg_buf_size(LINE_SIZE);
	char buf[LINE_SIZE * 2];
	for (int i = 0; i < LINE_SIZE * 2 - 1; ++i) {
		buf[i] = '0';
	}
	buf[LINE_SIZE * 2 - 1] = '\0';

	haclog_thread_context_init();

	HACLOG_INFO("%s", buf);

	haclog_thread_context_cleanup();
}

void test_long_message()
{
	write_log();

	FILE *fp = fopen(FILE_PATH, "rb");
	fseek(fp, 0L, SEEK_END);
	long n = ftell(fp);
	fclose(fp);

	TEST_ASSERT_TRUE(n <= LINE_SIZE);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_long_message);

	return UNITY_END();
}
