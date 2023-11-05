/******************************************************************************
 *  @file         haclog_bytesbuf.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-10-01
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        haclog bytes buffer
 *****************************************************************************/

#ifndef HACLOG_BYTES_BUFFER_H_
#define HACLOG_BYTES_BUFFER_H_

#include "haclog/haclog_macro.h"
#include "haclog/haclog_thread.h"

HACLOG_EXTERN_C_BEGIN

typedef struct haclog_bytes_buffer {
	char *buffer;
	haclog_atomic_int capacity;
	haclog_atomic_int w;
	// char cache_line_interval[HACLOG_CACHE_INTERVAL];
	haclog_atomic_int r;
} haclog_bytes_buffer_t;

/**
 * @brief initialize bytes buffer
 *
 * @param capacity   bytes buffer capacity
 *
 * @return
 *   - on success, return bytes buffer pointer
 *   - otherwise, return NULL and set haclog last error
 */
HACLOG_EXPORT
haclog_bytes_buffer_t *haclog_bytes_buffer_new(int capacity);

/**
 * @brief free bytes buffer
 *
 * @param bytes_buf  bytes buffer pointer
 */
HACLOG_EXPORT
void haclog_bytes_buffer_free(haclog_bytes_buffer_t *bytes_buf);

/**
 * @brief find contiguous memory for write from specify w pos
 *
 * @param bytes_buf  bytes buffer pointer
 * @param num_bytes  number of required bytes
 * @param r          reader position pointer
 * @param w          writer position
 *
 * @return
 *   - on success, return write position
 *   - on failed, return -1 and set haclog last error
 *   - on need wait reader move, return -2
 */
HACLOG_EXPORT
haclog_atomic_int haclog_bytes_buffer_w_fc(haclog_bytes_buffer_t *bytes_buf,
										   haclog_atomic_int num_bytes,
										   haclog_atomic_int r,
										   haclog_atomic_int w);

/**
 * @brief move writer to specify position
 *
 * @param bytes_buf  bytes buffer pointer
 * @param pos        expect position
 *
 * @return 
 *   - on success, return 0
 *   - on failed, return -1 and set haclog last error
 */
HACLOG_EXPORT
int haclog_bytes_buffer_w_move(haclog_bytes_buffer_t *bytes_buf,
							   haclog_atomic_int pos);

/**
 * @brief move reader to specify position
 *
 * @param bytes_buf  bytes buffer pointer
 * @param pos        expect position
 *
 * @return 
 *   - on success, return 0
 *   - on failed, return -1 and set haclog last error
 */
HACLOG_EXPORT
int haclog_bytes_buffer_r_move(haclog_bytes_buffer_t *bytes_buf,
							   haclog_atomic_int pos);

/**
 * @brief get address pointer
 *
 * @param bytes_buf  bytes buffer pointer
 * @param pos  position in bytes buffer
 *
 * @return
 *   - on success, return address pointer
 *   - on failed, return NULL and set haclog last error
 */
HACLOG_EXPORT
char *haclog_bytes_buffer_get(haclog_bytes_buffer_t *bytes_buf,
							  haclog_atomic_int pos);

/**
 * @brief wait all cache has been handle
 *
 * @param bytes_buf  bytes buffer pointer
 */
HACLOG_EXPORT
void haclog_bytes_buffer_join(haclog_bytes_buffer_t *bytes_buf);

HACLOG_EXTERN_C_END

#endif // !HACLOG_BYTES_BUFFER_H_
