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

const uint8_t* BASE_ADDRESS = (uint8_t*)0x80000000ull;
const uint8_t* PHYS_END_ADDRESS = (uint8_t*)0x100000000ull;
const INodeNo BAD_INODE = 0xFFFFFFFF;
const BlockNo BAD_BLOCK = 0xFFFFFFFF;
const size_t PAGE_SIZE = 4096;

typedef uint8_t* PhysAddr;
typedef uint8_t* MMapAddr;
typedef uint32_t INodeNo;
typedef uint32_t BlockNo;

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

void set_bit(uint8_t& bits, size_t pos) {
    bits |= (1 << pos);
}

void clear_bit(uint8_t& bits, size_t pos) {
    bits &= ~(1 << pos);
}

void set_bit_val(uint8_t& bits, bool val, size_t pos) {
    val ? set_bit(bits, pos) : clear_bit(bits, pos);
}

bool get_bit(uint8_t bits, size_t pos) {
    return bits & (1 << pos);
}

void panic(const char* format, ...) {
    std::cout << "PANIC: ";
    va_list args;
    va_start (args, format);
    vprintf(format, args);
    va_end (args);
    exit(-1);
}

struct INode {
    Perm permission;
    FType f_type;
    uint32_t uid;
    uint32_t gid;
    uint32_t f_size;
    uint64_t access_time;
    uint64_t change_time;
    uint64_t create_time;
    uint32_t flags;
    uint32_t hard_link_count;
    BlockNo direct_blk_no[16];
    BlockNo indirect_blk;
    BlockNo indirect_blk2;
    uint8_t reserved[136];
};

struct DEntry {
    INodeNo inode;
    Perm permission;
    FType f_type;
    uint16_t name_len;
    uint8_t f_name[118];
};

struct INodeList {
    INode nodes[8192];

    void clear_all() {
        memset(this->nodes, 0, sizeof(this));
    }
};

struct INodeBitmap {
    uint8_t data[1*4096];

    void clear_all() {
        memset(this->data, 0, sizeof(this));
    }
    
    void set(int pos) {
        size_t byte_pos = pos / sizeof(uint8_t);
        size_t bit_pos = pos % sizeof(uint8_t);
        set_bit(this->data[byte_pos], bit_pos);
    }
    
    void clear(int pos) {
        size_t byte_pos = pos / sizeof(uint8_t);
        size_t bit_pos = pos % sizeof(uint8_t);
        set_bit(this->data[byte_pos], bit_pos);
    }
    
    void set_val(int pos, bool val) {
        size_t byte_pos = pos / sizeof(uint8_t);
        size_t bit_pos = pos % sizeof(uint8_t);
        set_bit_val(this->data[byte_pos], val, bit_pos);
    }

    bool get(int pos) {
        size_t byte_pos = pos / sizeof(uint8_t);
        size_t bit_pos = pos % sizeof(uint8_t);
        return get_bit(this->data[byte_pos], bit_pos);
    }

    size_t first_available() {
        for (size_t i = 0; i < sizeof(this) * 8; i++) {
            if (!this->get(i)) {
                return i;
            }
        }
        panic("inode bitmap full");
    }
};

struct BlockBitmap {
    uint8_t data[16*4096];

    void clear_all() {
        memset(this->data, 0, sizeof(this));
    }
    
    void set(int pos) {
        size_t byte_pos = pos / sizeof(uint8_t);
        size_t bit_pos = pos % sizeof(uint8_t);
        set_bit(this->data[byte_pos], bit_pos);
    }
    
    void clear(int pos) {
        size_t byte_pos = pos / sizeof(uint8_t);
        size_t bit_pos = pos % sizeof(uint8_t);
        set_bit(this->data[byte_pos], bit_pos);
    }
    
    void set_val(int pos, bool val) {
        size_t byte_pos = pos / sizeof(uint8_t);
        size_t bit_pos = pos % sizeof(uint8_t);
        set_bit_val(this->data[byte_pos], val, bit_pos);
    }

    bool get(int pos) {
        size_t byte_pos = pos / sizeof(uint8_t);
        size_t bit_pos = pos % sizeof(uint8_t);
        return get_bit(this->data[byte_pos], bit_pos);
    }

