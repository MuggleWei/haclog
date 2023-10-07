#include "haclog/haclog.h"
#include "unity.h"

void setUp()
{
}

void tearDown()
{
}

void test_bytes_buf_case1()
{
	haclog_bytes_buffer_t *bytes_buf = haclog_bytes_buffer_new(1024 * 1024 * 8);
	bytes_buf->w = 3194368;
	bytes_buf->r = 3194480;

	const haclog_atomic_int hdr_size =
		(haclog_atomic_int)sizeof(haclog_serialize_hdr_t);
	haclog_atomic_int r =
		haclog_atomic_load(&bytes_buf->r, haclog_memory_order_relaxed);
	haclog_atomic_int w = bytes_buf->w;
	haclog_atomic_int w_hdr = 0;
	haclog_atomic_int w_const_args = 0;
	haclog_atomic_int w_str = 0;

	fprintf(stdout, "w=%d, r=%d\n", w, r);

	w_hdr = haclog_bytes_buffer_w_fc(bytes_buf, hdr_size, &r, w);
	TEST_ASSERT_NOT_EQUAL(w_hdr, -1);
	w = w_hdr + hdr_size;
	fprintf(stdout, "w=%d, r=%d\n", w, r);

	w_const_args = haclog_bytes_buffer_w_fc(bytes_buf, 40, &r, w);
	TEST_ASSERT_NOT_EQUAL(w_hdr, -1);
	w = w_const_args + 40;
	fprintf(stdout, "w=%d, r=%d\n", w, r);

	w_const_args = haclog_bytes_buffer_w_fc(bytes_buf, 128, &r, w);
	TEST_ASSERT_NOT_EQUAL(w_hdr, -1);
	w = w_const_args + 128;
	fprintf(stdout, "w=%d, r=%d\n", w, r);

	haclog_bytes_buffer_free(bytes_buf);
}

int main()
{
	UNITY_BEGIN();

	// RUN_TEST(test_bytes_buf_case1);

	return UNITY_END();
}
