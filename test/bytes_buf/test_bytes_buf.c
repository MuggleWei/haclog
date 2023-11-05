#include "haclog/haclog.h"
#include "unity.h"

void setUp()
{
}

void tearDown()
{
}

void test_bytes_buf_init_cap()
{
	haclog_bytes_buffer_t *bytes_buf = NULL;

	bytes_buf = haclog_bytes_buffer_new(1 * HACLOG_CACHE_INTERVAL);
	TEST_ASSERT_NULL(bytes_buf);

	bytes_buf = haclog_bytes_buffer_new(2 * HACLOG_CACHE_INTERVAL);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_w_fc_case1()
{
	haclog_bytes_buffer_t *bytes_buf = NULL;
	int capacity = 8 * HACLOG_CACHE_INTERVAL;
	haclog_atomic_int num_bytes = 0;
	haclog_atomic_int wpos = 0;

	bytes_buf = haclog_bytes_buffer_new(capacity);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	num_bytes = (capacity / 2) - HACLOG_CACHE_INTERVAL;
	wpos = haclog_bytes_buffer_w_fc(bytes_buf, num_bytes, 0, 0);
	TEST_ASSERT_EQUAL(wpos, 0);

	num_bytes = (capacity / 2) - HACLOG_CACHE_INTERVAL + 1;
	wpos = haclog_bytes_buffer_w_fc(bytes_buf, num_bytes, 0, 0);
	TEST_ASSERT_EQUAL(wpos, -1);

	TEST_ASSERT_EQUAL(haclog_last_error(), HACLOG_ERR_ARGUMENTS);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_w_fc_case2()
{
	haclog_bytes_buffer_t *bytes_buf = NULL;
	int capacity = 8 * HACLOG_CACHE_INTERVAL;
	haclog_atomic_int num_bytes = 0;
	haclog_atomic_int pos = 0;

	bytes_buf = haclog_bytes_buffer_new(capacity);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	num_bytes = HACLOG_CACHE_INTERVAL;
	haclog_bytes_buffer_w_move(bytes_buf, bytes_buf->capacity - num_bytes);
	pos = haclog_bytes_buffer_w_fc(bytes_buf, num_bytes, bytes_buf->r,
								   bytes_buf->w);
	TEST_ASSERT_EQUAL(pos, bytes_buf->capacity - num_bytes);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_w_fc_case3()
{
	haclog_bytes_buffer_t *bytes_buf = NULL;
	int capacity = 8 * HACLOG_CACHE_INTERVAL;
	haclog_atomic_int num_bytes = 0;
	haclog_atomic_int pos = 0;

	bytes_buf = haclog_bytes_buffer_new(capacity);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	num_bytes = HACLOG_CACHE_INTERVAL;
	haclog_bytes_buffer_w_move(bytes_buf, bytes_buf->capacity - num_bytes);

	num_bytes *= 2;
	pos = haclog_bytes_buffer_w_fc(bytes_buf, num_bytes, bytes_buf->r,
								   bytes_buf->w);
	TEST_ASSERT_EQUAL(pos, -2);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_w_fc_case4()
{
	haclog_bytes_buffer_t *bytes_buf = NULL;
	int capacity = 8 * HACLOG_CACHE_INTERVAL;
	haclog_atomic_int num_bytes = 0;
	haclog_atomic_int pos = 0;

	bytes_buf = haclog_bytes_buffer_new(capacity);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	num_bytes = HACLOG_CACHE_INTERVAL;
	haclog_bytes_buffer_w_move(bytes_buf, bytes_buf->capacity - num_bytes);

	num_bytes *= 2;
	haclog_bytes_buffer_r_move(bytes_buf, num_bytes + HACLOG_CACHE_INTERVAL);
	pos = haclog_bytes_buffer_w_fc(bytes_buf, num_bytes, bytes_buf->r,
								   bytes_buf->w);
	TEST_ASSERT_EQUAL(pos, 0);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_w_fc_case5()
{
	haclog_bytes_buffer_t *bytes_buf = NULL;
	int capacity = 8 * HACLOG_CACHE_INTERVAL;
	haclog_atomic_int num_bytes = 0;
	haclog_atomic_int pos = 0;

	bytes_buf = haclog_bytes_buffer_new(capacity);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	num_bytes = HACLOG_CACHE_INTERVAL;
	haclog_bytes_buffer_r_move(bytes_buf, num_bytes + HACLOG_CACHE_INTERVAL);

	pos = haclog_bytes_buffer_w_fc(bytes_buf, num_bytes, bytes_buf->r,
								   bytes_buf->w);
	TEST_ASSERT_EQUAL(pos, 0);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_w_fc_case6()
{
	haclog_bytes_buffer_t *bytes_buf = NULL;
	int capacity = 8 * HACLOG_CACHE_INTERVAL;
	haclog_atomic_int num_bytes = 0;
	haclog_atomic_int pos = 0;

	bytes_buf = haclog_bytes_buffer_new(capacity);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	num_bytes = HACLOG_CACHE_INTERVAL;
	haclog_bytes_buffer_r_move(bytes_buf, num_bytes + HACLOG_CACHE_INTERVAL);

	haclog_bytes_buffer_w_move(bytes_buf, 1);
	pos = haclog_bytes_buffer_w_fc(bytes_buf, num_bytes, bytes_buf->r,
								   bytes_buf->w);
	TEST_ASSERT_EQUAL(pos, -2);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_w_move_case1()
{
	haclog_bytes_buffer_t *bytes_buf =
		haclog_bytes_buffer_new(8 * HACLOG_CACHE_INTERVAL);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	for (int pos = 0; pos < bytes_buf->capacity; pos++) {
		int ret = haclog_bytes_buffer_w_move(bytes_buf, pos);
		TEST_ASSERT_EQUAL(ret, 0);
		TEST_ASSERT_EQUAL(bytes_buf->w, pos);
	}

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_w_move_case2()
{
	haclog_bytes_buffer_t *bytes_buf =
		haclog_bytes_buffer_new(8 * HACLOG_CACHE_INTERVAL);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	int ret = 0;
	ret = haclog_bytes_buffer_w_move(bytes_buf, -1);
	TEST_ASSERT_EQUAL(ret, -1);
	TEST_ASSERT_EQUAL(haclog_last_error(), HACLOG_ERR_ARGUMENTS);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_r_move_case1()
{
	haclog_bytes_buffer_t *bytes_buf =
		haclog_bytes_buffer_new(8 * HACLOG_CACHE_INTERVAL);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	for (int pos = 0; pos < bytes_buf->capacity; pos++) {
		int ret = haclog_bytes_buffer_r_move(bytes_buf, pos);
		TEST_ASSERT_EQUAL(ret, 0);
		TEST_ASSERT_EQUAL(bytes_buf->r, pos);
	}

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_r_move_case2()
{
	haclog_bytes_buffer_t *bytes_buf =
		haclog_bytes_buffer_new(8 * HACLOG_CACHE_INTERVAL);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	int ret = 0;
	ret = haclog_bytes_buffer_r_move(bytes_buf, -1);
	TEST_ASSERT_EQUAL(ret, -1);
	TEST_ASSERT_EQUAL(haclog_last_error(), HACLOG_ERR_ARGUMENTS);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_get_invalid()
{
	haclog_bytes_buffer_t *bytes_buf =
		haclog_bytes_buffer_new(8 * HACLOG_CACHE_INTERVAL);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	char *p = NULL;
	p = haclog_bytes_buffer_get(bytes_buf, -1);
	TEST_ASSERT_NULL(p);

	p = haclog_bytes_buffer_get(bytes_buf, bytes_buf->capacity);
	TEST_ASSERT_NULL(p);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_get_valid()
{
	haclog_bytes_buffer_t *bytes_buf =
		haclog_bytes_buffer_new(8 * HACLOG_CACHE_INTERVAL);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	char *p = NULL;
	for (int pos = 0; pos < bytes_buf->capacity; pos++) {
		p = haclog_bytes_buffer_get(bytes_buf, pos);
		TEST_ASSERT_NOT_NULL(p);
	}

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_join_case1()
{
	haclog_bytes_buffer_t *bytes_buf =
		haclog_bytes_buffer_new(8 * HACLOG_CACHE_INTERVAL);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	haclog_bytes_buffer_join(bytes_buf);
	TEST_ASSERT_EQUAL(bytes_buf->w, bytes_buf->r);

	haclog_bytes_buffer_free(bytes_buf);
}

#if HACLOG_PLATFORM_WINDOWS
static haclog_thread_ret_t __stdcall s_thread_bytes_buf_join_func(void *args)
#else
static haclog_thread_ret_t s_thread_bytes_buf_join_func(void *args)
#endif
{
	haclog_bytes_buffer_t *bytes_buf = (haclog_bytes_buffer_t *)args;
	haclog_bytes_buffer_join(bytes_buf);
	return NULL;
}
void test_bytes_buf_join_case2()
{
	haclog_bytes_buffer_t *bytes_buf =
		haclog_bytes_buffer_new(8 * HACLOG_CACHE_INTERVAL);
	TEST_ASSERT_NOT_NULL(bytes_buf);

	haclog_bytes_buffer_w_move(bytes_buf, HACLOG_CACHE_INTERVAL);

	haclog_thread_t th;
	haclog_thread_create(&th, s_thread_bytes_buf_join_func, bytes_buf);

	for (int i = 0; i < 100; i++) {
		haclog_nsleep(1 * 1000 * 1000);
	}
	haclog_bytes_buffer_r_move(bytes_buf, HACLOG_CACHE_INTERVAL);

	haclog_thread_join(&th);

	TEST_ASSERT_EQUAL(bytes_buf->w, bytes_buf->r);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_case1()
{
	haclog_bytes_buffer_t *bytes_buf = haclog_bytes_buffer_new(1024 * 1024 * 8);
	bytes_buf->w = 3194248;
	bytes_buf->r = 3194480;

	const haclog_atomic_int hdr_size =
		(haclog_atomic_int)sizeof(haclog_serialize_hdr_t);
	haclog_atomic_int r =
		haclog_atomic_load(&bytes_buf->r, haclog_memory_order_relaxed);
	haclog_atomic_int w = bytes_buf->w;
	haclog_atomic_int w_hdr = 0;
	haclog_atomic_int w_const_args = 0;
	haclog_atomic_int w_str = 0;

	w_hdr = haclog_bytes_buffer_w_fc(bytes_buf, hdr_size, r, w);
	TEST_ASSERT_EQUAL(w_hdr, w);
	w = w_hdr + hdr_size;

	w_const_args = haclog_bytes_buffer_w_fc(bytes_buf, 40, r, w);
	TEST_ASSERT_EQUAL(w_const_args, w);
	w = w_const_args + 40;

	w_str = haclog_bytes_buffer_w_fc(bytes_buf, 128, r, w);
	TEST_ASSERT_EQUAL(w_str, -2);

	haclog_bytes_buffer_free(bytes_buf);
}

void test_bytes_buf_case2()
{
	haclog_bytes_buffer_t *bytes_buf = haclog_bytes_buffer_new(1024 * 1024 * 8);
	bytes_buf->w = 8388512;
	bytes_buf->r = 8388600;
	unsigned long extra_len = 128;
	haclog_atomic_int w = haclog_bytes_buffer_w_fc(bytes_buf, extra_len,
												   bytes_buf->r, bytes_buf->w);
	TEST_ASSERT_EQUAL(w, -2);

	haclog_bytes_buffer_free(bytes_buf);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_bytes_buf_init_cap);
	RUN_TEST(test_bytes_buf_w_fc_case1);
	RUN_TEST(test_bytes_buf_w_fc_case2);
	RUN_TEST(test_bytes_buf_w_fc_case3);
	RUN_TEST(test_bytes_buf_w_fc_case4);
	RUN_TEST(test_bytes_buf_w_fc_case5);
	RUN_TEST(test_bytes_buf_w_fc_case6);
	RUN_TEST(test_bytes_buf_w_move_case1);
	RUN_TEST(test_bytes_buf_w_move_case2);
	RUN_TEST(test_bytes_buf_r_move_case1);
	RUN_TEST(test_bytes_buf_r_move_case2);
	RUN_TEST(test_bytes_buf_get_invalid);
	RUN_TEST(test_bytes_buf_get_valid);
	RUN_TEST(test_bytes_buf_join_case1);
	RUN_TEST(test_bytes_buf_join_case2);
	RUN_TEST(test_bytes_buf_case1);
	RUN_TEST(test_bytes_buf_case2);

	return UNITY_END();
}
