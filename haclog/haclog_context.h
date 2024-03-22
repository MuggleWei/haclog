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
#include "haclog/handler/haclog_handler.h"

HACLOG_EXTERN_C_BEGIN

typedef struct haclog_thread_context_list {
	struct haclog_thread_context_list *next;
	haclog_thread_context_t *th_ctx;
} haclog_thread_context_list_t;

/**
 * @brief before haclog backend run callback
 */
typedef void (*haclog_before_run_callback)();

typedef struct haclog_context {
	haclog_spinlock_t spinlock; //!< context spinlock
	haclog_thread_context_list_t th_ctx_add_list; //!< add list
	haclog_thread_context_list_t th_ctx_head; //!< thread context list head
	unsigned int n_handler; //!< number of handler
	int level; //!< min level of handler
	haclog_handler_t *handlers[8]; //!< handler array
	unsigned long bytes_buf_size; //!< bytes buffer size of thread context
	unsigned long msg_buf_size; //!< buffer size of log write
	haclog_before_run_callback before_run_cb; //!< before backend run callback
} haclog_context_t;

/**
 * @brief get haclog context
 *
 * @return haclog context
 */
HACLOG_EXPORT
haclog_context_t *haclog_context_get();

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
 * @brief add handler into context
 *
 * @param handler  log handler
 *
 * @return 
 *   - on success, return 0
 *   - on failed, return error code
 */
HACLOG_EXPORT
int haclog_context_add_handler(haclog_handler_t *handler);

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

/**
 * @brief set output message buffer size
 *
 * @param bufsize  buffer size of output message
 */
HACLOG_EXPORT
void haclog_context_set_msg_buf_size(unsigned long bufsize);

/**
 * @brief set before backend run callback
 *
 * @param fn  callback function
 */
HACLOG_EXPORT
void haclog_context_set_before_run_cb(haclog_before_run_callback fn);

HACLOG_EXTERN_C_END

#endif // !HACLOG_CONTEXT_H_
