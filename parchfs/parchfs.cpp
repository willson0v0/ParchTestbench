#include "parchfs.hpp"

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
__attribute__((noreturn))
void panic(const char* format, ...) {
    std::cout << "PANIC: ";
    va_list args;
    va_start (args, format);
    vprintf(format, args);
    va_end (args);
    exit(-1);
    __builtin_unreachable();
}


void INode::write(size_t off, uint8_t* buf, size_t len, ParchFS* fs) {
    if (off + len >= this->f_size) {
        // expand
        this->get_blkno(off + len, fs, true);
    }
    while (len > 0) {
        BlockNo blk_no = this->get_blkno(off, fs, true);
        MMapAddr blk = fs->get_blk(blk_no);
        size_t blk_off = off % BLK_SIZE;
        size_t copy_len = len > BLK_SIZE - blk_off ? BLK_SIZE - blk_off : len;
        memcpy(blk+blk_off, buf, copy_len);
        len -= copy_len;
        buf += copy_len;
        off += copy_len;
    }
}

void INode::read(size_t off, uint8_t* buf, size_t len, ParchFS* fs) {
    assert(off + len < this->f_size);
    while (len > 0) {
        BlockNo blk_no = this->get_blkno(off, fs);
        MMapAddr blk = fs->get_blk(blk_no);
        size_t blk_off = off % BLK_SIZE;
        size_t copy_len = len > BLK_SIZE - blk_off ? BLK_SIZE - blk_off : len;
        memcpy(buf, blk+blk_off, copy_len);
        len -= copy_len;
        buf += copy_len;
        off += copy_len;
    }
}

BlockNo INode::get_blkno(size_t off, ParchFS* fs, bool create) {
    if(create && off > this->f_size) {
        this->f_size = off;
    }
    assert(off <= this->f_size);

    // direct
    if(create) {
        for (int i = 0; i < std::min(DIRECT_BLK_COUNT, off / BLK_SIZE + 1); i++) {
            if (this->direct_blk_no[i] == BAD_BLOCK) {
                this->direct_blk_no[i] = fs->alloc_blk();
            }
        }
    }
    if (off < BLK_SIZE * DIRECT_BLK_COUNT) {
        return this->direct_blk_no[off / BLK_SIZE];
    }
    off -= BLK_SIZE * DIRECT_BLK_COUNT;

    // indirect 1
    if(create) {
        if (this->indirect_blk == BAD_BLOCK) {
            this->indirect_blk = fs->alloc_blk();
        }

        BlockNo* blks = (BlockNo*)fs->get_blk(this->indirect_blk);
        for (int i = 0; i < std::min(BLKNO_PER_BLK, off / BLK_SIZE + 1); i++) {
            if (blks[i] == BAD_BLOCK) {
                blks[i] = fs->alloc_blk();
            }
        }
    }
    if (off < BLK_SIZE * BLKNO_PER_BLK) {
        BlockNo* blks = (BlockNo*)fs->get_blk(this->indirect_blk);
        return blks[off / BLK_SIZE];
    }
    off -= BLK_SIZE * BLKNO_PER_BLK;

    // indirect 2
    if(create) {
        if (this->indirect_blk2 == BAD_BLOCK) {
            this->indirect_blk2 = fs->alloc_blk();
        }

        BlockNo* indirect_blks = (BlockNo*)fs->get_blk(this->indirect_blk2);
        for (int i = 0; i < std::min(BLKNO_PER_BLK, off / (BLKNO_PER_BLK * BLK_SIZE) + 1); i++) {
            if (indirect_blks[i] == BAD_BLOCK) {
                indirect_blks[i] = fs->alloc_blk();
            }
            
            BlockNo* indirect2_blks = (BlockNo*)fs->get_blk(indirect_blks[i]);
            for (int j = 0; j < BLKNO_PER_BLK; j++) {
                if (indirect2_blks[i] == BAD_BLOCK) {
                    indirect2_blks[i] = fs->alloc_blk();

                    size_t i2_cap = i * BLKNO_PER_BLK * BLK_SIZE + j * BLK_SIZE;
                    if (i2_cap > off) {
                        break;
                    }
                }
            }
        }
    }
    if (off < BLK_SIZE * BLKNO_PER_BLK * BLKNO_PER_BLK) {
        size_t page_offset = off / BLK_SIZE;
        BlockNo indirect_no = ((BlockNo*)(fs->get_blk(this->indirect_blk2)))[page_offset / BLKNO_PER_BLK];
        BlockNo indirect_no2 = ((BlockNo*)(fs->get_blk(indirect_no)))[page_offset % BLKNO_PER_BLK];
        return indirect_no2;
    }
    panic("File size too large.");
}

