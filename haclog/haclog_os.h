/******************************************************************************
 *  @file         haclog_os.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-08
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog os
 *****************************************************************************/

#ifndef HACLOG_OS_H_
#define HACLOG_OS_H_

#include "haclog/haclog_macro.h"

HACLOG_EXTERN_C_BEGIN

/**
 * @brief get current process file path
 *
 * @param path  the returned path
 * @param size  the max size of path (include '\0')
 *
 * @return on success returns 0, otherwise return errno
 */
HACLOG_EXPORT
int haclog_os_process_path(char *path, unsigned int size);

/**
 * @brief get current working directory
 *
 * @param path  the returned path
 * @param size  the max size of path (include '\0')
 *
 * @return on success returns 0, otherwise return errno
 */
HACLOG_EXPORT
int haclog_os_curdir(char *path, unsigned int size);

/**
 * @brief change working directory
 *
 * @param path  target working directory
 *
 * @return on success returns 0, otherwise return errno
 */
HACLOG_EXPORT
int haclog_os_chdir(const char *path);

/**
 * @brief recursive create directory named path
 *
 * @param path: path need to create
 *
 * @return success returns 0, otherwise return errno
 */
HACLOG_EXPORT
int haclog_os_mkdir(const char *path);

/**
 * @brief remove the file path
 *
 * @param path  the file need to be remove
 *
 * @return success returns 0, otherwise return errno
 */
HACLOG_EXPORT
int haclog_os_remove(const char *path);

/**
 * @brief delete an empty directory
 *
 * @param path  directory need to be delete
 *
 * @return success returns 0, otherwise return errno
 */
HACLOG_EXPORT
int haclog_os_rmdir(const char *path);

/**
 * @brief rename the file or directory src to dst
 *
 * @param src  source file path
 * @param dst  destination fiel path
 *
 * @return success returns 0, otherwise return errno
 */
HACLOG_EXPORT
int haclog_os_rename(const char *src, const char *dst);

HACLOG_EXTERN_C_END

#endif // !HACLOG_OS_H_
