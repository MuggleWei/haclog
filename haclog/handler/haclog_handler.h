/******************************************************************************
 *  @file         haclog_handler.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-08
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog handler
 *****************************************************************************/

#ifndef HACLOG_HANDLER_H_
#define HACLOG_HANDLER_H_

#include "haclog/haclog_macro.h"
#include "haclog/haclog_vsprintf.h"

HACLOG_EXTERN_C_BEGIN

enum {
	HACLOG_LEVEL_OFFSET = 8,
	HACLOG_LEVEL_TRACE = 0,
	HACLOG_LEVEL_DEBUG = 1 << HACLOG_LEVEL_OFFSET,
	HACLOG_LEVEL_INFO = 2 << HACLOG_LEVEL_OFFSET,
	HACLOG_LEVEL_WARNING = 3 << HACLOG_LEVEL_OFFSET,
	HACLOG_LEVEL_WARN = HACLOG_LEVEL_WARNING,
	HACLOG_LEVEL_ERROR = 4 << HACLOG_LEVEL_OFFSET,
	HACLOG_LEVEL_FATAL = 5 << HACLOG_LEVEL_OFFSET,
	HACLOG_LEVEL_MAX = 6,
};

struct haclog_handler;

/**
 * @brief prototype of log format function
 *
 * @param meta     log meta info
 * @param buf      the formated meta message output buffer
 * @param bufsize  size of buffer
 *
 * @return
 *   - on success, return number of bytes in formated message (exclude '\0')
 *   - on failed, return -1
 */
typedef int (*func_haclog_handler_meta_fmt)(haclog_meta_info_t *meta, char *buf,
									   size_t bufsize);

/**
 * @brief before write
 *
 * @param handler  handler pointer
 * @param meta     log meta info
 *
 * @return
 *   - on success, return 0
 *   - on failed, return error code
 */
typedef int (*func_haclog_handler_before_write)(struct haclog_handler *handler,
												haclog_meta_info_t *meta);

/**
 * @brief after write
 *
 * @param handler  handler pointer
 * @param meta     log meta info
 *
 * @return
 *   - on success, return 0
 *   - on failed, return error code
 */
typedef int (*func_haclog_handler_after_write)(struct haclog_handler *handler,
											   haclog_meta_info_t *meta);

/**
 * @brief prototype of log handler write function
 *
 * @param handler    handler pointer
 * @param meta       log meta info
 * @param msg        log message
 * @param msglen     length of log message
 *
 * @return
 *   - on success, return number of bytes writed
 *   - on failed, return -1
 */
typedef int (*func_haclog_handler_write)(struct haclog_handler *handler,
										 haclog_meta_info_t *meta,
										 const char *msg, int msglen);

/**
 * @brief destroy log handler
 *
 * @param handler   handler pointer
 */
typedef void (*func_haclog_handler_destroy)(struct haclog_handler *handler);

/**
 * @brief haclog handler
 */
typedef struct haclog_handler {
	func_haclog_handler_before_write before_write;
	func_haclog_handler_meta_fmt meta_fmt;
	func_haclog_handler_write write;
	func_haclog_handler_after_write after_write;
	func_haclog_handler_destroy destroy;
	int level;
} haclog_handler_t;

/**
 * @brief convert log level to string
 *
 * @param log_level  log level, see: HACLOG_LEVEL_*
 *
 * @return 
 */
HACLOG_EXPORT
const char *haclog_level_to_str(int log_level);

/**
 * @brief set log handler level
 *
 * @param handler  log handler pointer
 * @param level    log level
 */
HACLOG_EXPORT
void haclog_handler_set_level(haclog_handler_t *handler, int level);

/**
 * @brief get log handler's level
 *
 * @param handler  log handler pointer
 *
 * @return log handler's level
 */
HACLOG_EXPORT
int haclog_handler_get_level(haclog_handler_t *handler);

/**
 * @brief detect level should write in log handler
 *
 * @param handler  log handler pointer
 * @param level    log level
 *
 * @return
 *   - should: return 1
 *   - not: return 0
 */
HACLOG_EXPORT
int haclog_handler_should_write(haclog_handler_t *handler, int level);

/**
 * @brief log handler write
 *
 * @param handler    handler pointer
 * @param meta       log meta info
 * @param msg        log message
 * @param msglen     length of log message
 *
 * @return
 *   - on success, return number of bytes writed
 *   - on failed, return -1
 */
HACLOG_EXPORT
int haclog_handler_write(haclog_handler_t *handler, haclog_meta_info_t *meta,
						 const char *msg, int msglen);

/**
 * @brief 
 *
 * @param meta
 * @param buf
 * @param bufsize
 *
 * @return 
 */
HACLOG_EXPORT
int haclog_handler_default_fmt(haclog_meta_info_t *meta, char *buf,
							   size_t bufsize);

HACLOG_EXTERN_C_END

#endif // !HACLOG_HANDLER_H_
