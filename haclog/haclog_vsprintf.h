/******************************************************************************
 *  @file         haclog_vsprintf.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-09-26
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog vsprintf
 *****************************************************************************/

#ifndef HACLOG_VSPRINTF_H_
#define HACLOG_VSPRINTF_H_

#include "haclog/haclog_macro.h"
#include "haclog/haclog_bytes_buffer.h"
#include <time.h>

HACLOG_EXTERN_C_BEGIN

/**
 * @brief printf specifier information
 *
 * refer: https://cplusplus.com/reference/cstdio/printf/
 * format specifier follows this prototype:
 *     %[flags][width][.precision][length]specifier
 */
typedef struct haclog_printf_spec {
	// range of specifier: [pos_begin, pos_end)
	unsigned int pos_begin : 16;
	unsigned int pos_end : 16;

	// width
	// 0: without width
	// HACLOG_PRINTF_SPEC_DYNAMIC: dynamic width
	// otherwise: number width
	int width : 16;

	// precision
	// 0: without precision
	// HACLOG_PRINTF_SPEC_DYNAMIC: dynamic precision
	// otherwise: number precision
	int precision : 16;

	// length
	// HACLOG_PRINTF_LENGTH_*
	unsigned int length : 16;

	// flags
	unsigned int flags : 8;

	// type
	unsigned int type : 8;

	// format type
	unsigned int fmt_type : 8;
	unsigned int reserve : 24;
} haclog_printf_spec_t;

/**
 * @brief printf location information
 */
typedef struct haclog_printf_loc {
	const char *file; //!< source file
	const char *func; //!< source function name
	int line; //!< source file line
	int level; //!< log level
} haclog_printf_loc_t;

/**
 * @brief printf primitive
 */
typedef struct haclog_printf_primitive {
	const char *fmt; //!< format string
	unsigned int fmt_len; //!< length of format string(not include '\0')
	unsigned int num_params; //!< number of parameters
	unsigned int num_args; //!< number of arguments
	haclog_printf_loc_t loc; //!< source location info
	haclog_printf_spec_t *specs; //!< printf specifier information array
	unsigned int param_size; //!< parameter size
} haclog_printf_primitive_t;

/**
 * @brief number type serialize placeholder
 */
typedef unsigned long long haclog_serialize_placeholder;

/**
 * @brief haclog serialize head
 */
typedef struct haclog_serialize_hdr {
	struct timespec ts; //!< timestamp
	haclog_atomic_int pos_const; //!< const arguments position
	haclog_atomic_int pos_str; //!< string arguments position
	unsigned long extra_len; //!< extra length
	unsigned long pos_cache_line; //!< cache line position
	haclog_printf_primitive_t *primitive; //!< primitive pointer
} haclog_serialize_hdr_t;

/**
 * @brief log message
 */
typedef struct haclog_meta_info {
	haclog_printf_loc_t *loc;
	haclog_thread_id tid;
	struct timespec ts;
} haclog_meta_info_t;

/**
 * @brief generate printf primitive
 *
 * @param fmt  format string
 * @param loc  location information
 *
 * @return
 *   - on success, return primitive pointer
 *   - otherwise, return NULL and set haclog last error
 */
HACLOG_EXPORT
haclog_printf_primitive_t *
haclog_printf_primitive_gen(const char *fmt, const haclog_printf_loc_t *loc);

/**
 * @brief cleanup prinf primitive
 *
 * @param primitive  printf primitive
 */
HACLOG_EXPORT
void haclog_printf_primitive_clean(haclog_printf_primitive_t *primitive);

/**
 * @brief get number of parameters and arguments
 *
 * @param fmt         format string
 * @param num_params  number of parameters
 * @param num_args    number of arguments
 *
 * @return 
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
HACLOG_EXPORT
int haclog_printf_num_params(const char *fmt, unsigned int *num_params,
							 unsigned int *num_args);

/**
 * @brief get sizeof printf specifier size
 *
 * @param spec printf specifier pointer
 *
 * @return sizeof printf specifier
 *
 * @NOTE
 * only include param size, don't care about pointer content length
 */
HACLOG_EXPORT
int haclog_printf_spec_param_size(haclog_printf_spec_t *spec);

/**
 * @brief show primitive for debug
 *
 * @param primitive  printf primitive
 */
HACLOG_EXPORT
void haclog_printf_primitive_show(haclog_printf_primitive_t *primitive);

// format attribute
#if __GNUC__
	#define HACLOG_PRINT_FORMAT_CHECK __attribute__((format(printf, 3, 4)));
#else
	#define HACLOG_PRINT_FORMAT_CHECK
#endif

/**
 * @brief serialize primitive and arguments into bytes buffer
 *
 * @param bytes_buf  bytes buffer pointer
 * @param primitive  printf primitive pointer
 * @param ...        variadic arguments
 */
HACLOG_EXPORT
void haclog_printf_primitive_serialize(haclog_bytes_buffer_t *bytes_buf,
									   haclog_printf_primitive_t *primitive,
									   const char *fmt_str,
									   ...) HACLOG_PRINT_FORMAT_CHECK;

/**
 * @brief format primitive to string (include '\0')
 *
 * @param bytes_buf  bytes buffer pointer
 * @param meta       log meta info pointer
 * @param w          writer position
 * @param buf        buffer store the result
 * @param bufsize    size of buffer
 *
 * @return 
 *   - on success, return the number of characters printed (exclude '\0')
 *   - on nothing format, return -2
 *   - on failed, return -1 and set haclog last error
 */
HACLOG_EXPORT
int haclog_printf_primitive_format(haclog_bytes_buffer_t *bytes_buf,
								   haclog_meta_info_t *meta,
								   haclog_atomic_int w, char *buf,
								   size_t bufsize);

HACLOG_EXTERN_C_END

#endif // !HACLOG_VSPRINTF_H_
