#include "haclog_bytes_buffer.h"
#include <stdlib.h>
#include <string.h>
#include "haclog/haclog_sleep.h"
#include "haclog/haclog_stacktrace.h"
#include "haclog/haclog_thread.h"
#include "haclog/haclog_err.h"

#define HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL HACLOG_CACHE_INTERVAL

haclog_bytes_buffer_t *haclog_bytes_buffer_new(int capacity)
{
	haclog_bytes_buffer_t *bytes_buf = NULL;
	bytes_buf = (haclog_bytes_buffer_t *)malloc(sizeof(haclog_bytes_buffer_t));
	if (bytes_buf == NULL) {
		haclog_set_error(HACLOG_ERR_ALLOC_MEM);
		return NULL;
	}

	memset(bytes_buf, 0, sizeof(*bytes_buf));

	if (capacity < 2 * HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL) {
		haclog_set_error(HACLOG_ERR_ARGUMENTS);
		haclog_bytes_buffer_free(bytes_buf);
		return NULL;
	}
	bytes_buf->capacity = capacity;
	bytes_buf->buffer = (char *)malloc(capacity);
	if (bytes_buf->buffer == NULL) {
		haclog_set_error(HACLOG_ERR_ALLOC_MEM);
		haclog_bytes_buffer_free(bytes_buf);
		return NULL;
	}

	return bytes_buf;
}

void haclog_bytes_buffer_free(haclog_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf) {
		if (bytes_buf->buffer) {
			free(bytes_buf->buffer);
			bytes_buf->buffer = NULL;
		}
		bytes_buf->capacity = 0;
		bytes_buf->w = 0;
		bytes_buf->r = 0;

		free(bytes_buf);
	}
}

haclog_atomic_int haclog_bytes_buffer_w_fc(haclog_bytes_buffer_t *bytes_buf,
										   haclog_atomic_int num_bytes,
										   haclog_atomic_int r,
										   haclog_atomic_int w)
{
	if (num_bytes >
		(bytes_buf->capacity / 2) - HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL) {
		// beyond the num_bytes limit
		haclog_set_error(HACLOG_ERR_ARGUMENTS);
		return -1;
	}

	if (w >= r) {
		/*
		           r      w
		 |---------|------|----|
		 */
		// contiguous writable
		haclog_atomic_int cw = bytes_buf->capacity - w;
		if (cw > num_bytes) {
			return w;
		} else if (cw < num_bytes) {
			if (r - HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL < num_bytes) {
				// need wait reader move
				return -2;
			} else {
				/*
				 w         r
				 |---------|-----------|
				 */
				return 0;
			}
		} else {
			/*
	      (real w)     r            (w)
			 |---------|-----------|
			 */
			// NOTE:
			//   In this situation, w move to cap, if next action is 
			//   haclog_bytes_buffer_w_move, w actually move to 0, it need to
			//   detect r position
			if (r == 0) {
				// need wait reader move
				return -2;
			} else {
				return w;
			}
		}
	} else {
		/*
		       w         r
		 |-----|---------|-----|
		 */
		// contiguous writable
		haclog_atomic_int cw = r - w - HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL;
		if (cw < num_bytes) {
			return -2;
		} else {
			return w;
		}
	}
}

int haclog_bytes_buffer_w_move(haclog_bytes_buffer_t *bytes_buf,
							   haclog_atomic_int pos)
{
	if (pos < 0 || pos >= bytes_buf->capacity) {
		if (pos == bytes_buf->capacity) {
			pos = 0;
		} else {
			haclog_set_error(HACLOG_ERR_ARGUMENTS);
			return -1;
		}
	}

	haclog_atomic_store(&bytes_buf->w, pos, haclog_memory_order_release);
	return 0;
}

int haclog_bytes_buffer_r_move(haclog_bytes_buffer_t *bytes_buf,
							   haclog_atomic_int pos)
{
	if (pos < 0 || pos >= bytes_buf->capacity) {
		if (pos == bytes_buf->capacity) {
			pos = 0;
		} else {
			haclog_set_error(HACLOG_ERR_ARGUMENTS);
			return -1;
		}
	}

	haclog_atomic_store(&bytes_buf->r, pos, haclog_memory_order_relaxed);
	return 0;
}

char *haclog_bytes_buffer_get(haclog_bytes_buffer_t *bytes_buf,
							  haclog_atomic_int pos)
{
	if (pos < 0 || pos >= bytes_buf->capacity) {
		haclog_set_error(HACLOG_ERR_ARGUMENTS);
		return NULL;
	}

	return bytes_buf->buffer + pos;
}

void haclog_bytes_buffer_join(haclog_bytes_buffer_t *bytes_buf)
{
	do {
		haclog_atomic_int r =
			haclog_atomic_load(&bytes_buf->r, haclog_memory_order_relaxed);
		if (r == bytes_buf->w) {
			break;
		}
		haclog_nsleep(1 * 1000 * 1000);
	} while (1);
}
