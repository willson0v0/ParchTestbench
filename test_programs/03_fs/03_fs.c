#include <basic_io.h>
#include <syscall.h>
#include <utils.h>

#define LARGE_SIZE 131072

void single_test() {
	tb_printf("\r\n\r\nSingle file test:\r\n");
	FileDescriptor fd;
	tb_printf("Creating test file with wrong permission...\r\n");
	TB_NEG(fd = tb_open("/fs_test/test0", O_READ | O_WRITE));
	tb_printf("Creating test file...\r\n");
	TB_NON_NEG(fd = tb_open("/fs_test/test0", O_READ | O_WRITE | O_CREATE));
	tb_printf("Writing test file...\r\n");
	TB_ASSERT(tb_write(fd, "test input", 11) == 11);
	tb_printf("Seeking test file...\r\n");
	TB_ASSERT(tb_seek(fd, 0) == 0);
	char buf[15];
	tb_printf("Reading test file...\r\n");
	TB_ASSERT(tb_read(fd, buf, 15) == 11);
	TB_ASSERT(tb_strcmp(buf, "test input") == 0);
	tb_printf("Reopening test file...\r\n");
	TB_NON_NEG(tb_close(fd));
	TB_NON_NEG(fd = tb_open("/fs_test/test0", O_READ | O_WRITE));
	TB_ASSERT(tb_read(fd, buf, 15) == 11);
	TB_ASSERT(tb_strcmp(buf, "test input") == 0);
	tb_printf("Deleting test file...\r\n");
	TB_NON_NEG(tb_delete("/fs_test/test0"));
	TB_NEG(fd = tb_open("/fs_test/test0", O_READ | O_WRITE));
	tb_printf("Basic single file test passed.\r\n");
}

void multi_create() {
	tb_printf("\r\n\r\nMultiple file test.\r\nCreating many test file...\r\n");
	char* name = "/fs_test/t00";

	for(int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			name[10] = '0' + i;
			name[11] = '0' + j;
			
			FileDescriptor fd;
			TB_NON_NEG(fd = tb_open(name, O_READ | O_WRITE | O_CREATE));
			TB_ASSERT(tb_write(fd, name, 13) == 13);
			TB_NON_NEG(tb_close(fd));
		}
	}

	tb_printf("Created many test file, checking content...\r\n");

	for(int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			name[10] = '0' + i;
			name[11] = '0' + j;
			char buf[20];
			
			FileDescriptor fd;
			TB_NON_NEG(fd = tb_open(name, O_READ | O_WRITE));
			TB_ASSERT(tb_read(fd, buf, 20) == 13);
			TB_ASSERT(tb_strcmp(buf, name) == 0);
		}
	}

	tb_printf("Checking directory content...\r\n");

	i64 res = tb_fork();
	if(res == 0) {
		char* param1 = "/fs_test";
		char * arg[3] = {param1, 0};
		tb_exec("/ls", arg);
	} else {
		tb_waitpid(res, 0);
	}

	tb_printf("Multiple file test passed.\r\n");
}

void large_file() {
	tb_printf("\r\n\r\nLarge file test.\r\n");

	FileDescriptor fd;
	tb_printf("\r\nCreating test file...\r\n");
	TB_NON_NEG(fd = tb_open("/fs_test/test_big", O_READ | O_WRITE | O_CREATE));
	tb_printf("Writing test file...\r\n");
	for (u64 i = 0; i < LARGE_SIZE; i++) {
		TB_ASSERT(tb_write(fd, &i, sizeof(i)) == sizeof(i));
	}
	tb_printf("%d byte written. \r\nSeeking test file...\r\n", sizeof(u64) * LARGE_SIZE);
	TB_ASSERT(tb_seek(fd, 0) == 0);
	char buf[15];
	tb_printf("Reading test file...\r\n");
	for (u64 i = 0; i < LARGE_SIZE; i++) {
		u64 buf;
		TB_ASSERT(tb_read(fd, &buf, sizeof(buf)) == sizeof(buf));
		TB_ASSERT(buf == i);
	}
	tb_printf("Reopening and reading test file...\r\n");
	TB_NON_NEG(tb_close(fd));
	TB_NON_NEG(fd = tb_open("/fs_test/test_big", O_READ | O_WRITE));
	for (u64 i = 0; i < LARGE_SIZE; i++) {
		u64 buf;
		TB_ASSERT(tb_read(fd, &buf, sizeof(buf)) == sizeof(buf));
		TB_ASSERT(buf == i);
	}
	tb_printf("Checking mmaped read...\r\n");
	u64* ptr = tb_mmap(0, sizeof(u64) * LARGE_SIZE, 0b011, 0x80, fd, 0);
	
	for (u64 i = 0; i < LARGE_SIZE; i++) {
		TB_ASSERT(ptr[i] == i);
	}
	tb_printf("Checking mmaped write...\r\n");
	for (u64 i = 0; i < LARGE_SIZE; i++) {
		ptr[i] = LARGE_SIZE-i;
	}
	tb_printf("Unmapping...\r\n");
	TB_ASSERT(tb_munmap(ptr, sizeof(u64) * LARGE_SIZE) == 0);

	tb_printf("Reopening and reading test file...\r\n");
	TB_NON_NEG(tb_close(fd));
	TB_NON_NEG(fd = tb_open("/fs_test/test_big", O_READ | O_WRITE));
	for (u64 i = 0; i < LARGE_SIZE; i++) {
		u64 buf;
		TB_ASSERT(tb_read(fd, &buf, sizeof(buf)) == sizeof(buf));
		TB_ASSERT(buf == LARGE_SIZE - i);
	}

	tb_printf("Large file test passed.\r\n");
}

int main() {
	SysStat last, init;
	tb_sysstat(&init);
	
	tb_printf("File system test\r\n");
	tb_printf("Creating test folder...\r\n");

	TB_ASSERT(tb_mkdir("/fs_test", 0777) == 0);
	single_test();
	multi_create();
	large_file();
	
	tb_printf("Deleting test folder...\r\n");
	TB_NON_NEG(tb_delete("/fs_test"));
	TB_NEG(tb_open("/fs_test", O_READ));
	
	tb_sysstat(&last);
	
	tb_printf("memory usage: \n");
	tb_printf("\tpersistant_usage : %d B -> %d B (+- %d B)\n", init.persistant_usage , last.persistant_usage	, last.persistant_usage - init.persistant_usage);
	tb_printf("\truntime_usage    : %d B -> %d B (+- %d B)\n", init.runtime_usage	 , last.runtime_usage		, last.runtime_usage - init.runtime_usage);
	tb_printf("\tkernel_usage     : %d B -> %d B (+- %d B)\n", init.kernel_usage	 , last.kernel_usage		, last.kernel_usage - init.kernel_usage);
	tb_printf("\ttotal_available  : %d B -> %d B (+- %d B)\n", init.total_available	 , last.total_available		, last.total_available - init.total_available);

	tb_printf("File system ok.\r\n");
}