DEntry* INode::get_dentry(uint32_t pos, ParchFS* fs) {
    if (f_type != FType::DIR) {
        panic("get_dentry called for non-dir inode");
    }
    // expand inode size
    this->get_blkno((pos+1) * sizeof(DEntry), fs, true);
    MMapAddr blk_addr = fs->get_blk(this->get_blkno(pos * sizeof(DEntry), fs, false));
    return (DEntry*)(blk_addr + (pos % DENTRY_PER_BLK) * sizeof(DEntry));
}

void INode::add_dentry(DEntry d, ParchFS* fs) {
    if (f_type != FType::DIR) {
        panic("add_dentry called for non-dir inode");
    }

    // loop all current dir and one more (auto allocate new space)
    for (int i = 0; i <= this->f_size / sizeof(DEntry); i++) {
        DEntry* de = this->get_dentry(i, fs);
        if (de->inode == BAD_INODE) {
            memset(de, 0, sizeof(DEntry));
            std::cout 
                << "adding new detry \"" 
                << std::string((char*)d.f_name, d.name_len) 
                << "\" into inode " 
                << this->get_no(fs) 
                << " @ pos " << i 
                << std::endl;
            *de = d;
            return;
        }
    }
}

size_t INode::get_no(ParchFS* fs) {
    return this - fs->inode_list->nodes;
}

void INodeList::clear_all() {
    memset(this->nodes, 0, sizeof(this->nodes));
}


void INodeBitmap::clear_all() {
    memset(this->data, 0, sizeof(this->data));
}

void INodeBitmap::set(int pos) {
    size_t byte_pos = pos / 8;
    size_t bit_pos = pos % 8;
    set_bit(this->data[byte_pos], bit_pos);
}

void INodeBitmap::clear(int pos) {
    size_t byte_pos = pos / 8;
    size_t bit_pos = pos % 8;
    set_bit(this->data[byte_pos], bit_pos);
}

void INodeBitmap::set_val(int pos, bool val) {
    size_t byte_pos = pos / 8;
    size_t bit_pos = pos % 8;
    set_bit_val(this->data[byte_pos], val, bit_pos);
}

bool INodeBitmap::get(int pos) {
    size_t byte_pos = pos / 8;
    size_t bit_pos = pos % 8;
    return get_bit(this->data[byte_pos], bit_pos);
}

size_t INodeBitmap::first_available() {
    for (size_t i = 0; i < sizeof(this->data) * 8; i++) {
        if (!this->get(i)) {
            return i;
        }
    }
    panic("inode bitmap full");
}

void FSBlockBitmap::clear_all() {
    memset(this->data, 0, sizeof(this->data));
}

void FSBlockBitmap::set(int pos) {
    size_t byte_pos = pos / 8;
    size_t bit_pos = pos % 8;
    set_bit(this->data[byte_pos], bit_pos);
}

void FSBlockBitmap::clear(int pos) {
    size_t byte_pos = pos / 8;
    size_t bit_pos = pos % 8;
    set_bit(this->data[byte_pos], bit_pos);
}

void FSBlockBitmap::set_val(int pos, bool val) {
    size_t byte_pos = pos / 8;
    size_t bit_pos = pos % 8;
    set_bit_val(this->data[byte_pos], val, bit_pos);
}

bool FSBlockBitmap::get(int pos) {
    size_t byte_pos = pos / 8;
    size_t bit_pos = pos % 8;
    return get_bit(this->data[byte_pos], bit_pos);
}

