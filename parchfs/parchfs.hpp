#ifndef PARCHFS_HPP
#define PARCHFS_HPP

#include <iostream>
#include <fstream>
#include <stdint-gcc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sstream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <algorithm>
#include <assert.h>
#include <vector>
#include <filesystem>

typedef uint8_t* PhysAddr;
typedef uint8_t* MMapAddr;
typedef uint32_t INodeNo;
typedef uint32_t BlockNo;

struct INode            ;
struct DEntry           ;
struct INodeList        ;
struct INodeBitmap      ;
struct BlockBitmap    ;
struct SuperBlock       ;
struct ParchFS          ;

const uint8_t* BASE_ADDRESS = (uint8_t*)0x80000000ull;
const uint8_t* PHYS_END_ADDRESS = (uint8_t*)0x100000000ull;
const INodeNo BAD_INODE = 0x0;
const BlockNo BAD_BLOCK = 0x0;
const INodeNo ROOT_INODE = 1;
const size_t BLK_SIZE = 4096;
const size_t DIRECT_BLK_COUNT = 16;
const size_t BLKNO_PER_BLK = BLK_SIZE / sizeof(BlockNo);
const size_t DENTRY_NAME_LEN = 118;

enum Perm: uint16_t {
    OWNER_R = 0400,
    OWNER_W = 0200,
    OWNER_X = 0100,
    GROUP_R = 0040,
    GROUP_W = 0020,
    GROUP_X = 0010,
    OTHER_R = 0004,
    OTHER_W = 0002,
    OTHER_X = 0001,
};

enum FType: uint16_t {
    SOCKET  = 0001,
    LINK    = 0002,
    REGULAR = 0004,
    BLOCK   = 0010,
    DIR     = 0020,
    CHAR    = 0040,
    FIFO    = 0100,
};

struct INode {
    Perm permission;
    FType f_type;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t hard_link_count;
    BlockNo direct_blk_no[DIRECT_BLK_COUNT];
    BlockNo indirect_blk;
    BlockNo indirect_blk2;
    uint64_t f_size;
    uint64_t access_time;
    uint64_t change_time;
    uint64_t create_time;
    uint8_t reserved[128];

    void write(size_t off, uint8_t* buf, size_t len, ParchFS* fs);
    void read(size_t off, uint8_t* buf, size_t len, ParchFS* fs);
    BlockNo get_blkno(size_t off, ParchFS* fs, bool create=false);
    DEntry* get_dentry(uint32_t pos, ParchFS* fs);
    void add_dentry(DEntry d, ParchFS* fs);
    size_t get_no(ParchFS* fs);
};

struct DEntry {
    INodeNo inode;
    Perm permission;
    FType f_type;
    uint16_t name_len;
    uint8_t f_name[DENTRY_NAME_LEN];
};

struct INodeList {
    INode nodes[8192];

    void clear_all();
};

struct INodeBitmap {
    uint8_t data[1*4096];

    void clear_all();
    void set(int pos);
    void clear(int pos);
    void set_val(int pos, bool val);
    bool get(int pos);
    size_t first_available();
};

struct BlockBitmap {
    uint8_t data[16*4096];

    void clear_all();
    void set(int pos);
    void clear(int pos);
    void set_val(int pos, bool val);
    bool get(int pos);
    size_t first_available();
};

struct SuperBlock {
    uint64_t magic;
    uint64_t xregs[31];
    uint64_t base_kernel_satp;
    uint64_t inode_count;
    uint64_t block_count;
    uint64_t free_inode;
    uint64_t free_block;
    uint64_t last_access;
    uint32_t root_inode;
    uint8_t reserved[3788];

    void init(size_t managed_length);
};

struct MMAPBinContent {
    MMapAddr mmap_ptr;
    int fd;
    size_t size;

    MMAPBinContent(const char* path);
    ~MMAPBinContent();
    void dump(std::string dest);
    MMapAddr get_blk(BlockNo blk);
};

struct ParchFS {
    SuperBlock* superblock;
    BlockBitmap* fs_block_bitmap;
    BlockBitmap* mm_block_bitmap;
    INodeBitmap* inode_bitmap;
    INodeList* inode_list;
    MMAPBinContent mmap_bin;

    PhysAddr s_kernel = nullptr;
    PhysAddr e_kernel = nullptr;
    PhysAddr s_reserve = nullptr;
    PhysAddr e_reserve = nullptr;

    ParchFS(const char* src_bin, const char* src_sym);

    BlockNo pa2blkno(PhysAddr pa);
    BlockNo ma2blkno(MMapAddr ma);
    PhysAddr ma2pa(MMapAddr ma);
    MMapAddr blkno2ma(BlockNo blkno);
    void write_blk(BlockNo blkno, uint8_t buf[BLK_SIZE]);
    INodeNo make_dir(INodeNo parent, uint8_t* name, size_t name_len);
    INodeNo make_file(INodeNo parent, uint8_t* name, size_t name_len);
    BlockNo alloc_blk();
    MMapAddr get_blk(BlockNo blk);
    INodeNo alloc_inode();
    INode* get_inode(INodeNo inode);
    void make_fs(INodeNo parent_inode, std::string root_path, std::string cur_path);
    void dump(std::string res_name);
};

static_assert(sizeof(INode          ) == 256            , "INode malformed");
static_assert(sizeof(DEntry         ) == 128            , "DEntry malformed");
static_assert(sizeof(INodeList      ) == 512 * BLK_SIZE, "INodeList malformed");
static_assert(sizeof(INodeBitmap    ) == BLK_SIZE      , "INodeBitmap malformed");
static_assert(sizeof(BlockBitmap  ) == 16  *BLK_SIZE , "BlockBitmap malformed");
static_assert(sizeof(SuperBlock     ) == BLK_SIZE      , "SuperBlock malformed");

const size_t DENTRY_PER_BLK = BLK_SIZE / sizeof(DEntry);

#endif