#include "haclog_os.h"
#include "haclog/haclog_path.h"
#include "haclog/haclog_err.h"

#if HACLOG_PLATFORM_WINDOWS

	#include <windows.h>
	#include <stdio.h>

int haclog_os_process_path(char *path, unsigned int size)
{
	// get module file name in unicode characters
	WCHAR unicode_buf[HACLOG_MAX_PATH] = { 0 };
	DWORD ret = GetModuleFileNameW(NULL, unicode_buf, HACLOG_MAX_PATH);
	if (ret == 0) {
		return HACLOG_ERR_SYS_CALL;
	}

	// convert to utf8
	ret = WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, path, size, NULL,
							  FALSE);
	if (ret == 0) {
		return HACLOG_ERR_SYS_CALL;
	}

	return 0;
}

int haclog_os_curdir(char *path, unsigned int size)
{
	// get current dir in unicode characters
	WCHAR unicode_buf[HACLOG_MAX_PATH] = { 0 };
	DWORD ret = GetCurrentDirectoryW(HACLOG_MAX_PATH, unicode_buf);
	if (ret == 0) {
		return HACLOG_ERR_SYS_CALL;
	}

	// convert to utf8
	ret = WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, path, size, NULL,
							  FALSE);
	if (ret == 0) {
		return HACLOG_ERR_SYS_CALL;
	}

	return 0;
}

int haclog_os_chdir(const char *path)
{
	// convert to unicode characters
	WCHAR unicode_buf[HACLOG_MAX_PATH];
	int ret =
		MultiByteToWideChar(CP_UTF8, 0, path, -1, unicode_buf, HACLOG_MAX_PATH);
	if (ret == 0) {
		return HACLOG_ERR_SYS_CALL;
	}

	return SetCurrentDirectoryW(unicode_buf) ? 0 : HACLOG_ERR_SYS_CALL;
}

static BOOL haclog_windows_create_dir_utf8(const char *path)
{
	// convert to unicode characters
	WCHAR unicode_buf[HACLOG_MAX_PATH];
	int ret =
		MultiByteToWideChar(CP_UTF8, 0, path, -1, unicode_buf, HACLOG_MAX_PATH);
	if (ret == 0) {
		return FALSE;
	}

	return CreateDirectoryW(unicode_buf, NULL);
}

int haclog_os_mkdir(const char *path)
{
	// adapted from: https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
	const size_t len = strlen(path);
	char _path[HACLOG_MAX_PATH];
	char *p;

	if (len == 0) {
		return 0;
	}

	if (len > sizeof(_path) - 1) {
		return HACLOG_ERR_ARGUMENTS;
	}
	strcpy(_path, path);

	/* Iterate the string */
	for (p = _path + 1; *p; p++) {
		if (*p == '/' || *p == '\\') {
			char c = *p;

			/* Temporarily truncate */
			*p = '\0';

			if (strlen(_path) == 2 && _path[1] == ':') {
				// don't need to create windows drive letter
			} else if (!haclog_windows_create_dir_utf8(_path)) {
				DWORD err = GetLastError();
				if (err != ERROR_ALREADY_EXISTS) {
					return HACLOG_ERR_SYS_CALL;
				}
			}

			*p = c;
		}
	}

	if (strlen(_path) == 2 && _path[1] == ':') {
		// don't need to create windows drive letter
	} else if (!haclog_windows_create_dir_utf8(_path)) {
		DWORD err = GetLastError();
		if (err != ERROR_ALREADY_EXISTS) {
			return HACLOG_ERR_SYS_CALL;
		}
	}

	return 0;
}

int haclog_os_remove(const char *path)
{
	// convert to unicode characters
	WCHAR unicode_buf[HACLOG_MAX_PATH];
	int ret =
		MultiByteToWideChar(CP_UTF8, 0, path, -1, unicode_buf, HACLOG_MAX_PATH);
	if (ret == 0) {
		return HACLOG_ERR_SYS_CALL;
	}

	return DeleteFileW(unicode_buf) ? 0 : HACLOG_ERR_SYS_CALL;
}

int haclog_os_rmdir(const char *path)
{
	// convert to unicode characters
	WCHAR unicode_buf[HACLOG_MAX_PATH];
	int ret =
		MultiByteToWideChar(CP_UTF8, 0, path, -1, unicode_buf, HACLOG_MAX_PATH);
	if (ret == 0) {
		return HACLOG_ERR_SYS_CALL;
	}

	return RemoveDirectoryW(unicode_buf) ? 0 : HACLOG_ERR_SYS_CALL;
}

