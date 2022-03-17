#ifndef SYSCALL_H
# define SYSCALL_H

#include "type.h"

u64 tb_write(FileDescriptor, char*, u64);
u64 tb_read(FileDescriptor, char*, u64);
FileDescriptor tb_open(char*, int);
FileDescriptor tb_openat(FileDescriptor, char*, int);
u64 tb_close(FileDescriptor);
FileDescriptor tb_dup(FileDescriptor);
u64 tb_fork();
u64 tb_exec(char*, char* []);
__attribute__((noreturn)) void tb_exit(u64);
void* tb_mmap(FileDescriptor, u64);
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

#endif