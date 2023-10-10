/******************************************************************************
 *  @file         haclog_path.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-08
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog path
 *****************************************************************************/

#ifndef HACLOG_PATH_H_
#define HACLOG_PATH_H_

#include "haclog/haclog_macro.h"

HACLOG_EXTERN_C_BEGIN

#if HACLOG_PLATFORM_WINDOWS
	#define HACLOG_MAX_PATH 260
#else
	#define HACLOG_MAX_PATH 1024
#endif

/**
 * @brief return a normalized absolutized version of the path
 *
 * @param path  the original path
 * @param ret   normalized absolutized version of path
 * @param size  the max size of returned path (include '\0')
 * 
 * @return on success returns 0, otherwise return errno
 */
HACLOG_EXPORT 
int haclog_path_abspath(const char *path, char *ret, unsigned int size);

/**
 * @brief return the base name of pathname path
 *
 * @param path  the original path
 * @param ret   the base name
 * @param size  the max size of returned basename (include '\0')
 *
 * @return on success returns 0, otherwise return errno
 */
HACLOG_EXPORT 
int haclog_path_basename(const char *path, char *ret, unsigned int size);

/**
 * @brief return the dirname of pathname path
 *
 * @param path  the original path
 * @param ret   the dirname
 * @param size  the max size of returned dirname (include '\0')
 *
 * @return on success returns 0, otherwise return errno
 */
HACLOG_EXPORT 
int haclog_path_dirname(const char *path, char *ret, unsigned int size);

/**
 * @brief check whether the path is absolutized version of path
 *
 * @param path file path
 *
 * @return on success returns nonzero, otherwise returns zero
 */
HACLOG_EXPORT 
int haclog_path_isabs(const char *path);

/**
 * @brief check whether the path exists
 *
 * @param path
 *
 * @return on success returns nonzero, otherwise returns zero
 */
HACLOG_EXPORT
int haclog_path_exists(const char *path);

/**
 * @brief join two path
 *
 * @param path1  path 1
 * @param path2  path 2
 * @param ret    the joined path
 * @param size   the max size of returned path (include '\0')
 *
 * @return on success returns 0, otherwise return errno
 */
HACLOG_EXPORT
int haclog_path_join(const char *path1, const char *path2, char *ret, unsigned int size);

/**
 * @brief normalize a pathname
 *
 * @param path  original path
 * @param ret   normalized path
 * @param size  the max size of returned path (include '\0')
 *
 * @return on success returns 0, otherwise return errno
 */
HACLOG_EXPORT
int haclog_path_normpath(const char *path, char *ret, unsigned int size);

HACLOG_EXTERN_C_END

#endif // !HACLOG_PATH_H_
