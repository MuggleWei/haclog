#include "haclog/handler/haclog_console_handler.h"
#include "haclog/handler/haclog_file_handler.h"
#define HACLOG_HOLD_LOG_MACRO 1
#include "haclog/haclog.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

struct log_msg {
	uint64_t u64;
	uint32_t u32;
	int64_t i64;
	int32_t i32;
	char s[128];
};

void run(struct log_msg *arr, int cnt)
{
	struct timespec ts1, ts2;
	timespec_get(&ts1, TIME_UTC);
	for (int i = 0; i < cnt; i++) {
		struct log_msg *msg = arr + i;
		// char buf[2048];
		// snprintf(buf, sizeof(buf),
		//          "u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
		//          (unsigned long long)msg->u64, (long long)msg->i64,
		//          (unsigned long)msg->u32, (long)msg->i32, msg->s);
		LOG_INFO("u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
				 (unsigned long long)msg->u64, (long long)msg->i64,
				 (unsigned long)msg->u32, (long)msg->i32, msg->s);
	}
	timespec_get(&ts2, TIME_UTC);
	unsigned long elapsed =
		(ts2.tv_sec - ts1.tv_sec) * 1000000000 + ts2.tv_nsec - ts1.tv_nsec;
	printf("avg elapsed: %.3f ns\n", (double)elapsed / cnt);
}

int main()
{
	// static haclog_console_handler_t console_handler = {};
	// if (haclog_console_handler_init(&console_handler, 1) != 0) {
	//     exit(EXIT_FAILURE);
	// }
	// haclog_handler_set_level((haclog_handler_t *)&console_handler,
	//                          LOG_LEVEL_WARNING);
	// haclog_context_add_handler((haclog_handler_t *)&console_handler);

	// static haclog_file_handler_t file_handler = {};
	// if (haclog_file_handler_init(&file_handler, "log/hello.log", "w") != 0) {
	//     exit(EXIT_FAILURE);
	// }
	// haclog_handler_set_level((haclog_handler_t *)&file_handler,
	//                          LOG_LEVEL_DEBUG);
	// haclog_context_add_handler((haclog_handler_t *)&file_handler);

	haclog_backend_run();

	if (haclog_thread_context_init() == NULL) {
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	int cnt = 10000;
	struct log_msg *arr =
		(struct log_msg *)malloc(sizeof(struct log_msg) * cnt);
	for (int i = 0; i < cnt; i++) {
		arr[i].u64 = (uint64_t)i;
		arr[i].u32 = (uint32_t)i;
		arr[i].i64 = (int64_t)i;
		arr[i].i32 = (int32_t)i;
		for (int j = 0; j < (int)sizeof(arr[i].s) - 1; j++) {
			arr[i].s[j] = (rand() % 26) + 'a';
		}
		arr[i].s[sizeof(arr[0].s) - 1] = '\0';
	}

	for (int i = 0; i < 16; i++) {
		run(arr, cnt);
		haclog_nsleep(500000000);
	}

	free(arr);

	haclog_thread_context_cleanup();

	return 0;
}
