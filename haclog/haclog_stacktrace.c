#include "haclog_stacktrace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#if HACLOG_PLATFORM_WINDOWS
	#if !defined(NDEBUG)
		#include <windows.h>
		#include <dbghelp.h>
	#endif
#else
	#if HACLOG_HAVE_BACKTRACE
		#include HACLOG_BACKTRACE_HEADER
	#endif
#endif

void haclog_print_stacktrace()
{
#if HACLOG_PLATFORM_WINDOWS
	#if !defined(NDEBUG)
	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);

	void *stacks[HACLOG_MAX_STACKTRACE_FRAME_NUM];
	unsigned int cnt_frame =
		CaptureStackBackTrace(0, HACLOG_MAX_STACKTRACE_FRAME_NUM, stacks, NULL);

	SYMBOL_INFO *symbol =
		(SYMBOL_INFO *)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
	symbol->MaxNameLen = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	for (unsigned int i = 0; i < cnt_frame; i++) {
		SymFromAddr(process, (DWORD64)(stacks[i]), 0, symbol);
		fprintf(stdout, "#%u: %s - 0x%0X\n", i, symbol->Name,
				(unsigned int)symbol->Address);
	}

	free(symbol);
	#endif()
#else // HACLOG_PLATFORM_WINDOWS
	#if HACLOG_HAVE_BACKTRACE
	void *stacks[HACLOG_MAX_STACKTRACE_FRAME_NUM];
	unsigned int cnt_frame = backtrace(stacks, HACLOG_MAX_STACKTRACE_FRAME_NUM);
	char **symbols = backtrace_symbols(stacks, cnt_frame);
	if (symbols == NULL) {
		return;
	}

	for (unsigned int i = 1; i < cnt_frame; ++i) {
		fprintf(stdout, "#%u %s\n", i - 1, symbols[i]);
	}

	free(symbols);
	#endif // HACLOG_HAVE_BACKTRACE
#endif // HACLOG_PLATFORM_WINDOWS
}

void haclog_assert()
{
#if !defined(NDEBUG)
		haclog_print_stacktrace();
	#if HACLOG_PLATFORM_WINDOWS
		__debugbreak();
	#endif
		assert(0);
#endif
}
