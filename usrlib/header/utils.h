#ifndef UTILS_H
#define UTILS_H

#include "type.h"

void tb_memcpy(u8*, u8*, u64);
void tb_strcpy(char*, char*);
u64 tb_strlen(char*);
void* tb_memset(u8*, u8, u64);
void* tb_malloc(u64);
void tb_free(void*);

#endif