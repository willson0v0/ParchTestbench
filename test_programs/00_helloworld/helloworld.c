#include <syscall.h>

int main() {
	tb_write(1, "Hello world!\n", 13, 0);
	tb_exit(0);
}