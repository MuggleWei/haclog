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
	ret = WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, path, size - 1, NULL,
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
	ret = WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, path, size - 1, NULL,
							  FALSE);
	if (ret == 0) {
		return HACLOG_ERR_SYS_CALL;
	}

	return 0;
}

int haclog_os_chdir(const char *path)
{
	return SetCurrentDirectoryA(path) ? 0 : HACLOG_ERR_SYS_CALL;
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
			} else if (!CreateDirectoryA(_path, NULL)) {
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
	} else if (!CreateDirectoryA(_path, NULL)) {
		DWORD err = GetLastError();
		if (err != ERROR_ALREADY_EXISTS) {
			return HACLOG_ERR_SYS_CALL;
		}
	}

	return 0;
}

int haclog_os_remove(const char *path)
{
	return DeleteFileA(path) ? 0 : HACLOG_ERR_SYS_CALL;
}

int haclog_os_rmdir(const char *path)
{
	return RemoveDirectoryA(path) ? 0 : HACLOG_ERR_SYS_CALL;
}

int haclog_os_rename(const char *src, const char *dst)
{
	return rename(src, dst) == 0 ? 0 : HACLOG_ERR_SYS_CALL;
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
