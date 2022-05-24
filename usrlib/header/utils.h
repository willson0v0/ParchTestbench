#ifndef UTILS_H
#define UTILS_H

#include "type.h"

#define TB_ASSERT(content) do_assert(content, #content)
#define TB_NON_NEG(content) do_assert((i64)(content) >= 0, #content)
#define TB_NEG(content) do_assert((i64)(content) < 0, #content)

void tb_memcpy(u8*, u8*, u64);
void tb_strcpy(char*, char*);
u64 tb_strlen(char*);
void tb_memset(u8*, u8, u64);
void* tb_malloc(u64);
void tb_free(void*);
u64 tb_strcmp(char*, char*);
void do_assert(int, char*);
void tb_msync(void*, u64);

#endif