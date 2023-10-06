/******************************************************************************
 *  @file         haclog_err.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-09-26
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog error
 *****************************************************************************/

#ifndef HACLOG_ERROR_H_
#define HACLOG_ERROR_H_

#include "haclog/haclog_macro.h"

HACLOG_EXTERN_C_BEGIN

enum {
	HACLOG_ERR_OK = 0, //!< no error
	HACLOG_ERR_UNKNOWN, //!< unknown error
	HACLOG_ERR_ALLOC_MEM, //!< failed allocated memory space
	HACLOG_ERR_PRINTF_SPEC_LENGTH, //!< invalid length in format specifier
	HACLOG_ERR_PRINTF_TYPE, //!< unrecognized format specifier
	HACLOG_ERR_SYS_CALL, //!< error system call
	HACLOG_ERR_ARGUMENTS, //!< invalid arguments
	MAX_HACLOG_ERR,
};

/**
 * @brief get haclog last error
 *
 * @return  error code
 */
HACLOG_EXPORT
unsigned int haclog_last_error();

/**
 * @brief set haclog last error
 *
 * @param err  error code
 */
HACLOG_EXPORT
void haclog_set_error(unsigned int err);

HACLOG_EXTERN_C_END

#endif // !HACLOG_ERROR_H_
