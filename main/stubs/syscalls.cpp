#include <errno.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef errno
extern int errno;

// 实现 _write 系统调用（最小实现）
int _write(int file, char *ptr, int len)
{
	(void)file;
	(void)ptr;
	return -1;
}

// 其他可能需要的基本系统调用
void _exit(int status)
{
	while (1);
}

int _close(int file)
{
	return -1;
}

int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

int _read(int file, char *ptr, int len)
{
	return 0;
}

// 实现 _getpid (通常返回1表示单进程环境)
int _getpid(void) {
    return 1;
}

// 实现 _kill (最小实现)
int _kill(int pid, int sig) {
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

#ifdef __cplusplus
}
#endif

