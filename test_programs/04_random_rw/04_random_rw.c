#include <basic_io.h>
#include <syscall.h>
#include <utils.h>
#include <gem5/m5ops.h>

u64 buf[4096];

typedef struct {
	u64 regular;
	u64 mmaped;
} Result;

Result res_1[5];
Result res_2[5];
Result res_3[5];

void copy_u64(u64* src, u64* dst, u64 len) {
	while (len --> 0) {
		*(dst++) = *(src++);
	}
}

struct rand_state {
  	u64 a;
};

struct rand_state state;

u64 rand() {
	u64 x = state.a;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	return state.a = x;
}

void init_buf() {
	for (u64 i = 0; i < sizeof(buf) / sizeof(u64); i++) {
		buf[i] = rand();
	}
}

typedef enum {
	TEST_RAND_READ = 1,
	TEST_RAND_WRITE = 1,
	TEST_RAND_RW = 1,
} TestType;

void create_test_file(u64 file_size, char* path) {
	FileDescriptor fd;
	TB_NON_NEG(fd = tb_open(path, O_READ | O_WRITE | O_CREATE));
	u64 iter = file_size / sizeof(buf);
	while (iter --> 0) {
		TB_ASSERT(tb_write(fd, (u8*)buf, sizeof(buf)) == sizeof(buf));
	}
}

u64 random_file(char* path, u64 batch_size, u64 file_size, u64 cycle_count, TestType t) {
	u64 start_time = tb_time();
	create_test_file(file_size, path);
	FileDescriptor fd;
	TB_NON_NEG(fd = tb_open(path, O_READ | O_WRITE));
	for (u64 i = 0; i < cycle_count; i++) {
		u64 offset = rand() % (file_size - batch_size);
		offset = offset - (offset % sizeof(u64));
		TB_NON_NEG(tb_seek(fd, offset));
		if (t == TEST_RAND_WRITE) {
			TB_ASSERT(tb_write(fd, (u8*)buf, batch_size) == batch_size);
		} else if (t == TEST_RAND_READ) {
			TB_ASSERT(tb_read(fd, (u8*)buf, batch_size) == batch_size);
		} else if (t == TEST_RAND_RW) {
			if(rand() % 2) {
				TB_ASSERT(tb_read(fd, (u8*)buf, batch_size) == batch_size);
			} else {
				TB_ASSERT(tb_write(fd, (u8*)buf, batch_size) == batch_size);
			}
		}
	}
	TB_NON_NEG(tb_close(fd));
	TB_NON_NEG(tb_delete(path));
	return tb_time() - start_time;
}

u64 random_mmap(char* path, u64 batch_size, u64 file_size, u64 cycle_count, TestType t) {
	// m5_reset_stats(0, 0);
	u64 start_time = tb_time();
	create_test_file(file_size, path);
	FileDescriptor fd;
	TB_NON_NEG(fd = tb_open(path, O_READ | O_WRITE));
	void* head_addr = tb_mmap(0, file_size, 0b11, 0x80, fd, 0);
	for (u64 i = 0; i < cycle_count; i++) {
		u64 offset = rand() % (file_size - batch_size);
		offset = offset - (offset % sizeof(u64));
		if (t == TEST_RAND_WRITE) {
			copy_u64((u64*)buf, (u64*)(head_addr + offset), batch_size / sizeof(u64));
		} else if (t == TEST_RAND_READ) {
			copy_u64((u64*)(head_addr + offset), (u64*)buf, batch_size / sizeof(u64));
		} else if (t == TEST_RAND_RW) {
			if(rand() % 2) {
				copy_u64((u64*)(head_addr + offset), (u64*)buf, batch_size / sizeof(u64));
			} else {
				copy_u64((u64*)buf, (u64*)(head_addr + offset), batch_size / sizeof(u64));
				tb_msync(head_addr + offset, batch_size);
			}
		}
	}
	tb_msync(head_addr, file_size);
	TB_NON_NEG(tb_munmap(head_addr, file_size));	// flush cache
	TB_NON_NEG(tb_close(fd));
	TB_NON_NEG(tb_delete(path));
	// m5_dump_stats(0, 0);
	return tb_time() - start_time;
}


void random_test(char* path, u64 batch_size, u64 byte_count, Result* res) {
	u64 f_size = 1048576;
	tb_printf("Testing with file size %u, batch_size %u, total r/w %u bytes\r\n", f_size, batch_size, byte_count);

	// res_ptr->regular_c = random_file(batch_size, f_size, byte_count/batch_size, TEST_RAND_RW);
	// res_ptr->regular_r = random_file(batch_size, f_size, byte_count/batch_size, TEST_RAND_READ);
	// res_ptr->regular_w = random_file(batch_size, f_size, byte_count/batch_size, TEST_RAND_WRITE);
	// tb_printf("random_test %u regular: read %u ms, write %u ms, combined %u ms\r\n", 
	// 	batch_size, 
	// 	res_ptr->regular_r, 
	// 	res_ptr->regular_w, 
	// 	res_ptr->regular_c
	// );
	
	// random_mmap(path, batch_size, f_size, byte_count/batch_size, TEST_RAND_RW);
	// random_mmap(path, batch_size, f_size, byte_count/batch_size, TEST_RAND_READ);
	// random_mmap(path, batch_size, f_size, byte_count/batch_size, TEST_RAND_WRITE);
	res->regular = random_file(path, batch_size, f_size, byte_count/batch_size, TEST_RAND_RW);
	res->mmaped = random_mmap(path, batch_size, f_size, byte_count/batch_size, TEST_RAND_RW);
}

u64 my_stoi(char* s) {
	u64 res = 0;
	while (*s) {
		res *= 10;
		res += (*s) - '0';
		s++;
	}
	return res;
}

int main(int argc, char** argv) {
	// char* path = argv[argc - 3];
	// u64 batch_size = my_stoi(argv[argc - 2]);
	// u64 total_bytes = my_stoi(argv[argc - 1]);
	char* path = "test_file";
	tb_printf("File system test\r\n");
	tb_printf("Initializing buffer...\r\n");
	init_buf();
	
	u64 batch_size = 4096;
	u64 total_bytes = 104857600;

	for (int i = 0; i < 5; i++) {
		random_test(path,  4096, total_bytes * (i + 1), &res_1[i]);
		random_test(path,  8192, total_bytes * (i + 1), &res_2[i]);
		random_test(path, 16384, total_bytes * (i + 1), &res_3[i]);
	}

	
	for (int i = 0; i < 5; i++) {tb_printf("\t%d", i);} tb_printf("\r\n");
	tb_printf(" 4KiB"); for (int i = 0; i < 5; i++) {tb_printf("\t%u", res_1[i].regular);} tb_printf("\r\n");
	tb_printf(" 8KiB"); for (int i = 0; i < 5; i++) {tb_printf("\t%u", res_2[i].regular);} tb_printf("\r\n");
	tb_printf("16KiB"); for (int i = 0; i < 5; i++) {tb_printf("\t%u", res_3[i].regular);} tb_printf("\r\n");
	tb_printf(" 4KiB"); for (int i = 0; i < 5; i++) {tb_printf("\t%u", res_1[i].mmaped );} tb_printf("\r\n");
	tb_printf(" 8KiB"); for (int i = 0; i < 5; i++) {tb_printf("\t%u", res_2[i].mmaped );} tb_printf("\r\n");
	tb_printf("16KiB"); for (int i = 0; i < 5; i++) {tb_printf("\t%u", res_3[i].mmaped );} tb_printf("\r\n");
}