#include "haclog_console_handler.h"
#include "haclog/haclog_vsprintf.h"
#include <stdio.h>
#include <string.h>

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

static int haclog_console_handler_write(struct haclog_handler *base_handler,
										haclog_meta_info_t *meta,
										const char *msg, int msglen)
{
	FILE *fp = stdout;
	if (meta->loc->level >= HACLOG_LEVEL_WARNING) {
		fp = stderr;
	}

	haclog_console_handler_t *handler =
		(haclog_console_handler_t *)base_handler;

	int ret = 0;
	if (handler->enable_color && meta->loc->level >= HACLOG_LEVEL_WARNING) {
#if MUGGLE_PLATFORM_WINDOWS
		const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

		// get the current text color
		CONSOLE_SCREEN_BUFFER_INFO sb_info;
		GetConsoleScreenBufferInfo(stdout_handle, &sb_info);
		const WORD old_sb_attrs = sb_info.wAttributes;

		// change text color
		fflush(fp);

		if (msg->level >= MUGGLE_LEVEL_ERROR) {
			SetConsoleTextAttribute(stdout_handle,
									FOREGROUND_RED | FOREGROUND_INTENSITY);
		} else if (msg->level >= MUGGLE_LEVEL_WARNING) {
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED |
													   FOREGROUND_GREEN |
													   FOREGROUND_INTENSITY);
		}

		ret = (int)fwrite(msg, 1, msglen, fp);

		fflush(fp);

		// restores text color
		SetConsoleTextAttribute(stdout_handle, old_sb_attrs);
#else
		if (meta->loc->level >= HACLOG_LEVEL_ERROR) {
			fwrite(UNIX_TERMINAL_COLOR_RED, 1, strlen(UNIX_TERMINAL_COLOR_RED),
				   fp);
		} else {
			fwrite(UNIX_TERMINAL_COLOR_YEL, 1, strlen(UNIX_TERMINAL_COLOR_YEL),
				   fp);
		}
		ret = (int)fwrite(msg, 1, msglen, fp);
		fwrite(UNIX_TERMINAL_COLOR_RST, 1, strlen(UNIX_TERMINAL_COLOR_RST), fp);

		fflush(fp);
#endif
	} else {
		ret = (int)fwrite(msg, 1, msglen, fp);
		fflush(fp);
	}

	return ret;
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

	handler->base.fmt = haclog_handler_default_fmt;
	handler->base.write = haclog_console_handler_write;
	handler->base.destroy = haclog_console_handler_destroy;
	handler->base.level = HACLOG_LEVEL_INFO;

	return 0;
}
