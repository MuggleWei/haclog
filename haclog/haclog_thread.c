#include "haclog_thread.h"
#include "haclog/haclog_err.h"
#if HACLOG_PLATFORM_WINDOWS
	#include <process.h>
#else
	#include <unistd.h>
	#include <sys/syscall.h>
	#include <sched.h>
#endif

#if HACLOG_PLATFORM_WINDOWS

int haclog_thread_create(haclog_thread_t *thread, haclog_thread_routine routine,
						 void *args)
{
	thread->handle = (HANDLE)_beginthreadex(
		NULL, 0, routine, args, 0, &thread->id);
	if (thread->handle == NULL)
	{
		return HACLOG_ERR_SYS_CALL;
	}

	return 0;
}

int haclog_thread_join(haclog_thread_t *thread)
{
	if (WaitForSingleObject(thread->handle, INFINITE) == WAIT_FAILED)
	{
		return HACLOG_ERR_SYS_CALL;
	}

	if (!CloseHandle(thread->handle))
	{
		return HACLOG_ERR_SYS_CALL;
	}

	return 0;
}

int haclog_thread_detach(haclog_thread_t *thread)
{
	return 0;
}

haclog_thread_id haclog_thread_readable_id()
{
	return GetCurrentThreadId();
}

int haclog_thread_hardware_concurrency()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return (int)sysinfo.dwNumberOfProcessors;
}

void haclog_thread_yield()
{
	SwitchToThread();
}

#else

int haclog_thread_create(haclog_thread_t *thread, haclog_thread_routine routine,
						 void *args)
{
	return pthread_create(&thread->th, NULL, routine, args) == 0 ?
			   0 :
			   HACLOG_ERR_SYS_CALL;
}

int haclog_thread_join(haclog_thread_t *thread)
{
	return pthread_join(thread->th, NULL) == 0 ? 0 : HACLOG_ERR_SYS_CALL;
}

int haclog_thread_detach(haclog_thread_t *thread)
{
	return pthread_detach(thread->th) == 0 ? 0 : HACLOG_ERR_SYS_CALL;
}

haclog_thread_id haclog_thread_readable_id()
{
	#if HACLOG_PLATFORM_LINUX
	return syscall(SYS_gettid);
	#elif HACLOG_PLATFORM_APPLE
	uint64_t tid;
	pthread_threadid_np(NULL, &tid);
	return tid;
	#elif HACLOG_PLATFORM_FREEBSD
	long tid;
	thr_self(&tid);
	return (int)tid;
	#else
	return syscall(SYS_gettid);
	#endif
}

int haclog_thread_hardware_concurrency()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void haclog_thread_yield()
{
	sched_yield();
}

#endif
