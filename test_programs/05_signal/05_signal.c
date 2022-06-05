#include <basic_io.h>
#include <syscall.h>
#include <utils.h>

void sleep(u64 time) {
	u64 start_time = tb_time();
	while (tb_time() < start_time + time) {}
}

void sighandler() {
	sleep(200);
	tb_printf("Hello world from singnal handler!\r\n");
}


void parent(PID child_pid) {
	sleep(2500);
	tb_signal(child_pid, SIGUSR1);
	sleep(3000);
	tb_signal(child_pid, SIGKILL);
	u64 code;
	tb_waitpid(child_pid, &code);
	tb_printf("child killed with exit code %d\r\n", code);
	tb_exit(0);
}

void child() {
	tb_sigaction(SIGUSR1, sighandler);
	while(1) {
		tb_printf("Child pending...\r\n");
		sleep(1000);
	}
}

int main(int argc, char** argv) {
	tb_printf("Forking...\n");
	u64 res = tb_fork();
	if (res == 0) {
		child();
	} else {
		parent(res);
	}
}