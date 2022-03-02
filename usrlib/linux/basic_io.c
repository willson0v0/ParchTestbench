#include <basic_io.h>
#include <stdio.h>

int tb_printf(char* format, ...) {
	va_list arg;
	int done;
	va_start(arg, format);
	done = vfprintf(stdout, format, arg);
	va_end(arg);

	return done;
}

char tb_getc() {
	getchar();
}

u64 tb_getline(char* buf, u64 length) {
	return getline(buf, length, 0);
}