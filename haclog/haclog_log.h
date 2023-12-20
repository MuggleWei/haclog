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
#include "haclog/haclog_stacktrace.h"
#include <assert.h>

HACLOG_EXTERN_C_BEGIN

// minimum required: c11 or c++11
#if __cplusplus
	#if defined(_MSC_VER)
static_assert(_MSC_VER >= 1900, "haclog minimum required VS2015");
	#else
static_assert(__cplusplus >= 201103L, "haclog minimum required c11 or c++11");
	#endif
#elif __STDC_VERSION__
	#if __STDC_VERSION__ < 201112L
static_assert(0, "haclog minimum required c11 or c++11");
	#endif
#elif _MSC_VER
static_assert(_MSC_VER >= 1900, "haclog minimum required VS2015");
#else
static_assert(0, "haclog can't find c or c++ version");
#endif

#if defined(_MSC_VER) && _MSC_VER < 1920
	// MSVC version < VS 16.0 (v142 toolset)
	#define HACLOG_LOC(lvl)                                \
		haclog_constexpr const haclog_printf_loc_t loc = { \
			__FILE__,                                      \
			__FUNCTION__,                                  \
			__LINE__,                                      \
			lvl,                                           \
		}
#else
	#define HACLOG_LOC(lvl)                                \
		haclog_constexpr const haclog_printf_loc_t loc = { \
			.file = __FILE__,                              \
			.func = __FUNCTION__,                          \
			.line = __LINE__,                              \
			.level = lvl,                                  \
		}
#endif

#define HACLOG_SERIALIZE(bytes_buf, lvl, fmt, ...)                             \
	do {                                                                       \
		static haclog_printf_primitive_t *primitive = NULL;                    \
		/* NOTE: for double-checked lock safe, min required c11 or c++11 */    \
		if (primitive == NULL) {                                               \
			static haclog_spinlock_t spinlock = HACLOG_SPINLOCK_STATUS_UNLOCK; \
			haclog_spinlock_lock(&spinlock);                                   \
			if (primitive == NULL) {                                           \
				HACLOG_LOC(lvl);                                               \
				primitive = haclog_printf_primitive_gen(fmt, &loc);            \
			}                                                                  \
			haclog_spinlock_unlock(&spinlock);                                 \
		}                                                                      \
		haclog_printf_primitive_serialize(bytes_buf, primitive, fmt,           \
										  ##__VA_ARGS__);                      \
	} while (0)

#ifdef __cplusplus
	#define HACLOG_LOG_DEFAULT(level, format, ...)                            \
		do {                                                                  \
			haclog_thread_context_t *th_ctx = haclog_thread_context_get();    \
			if (th_ctx) {                                                     \
				haclog_bytes_buffer_t *bytes_buf = th_ctx->bytes_buf;         \
				haclog_constexpr const char *const_fmt = format;              \
				HACLOG_SERIALIZE(bytes_buf, level, const_fmt, ##__VA_ARGS__); \
			}                                                                 \
		} while (0)
#else
	#define HACLOG_LOG_DEFAULT(level, format, ...)                         \
		do {                                                               \
			haclog_thread_context_t *th_ctx = haclog_thread_context_get(); \
			if (th_ctx) {                                                  \
				haclog_bytes_buffer_t *bytes_buf = th_ctx->bytes_buf;      \
				HACLOG_SERIALIZE(bytes_buf, level, format, ##__VA_ARGS__); \
			}                                                              \
		} while (0)
#endif

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
#define HACLOG_FATAL(format, ...)                                  \
	HACLOG_LOG_DEFAULT(HACLOG_LEVEL_FATAL, format, ##__VA_ARGS__); \
	haclog_debug_break()

#define HACLOG_ASSERT(x)                    \
	do {                                    \
		if (!(x)) {                         \
			HACLOG_FATAL("Assertion: " #x); \
		}                                   \
	} while (0)

#define HACLOG_ASSERT_MSG(x, format, ...)                              \
	do {                                                               \
		if (!(x)) {                                                    \
			HACLOG_FATAL("Assertion: " #x ", " format, ##__VA_ARGS__); \
		}                                                              \
	} while (0)

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
	#define LOG_FATAL(format, ...)                                     \
		HACLOG_LOG_DEFAULT(HACLOG_LEVEL_FATAL, format, ##__VA_ARGS__); \
		haclog_debug_break()

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
