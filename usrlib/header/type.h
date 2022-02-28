#ifndef TYPE_H
#define TYPE_H

typedef unsigned long long int u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef long long int i64;
typedef int i32;
typedef short i16;
typedef char i8;

typedef u64 FileDescriptor;
typedef u64 PID;
typedef u64 SignalNum;
typedef void (*SigHandler)();

#endif