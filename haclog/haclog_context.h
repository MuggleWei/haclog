/******************************************************************************
 *  @file         haclog_context.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-07
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog context
 *****************************************************************************/

#ifndef HACLOG_CONTEXT_H_
#define HACLOG_CONTEXT_H_

#include "haclog/haclog_macro.h"
#include "haclog/haclog_spinlock.h"
#include "haclog/haclog_thread_context.h"

HACLOG_EXTERN_C_BEGIN

typedef struct haclog_thread_context_list {
	struct haclog_thread_context_list *next;
	haclog_thread_context_t *th_ctx;
} haclog_thread_context_list_t;

typedef struct haclog_context {
	haclog_spinlock_t spinlock; //!< context spinlock
	haclog_thread_context_list_t th_ctx_head; //!< thread context list head
	unsigned long bytes_buf_size; //!< default bytes buffer size
} haclog_context_t;

/**
 * @brief insert thread context into context
 *
 * @param th_ctx  thread context
 *
 * @return
 *   - on success, return 0
 *   - on failed, return error code
 */
HACLOG_EXPORT
int haclog_context_insert_thread_context(haclog_thread_context_t *th_ctx);

/**
 * @brief remove thread context
 *
 * @param th_ctx
 */
HACLOG_EXPORT
void haclog_context_remove_thread_context(haclog_thread_context_t *th_ctx);

/**
 * @brief set bytes buffer default size
 *
 * @param bufsize  default size
 */
HACLOG_EXPORT
void haclog_context_set_bytes_buf_size(unsigned long bufsize);

/**
 * @brief get bytes buffer default size
 *
 * @return bytes buffer default size
 */
HACLOG_EXPORT
unsigned long haclog_context_get_bytes_buf_size();

HACLOG_EXTERN_C_END

#endif // !HACLOG_CONTEXT_H_
