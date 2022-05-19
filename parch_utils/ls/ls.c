#include <syscall.h>
#include <basic_io.h>
#include <utils.h>

char* ftype_name[] = {
    [SOCKET] "SOCKET",
    [LINK] "LINK",
    [REGULAR] "REGULAR",
    [BLOCK] "BLOCK",
    [DIR] "DIR",
    [CHAR] "CHAR",
    [FIFO] "FIFO",
    [UNKNOWN] "UNKNOWN",
    [MOUNT] "MOUNT",
};

int main(u64 argc, char* argv[]) {
    Dirent buf[128];
    char name_buf[1024];
    char* dir_name;
    if (argc == 1) {
        tb_getcwd(name_buf, 1024);
        dir_name = name_buf;
    } else if (argc == 2) {
        dir_name = argv[1];
    } else {
        tb_printf("ls only accept 1 argument, but %d was found. Exiting.\r\n", argc - 1);
        tb_exit(-1);
    }

    FileDescriptor desc = tb_open(dir_name, 1); // read
    if ((i64)desc < 0) {
        tb_printf("tb_open() return %d. Exiting.\r\n", desc);
        tb_exit(-1);
    }

    i64 len = tb_getdents(desc, buf, 128);
    
    if (len < 0) {
        tb_printf("tb_getdents() return %d. Exiting.\r\n", desc);
        tb_exit(-1);
    }

    tb_printf("inode\tf_type\tname\r\n");
    for (i64 i = 0; i < len; i++) {
        tb_printf("%l\t%s\t%s\r\n", buf[i].inode, ftype_name[buf[i].f_type], buf[i].name);
    }
}