int haclog_os_rename(const char *src, const char *dst)
{
	// convert to unicode characters
	int ret = 0;
	WCHAR unicode_src[HACLOG_MAX_PATH];
	WCHAR unicode_dst[HACLOG_MAX_PATH];
	ret =
		MultiByteToWideChar(CP_UTF8, 0, src, -1, unicode_src, HACLOG_MAX_PATH);
	if (ret == 0) {
		return HACLOG_ERR_SYS_CALL;
	}
	ret =
		MultiByteToWideChar(CP_UTF8, 0, dst, -1, unicode_dst, HACLOG_MAX_PATH);
	if (ret == 0) {
		return HACLOG_ERR_SYS_CALL;
	}

	return _wrename(unicode_src, unicode_dst) == 0 ? 0 : HACLOG_ERR_SYS_CALL;
}

#else

	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
	#include <stdio.h>
	#include <string.h>

	#if HACLOG_PLATFORM_APPLE
		#include <libproc.h>
	#endif

int haclog_os_process_path(char *path, unsigned int size)
{
	#if HACLOG_PLATFORM_APPLE
	memset(path, 0, size);

	pid_t pid = getpid();
	int ret = proc_pidpath(pid, path, size);
	if (ret <= 0) {
		return HACLOG_ERR_SYS_CALL;
	}

	return 0;
	#else
	char sz_tmp[64];
	ssize_t len;

	snprintf(sz_tmp, 63, "/proc/%ld/exe", (long)getpid());
	len = readlink(sz_tmp, path, size - 1);
	if (len >= 0) {
		path[len] = '\0';
		return 0;
	}

	return HACLOG_ERR_SYS_CALL;
	#endif
}

int haclog_os_curdir(char *path, unsigned int size)
{
	return getcwd(path, (size_t)size) != NULL ? 0 : HACLOG_ERR_SYS_CALL;
}

int haclog_os_chdir(const char *path)
{
	return chdir(path) == 0 ? 0 : HACLOG_ERR_SYS_CALL;
}

int haclog_os_mkdir(const char *path)
{
	// adapted from: https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
	const size_t len = strlen(path);
	char _path[HACLOG_MAX_PATH];
	char *p;

	if (len == 0) {
		return 0;
	}

	if (len > sizeof(_path) - 1) {
		return HACLOG_ERR_ARGUMENTS;
	}
	strcpy(_path, path);

	/* Iterate the string */
	for (p = _path + 1; *p; p++) {
		if (*p == '/') {
			/* Temporarily truncate */
			*p = '\0';

			if (mkdir(_path, S_IRWXU) != 0) {
				if (errno != EEXIST) {
					return HACLOG_ERR_SYS_CALL;
				}
			}

			*p = '/';
		}
	}

	if (mkdir(_path, S_IRWXU) != 0) {
		if (errno != EEXIST) {
			return HACLOG_ERR_SYS_CALL;
		}
	}

	return 0;
}

int haclog_os_remove(const char *path)
{
	return remove(path) == 0 ? 0 : HACLOG_ERR_SYS_CALL;
}

int haclog_os_rmdir(const char *path)
{
	return rmdir(path) == 0 ? 0 : HACLOG_ERR_SYS_CALL;
}

int haclog_os_rename(const char *src, const char *dst)
{
	return rename(src, dst) == 0 ? 0 : HACLOG_ERR_SYS_CALL;
}

#endif

FILE *haclog_os_fopen(const char *filepath, const char *mode)
{
	int ret = 0;
	const char *abs_filepath = NULL;
	char tmp_path[HACLOG_MAX_PATH];
	if (haclog_path_isabs(filepath)) {
		abs_filepath = filepath;
	} else {
		char cur_path[HACLOG_MAX_PATH];
		ret = haclog_os_curdir(cur_path, sizeof(cur_path));
		if (ret != 0) {
			return NULL;
		}

		ret = haclog_path_join(cur_path, filepath, tmp_path, sizeof(tmp_path));
		if (ret != 0) {
			return NULL;
		}

		abs_filepath = tmp_path;
	}

	char file_dir[HACLOG_MAX_PATH];
	ret = haclog_path_dirname(abs_filepath, file_dir, sizeof(file_dir));
	if (ret != 0) {
		return NULL;
	}

	if (!haclog_path_exists(file_dir)) {
		ret = haclog_os_mkdir(file_dir);
		if (ret != 0) {
			return NULL;
		}
	}

#if HACLOG_PLATFORM_WINDOWS
	// convert to unicode characters
	WCHAR unicode_abs_filepath[HACLOG_MAX_PATH];
	ret = MultiByteToWideChar(CP_UTF8, 0, abs_filepath, -1,
							  unicode_abs_filepath, HACLOG_MAX_PATH);
	if (ret == 0) {
		return NULL;
	}

	WCHAR unicode_mode[16];
	ret = MultiByteToWideChar(CP_UTF8, 0, mode, -1, unicode_mode, 16);
	if (ret == 0) {
		return NULL;
	}

	return _wfopen(unicode_abs_filepath, unicode_mode);
#else
	return fopen(abs_filepath, mode);
#endif
}
