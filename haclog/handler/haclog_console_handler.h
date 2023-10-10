/******************************************************************************
 *  @file         haclog_console_handler.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-08
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog console handler
 *****************************************************************************/

#ifndef HACLOG_CONSOLE_HANDLER_H_
#define HACLOG_CONSOLE_HANDLER_H_

#include "haclog/haclog_macro.h"
#include "haclog/handler/haclog_handler.h"
#include <stdio.h>

HACLOG_EXTERN_C_BEGIN

typedef struct haclog_console_handler {
	haclog_handler_t base; //!< base log handler
	FILE *fp; //!< current file handle
	int enable_color; //!< enable color
#if HACLOG_PLATFORM_WINDOWS
	WORD sb_attrs;
#endif
} haclog_console_handler_t;

/**
 * @brief initialize a console log handler
 *
 * @param handler       console log handler pointer
 * @param enable_color  enable console log handler color
 *
 * @return
 *   - on success, return 0
 *   - otherwise return error code
 */
HACLOG_EXPORT
int haclog_console_handler_init(haclog_console_handler_t *handler,
								int enable_color);

HACLOG_EXTERN_C_END

#endif // !HACLOG_CONSOLE_HANDLER_H_
