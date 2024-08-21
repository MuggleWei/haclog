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

haclog_thread_ret_t thread_enable_auto_init(void *args)
{
	HACLOG_UNUSED(args);

	LOG_INFO("enable auto init");

	haclog_thread_context_cleanup();

#if HACLOG_PLATFORM_WINDOWS
	return 0;
#else
	return NULL;
#endif
}

haclog_thread_ret_t thread_disable_auto_init(void *args)
{
	HACLOG_UNUSED(args);

	LOG_INFO("disable auto init");

#if HACLOG_PLATFORM_WINDOWS
	return 0;
#else
	return NULL;
#endif
}

int main()
{
	add_console_handler();

	haclog_backend_run();

	haclog_thread_t th1;
	haclog_thread_create(&th1, thread_enable_auto_init, NULL);
	haclog_thread_join(&th1);

	haclog_thread_context_set_auto_init(0);
	haclog_thread_t th2;
	haclog_thread_create(&th2, thread_disable_auto_init, NULL);
	haclog_thread_join(&th2);

	haclog_thread_context_set_auto_init(1);
	haclog_thread_t th3;
	haclog_thread_create(&th3, thread_enable_auto_init, NULL);
	haclog_thread_join(&th3);

	return 0;
}
