#include <../header/basic_io.h>
#include <../header/syscall.h>

int vfprintf(int fd, char* format, va_list arg) {
	tb_write(fd, "Not Implemented", -1);
}

int tb_printf(char* format, ...) {
	va_list arg;
	int done;
	va_start(arg, format);
	done = vfprintf(1, format, arg);
	va_end(arg);

	return done;
}