/******************************************************************************
 *  @file         haclog_time.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-12-06
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog time
 *****************************************************************************/

#ifndef HACLOG_TIME_H_
#define HACLOG_TIME_H_

#include "haclog/haclog_macro.h"
#include <time.h>

HACLOG_EXTERN_C_BEGIN

#if HACLOG_PLATFORM_ANDROID 
	#define haclog_realtime_get(ts) clock_gettime(CLOCK_REALTIME, &ts)
#else
	#define haclog_realtime_get(ts) timespec_get(&ts, TIME_UTC);
#endif

HACLOG_EXTERN_C_END

#endif // !HACLOG_TIME_H_
