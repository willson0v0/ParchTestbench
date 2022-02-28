#include <syscall.h>
#include <basic_io.h>

int main() {
    tb_printf("[init_proc] Hello world!\n");
    int ret = tb_fork();
    if (ret == 0) {
        tb_exec("/shell", 0);
    } else {
        while(1) {
            u64 ret;
            PID dead = tb_waitpid(-1, &ret);
            tb_printf("[init_proc] Zombie %d killed, ret val %d.\n", dead, ret);
        }
    }
}