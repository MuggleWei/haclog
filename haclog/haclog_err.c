#include "haclog_err.h"
#include "haclog/haclog_thread.h"

static haclog_thread_local unsigned int s_haclog_err = 0;

unsigned int haclog_last_error()
{
	return s_haclog_err;
}

void haclog_set_error(unsigned int err)
{
	s_haclog_err = err;
}