size_t FSBlockBitmap::first_available() {
    for (size_t i = 0; i < sizeof(this->data) * 8; i++) {
        if (!this->get(i)) {
            return i;
        }
    }
    panic("fs block bitmap full");
}


void SuperBlock::init(size_t managed_length) {
    magic = 0xBEEF'BEEF'BEEF'BEEFull;
    for (uint64_t& i : xregs) {
        i = 0;
    }
    base_kernel_satp = 0;
    inode_count = sizeof(INodeList) / sizeof(INode);
    block_count = managed_length / BLK_SIZE;
    free_inode = inode_count;
    free_block = block_count;
    last_access = std::time(nullptr);
    root_inode = ROOT_INODE;
    memset(reserved, 0, sizeof(reserved));
}


MMAPBinContent::MMAPBinContent(const char* path) {
    std::cout << "MMapping parch.bin for contents...\n";
    this->fd = open(path, O_RDWR, 0);
    if (this->fd == -1) {
        panic("Failed to open file %s", path);
    }
    struct stat st;
    if(fstat(this->fd, &st) == 0) {
        this->size = st.st_size;
        this->mmap_ptr = (MMapAddr)mmap(NULL, this->size, PROT_READ | PROT_WRITE, MAP_PRIVATE, this->fd, 0);
    } else {
        panic("Cannot stat bin file");
    }
}

MMAPBinContent::~MMAPBinContent() {
    munmap(mmap_ptr, size);
    close(this->fd);
}

void MMAPBinContent::dump(std::string dest) {
    std::ofstream ofile(dest, std::ios::trunc | std::ios::binary);
    if (!ofile.is_open()) {
        panic("Cannot open %s", dest);
    }
    ofile.write((char*)(this->mmap_ptr), size);
    ofile.close();
}

MMapAddr MMAPBinContent::get_blk(BlockNo blk) {
    return mmap_ptr + blk * BLK_SIZE;
}

BlockNo ParchFS::pa2blkno(PhysAddr pa) {
    return (pa - s_kernel) / BLK_SIZE;
}

BlockNo ParchFS::ma2blkno(MMapAddr ma) {
    return (ma - mmap_bin.mmap_ptr) / BLK_SIZE;
}

PhysAddr ParchFS::ma2pa(MMapAddr ma) {
    return (ma - mmap_bin.mmap_ptr + this->s_kernel);
}

MMapAddr ParchFS::blkno2ma(BlockNo blkno) {
    return mmap_bin.mmap_ptr + blkno * BLK_SIZE;
}

