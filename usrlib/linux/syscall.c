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
#include <sys/time.h>
#include <string.h>

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

void* tb_mmap(void* tgt, u64 length, u64 prot, u64 flag, FileDescriptor fd, u64 offset) 
{
	u64 linux_prot = 0;
	if(prot & (1 << 0)) {
		linux_prot |= PROT_READ;
	}
	if(prot & (1 << 1)) {
		linux_prot |= PROT_WRITE;
	}
	if(prot & (1 << 2)) {
		linux_prot |= PROT_EXEC;
	}
	u64 linux_flag = 0;
	if (flag & 0x80) {
		linux_flag |= MAP_SHARED;
	} else {
		linux_flag |= MAP_PRIVATE;
	}
	void* res = mmap(NULL, length,  linux_prot, MAP_SHARED, fd, 0);
	if (res == MAP_FAILED) {
		printf("err %s\n", strerror(errno));
	}
	return res;
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
	i64 res = remove(path);
	if(res < 0) {
		printf("err %s\n", strerror(errno));
	}
	return res;
}

u64 tb_mkdir(void* path, u64 perm) {
	u64 res = mkdir(path, perm);
	if ((i64)res < 0) {
		printf("err %s\n", strerror(errno));
	}
	return res;
}

u64 tb_seek(FileDescriptor fd, u64 offset) {
	return lseek(fd, offset, SEEK_SET);
}

u64 tb_munmap(void* ptr, u64 length) {
	return munmap(ptr, length);
}

u64 tb_time() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    u64 milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return milliseconds;
}