#include <syscall.h>
#include <basic_io.h>

int main() {
    tb_printf("[shell] Hello world!\n");
    while(1) {
        char linebuf[1024];
        tb_getline(linebuf, 1024);
        tb_printf("get input: %s\n", linebuf);
    }
}