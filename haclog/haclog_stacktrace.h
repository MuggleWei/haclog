/******************************************************************************
 *  @file         haclog_stacktrace.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-07
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog stacktrace
 *****************************************************************************/

#ifndef HACLOG_STACKTRACE_H_
#define HACLOG_STACKTRACE_H_

#include "haclog/haclog_macro.h"

HACLOG_EXTERN_C_BEGIN

#define HACLOG_MAX_STACKTRACE_FRAME_NUM 256

/**
 * @brief stack trace structure
 */
typedef struct haclog_stacktrace {
	unsigned int cnt_frame;
	char **symbols;
} haclog_stacktrace_t;

/**
 * @brief output stack trace info to stdout
 */
HACLOG_EXPORT
void haclog_print_stacktrace();

HACLOG_EXPORT
void haclog_debug_break();

HACLOG_EXTERN_C_END

#endif // !HACLOG_STACKTRACE_H_
