/******************************************************************************
 *  @file         haclog.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-09-26
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog
 *****************************************************************************/

#ifndef HACLOG_HACLOG_H_
#define HACLOG_HACLOG_H_

#include "haclog/haclog_config.h"
#include "haclog/haclog_macro.h"
#include "haclog/haclog_err.h"
#include "haclog/haclog_stacktrace.h"
#include "haclog/haclog_thread.h"
#include "haclog/haclog_sleep.h"
#include "haclog/haclog_path.h"
#include "haclog/haclog_os.h"
#include "haclog/haclog_spinlock.h"
#include "haclog/haclog_vsprintf.h"
#include "haclog/haclog_thread_context.h"
#include "haclog/haclog_context.h"
#include "haclog/handler/haclog_handler.h"
#include "haclog/handler/haclog_console_handler.h"
#include "haclog/handler/haclog_file_handler.h"
#include "haclog/haclog_log.h"

#endif // !HACLOG_HACLOG_H_
