#include <syscall.h>
#include <basic_io.h>

int main() {
    tb_open("/dev/pts", 1); // read
    tb_open("/dev/pts", 2); // write
    tb_open("/dev/pts", 2); // write

    tb_printf("[init_proc] Hello world!\r\n");
    int ret = tb_fork();
    if (ret == 0) {
        tb_exec("/shell", 0);
    } else {
        while(1) {
            u64 ret;
            PID dead = tb_waitpid(-1, &ret);
            tb_printf("[init_proc] Zombie %d killed, ret val %d.\r\n", dead, ret);
            if (dead == 2) {
                
            }
        }
    }
}