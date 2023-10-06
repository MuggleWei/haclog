/******************************************************************************
 *  @file         haclog_thread.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-09-27
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog thread
 *****************************************************************************/

#ifndef HACLOG_THREAD_H_
#define HACLOG_THREAD_H_

#include "haclog/haclog_macro.h"
#if HACLOG_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <pthread.h>
#endif

HACLOG_EXTERN_C_BEGIN

typedef unsigned long haclog_thread_id;

#if HACLOG_PLATFORM_WINDOWS

	#define haclog_thread_local __declspec(thread)

typedef unsigned int haclog_thread_ret_t;

typedef haclog_thread_ret_t __stdcall haclog_thread_routine(void *args);

typedef struct haclog_thread {
	HANDLE handle;
	unsigned int id;
} haclog_thread_t;

	#define haclog_atomic_byte LONG
	#define haclog_atomic_int LONG

	#define haclog_memory_order_relaxed 0
	#define haclog_memory_order_consume 0
	#define haclog_memory_order_acquire 0
	#define haclog_memory_order_release 0
	#define haclog_memory_order_acq_rel 0
	#define haclog_memory_order_seq_cst 0

	#define haclog_atomic_load(ptr, memorder) InterlockedOr(ptr, 0)

	#define haclog_atomic_store(ptr, val, memorder) \
		InterlockedExchange(ptr, val)

	#define haclog_atomic_fetch_add(ptr, val, memorder) \
		InterlockedExchangeAdd(ptr, val)

	#define haclog_thread_fence(memorder) MemoryBarrier()

	#define haclog_atomic_test_and_set(ptr, memorder) \
		!_interlockedbittestandset(ptr, 0)

	#define haclog_atomic_clear(ptr, memorder) \
		_interlockedbittestandreset(ptr, 0)

#else

	#define haclog_thread_local __thread

typedef void *haclog_thread_ret_t;

typedef haclog_thread_ret_t haclog_thread_routine(void *args);

typedef struct haclog_thread {
	pthread_t th;
} haclog_thread_t;

	#define haclog_atomic_byte char // only use in test_and_set
	#define haclog_atomic_int int

	#define haclog_memory_order_relaxed __ATOMIC_RELAXED
	#define haclog_memory_order_consume __ATOMIC_CONSUME
	#define haclog_memory_order_acquire __ATOMIC_ACQUIRE
	#define haclog_memory_order_release __ATOMIC_RELEASE
	#define haclog_memory_order_acq_rel __ATOMIC_ACQ_REL
	#define haclog_memory_order_seq_cst __ATOMIC_SEQ_CST

	#define haclog_atomic_load(ptr, memorder) __atomic_load_n(ptr, memorder)

	#define haclog_atomic_store(ptr, val, memorder) \
		__atomic_store_n(ptr, val, memorder)

	#define haclog_atomic_fetch_add(ptr, val, memorder) \
		__atomic_fetch_add(ptr, val, memorder)

	#define haclog_thread_fence(memorder) __atomic_thread_fence(memorder)

	#define haclog_atomic_test_and_set(ptr, memorder) \
		!__atomic_test_and_set(ptr, memorder)

	#define haclog_atomic_clear(ptr, memorder) __atomic_clear(ptr, memorder)

#endif

/**
 * @brief start a new thread in the calling process
 *
 * @param thread   pointer to thread handler
 * @param routine  thread callback
 * @param args     arguments passing to thread function
 *
 * @return 
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
HACLOG_EXPORT
int haclog_thread_create(haclog_thread_t *thread, haclog_thread_routine routine,
						 void *args);

/**
 * @brief wait for thread to terminate
 *
 * @param thread  pointer to thread handler
 *
 * @return
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
HACLOG_EXPORT
int haclog_thread_join(haclog_thread_t *thread);

/**
 * @brief marks the thread as detached
 *
 * @param thread  pointer to thread handler
 *
 * @return
 *   - on success, return 0
 *   - on failed, return haclog error code
 */
HACLOG_EXPORT
int haclog_thread_detach(haclog_thread_t *thread);

/**
 * @brief get current thread readable id
 *
 * @return current thread readable id
 */
HACLOG_EXPORT
haclog_thread_id haclog_thread_readable_id();

/**
 * @brief get the number of concurrent threads supported by the implementation
 *
 * @return number of concurrent threads supported
 */
HACLOG_EXPORT
int haclog_thread_hardware_concurrency();

/**
 * @brief calling thread to yield execution and relinquish the CPU
 */
HACLOG_EXPORT
void haclog_thread_yield();

HACLOG_EXTERN_C_END

#endif // !HACLOG_THREAD_H_
