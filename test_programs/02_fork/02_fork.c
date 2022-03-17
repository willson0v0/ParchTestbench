#include <basic_io.h>
#include <syscall.h>

#define FORK_TEST_COUNT 128

int main() {
	tb_printf("Fork test\n");
	int i;
	SysStat last, init;
	tb_sysstat(&init);
	for (i = 0; i < FORK_TEST_COUNT; i++) {

		// SysStat buf;
		// tb_sysstat(&buf);
		// tb_printf("==========\ncurrent memory usage: \n");
		// tb_printf("\tpersistant_usage : %d B (+- %d B)\n", buf.persistant_usage, buf.persistant_usage - last.persistant_usage);
		// tb_printf("\truntime_usage    : %d B (+- %d B)\n", buf.runtime_usage, buf.runtime_usage - last.runtime_usage);
		// tb_printf("\tkernel_usage     : %d B (+- %d B)\n", buf.kernel_usage, buf.kernel_usage - last.kernel_usage);
		// tb_printf("\ttotal_available  : %d B (+- %d B)\n", buf.total_available, buf.total_available - last.total_available);
		// tb_printf("forked %d times\n", i);
		// last = buf;

		i64 pid = tb_fork();
		if (pid < 0) {
			break;
		}
		if (pid == 0) {
			return 0;
		}
	}


	while (i --> 0) {
		i64 ret_val;
		if(tb_waitpid(0, &ret_val) < 0) {
			tb_printf("Waitpid failed.");
			return 1;
		}
	}

	tb_sysstat(&last);
	
	tb_printf("memory usage: \n");
	tb_printf("\tpersistant_usage : %d B -> %d B (+- %d B)\n", init.persistant_usage , last.persistant_usage	, last.persistant_usage - init.persistant_usage);
	tb_printf("\truntime_usage    : %d B -> %d B (+- %d B)\n", init.runtime_usage	 , last.runtime_usage		, last.runtime_usage - init.runtime_usage);
	tb_printf("\tkernel_usage     : %d B -> %d B (+- %d B)\n", init.kernel_usage	 , last.kernel_usage		, last.kernel_usage - init.kernel_usage);
	tb_printf("\ttotal_available  : %d B -> %d B (+- %d B)\n", init.total_available	 , last.total_available		, last.total_available - init.total_available);

	tb_printf("fork ok.");
}