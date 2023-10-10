#include "haclog/haclog.h"
#include <stdlib.h>
#include <string.h>

#define BUF_CAP (1024 * 1024 * 8)

const haclog_atomic_int hdr_size =
	(haclog_atomic_int)sizeof(haclog_serialize_hdr_t);
const unsigned int param_size = 40;
const unsigned int extra_len = 128;

haclog_atomic_int total_w_steps = 0;
haclog_atomic_int total_r_steps = 0;

int is_prime(unsigned long long num)
{
	if (num <= 1)
		return 0;
	if (num % 2 == 0 && num > 2)
		return 0;
	for (unsigned long long i = 3; i < num / 2; i += 2) {
		if (num % i == 0)
			return 0;
	}
	return 1;
}

haclog_thread_ret_t consumer(void *arg)
{
	FILE *fp = fopen("debug_r.log", "w");

	haclog_bytes_buffer_t *bytes_buf = (haclog_bytes_buffer_t *)arg;
	unsigned long long cnt = 0;
	while (1) {
		haclog_atomic_int w =
			haclog_atomic_load(&bytes_buf->w, haclog_memory_order_acquire);
		if (bytes_buf->r == w) {
			haclog_thread_yield();
			continue;
		}

		if (bytes_buf->capacity - bytes_buf->r < hdr_size) {
			bytes_buf->r = 0;
		}

		haclog_serialize_hdr_t *hdr =
			(haclog_serialize_hdr_t *)haclog_bytes_buffer_get(bytes_buf,
															  bytes_buf->r);

		unsigned long long *p = (unsigned long long *)haclog_bytes_buffer_get(
			bytes_buf, hdr->pos_const);
		if (is_prime(*p)) {
			++cnt;
		}

		char *s = haclog_bytes_buffer_get(bytes_buf, hdr->pos_str);
		if (memcmp(s, "HACLOG", 6) != 0) {
			haclog_debug_break();
			fprintf(stdout, "total prime: %llu", cnt);
			fprintf(stderr, "ohh!");
			exit(EXIT_FAILURE);
		}

		haclog_atomic_int next_r = 0;
		if (hdr->extra_len > 0) {
			next_r = hdr->pos_str + hdr->extra_len;
		} else {
			next_r = hdr->pos_const + param_size;
		}

		total_r_steps++;
		fprintf(fp, "w: %d, r: %d -> %d, total_r: %d\n", w, bytes_buf->r,
				next_r, total_r_steps);
		if (total_r_steps % 100000 == 0) {
			fprintf(stdout, "total_w: %d, total_r: %d, diff: %d\n", total_w_steps,
					total_r_steps, total_w_steps - total_r_steps);
		}

		haclog_atomic_store(&bytes_buf->r, next_r, haclog_memory_order_relaxed);
	}

	return NULL;
}

void producer(haclog_bytes_buffer_t *bytes_buf)
{
	FILE *fp = fopen("debug_w.log", "w");

	haclog_atomic_int w_hdr = 0;
	haclog_atomic_int w_const_args = 0;
	haclog_atomic_int w_str = 0;
	for (unsigned long long i = 0; i < 32 * 1000000; i++) {
		haclog_atomic_int w = bytes_buf->w;
		haclog_atomic_int r =
			haclog_atomic_load(&bytes_buf->r, haclog_memory_order_relaxed);

		do {
			w_hdr = haclog_bytes_buffer_w_fc(bytes_buf, hdr_size, r, w);
			if (w_hdr >= 0) {
				break;
			}

			if (w_hdr == -1) {
				haclog_debug_break();
				exit(EXIT_FAILURE);
			}

			haclog_thread_yield();
			r = haclog_atomic_load(&bytes_buf->r, haclog_memory_order_relaxed);
		} while (1);
		w = w_hdr + hdr_size;

		do {
			w_const_args =
				haclog_bytes_buffer_w_fc(bytes_buf, param_size, r, w);
			if (w_const_args >= 0) {
				break;
			}

			if (w_const_args == -1) {
				haclog_debug_break();
				exit(EXIT_FAILURE);
			}

			haclog_thread_yield();
			r = haclog_atomic_load(&bytes_buf->r, haclog_memory_order_relaxed);
		} while (1);
		w = w_const_args + param_size;

		do {
			w_str = haclog_bytes_buffer_w_fc(bytes_buf, extra_len, r, w);
			if (w_str >= 0) {
				break;
			}

			if (w_str == -1) {
				haclog_debug_break();
				exit(EXIT_FAILURE);
			}

			haclog_thread_yield();
			r = haclog_atomic_load(&bytes_buf->r, haclog_memory_order_relaxed);
		} while (1);

		// fillup hdr
		haclog_serialize_hdr_t *hdr =
			(haclog_serialize_hdr_t *)haclog_bytes_buffer_get(bytes_buf, w_hdr);
		timespec_get(&hdr->ts, TIME_UTC);
		hdr->pos_const = w_const_args;
		hdr->pos_str = w_str;
		hdr->extra_len = extra_len;
		hdr->primitive = NULL;

		// write index
		unsigned long long *p = (unsigned long long *)haclog_bytes_buffer_get(
			bytes_buf, w_const_args);
		*p = i;

		// write magic word
		char *s = haclog_bytes_buffer_get(bytes_buf, w_str);
		memcpy(s, "HACLOG", 6);

		haclog_atomic_int next_w = w_str + extra_len;
		total_w_steps++;
		fprintf(fp, "w: %d -> %d, r: %d, total_w: %d\n", bytes_buf->w, next_w,
				r, total_w_steps);

		haclog_atomic_store(&bytes_buf->w, next_w, haclog_memory_order_release);
	}
}

int main()
{
	haclog_bytes_buffer_t *bytes_buf = haclog_bytes_buffer_new(BUF_CAP);

	haclog_thread_t th_consumer;
	haclog_thread_create(&th_consumer, consumer, bytes_buf);
	haclog_thread_detach(&th_consumer);

	producer(bytes_buf);

	return 0;
}
