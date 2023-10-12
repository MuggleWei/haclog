/******************************************************************************
 *  @file         haclog_file_rotate_handler.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-08
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog file rotate handler
 *****************************************************************************/

#ifndef HACLOG_FILE_ROTATE_HANDLER_H_
#define HACLOG_FILE_ROTATE_HANDLER_H_

#include "haclog/haclog_macro.h"
#include "haclog/haclog_path.h"
#include "haclog/handler/haclog_handler.h"
#include <stdio.h>

HACLOG_EXTERN_C_BEGIN

typedef struct haclog_file_rotate_handler {
	haclog_handler_t base; //!< base log handler
	char filepath[HACLOG_MAX_PATH];
	FILE *fp;
	unsigned int backup_count;
	long max_bytes;
	long offset;
} haclog_file_rotate_handler_t;

/**
 * @brief initialize file rotate log handler
 *
 * @param handler       file rotate log handler pointer
 * @param filepath      file path
 * @param max_bytes     max bytes per file
 * @param backup_count  max backup file count
 *
 * @return
 *   - on success, return 0
 *   - otherwise return error code
 */
HACLOG_EXPORT
int haclog_file_rotate_handler_init(haclog_file_rotate_handler_t *handler,
									const char *filepath,
									long max_bytes,
									unsigned int backup_count);

HACLOG_EXTERN_C_END

#endif // !HACLOG_FILE_ROTATE_HANDLER_H_
