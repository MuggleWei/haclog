/******************************************************************************
 *  @file         haclog_macro.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-09-26
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog macro
 *****************************************************************************/

#ifndef HACLOG_MACRO_H_
#define HACLOG_MACRO_H_

#include "haclog/haclog_config.h"

// dll export
#if defined(_WIN32) && defined(HACLOG_USE_DLL)
	#ifdef HACLOG_EXPORTS
		#define HACLOG_EXPORT __declspec(dllexport)
	#else
		#define HACLOG_EXPORT __declspec(dllimport)
	#endif
#else
	#define HACLOG_EXPORT
#endif

// extern c
#ifdef __cplusplus
	#define HACLOG_EXTERN_C_BEGIN extern "C" {
	#define HACLOG_EXTERN_C_END }
#else
	#define HACLOG_EXTERN_C_BEGIN
	#define HACLOG_EXTERN_C_END
#endif

// platform
#ifdef _WIN32
	#define HACLOG_PLATFORM_WINDOWS 1
	#ifdef _WIN64
		#define HACLOG_PLATFORM_WIN64 1
	#endif
#elif __APPLE__
	#define HACLOG_PLATFORM_APPLE 1
	#include "TargetConditionals.h"
	#if TARGET_IPHONE_SIMULATOR
		// iOS Simulator
	#elif TARGET_OS_IPHONE
		// iOS device
	#elif TARGET_OS_MAC
		// Other kinds of Mac OS
	#else
		// Unknown Apple platform
	#endif
#elif __linux__
	#define HACLOG_PLATFORM_LINUX 1
#elif __FreeBSD__
	#define HACLOG_PLATFORM_FREEBSD 1
#elif __unix__
	#define HACLOG_PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
	// POSIX
#else
	// Unknown compiler
#endif

// unused
#define HACLOG_UNUSED(x) (void)x

// round to a number that number=2^x and number!=0
#define HACLOG_ROUND_TO_2POWX(value, roundto) \
	(((value) + ((roundto)-1)) & ~((roundto)-1))

#endif // !HACLOG_MACRO_H_
