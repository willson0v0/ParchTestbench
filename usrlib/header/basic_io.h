#ifndef BASIC_IO_H
#define BASIC_IO_H

#include <stdarg.h>
#include <type.h>

int tb_printf(char* format, ...);
char tb_getc();
u64 tb_getline(char *, u64);

#endif