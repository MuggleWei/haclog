# haclog

![linux-gcc](https://github.com/MuggleWei/haclog/actions/workflows/linux-gcc.yaml/badge.svg?branch=master)
![linux-clang](https://github.com/MuggleWei/haclog/actions/workflows/linux-clang.yaml/badge.svg?branch=master)
![win-msvc](https://github.com/MuggleWei/haclog/actions/workflows/win-msvc.yaml/badge.svg?branch=master)
![macos-clang](https://github.com/MuggleWei/haclog/actions/workflows/macos-clang.yaml/badge.svg?branch=master)

* [readme EN](./README.md)
* [readme 中文](./README_cn.md)

haclog(**H**appy **A**sync **C** Log) is a plain C asynchronous log library. The main goal of this library is to make the writing time of **log front-end(log producer thread)** as small as possible.  

<img src="./doc/img/haclog.svg" />

### Build
It's easy to build this project with CMake, the typical workflow starts with:
```
mkdir build
cd build
cmake ..
```

## Usage samples
### hello
```
#include <stdlib.h>
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

int main()
{
	// add console handler
	add_console_handler();

	// run haclog backend
	haclog_backend_run();

	// initialize thread context
	haclog_thread_context_init();

	HACLOG_INFO("Hello World");

	double pi = 3.14159265359;
	HACLOG_DEBUG("π = %.5f", pi);

	// cleanup thread context
	haclog_thread_context_init();

	return 0;
}
```

* Use `haclog_context_add_handler`, you can add different log handler into context, built-in support `handler` is as follows
  * `haclog_console_handler_t`: output to console(stdout/stderr)
  * `haclog_file_handler_t`: output to file
  * `haclog_file_rotate_handler_t`: output to file and rotate according to file size
  * `haclog_file_time_rot_handler_t`: output to file and rotate according to time
* After add `handler`, call `haclog_backend_run` to run log backend
* In each thread, before write log, call `haclog_thread_context_init` to initialize `haclog` thread context
* When thread exit, call `haclog_thread_context_init` to cleanup `haclog` thread context

**NOTE**: in `haclog`, the format string in each log must be a **string literal**, When using C++, the compiler will help the user to automatically detect it; when using C, the user needs to gurantee this by self  
```
/* Bad!!! Don't do that, format string must be a string literal */
// char *fmt_str = NULL;
// int v = rand();
// if (v % 2 == 0) {
// 	fmt_str = "%d is even";
// } else {
// 	fmt_str = "%d is odd";
// }
// HACLOG_FATAL(fmt_str, v);
```

### file handler
```
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
```

### rotate file handler
```
void add_file_rotate_handler()
{
	static haclog_file_rotate_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_file_rotate_handler_init(&handler, "logs/hello.rot.log", 16, 5) != 0) {
		fprintf(stderr, "failed init file handler");
		exit(EXIT_FAILURE);
	}
	haclog_handler_set_level((haclog_handler_t *)&handler, HACLOG_LEVEL_DEBUG);
	haclog_context_add_handler((haclog_handler_t *)&handler);
}
```

### time rotate file handler
```
void add_file_time_rot_handler()
{
	static haclog_file_time_rot_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_file_time_rotate_handler_init( &handler, "logs/hello.time_rot.log", HACLOG_TIME_ROTATE_UNIT_DAY, 2, 0) != 0) {
		fprintf(stderr, "failed init file time rotate handler");
		exit(EXIT_FAILURE);
	}
	haclog_handler_set_level((haclog_handler_t *)&handler, HACLOG_LEVEL_DEBUG);
	haclog_context_add_handler((haclog_handler_t *)&handler);
}
```