#include "haclog_path.h"
#include "haclog/haclog_err.h"
#include <string.h>

#if HACLOG_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include "haclog/haclog_err.h"
#include "haclog/haclog_os.h"

static int haclog_str_startswith(const char *str, const char *prefix)
{
	if (str == NULL || prefix == NULL) {
		return 0;
	}

	size_t str_len = strlen(str);
	size_t prefix_len = strlen(prefix);

	if (str_len < prefix_len) {
		return 0;
	}

	for (size_t i = 0; i < prefix_len; ++i) {
		if (str[i] != prefix[i]) {
			return 0;
		}
	}

	if (str_len != 0 && prefix_len == 0) {
		return 0;
	}

	return 1;
}

static int haclog_str_endswith(const char *str, const char *suffix)
{
	if (str == NULL || suffix == NULL) {
		return 0;
	}

	size_t str_len = strlen(str);
	size_t suffix_len = strlen(suffix);

	if (str_len < suffix_len) {
		return 0;
	}

	for (size_t i = 0; i < suffix_len; ++i) {
		if (str[str_len - 1 - i] != suffix[suffix_len - 1 - i]) {
			return 0;
		}
	}

	if (str_len != 0 && suffix_len == 0) {
		return 0;
	}

	return 1;
}

int haclog_path_abspath(const char *path, char *ret, unsigned int size)
{
	if (size <= 1) {
		return HACLOG_ERR_ARGUMENTS;
	}

	if (haclog_path_isabs(path)) {
		if (strlen(path) > size - 1) {
			return HACLOG_ERR_ARGUMENTS;
		}

		strncpy(ret, path, size - 1);
		return 0;
	}

	char cur_dir[HACLOG_MAX_PATH];
	int r;
	r = haclog_os_curdir(cur_dir, sizeof(cur_dir));
	if (r != 0) {
		return r;
	}

	char full_path[HACLOG_MAX_PATH];
	r = haclog_path_join(cur_dir, path, full_path, sizeof(full_path));
	if (r != 0) {
		return r;
	}

	r = haclog_path_normpath(full_path, ret, size);
	if (r != 0) {
		return r;
	}

	return 0;
}

int haclog_path_basename(const char *path, char *ret, unsigned int size)
{
	if (size <= 1) {
		return HACLOG_ERR_ARGUMENTS;
	}

	int total_len = (int)strlen(path);
	if (total_len <= 0) {
		return HACLOG_ERR_ARGUMENTS;
	}

	int pos = total_len - 1;
	while (pos >= 0) {
		if (path[pos] == '/' || path[pos] == '\\') {
			break;
		}
		--pos;
	}

	if (pos < 0) {
		if ((unsigned int)total_len > size - 1) {
			return HACLOG_ERR_ARGUMENTS;
		}

		strncpy(ret, path, size - 1);
		return 0;
	}

	int len = total_len - 1 - pos;
	if (len <= 0) {
		return HACLOG_ERR_ARGUMENTS;
	}

	if ((unsigned int)len >= size) {
		len = size - 1;
	}
	memcpy(ret, path + pos + 1, len);
	ret[len] = '\0';

	return 0;
}

int haclog_path_dirname(const char *path, char *ret, unsigned int size)
{
	if (size <= 1) {
		return HACLOG_ERR_ARGUMENTS;
	}

	int total_len = (int)strlen(path);
	if (total_len <= 0) {
		return HACLOG_ERR_ARGUMENTS;
	}

	int pos = total_len - 1;
	while (pos >= 0) {
		if (path[pos] == '/' || path[pos] == '\\') {
			break;
		}
		--pos;
	}

	if (pos < 0) {
		return HACLOG_ERR_ARGUMENTS;
	}

	// handle path = "/"
	if (pos == 0) {
		pos = 1;
	}

	// handle path = "c:/"
	if (pos - 1 > 0 && path[pos - 1] == ':') {
		pos = pos + 1;
	}

	if (pos >= (int)size) {
		return HACLOG_ERR_ARGUMENTS;
	}

	memcpy(ret, path, pos);
	ret[pos] = '\0';

	return 0;
}

int haclog_path_isabs(const char *path)
{
	size_t len = strlen(path);

	if (len > 1 && path[0] == '/') {
		return 1;
	}

	if (len > 2 &&
		((path[0] >= 'a' && path[0] <= 'z') ||
		 (path[0] >= 'A' && path[0] <= 'Z')) &&
		path[1] == ':' && (path[2] == '/' || path[2] == '\\')) {
		return 1;
	}

	return 0;
}

