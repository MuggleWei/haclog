/******************************************************************************
 *  @file         haclog_file_handler.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-08
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog file handler
 *****************************************************************************/

#ifndef HACLOG_FILE_HANDLER_H_
#define HACLOG_FILE_HANDLER_H_

#include "haclog/haclog_macro.h"
#include "haclog/handler/haclog_handler.h"
#include <stdio.h>

HACLOG_EXTERN_C_BEGIN

typedef struct haclog_file_handler {
	haclog_handler_t base; //!< base log handler
	FILE *fp;
} haclog_file_handler_t;

/**
 * @brief initialize a file log handler
 *
 * @param handler       file log handler pointer
 * @param filepath  file path
 * @param mode      file open mode
 *
 * @return
 *   - on success, return 0
 *   - otherwise return error code
 */
HACLOG_EXPORT
int haclog_file_handler_init(haclog_file_handler_t *handler,
							 const char *filepath, const char *mode);

HACLOG_EXTERN_C_END

#endif
