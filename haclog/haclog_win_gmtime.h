/******************************************************************************
 *  @file         haclog_win_gmtime.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-09
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog windows gmtime
 *****************************************************************************/

#ifndef HACLOG_WIN_GMTIME_H_
#define HACLOG_WIN_GMTIME_H_

#include "haclog/haclog_macro.h"

HACLOG_EXTERN_C_BEGIN

#if HACLOG_PLATFORM_WINDOWS

HACLOG_EXPORT
struct tm* gmtime_r(const time_t *timep, struct tm *result);

HACLOG_EXPORT
struct tm* localtime_r(const time_t *timep, struct tm *result);

HACLOG_EXPORT
time_t timegm(struct tm *p_tm);

#endif

HACLOG_EXTERN_C_END

#endif // !HACLOG_WIN_GMTIME_H_
