## haclog

![linux-gcc](https://github.com/MuggleWei/haclog/actions/workflows/linux-gcc.yaml/badge.svg?branch=master)
![linux-clang](https://github.com/MuggleWei/haclog/actions/workflows/linux-clang.yaml/badge.svg?branch=master)
![win-msvc](https://github.com/MuggleWei/haclog/actions/workflows/win-msvc.yaml/badge.svg?branch=master)
![macos-intel-clang](https://github.com/MuggleWei/haclog/actions/workflows/macos-intel-clang.yaml/badge.svg?branch=master)
![macos-arm64-clang](https://github.com/MuggleWei/haclog/actions/workflows/macos-arm64-clang.yaml/badge.svg?branch=master)

* [readme EN](./README.md)
* [readme 中文](./README_cn.md)

## 概述

haclog(**H**appy **A**sync **C** Log) 是一个纯 C 的异步日志库, 该库的主要目标是使**日志前端(日志生产者线程)**的写入耗时尽可能的少  

<img src="./doc/img/haclog.svg" />

## 编译
可以很轻松的通过cmake构建此库, 通常要做的就是:
```
mkdir build
cd build
cmake ..
```

## 使用示例
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

* 通过 `haclog_context_add_handler`, 可以添加不同的日志处理器, 当前内置了以下的 `handler`
  * `haclog_console_handler_t`: 输出到终端
  * `haclog_file_handler_t`: 输出到文件
  * `haclog_file_rotate_handler_t`: 输出到文件, 并根据文件大小进行旋转
  * `haclog_file_time_rot_handler_t`: 输出到文件, 并根据时间进行旋转
* 当添加完需要使用的 `handler` 之后, 调用 `haclog_backend_run` 运行日志后端
* 在每个线程当中, 在写日志之前，需要调用 `haclog_thread_context_init` 初始化 `haclog` 线程上下文
* 在每个线程退出时, 调用 `haclog_thread_context_init` 进行 `haclog` 的清理工作

**特别注意**: `haclog` 每条日志当中的格式化字符串必须为**字面常量(string literal)**, 当使用 C++ 调用 `haclog` 时, 编译器将帮助用户进行自动检测; 而当使用 C 时, 用户需要自己来保证这点  
```
/* 太糟了!!! 别这么做, format 字符串需要为字面常量 */
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
