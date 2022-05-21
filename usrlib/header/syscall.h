#ifndef SYSCALL_H
# define SYSCALL_H

#include "type.h"

#define LINUX_RDONLY	00000000
#define LINUX_WRONLY	00000001
#define LINUX_RDWR		00000002
#define LINUX_CREAT		00000100	/* not fcntl */
#define LINUX_NOFOLLOW	00400000	/* don't follow links */

#define O_READ      0b0000001
#define O_WRITE     0b0000010
#define O_CREATE    0b0000100
#define O_NO_FOLLOW 0b0100000	/* don't follow links */

u64 tb_write(FileDescriptor, char*, u64);
u64 tb_read(FileDescriptor, char*, u64);
FileDescriptor tb_open(char*, int);
FileDescriptor tb_openat(FileDescriptor, char*, int);
u64 tb_close(FileDescriptor);
FileDescriptor tb_dup(FileDescriptor);
u64 tb_fork();
u64 tb_exec(char*, char* []);
__attribute__((noreturn)) void tb_exit(u64);
void* tb_mmap(void*, u64, u64, u64, FileDescriptor, u64);
u64 tb_munmap(void*, u64);
PID tb_waitpid(PID, u64*);
u64 tb_signal(PID, SignalNum); 
u64 tb_sigaction(PID, SigHandler); 
u64 tb_sigreturn(); 
u64 tb_getcwd(char*, u64);
u64 tb_chdir(char*);
char* tb_sbrk(i64);
u64 tb_getdents(FileDescriptor, Dirent*, u64);
u64 tb_pipe(FileDescriptor fds[2]);
u64 tb_sysstat(SysStat* stat_ptr);
u64 tb_ioctl(FileDescriptor, u64, void*, u64, void*, u64);
u64 tb_delete(void*);
u64 tb_mkdir(void*, u64);
u64 tb_seek(FileDescriptor, u64);

#endif