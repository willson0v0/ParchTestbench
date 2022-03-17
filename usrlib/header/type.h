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

typedef struct
{
    u32 inode;
    u16 f_type;
    char name[122];
} Dirent;

typedef enum {
    SOCKET  = 0001,
    LINK    = 0002,
    REGULAR = 0004,
    BLOCK   = 0010,
    DIR     = 0020,
    CHAR    = 0040,
    FIFO    = 0100,
    UNKNOWN = 0200,
    MOUNT   = 0400,
} FType;

typedef enum {
    OWNER_R = 0400,
    OWNER_W = 0200,
    OWNER_X = 0100,
    GROUP_R = 0040,
    GROUP_W = 0020,
    GROUP_X = 0010,
    OTHER_R = 0004,
    OTHER_W = 0002,
    OTHER_X = 0001,
} FPermission;


typedef enum {
    READ      = (1 << 0),
    WRITE     = (1 << 1),
    CREATE    = (1 << 2),
    EXEC      = (1 << 3),
    SYS       = (1 << 4),
    NO_FOLLOW = (1 << 5),
} OpenMode;

typedef struct {
    u64 persistant_usage;
    u64 runtime_usage;
    u64 kernel_usage;
    u64 total_available;
} SysStat;


#endif