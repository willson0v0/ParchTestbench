#include <basic_io.h>
#include <syscall.h>
#include <utils.h>

int main(int argc, char** argv) {
	FileDescriptor fds[2];
	tb_pipe(fds);
	tb_printf("Forking...\r\n");
	u64 res = tb_fork();
	if (res == 0) {
		tb_close(fds[0]);
		tb_write(fds[1], "ping", 5);
		while(1) {}
	} else {
		char buf[10];
		tb_close(fds[1]);
		tb_read(fds[0], buf, 5);
		TB_ASSERT(tb_strcmp(buf, "ping") == 0);
		tb_printf("Pipe good.\r\n");
		tb_signal(res, SIGKILL);
		u64 code;
		tb_waitpid(res, &code);
		tb_printf("Child exit with code %d\r\n", code);
	}
}