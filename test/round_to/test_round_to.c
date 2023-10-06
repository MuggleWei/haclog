#include "haclog/haclog.h"
#include "unity.h"

void setUp()
{
}

void tearDown()
{
}

void test_round_to()
{
	unsigned int round_to_arr[] = { 2, 4, 8, 16, 32 };

	for (unsigned int i = 0; i < sizeof(round_to_arr) / sizeof(round_to_arr[0]);
		 i++) {
		unsigned int k = round_to_arr[i];
		for (unsigned int j = 1; j < 1024; j++) {
			unsigned int v = HACLOG_ROUND_TO_2POWX(j, k);
			if (j % k == 0) {
				TEST_ASSERT_EQUAL(j, v);
			} else {
				TEST_ASSERT_TRUE(v > j);
				TEST_ASSERT_EQUAL(v % k, 0);
			}
		}
	}
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_round_to);

	return UNITY_END();
}
