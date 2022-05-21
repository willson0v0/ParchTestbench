#include <basic_io.h>
#include <syscall.h>
#include <utils.h>

int main() {
	tb_printf("File system test\n");
	tb_printf("Creating test folder...\n");

	TB_ASSERT(tb_mkdir("/fs_test", 0777) == 0);
	FileDescriptor fd;
	tb_printf("Creating test file with wrong permission...\n");
	TB_NEG(fd = tb_open("/fs_test/test0", O_READ | O_WRITE));

	tb_printf("Checking folder...\n=======\n");
	i64 res = tb_fork();
	if(res == 0) {
		char* param1 = "/fs_test";
		char * arg[3] = {param1, 0};
		tb_exec("/ls", arg);
	} else {
		tb_waitpid(res, 0);
	}

	tb_printf("\n=========\nCreating test file...\n");
	TB_NON_NEG(fd = tb_open("/fs_test/test0", O_READ | O_WRITE | O_CREATE));
	tb_printf("Writing test file...\n");
	TB_ASSERT(tb_write(fd, "test input", 11) == 11);
	tb_printf("Seeking test file...\n");
	TB_ASSERT(tb_seek(fd, 0) == 0);
	char buf[15];
	tb_printf("Reading test file...\n");
	TB_ASSERT(tb_read(fd, buf, 15) == 11);
	TB_ASSERT(tb_strcmp(buf, "test input") == 0);
	tb_printf("Reopening test file...\n");
	TB_NON_NEG(tb_close(fd));
	TB_NON_NEG(fd = tb_open("/fs_test/test0", O_READ | O_WRITE));
	TB_ASSERT(tb_read(fd, buf, 15) == 11);
	TB_ASSERT(tb_strcmp(buf, "test input") == 0);
	tb_printf("Deleting test file...\n");
	TB_NON_NEG(tb_delete("/fs_test/test0"));
	TB_NEG(fd = tb_open("/fs_test/test0", O_READ | O_WRITE));
	TB_NON_NEG(tb_delete("/fs_test"));
	TB_NEG(fd = tb_open("/fs_test", O_READ));

	tb_printf("File system ok.\n");
}