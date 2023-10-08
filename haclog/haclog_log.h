/******************************************************************************
 *  @file         haclog_log.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-07
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog log
 *****************************************************************************/

#ifndef HACLOG_LOG_H_
#define HACLOG_LOG_H_

#include "haclog/haclog_macro.h"
#include "haclog/haclog_vsprintf.h"
#include "haclog/haclog_thread_context.h"
#include "haclog/handler/haclog_handler.h"
#include <assert.h>

HACLOG_EXTERN_C_BEGIN

// minimum required: c11 or c++11
#if __cplusplus
static_assert(__cplusplus >= 201103L, "haclog minimum required c11 or c++11");
#elif __STDC_VERSION__
	#if __STDC_VERSION__ < 201112L
static_assert(0, "haclog minimum required c11 or c++11");
	#endif
#else
	#if !defined(HACLOG_PLATFORM_WINDOWS)
static_assert(0, "haclog can't find c or c++ version");
	#endif
#endif

#define HACLOG_SERIALIZE(bytes_buf, lvl, fmt, ...)                             \
	do {                                                                       \
		static haclog_printf_primitive_t *primitive = NULL;                    \
		/* NOTE: for double-checked lock safe, min required c11 or c++11 */    \
		if (primitive == NULL) {                                               \
			static haclog_spinlock_t spinlock = HACLOG_SPINLOCK_STATUS_UNLOCK; \
			haclog_spinlock_lock(&spinlock);                                   \
			if (primitive == NULL) {                                           \
				const haclog_printf_loc_t loc = {                              \
					.file = __FILE__,                                          \
					.func = __FUNCTION__,                                      \
					.line = __LINE__,                                          \
					.level = lvl,                                              \
				};                                                             \
				primitive = haclog_printf_primitive_gen(fmt, &loc);            \
			}                                                                  \
			haclog_spinlock_unlock(&spinlock);                                 \
		}                                                                      \
		haclog_printf_primitive_serialize(bytes_buf, primitive, fmt,           \
										  ##__VA_ARGS__);                      \
	} while (0)

#define HACLOG_LOG_DEFAULT(level, format, ...)                             \
	do {                                                                   \
		haclog_thread_context_t *th_ctx = haclog_thread_context_get();     \
		if (th_ctx) {                                                      \
			haclog_bytes_buffer_t *bytes_buf = th_ctx->bytes_buf;          \
			if (bytes_buf) {                                               \
				HACLOG_SERIALIZE(bytes_buf, level, format, ##__VA_ARGS__); \
			}                                                              \
		}                                                                  \
	} while (0)

#define HACLOG_TRACE(format, ...) \
	HACLOG_LOG_DEFAULT(HACLOG_LEVEL_TRACE, format, ##__VA_ARGS__)
#define HACLOG_DEBUG(format, ...) \
	HACLOG_LOG_DEFAULT(HACLOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define HACLOG_INFO(format, ...) \
	HACLOG_LOG_DEFAULT(HACLOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define HACLOG_WARNING(format, ...) \
	HACLOG_LOG_DEFAULT(HACLOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define HACLOG_ERROR(format, ...) \
	HACLOG_LOG_DEFAULT(HACLOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define HACLOG_FATAL(format, ...) \
	HACLOG_LOG_DEFAULT(HACLOG_LEVEL_FATAL, format, ##__VA_ARGS__)

#if HACLOG_HOLD_LOG_MACRO

	#define LOG_TRACE(format, ...) \
		HACLOG_LOG_DEFAULT(HACLOG_LEVEL_TRACE, format, ##__VA_ARGS__)
	#define LOG_DEBUG(format, ...) \
		HACLOG_LOG_DEFAULT(HACLOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
	#define LOG_INFO(format, ...) \
		HACLOG_LOG_DEFAULT(HACLOG_LEVEL_INFO, format, ##__VA_ARGS__)
	#define LOG_WARNING(format, ...) \
		HACLOG_LOG_DEFAULT(HACLOG_LEVEL_WARNING, format, ##__VA_ARGS__)
	#define LOG_ERROR(format, ...) \
		HACLOG_LOG_DEFAULT(HACLOG_LEVEL_ERROR, format, ##__VA_ARGS__)
	#define LOG_FATAL(format, ...) \
		HACLOG_LOG_DEFAULT(HACLOG_LEVEL_FATAL, format, ##__VA_ARGS__)

	#define LOG_LEVEL_TRACE HACLOG_LEVEL_TRACE
	#define LOG_LEVEL_DEBUG HACLOG_LEVEL_DEBUG
	#define LOG_LEVEL_INFO HACLOG_LEVEL_INFO
	#define LOG_LEVEL_WARNING HACLOG_LEVEL_WARNING
	#define LOG_LEVEL_ERROR HACLOG_LEVEL_ERROR
	#define LOG_LEVEL_FATAL HACLOG_LEVEL_FATAL

#endif

/**
 * @brief run backend
 */
HACLOG_EXPORT
void haclog_backend_run();

HACLOG_EXTERN_C_END

#endif // !HACLOG_LOG_H_
