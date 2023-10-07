#include "haclog_bytes_buffer.h"
#include "haclog/haclog_thread.h"
#include <stdlib.h>
#include <string.h>
#include "haclog/haclog_err.h"

#define HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL ((int)sizeof(void *))

haclog_bytes_buffer_t *haclog_bytes_buffer_new(int capacity)
{
	haclog_bytes_buffer_t *bytes_buf = NULL;
	bytes_buf = (haclog_bytes_buffer_t *)malloc(sizeof(haclog_bytes_buffer_t));
	if (bytes_buf == NULL) {
		haclog_set_error(HACLOG_ERR_ALLOC_MEM);
		return NULL;
	}

	memset(bytes_buf, 0, sizeof(*bytes_buf));

	if (capacity <= 8) {
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
										   haclog_atomic_int *p_r,
										   haclog_atomic_int w)
{
	if (num_bytes >
		(bytes_buf->capacity / 2) - HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL) {
		// beyond the num_bytes limit
		haclog_set_error(HACLOG_ERR_ARGUMENTS);
		return -1;
	}

	if (w >= *p_r) {
		/*
		           r      w
		 |---------|------|----|
		 */
		// contiguous writable
		haclog_atomic_int cw = bytes_buf->capacity - w;
		if (cw < num_bytes) {
			while (*p_r - HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL < num_bytes) {
				haclog_thread_yield();
				*p_r = haclog_atomic_load(&bytes_buf->r,
										  haclog_memory_order_relaxed);
			}

			/*
			 w         r
			 |---------|-----------|
			 */
			return 0;
		} else {
			return w;
		}
	} else {
		/*
		       w         r
		 |-----|---------|-----|
		 */
		// contiguous writable
		haclog_atomic_int cw = *p_r - w - HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL;
		if (cw < num_bytes) {
			haclog_atomic_int w_to_end = bytes_buf->capacity - w;
			if (w_to_end < num_bytes) {
				while (*p_r - HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL < num_bytes) {
					haclog_thread_yield();
					*p_r = haclog_atomic_load(&bytes_buf->r,
											  haclog_memory_order_relaxed);
				}

				/*
				 w         r
				 |---------|-----------|
				 */
				return 0;
			} else {
				while (1) {
					haclog_thread_yield();
					*p_r = haclog_atomic_load(&bytes_buf->r,
											  haclog_memory_order_relaxed);

					if (*p_r < w) {
						/*
						   r   w
						 |-|---|---------------|
						 */
						return w;
					} else {
						cw = *p_r - w - HACLOG_BYTES_BUFFER_WR_MIN_INTERVAL;
						if (cw < num_bytes) {
							continue;
						}
						/*
						       w             r
						 |-----|-------------|-|
						 */
						return w;
					}
				}
			}
		} else {
			return w;
		}
	}
}

int haclog_bytes_buffer_w_move(haclog_bytes_buffer_t *bytes_buf,
							   haclog_atomic_int pos)
{
	if (pos < 0 || pos >= bytes_buf->capacity) {
		haclog_set_error(HACLOG_ERR_ARGUMENTS);
		return -1;
	}

	haclog_atomic_store(&bytes_buf->w, pos, haclog_memory_order_release);

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
		haclog_thread_yield();
	} while (1);
}
