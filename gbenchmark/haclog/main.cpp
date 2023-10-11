#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include "benchmark/benchmark.h"
#include "haclog/haclog.h"

#define ITER_COUNT 6000
#define REPEAT_COUNT 5

#define MIN_TIME 3.0

struct LogMsg {
	uint64_t u64;
	uint32_t u32;
	int64_t i64;
	int32_t i32;
	char s[128];
};

void GenLogMsgArray(uint32_t cnt, std::vector<LogMsg> &log_msgs)
{
	log_msgs.clear();

	srand(time(NULL));
	for (uint32_t i = 0; i < cnt; i++) {
		LogMsg msg = {};
		msg.u64 = (uint64_t)i;
		msg.u32 = (uint32_t)i;
		msg.i64 = (int64_t)i;
		msg.i32 = (int32_t)i;
		for (int i = 0; i < (int)sizeof(msg.s) - 1; i++) {
			msg.s[i] = (rand() % 26) + 'a';
		}
		log_msgs.push_back(msg);
	}
}

std::once_flag init_flag;

class ConstIterFixture : public benchmark::Fixture {
public:
	ConstIterFixture()
	{
		GenLogMsgArray(10000, log_msgs);
	}

	void SetUp(const benchmark::State &)
	{
		std::call_once(init_flag, []() {
			static haclog_file_handler_t file_handler = {};
			if (haclog_file_handler_init(
					&file_handler, "logs/haclog_gbenchmark_const_iter.log",
					"w") != 0) {
				exit(EXIT_FAILURE);
			}
			haclog_handler_set_level((haclog_handler_t *)&file_handler,
									 HACLOG_LEVEL_DEBUG);
			haclog_context_add_handler((haclog_handler_t *)&file_handler);

			haclog_context_set_bytes_buf_size(2 * 1024 * 1024);

			haclog_backend_run();
			HACLOG_INFO("init success");
		});

		haclog_thread_context_init();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	void TearDown(const benchmark::State &)
	{
		haclog_thread_context_cleanup();
	}

public:
	std::vector<LogMsg> log_msgs;
};

BENCHMARK_DEFINE_F(ConstIterFixture, run)(benchmark::State &state)
{
	static thread_local int idx = 0;
	const size_t cnt = log_msgs.size();
	for (auto _ : state) {
		idx = (idx + 1) % cnt;
		LogMsg &msg = log_msgs[idx];
		HACLOG_INFO("u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
					(unsigned long long)msg.u64, (long long)msg.i64,
					(unsigned long)msg.u32, (long)msg.i32, msg.s);
	}
}

// min time
BENCHMARK_REGISTER_F(ConstIterFixture, run)->Threads(1)->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(ConstIterFixture, run)->Threads(2)->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(ConstIterFixture, run)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1)
	->MinTime(MIN_TIME);

// iteration * repeat
BENCHMARK_REGISTER_F(ConstIterFixture, run)
	->Threads(1)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(ConstIterFixture, run)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(ConstIterFixture, run)
	->Threads(std::thread::hardware_concurrency())
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(ConstIterFixture, run)
	->Threads(std::thread::hardware_concurrency() * 2)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);

BENCHMARK_MAIN();