    size_t first_available() {
        for (size_t i = 0; i < sizeof(this) * 8; i++) {
            if (!this->get(i)) {
                return i;
            }
        }
        panic("inode bitmap full");
    }
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

    void init(size_t managed_length) {
        magic = 0xBEEF'BEEF'BEEF'BEEFull;
        for (uint64_t& i : xregs) {
            i = 0;
        }
        base_kernel_satp = 0;
        inode_count = sizeof(INodeList) / sizeof(INode);
        block_count = managed_length / PAGE_SIZE;
        free_inode = inode_count;
        free_block = block_count;
        last_access = std::time(nullptr);
        root_inode = 0;
        memset(reserved, 0, sizeof(reserved));
    }
};

struct MMAPBinContent {
    MMapAddr mmap_ptr;
    int fd;
    size_t size;

    MMAPBinContent(const char* path) {
        this->fd = open(path, O_RDWR, 0);
        if (this->fd == -1) {
            panic("Failed to open file %s", path);
        }
        struct stat st;
        if(fstat(this->fd, &st) != 0) {
            this->size = st.st_size;
            this->mmap_ptr = (MMapAddr)mmap(NULL, this->size, PROT_READ | PROT_WRITE, MAP_PRIVATE, this->fd, 0);
        }
    }

    ~MMAPBinContent() {
        munmap(mmap_ptr, size);
        close(this->fd);
    }

    void dump(const char* dest) {
        std::ofstream ofile(dest, std::ios::trunc | std::ios::binary);
        if (!ofile.is_open()) {
            panic("Cannot open %s", dest);
        }
        ofile.write((char*)(this->mmap_ptr), size);
        ofile.close();
    }
};

struct ParchFS {
    SuperBlock* superblock;
    BlockBitmap* block_bitmap;
    INodeBitmap* inode_bitmap;
    INodeList* inode_list;
    MMAPBinContent mmap_bin;

    PhysAddr s_kernel = nullptr;
    PhysAddr e_kernel = nullptr;
    PhysAddr s_reserve = nullptr;
    PhysAddr e_reserve = nullptr;

    BlockNo pa2blkno(PhysAddr pa) {
        return (pa - s_kernel) / PAGE_SIZE;
    }

    BlockNo ma2blkno(MMapAddr ma) {
        return (ma - mmap_bin.mmap_ptr) / PAGE_SIZE;
    }

    MMapAddr blkno2ma(BlockNo blkno) {
        return mmap_bin.mmap_ptr + blkno * PAGE_SIZE;
    }

    ParchFS(const char* src_bin, const char* src_sym): mmap_bin(MMAPBinContent(src_bin)) {
        std::cout << "Reading parch.sym for symbols...\n";
        std::ifstream parch_sym(src_sym);
        if (!parch_sym.is_open()) {
            panic("Cannot open %s", src_sym);
        }
        std::string line;
        while (std::getline(parch_sym, line)) {
            size_t space_pos = 0;
            if ((space_pos = line.find(' ')) != std::string::npos) {
                std::string addr = line.substr(0, space_pos);
                std::string symbol = line.substr(space_pos + 1);
                std::stringstream ss;
                ss << std::hex << addr;
                if (symbol == "skernel") {
                    ss >> s_kernel;
                } else if (symbol == "ekernel") {
                    ss >> e_kernel;
                }  else if (symbol == "sreserve") {
                    ss >> s_reserve;
                }  else if (symbol == "ereserve") {
                    ss >> e_reserve;
                } 
            } else {
                panic("malformed parch.sym, bad line\"%s\"", line.c_str());
            }
        }

        if (s_kernel == nullptr || e_kernel == nullptr || s_reserve == nullptr || e_reserve == nullptr) {
            panic("Critical symbols not found.");
        }

        if (s_kernel != BASE_ADDRESS) {
            panic("Bad s_kernel");
        }

        if (e_reserve != PHYS_END_ADDRESS) {
            panic("Bad e_reserve");
        }

        superblock      = (SuperBlock*  )(mmap_bin.mmap_ptr + (e_reserve - s_kernel) - sizeof(SuperBlock));
        block_bitmap    = (BlockBitmap* )(mmap_bin.mmap_ptr + (e_reserve - s_kernel) - sizeof(SuperBlock) - sizeof(BlockBitmap));
        inode_bitmap    = (INodeBitmap* )(mmap_bin.mmap_ptr + (e_reserve - s_kernel) - sizeof(SuperBlock) - sizeof(BlockBitmap) - sizeof(INodeBitmap));
        inode_list      = (INodeList*   )(mmap_bin.mmap_ptr + (e_reserve - s_kernel) - sizeof(SuperBlock) - sizeof(BlockBitmap) - sizeof(INodeBitmap) - sizeof(INodeList));

        block_bitmap->clear_all();
        inode_bitmap->clear_all();
        inode_list->clear_all();
        superblock->init(s_reserve - e_kernel);

        // mark used block
        for (BlockNo blk_no = pa2blkno(s_kernel); blk_no <= pa2blkno(e_kernel); blk_no++) {
            block_bitmap->set(blk_no);
        }
        for (BlockNo blk_no = pa2blkno(s_reserve) - 1; blk_no < pa2blkno(e_reserve); blk_no++) {
            block_bitmap->set(blk_no);
        }

        // make rootdir on INode 0, parent Node 0
        if (make_dir(0)) {
            panic("root dir inode num != 0!");
        }
    }

