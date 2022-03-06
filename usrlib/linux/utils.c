#include "../header/utils.h"
#include "string.h"
#include "malloc.h"

void tb_memcpy(u8* src, u8* dst, u64 len) {
    memcpy(dst, src, len);
}

void tb_strcpy(char* src, char* dst) {
    strcpy(dst, src);
}
    
u64 tb_strlen(char* str) {
    return strlen(str);
}

void* tb_memset(u8* buf, u8 byte, u64 len) {
    memset(buf, byte, len);
}

void* tb_malloc(u64 len) {
    return malloc(len);
}

void tb_free(void* ptr) {
    free(ptr);
}