int haclog_path_exists(const char *path)
{
#if HACLOG_PLATFORM_WINDOWS
	// convert to utf16 characters
	WCHAR unicode_buf[HACLOG_MAX_PATH] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, path, -1, unicode_buf, HACLOG_MAX_PATH);

	// get file attributes
	DWORD attr = GetFileAttributesW(unicode_buf);
	// if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
	if (attr == INVALID_FILE_ATTRIBUTES) {
		return 0;
	}

	return 1;
#else
	if (access(path, F_OK) != -1)
		return 1;

	return 0;
#endif
}

int haclog_path_join(const char *path1, const char *path2, char *ret,
					 unsigned int size)
{
	unsigned int max_len = size - 1;
	if (max_len <= 0) {
		return HACLOG_ERR_ARGUMENTS;
	}

	int len_path1 = (int)strlen(path1);
	int len_path2 = (int)strlen(path2);
	if (len_path1 <= 0 || len_path2 <= 0) {
		return HACLOG_ERR_ARGUMENTS;
	}

	if ((unsigned int)len_path1 > max_len) {
		return HACLOG_ERR_ARGUMENTS;
	}

	strncpy(ret, path1, max_len);

	if (*path2 == '\0') {
		ret[len_path1] = '\0';
		return 0;
	}

	int pos = len_path1;
	if (!haclog_str_endswith(ret, "/") && !haclog_str_endswith(ret, "\\")) {
		if ((unsigned int)pos >= size) {
			return HACLOG_ERR_ARGUMENTS;
		}

		ret[pos] = '/';
		pos++;
		len_path1 += 1;
	}

	const char *p = path2;
	if (*p == '/') {
		if (len_path2 == 1) {
			return HACLOG_ERR_ARGUMENTS;
		}
		p += 1;

		len_path2 -= 1;
	}

	if ((unsigned int)(len_path1 + len_path2) > max_len) {
		return HACLOG_ERR_ARGUMENTS;
	}

	strncpy(ret + pos, p, len_path2);
	ret[len_path1 + len_path2] = '\0';

	return 0;
}

int haclog_path_normpath(const char *path, char *ret, unsigned int size)
{
	int pos = 0;
	const char *cursor = path;
	int len = (int)strlen(path);

	// cal max len
	if ((unsigned int)len >= size) {
		return HACLOG_ERR_ARGUMENTS;
	}

	if (!haclog_path_isabs(path)) {
		if (haclog_str_startswith(path, "./") ||
			haclog_str_startswith(path, ".\\")) {
			cursor += 2;
		}
	}

	while (*cursor != '\0') {
		if (*cursor == '.' && *(cursor + 1) == '.') {
			if (*(cursor + 2) != '\0' && *(cursor + 2) != '/' &&
				*(cursor + 2) != '\\') {
				return HACLOG_ERR_ARGUMENTS;
			}

			cursor += 2;
			if (pos == 0) {
				ret[pos++] = '.';
				ret[pos++] = '.';
				ret[pos++] = *cursor;
			} else {
				if (pos >= 3 && ret[pos - 3] == '.' && ret[pos - 2] == '.' &&
					(ret[pos - 1] == '/' || ret[pos - 1] == '\\')) {
					ret[pos++] = '.';
					ret[pos++] = '.';
					ret[pos++] = *cursor;
				} else {
					if (ret[pos - 1] != '/' && ret[pos - 1] != '\\') {
						return HACLOG_ERR_ARGUMENTS;
					}

					pos -= 2;
					if (pos < 0) {
						return HACLOG_ERR_ARGUMENTS;
					}

					int i = pos;
					while (i >= 0) {
						if (ret[i] == '/' || ret[i] == '\\') {
							break;
						}
						--i;
					}

					pos = i + 1;
				}
			}

		} else {
			ret[pos++] = *cursor;
		}

		if (*cursor != '\0') {
			++cursor;
		}
	}

	if (pos == 0) {
		if (size <= 2) {
			return HACLOG_ERR_ARGUMENTS;
		}

		ret[pos++] = '.';
		ret[pos++] = '/';
	}
	ret[pos] = '\0';

	return 0;
}
