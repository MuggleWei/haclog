/******************************************************************************
 *  @file         haclog_sleep.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-08
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog sleep
 *****************************************************************************/

#ifndef HACLOG_SLEEP_H_
#define HACLOG_SLEEP_H_

#include "haclog/haclog_macro.h"

HACLOG_EXTERN_C_BEGIN

/**
 * @brief sleep current thread for nanoseconds
 *
 * @param ns  nanoseconds
 *
 * @return
 *   - on success, return 0
 *   - interupt by signal will return HACLOG_ERR_INTERRUPT
 */
HACLOG_EXPORT
int haclog_nsleep(unsigned long ns);

HACLOG_EXTERN_C_END

#endif // !HACLOG_SLEEP_H_
