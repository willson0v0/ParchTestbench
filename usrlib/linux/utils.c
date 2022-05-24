#include "../header/utils.h"
#include "string.h"
#include "malloc.h"
#include "sys/mman.h"
#include "errno.h"

void tb_memcpy(u8* src, u8* dst, u64 len) {
    memcpy(dst, src, len);
}

void tb_strcpy(char* src, char* dst) {
    strcpy(dst, src);
}
    
u64 tb_strlen(char* str) {
    return strlen(str);
}

void tb_memset(u8* buf, u8 byte, u64 len) {
    memset(buf, byte, len);
}

void* tb_malloc(u64 len) {
    return malloc(len);
}

void tb_free(void* ptr) {
    free(ptr);
}

u64 tb_strcmp(char* c1, char* c2) {
    strcmp(c1, c2);
}

void do_assert(int res, char* op) {
	if (!res) {
		printf("Operation \"%s\" failed\n", op);
		exit(-1);
	}
}

void tb_msync(void* addr, u64 len) {
    if (msync(addr, len, MS_SYNC | MS_INVALIDATE) < 0) {
        printf("msync failed %s", strerror(errno));
    }
}