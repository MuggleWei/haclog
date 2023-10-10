#include "haclog_console_handler.h"
#include "haclog/haclog_vsprintf.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/*
 * unix terminal colors
 *
 * output colored text, need print or echo -e the control characters for 
 * required color, then output text, and then reset the output back to default.
 *
 * "\x1B[${code}m" or "\033[${code}m", ${code} represent color code
 *
 * reset codes: 0
 *
 * color codes:
 * | color        | foreground | background |
 * | ----         | ----       | ----       |
 * | default      | 39         | 49         |
 * | black        | 30         | 40         |
 * | dark red     | 31         | 41         |
 * | dark green   | 32         | 42         |
 * | dark yellow  | 33         | 43         |
 * | dark blue    | 34         | 44         |
 * | dark magenta | 35         | 45         |
 * | dark cyan    | 36         | 46         |
 * | light gray   | 37         | 47         |
 * | dark gray    | 90         | 100        |
 * | red          | 91         | 101        |
 * | green        | 92         | 102        |
 * | orange       | 93         | 103        |
 * | blue         | 94         | 104        |
 * | magenta      | 95         | 105        |
 * | cyan         | 96         | 106        |
 * | white        | 97         | 107        |
 *
 * format for foreground color is:
 * "\x1B[" + "<0 or 1, meaning nromal or bold>;" + "<color code>" + "m"
 *
 * format for background:
 * "\x1B[" + "<color code>" + "m"
 * */

#define UNIX_TERMINAL_COLOR(code) "\x1B[" #code "m"

// terminal color for *nix
#define UNIX_TERMINAL_COLOR_RST UNIX_TERMINAL_COLOR(0)
#define UNIX_TERMINAL_COLOR_RED UNIX_TERMINAL_COLOR(31)
#define UNIX_TERMINAL_COLOR_GRN UNIX_TERMINAL_COLOR(32)
#define UNIX_TERMINAL_COLOR_YEL UNIX_TERMINAL_COLOR(33)
#define UNIX_TERMINAL_COLOR_BLU UNIX_TERMINAL_COLOR(34)
#define UNIX_TERMINAL_COLOR_MAG UNIX_TERMINAL_COLOR(35)
#define UNIX_TERMINAL_COLOR_CYN UNIX_TERMINAL_COLOR(36)
#define UNIX_TERMINAL_COLOR_WHT UNIX_TERMINAL_COLOR(37)

static int haclog_console_handler_before_write(haclog_handler_t *base_handler,
											   haclog_meta_info_t *meta)
{
	haclog_console_handler_t *handler =
		(haclog_console_handler_t *)base_handler;

	if (handler->enable_color && meta->loc->level >= HACLOG_LEVEL_WARNING) {
		handler->fp = stderr;

#if HACLOG_PLATFORM_WINDOWS
		const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

		// get the current text color
		CONSOLE_SCREEN_BUFFER_INFO sb_info;
		GetConsoleScreenBufferInfo(stdout_handle, &sb_info);
		handler->sb_attrs = sb_info.wAttributes;

		if (meta->loc->level >= HACLOG_LEVEL_ERROR) {
			SetConsoleTextAttribute(stdout_handle,
									FOREGROUND_RED | FOREGROUND_INTENSITY);
		} else if (meta->loc->level >= HACLOG_LEVEL_WARNING) {
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED |
													   FOREGROUND_GREEN |
													   FOREGROUND_INTENSITY);
		}
#else
		if (meta->loc->level >= HACLOG_LEVEL_ERROR) {
			fwrite(UNIX_TERMINAL_COLOR_RED, 1, strlen(UNIX_TERMINAL_COLOR_RED),
				   handler->fp);
		} else {
			fwrite(UNIX_TERMINAL_COLOR_YEL, 1, strlen(UNIX_TERMINAL_COLOR_YEL),
				   handler->fp);
		}
#endif
	} else {
		handler->fp = stdout;
	}

	return 0;
}

static int haclog_console_handler_after_write(haclog_handler_t *base_handler,
											  haclog_meta_info_t *meta)
{
	haclog_console_handler_t *handler =
		(haclog_console_handler_t *)base_handler;

	fwrite("\n", 1, 1, handler->fp);

	if (handler->enable_color && meta->loc->level >= HACLOG_LEVEL_WARNING) {
#if HACLOG_PLATFORM_WINDOWS
		const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(stdout_handle, handler->sb_attrs);
#else
		fwrite(UNIX_TERMINAL_COLOR_RST, 1, strlen(UNIX_TERMINAL_COLOR_RST),
			   stderr);
#endif
	}

	fflush(handler->fp);

	return 0;
}

static int haclog_console_handler_write(haclog_handler_t *base_handler,
										const char *msg, int msglen)
{
	haclog_console_handler_t *handler =
		(haclog_console_handler_t *)base_handler;
	return (int)fwrite(msg, 1, msglen, handler->fp);
}

static int haclog_console_handler_writev(haclog_handler_t *base_handler,
										 const char *fmt_str, ...)
{
	haclog_console_handler_t *handler =
		(haclog_console_handler_t *)base_handler;

	va_list args;
	va_start(args, fmt_str);
	int n = vfprintf(handler->fp, fmt_str, args);
	va_end(args);

	return n;
}

static void haclog_console_handler_destroy(struct haclog_handler *handler)
{
	HACLOG_UNUSED(handler);
}

int haclog_console_handler_init(haclog_console_handler_t *handler,
								int enable_color)
{
	memset(handler, 0, sizeof(*handler));
	handler->enable_color = enable_color;

	handler->base.before_write = haclog_console_handler_before_write;
	handler->base.write_meta = haclog_handler_default_write_meta;
	handler->base.write = haclog_console_handler_write;
	handler->base.writev = haclog_console_handler_writev;
	handler->base.after_write = haclog_console_handler_after_write;
	handler->base.destroy = haclog_console_handler_destroy;
	handler->base.level = HACLOG_LEVEL_INFO;

	return 0;
}
