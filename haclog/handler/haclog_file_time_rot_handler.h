/******************************************************************************
 *  @file         haclog_file_time_rot_handler.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-09
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog file time rotate handler
 *****************************************************************************/

#ifndef HACLOG_FILE_TIME_ROTATE_HANDLER_H_
#define HACLOG_FILE_TIME_ROTATE_HANDLER_H_

#include "haclog/haclog_macro.h"
#include "haclog/haclog_path.h"
#include "haclog/handler/haclog_handler.h"
#include <stdio.h>

HACLOG_EXTERN_C_BEGIN

#define HACLOG_TIME_ROTATE_UNIT_SEC 's' //!< rotate unit - second
#define HACLOG_TIME_ROTATE_UNIT_MIN 'm' //!< rotate unit - minute
#define HACLOG_TIME_ROTATE_UNIT_HOUR 'h' //!< rotate unit - hour
#define HACLOG_TIME_ROTATE_UNIT_DAY 'd' //!< rotate unit - day

typedef struct haclog_file_time_rot_handler {
	haclog_handler_t base; //!< base log handler
	char filepath[HACLOG_MAX_PATH];
	FILE *fp;
	unsigned int rotate_unit;
	unsigned int rotate_mod;
	time_t last_sec;
	struct tm last_tm;
	unsigned int use_local_time;
} haclog_file_time_rot_handler_t;

/**
 * @brief initialize file time rotate log handler
 *
 * @param handler         log handler pointer
 * @param filepath        file path
 * @param rotate_unit     file rotate time unit, use macro MUGGLE_LOG_TIME_ROTATE_UNIT_*
 * @param rotate_mod      file rotate time mod
 * @param use_local_time  if true, use local time, otherwise use UTC+0
 *
 * @return 
 *   - success returns 0
 *   - otherwise return error code
 */
HACLOG_EXPORT
int haclog_file_time_rotate_handler_init(
	haclog_file_time_rot_handler_t *handler, const char *filepath,
	unsigned int rotate_unit, unsigned int rotate_mod,
	unsigned int use_local_time);

HACLOG_EXTERN_C_END

#endif // !HACLOG_FILE_TIME_ROTATE_HANDLER_H_