    INodeNo make_dir(INodeNo parent) {
        INodeNo dir_inode = inode_bitmap->first_available();

        inode_list->nodes[dir_inode].permission = Perm(0755); // default
        inode_list->nodes[dir_inode].f_type = FType::DIR;
        inode_list->nodes[dir_inode].uid = 0;   // root
        inode_list->nodes[dir_inode].gid = 0;
        inode_list->nodes[dir_inode].f_size = 2 * sizeof(DEntry);   // . and ..
        inode_list->nodes[dir_inode].access_time = std::time(nullptr);
        inode_list->nodes[dir_inode].change_time = std::time(nullptr);
        inode_list->nodes[dir_inode].create_time = std::time(nullptr);
        inode_list->nodes[dir_inode].flags = 0;
        inode_list->nodes[dir_inode].hard_link_count = 3;   // self, . ..
        
        uint8_t buf[PAGE_SIZE];
        memset(buf, 0, PAGE_SIZE);
        DEntry* de = (DEntry*) buf;

        de[0].inode = dir_inode;
        de[0].permission = Perm(0755);  // default
        de[0].f_type = FType::DIR;
        de[0].name_len = 1;
        strcpy((char*)(de[0].f_name), ".");
        
        de[1].inode = dir_inode;
        de[1].permission = inode_list->nodes[parent].permission;
        de[1].f_type = FType::DIR;
        de[1].name_len = 2;
        strcpy((char*)(de[1].f_name), "..");

        BlockNo dentry_blk = alloc_blk();
        write_blk(dentry_blk, buf);

        inode_list->nodes[dir_inode].direct_blk_no[0] = dentry_blk;
        for (int i = 1; i < 16; i++) {
            inode_list->nodes[dir_inode].direct_blk_no[i] = BAD_BLOCK;
        }

        inode_list->nodes[dir_inode].indirect_blk = BAD_BLOCK;
        inode_list->nodes[dir_inode].indirect_blk2 = BAD_BLOCK;

        return dir_inode;
    }

    BlockNo alloc_blk() {
        BlockNo res = block_bitmap->first_available();
        block_bitmap->set(res);
        superblock->free_block--;
        return res;
    }

    void write_blk(BlockNo blkno, uint8_t buf[PAGE_SIZE]) {
        if (!block_bitmap->get(blkno)) {
            panic("Writing to block that was not allocated.");
        }
        memcpy(blkno2ma(blkno), buf, PAGE_SIZE);
    }

    void write_file(INodeNo parent, const char* name, uint8_t* buf, size_t len) {
        // TODO
    }
};

static_assert(sizeof(INode)==256, "INode malformed");
static_assert(sizeof(DEntry)==128, "DEntry malformed");
static_assert(sizeof(INodeList)==512*PAGE_SIZE, "INodeList malformed");
static_assert(sizeof(INodeBitmap)==PAGE_SIZE, "INodeBitmap malformed");
static_assert(sizeof(BlockBitmap)==16*PAGE_SIZE, "BlockBitmap malformed");
static_assert(sizeof(SuperBlock)==PAGE_SIZE, "SuperBlock malformed");

int main() {
    
    return 0;
}