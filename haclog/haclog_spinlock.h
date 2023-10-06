/******************************************************************************
 *  @file         haclog_spinlock.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-06
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog spinlock
 *****************************************************************************/

#ifndef HACLOG_SPINLOCK_H_
#define HACLOG_SPINLOCK_H_

#include "haclog/haclog_macro.h"
#include "haclog/haclog_thread.h"

HACLOG_EXTERN_C_BEGIN

enum {
	HACLOG_SPINLOCK_STATUS_UNLOCK,
	HACLOG_SPINLOCK_STATUS_LOCK,
};

typedef haclog_atomic_byte haclog_spinlock_t;

HACLOG_EXPORT
void haclog_spinlock_lock(haclog_spinlock_t *spinlock);

HACLOG_EXPORT
void haclog_spinlock_unlock(haclog_spinlock_t *spinlock);

HACLOG_EXTERN_C_END

#endif // !HACLOG_SPINLOCK_H_
