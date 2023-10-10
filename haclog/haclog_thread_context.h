/******************************************************************************
 *  @file         haclog_context.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-07
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog thread context
 *****************************************************************************/

#ifndef HACLOG_THREAD_CONTEXT_H_
#define HACLOG_THREAD_CONTEXT_H_

#include "haclog/haclog_macro.h"
#include "haclog/haclog_bytes_buffer.h"
#include "haclog/haclog_thread.h"

HACLOG_EXTERN_C_BEGIN

enum {
	HACLOG_THREAD_CONTEXT_STATUS_NORMAL = 0,
	HACLOG_THREAD_CONTEXT_STATUS_WAIT_REMOVE,
	HACLOG_THREAD_CONTEXT_STATUS_DONE,
};

typedef struct haclog_thread_context {
	haclog_bytes_buffer_t *bytes_buf;
	haclog_thread_id tid;
	haclog_atomic_int status;
} haclog_thread_context_t;

/**
 * @brief init haclog thread context
 *
 * @return
 *   - on success, return thread context
 *   - otherwise, return NULL and set haclog last error
 */
HACLOG_EXPORT
haclog_thread_context_t *haclog_thread_context_init();

/**
 * @brief cleanup thread context
 */
HACLOG_EXPORT
void haclog_thread_context_cleanup();

/**
 * @brief get haclog thread context
 *
 * @return haclog current thread context
 */
HACLOG_EXPORT
haclog_thread_context_t *haclog_thread_context_get();

HACLOG_EXTERN_C_END

#endif // !HACLOG_THREAD_CONTEXT_H_
