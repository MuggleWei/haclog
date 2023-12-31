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
#elif __ANDROID__
	#define HACLOG_PLATFORM_ANDROID 1
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

// constexpr
#ifdef __cplusplus
	#if defined(_MSC_VER) && _MSC_VER < 1920
		#define haclog_constexpr
	#else
		#define haclog_constexpr constexpr
	#endif
#else
	#define haclog_constexpr
#endif

// unused
#define HACLOG_UNUSED(x) (void)x

// round to a number that number=2^x and number!=0
#define HACLOG_ROUND_TO_2POWX(value, roundto) \
	(((value) + ((roundto)-1)) & ~((roundto)-1))

// cache line
#define HACLOG_CACHE_LINE 64
#define HACLOG_CACHE_INTERVAL (2 * HACLOG_CACHE_LINE)

#endif // !HACLOG_MACRO_H_
