#include <syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int tb_write(int fd, char* buf, int length) {
	return write(fd, buf, length);
}

int tb_read(int fd, char* buf, int length) {
	return read(fd, buf, length);
}

int tb_open(char* path, int flags) {
	return open(path, flags);
}

int tb_openat(int dirfd, char* path, int flags) {
	return openat(dirfd, path, flags);
}

int tb_fork() {
	return fork();
}

int tb_exec(char* file, char* argv[]) {
	return fork();
}

__attribute__((noreturn)) void tb_exit(int code) {
	exit(code);
}