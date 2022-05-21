#include <../header/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/unistd.h>
#include <errno.h>

__attribute__((noreturn)) void todo(char* msg) {
	printf("%s", msg);
	exit(-1);
}


u64 tb_write(FileDescriptor fd, char* buf, u64 length) {
	return write(fd, buf, length);
}

u64 tb_read(FileDescriptor fd, char* buf, u64 length) {
	return read(fd, buf, length);
}

FileDescriptor tb_open(char* path, int flags) {
	int l_flag = 0;
	if (flags & O_READ) {
		if (flags & O_WRITE) {
			l_flag |= LINUX_RDWR;
		} else {
			l_flag |= LINUX_RDONLY;
		}
	}
	if (flags & O_CREATE) {
		l_flag |= LINUX_CREAT;
	}
	if (flags & O_NO_FOLLOW) {
		l_flag |= LINUX_NOFOLLOW;
	}
	return open(path, l_flag);
}

FileDescriptor tb_openat(FileDescriptor dirfd, char* path, int flags) {
	int l_flag = 0;
	if (flags & O_READ) {
		if (flags & O_WRITE) {
			l_flag |= LINUX_RDWR;
		} else {
			l_flag |= LINUX_RDONLY;
		}
	}
	if (flags & O_CREATE) {
		l_flag |= LINUX_CREAT;
	}
	if (flags & O_NO_FOLLOW) {
		l_flag |= LINUX_NOFOLLOW;
	}
	return openat(dirfd, path, l_flag);
}

u64 tb_close(FileDescriptor fd) {
	return close(fd);
}

FileDescriptor tb_dup(FileDescriptor fd) {
	return dup(fd);
}

u64 tb_fork() {
	return fork();
}

u64 tb_exec(char* file, char* argv[]) {
	return fork();
}

__attribute__((noreturn)) void tb_exit(u64 code) {
	exit(code);
}

void* tb_mmap(FileDescriptor fd, u64 flag) {
	struct stat st;
	fstat(fd, &st);
	u64 linux_flag = 0;
	if(flag & (1 << 1)) {
		linux_flag |= PROT_READ;
	}
	if(flag & (1 << 2)) {
		linux_flag |= PROT_WRITE;
	}
	if(flag & (1 << 3)) {
		linux_flag |= PROT_EXEC;
	}
	mmap(0, st.st_size, linux_flag, MAP_SHARED, fd, 0);
}

PID tb_waitpid(PID to_wait, u64* ret) {
	return waitpid(to_wait, ret, 0);
}


u64 tb_pipe(FileDescriptor fds[2]) {
	int pipefd[2];
	int res = pipe(pipefd);
	fds[0] = pipefd[0];
	fds[1] = pipefd[1];
	return res;
}

u64 tb_sysstat(SysStat* stat_ptr) {
	// TODO: read and parse /proc/meminfo for this
	SysStat s = {
		-1, -1, -1, -1
	};
	*stat_ptr = s;
	return 0;
}

u64 tb_ioctl(FileDescriptor _fd, u64 op, void* _a, u64 _b, void* _c, u64 _d) {
	-EINVAL;
}

u64 tb_delete(void* path) {
	return unlink(path);
}

u64 tb_mkdir(void* path, u64 perm) {
	return mkdir(path, perm);
}

u64 tb_seek(FileDescriptor fd, u64 offset) {
	return lseek(fd, offset, SEEK_SET);
}
