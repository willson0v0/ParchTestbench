#ifndef SYSCALL_H
# define SYSCALL_H

int tb_write(int, char*, int, int);
int tb_read(int, char*, int, int);
int tb_open(char*, int);
int tb_openat(int, char*, int);
int tb_fork();
int tb_exec(char*, char* []);
__attribute__((noreturn)) void tb_exit(int);

#endif