ParchFS::ParchFS(const char* src_bin, const char* src_sym): mmap_bin(MMAPBinContent(src_bin)) {
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
            size_t temp;
            ss << std::hex << addr;
            ss >> temp;
            if (symbol == "skernel") {
                s_kernel = (PhysAddr)temp;
            } else if (symbol == "ekernel") {
                e_kernel = (PhysAddr)temp;
            }  else if (symbol == "sreserve") {
                s_reserve = (PhysAddr)temp;
            }  else if (symbol == "ereserve") {
                e_reserve = (PhysAddr)temp;
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

    superblock      = (SuperBlock*      )(mmap_bin.mmap_ptr + (e_reserve - s_kernel) - sizeof(SuperBlock));
    fs_block_bitmap = (FSBlockBitmap*   )(mmap_bin.mmap_ptr + (e_reserve - s_kernel) - sizeof(SuperBlock) - sizeof(FSBlockBitmap));
    mm_block_bitmap = (FSBlockBitmap*   )(mmap_bin.mmap_ptr + (e_reserve - s_kernel) - sizeof(SuperBlock) - sizeof(FSBlockBitmap) - sizeof(MMBlockBitmap));
    inode_bitmap    = (INodeBitmap*     )(mmap_bin.mmap_ptr + (e_reserve - s_kernel) - sizeof(SuperBlock) - sizeof(FSBlockBitmap) - sizeof(MMBlockBitmap) - sizeof(INodeBitmap));
    inode_list      = (INodeList*       )(mmap_bin.mmap_ptr + (e_reserve - s_kernel) - sizeof(SuperBlock) - sizeof(FSBlockBitmap) - sizeof(MMBlockBitmap) - sizeof(INodeBitmap) - sizeof(INodeList));

    // magic assert
    assert(MMapAddr(superblock      ) - mmap_bin.mmap_ptr + uint64_t(s_kernel) == 0xFFFFF000);
    assert(MMapAddr(fs_block_bitmap ) - mmap_bin.mmap_ptr + uint64_t(s_kernel) == 0xFFFEF000);
    assert(MMapAddr(mm_block_bitmap ) - mmap_bin.mmap_ptr + uint64_t(s_kernel) == 0xFFFDF000);
    assert(MMapAddr(inode_bitmap    ) - mmap_bin.mmap_ptr + uint64_t(s_kernel) == 0xFFFDE000);
    assert(MMapAddr(inode_list      ) - mmap_bin.mmap_ptr + uint64_t(s_kernel) == 0xFFDDE000);

    // TODO: Check magic number for loading init-ed fs

    fs_block_bitmap->clear_all();
    mm_block_bitmap->clear_all();
    inode_bitmap->clear_all();
    inode_list->clear_all();
    superblock->init(s_reserve - e_kernel);

    fs_block_bitmap->set(BAD_BLOCK);
    inode_bitmap->set(BAD_INODE);

    // mark used block
    for (BlockNo blk_no = pa2blkno(s_kernel); blk_no <= pa2blkno(e_kernel); blk_no++) {
        fs_block_bitmap->set(blk_no);
    }
    for (BlockNo blk_no = pa2blkno(s_reserve) - 1; blk_no < pa2blkno(e_reserve); blk_no++) {
        fs_block_bitmap->set(blk_no);
    }

    // make rootdir on INode 1, parent Node 1
    if (this->make_dir(ROOT_INODE, {0}, 0) != ROOT_INODE) {
        panic("root dir inode num != 0!");
    }
}

INodeNo ParchFS::make_dir(INodeNo parent, uint8_t* name, size_t name_len) {
    assert(name_len <= DENTRY_NAME_LEN);
    assert(parent != BAD_INODE);
    INodeNo dir_inode_no = this->alloc_inode();
    INode* dir_inode = this->get_inode(dir_inode_no);

    dir_inode->permission = Perm(0755); // default
    dir_inode->f_type = FType::DIR;
    dir_inode->uid = 0;   // root
    dir_inode->gid = 0;
    dir_inode->f_size = 2 * sizeof(DEntry);   // . and ..
    dir_inode->access_time = std::time(nullptr);
    dir_inode->change_time = std::time(nullptr);
    dir_inode->create_time = std::time(nullptr);
    dir_inode->flags = 0;
    dir_inode->hard_link_count = 3;   // self, . ..

    dir_inode->add_dentry(
        DEntry{
            .inode{dir_inode_no},
            .permission{Perm(0755)},
            .f_type{FType::DIR},
            .name_len{1},
            .f_name={'.'}
        },
        this
    );

    dir_inode->add_dentry(
        DEntry{
            .inode{parent},
            .permission{Perm(0755)},
            .f_type{FType::DIR},
            .name_len{2},
            .f_name={'.', '.'}
        },
        this
    );

    // if this inode is root inode
    if(dir_inode_no == ROOT_INODE) {
        if(parent != ROOT_INODE) {
            panic("Root dir's parent must be itself!");
        }
        // Skip setting parent dirent 
    } else {
        INode* parent_inode = this->get_inode(parent);
        DEntry de;
        de.inode = dir_inode_no;
        de.permission = Perm(0755);
        de.f_type = FType::DIR;
        de.name_len = name_len;
        memcpy(de.f_name, name, name_len);
        parent_inode->add_dentry(de, this);
    }

    return dir_inode_no;
}

INodeNo ParchFS::alloc_inode() {
    INodeNo res = inode_bitmap->first_available();
    superblock->free_inode--;
    inode_bitmap->set(res);
    memset(this->get_inode(res), 0, sizeof(INode));
    std::cout << "allocated new inode no\t" << res << "\t@\t" << std::hex << uint64_t(this->ma2pa((MMapAddr)this->get_inode(res))) << std::dec << std::endl;
    return res;
}

BlockNo ParchFS::alloc_blk() {
    BlockNo res = fs_block_bitmap->first_available();
    fs_block_bitmap->set(res);
    superblock->free_block--;
    // 0 for BAD_INODE or BAD_BLOCK
    memset(this->get_blk(res), 0, BLK_SIZE);
    std::cout << "allocated new block no\t" << res << "\t@\t"<< std::hex  << uint64_t(this->ma2pa((MMapAddr)this->get_blk(res))) << std::dec << std::endl;
    return res;
}

void ParchFS::write_blk(BlockNo blkno, uint8_t buf[BLK_SIZE]) {
    if (!fs_block_bitmap->get(blkno)) {
        panic("Writing to block that was not allocated.");
    }
    memcpy(blkno2ma(blkno), buf, BLK_SIZE);
}

INodeNo ParchFS::make_file(INodeNo parent, uint8_t* name, size_t name_len) {
    assert(name_len <= DENTRY_NAME_LEN);
    assert(this->inode_bitmap->get(parent));
    assert(parent != BAD_INODE);
    INodeNo file_inode_no = this->alloc_inode();
    INode* file_inode = this->get_inode(file_inode_no);

    file_inode->permission = Perm(0755); // default
    file_inode->f_type = FType::REGULAR;
    file_inode->uid = 0;   // root
    file_inode->gid = 0;
    file_inode->f_size = 0;
    file_inode->access_time = std::time(nullptr);
    file_inode->change_time = std::time(nullptr);
    file_inode->create_time = std::time(nullptr);
    file_inode->flags = 0;
    file_inode->hard_link_count = 1;   // self

    // add parent dir dentry
    INode* parent_inode = this->get_inode(parent);
    assert(parent_inode->f_type == FType::DIR);
    DEntry de;
    de.inode = file_inode_no;
    de.permission = Perm(0755);
    de.f_type = FType::REGULAR;
    de.name_len = name_len;
    memset(de.f_name, 0, sizeof(de.f_name));
    memcpy(de.f_name, name, name_len);
    parent_inode->add_dentry(de, this);

    return file_inode_no;
}

MMapAddr ParchFS::get_blk(BlockNo blk) {
    assert(this->fs_block_bitmap->get(blk));
    assert(!this->mm_block_bitmap->get(blk));
    return this->mmap_bin.get_blk(blk);
}

INode* ParchFS::get_inode(INodeNo inode) {
    assert(this->inode_bitmap->get(inode));
    return &(this->inode_list->nodes[inode]);
}

void ParchFS::make_fs(INodeNo parent_inode, std::string root_path, std::string cur_path) {
    std::vector<std::string> vec;
    for (const auto & file: std::filesystem::directory_iterator(root_path)) {
        std::cout << file << std::endl;
        vec.push_back(file.path());
        auto file_name = file.path().filename().string();
        if (file.is_directory()) {
            INodeNo sub_inode = this->make_dir(parent_inode, (uint8_t*)(file_name.c_str()), file_name.length());
            std::string nxt_path = cur_path;
            nxt_path.append("/").append(file.path());
            make_fs(sub_inode, file.path(), nxt_path);
        } else if (file.is_regular_file()) {
            INodeNo file_no = this->make_file(parent_inode, (uint8_t*)(file_name.c_str()), file_name.length());
            std::ifstream stream(file.path(), std::ios::in | std::ios::binary);
            std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            this->get_inode(file_no)->write(0, &contents[0], contents.size(), this);
        }
    }
}

void ParchFS::dump(std::string name) {
    this->mmap_bin.dump(name);
}

// copy all rootfs stuff into this file system
int main(int argc, char* argv[]) {
    if(argc != 5) {
        panic("Must have 4 arguments.");
    }
    ParchFS fs(argv[1], argv[2]);
    fs.make_fs(ROOT_INODE, std::string(argv[4]), std::string(""));
    fs.dump(argv[3]);
    return 0;
}