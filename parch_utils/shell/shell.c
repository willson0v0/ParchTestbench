#include <syscall.h>
#include <basic_io.h>
#include <utils.h>

void print_cwd() {
    char cwd[256];
    tb_getcwd(cwd, 256);
    tb_printf("[ %s ] $ ", cwd);
}

int start_with(char* to_match, char* pattern) {
    while (*pattern && *to_match) {
        if (*(pattern++) != *(to_match++)) {
            return 0;
        }
    }
    return 1;
}

int serve_cd(char* cmd) {
    char* dir = &cmd[3];
    return tb_chdir(dir);
}

int serve_exit(char* cmd) {
    tb_printf("[shell] bye.");
    tb_exit(0);
}

int serve_help(char* cmd) {
    tb_printf("Built-in commands:\n");
    tb_printf("\tcd [dir]\t: change current working directory.\n");
    tb_printf("\thelp\t: display help message.\n");
    tb_printf("\texit\t: exit shell.\n");
    tb_printf("\nPlease enter built-in commands, or path to elf file to execute.\n");
    return 0;
}

void memset(char* ptr, u8 b, u64 len) {
    tb_memset(ptr, b, len);
}

int exec_cmd(char* cmd) {
    char name[128] = {0};
    char arg_str[16][32] = {0};
    // first, find program name
    char* p = cmd;
    while(*p != ' ' && *p != '\0') {
        p++;
    }
    tb_memcpy(cmd, name, (p - cmd));
    u64 argv_idx = 0;

    while (*p != '\0') {
        while (*p == ' ') {
            p++;
        }
        char* arg = p;
        while(*p != ' ' && *p != '\0') {
            p++;
        }
        tb_memcpy(arg, arg_str[argv_idx++], p - arg);
    }
    char* argv[16] = {0};
    for (int i = 0; i < argv_idx; i++) {
        argv[i] = arg_str[i];
    }

    int fork_res = tb_fork();
    if (fork_res == 0) {
        u64 res = tb_exec(name, argv);
        tb_printf("exec %s failed with errno %d. \n", name, res);
        tb_exit(-1);
    } else {
        u64 exec_res;
        tb_waitpid(fork_res, &exec_res);
        tb_printf("\nProgram exited with code %d\n", exec_res);
        return 0;
    }
}

int parse_cmd(char* cmd) {
    // empty input
    if (tb_strlen(cmd) == 0) {
        return 0;
    } else if(start_with(cmd, "cd ")) {
        return serve_cd(cmd);
    } else if(start_with(cmd, "help")) {
        return serve_help(cmd);
    } else if(start_with(cmd, "exit")) {
        return serve_exit(cmd);
    } else {
        exec_cmd(cmd);
    }
}

int main(u64 argc, char* argv[]) {
    tb_printf("[shell] Hello world!\n");

    if (argc > 1) {
        tb_close(0);
        FileDescriptor fd = tb_open(argv[1], READ | EXEC);
        if (fd != 0) {
            tb_printf("failed to substitute fd 0.");
            tb_exit(-1);
        }
    }

    while(1) {
        print_cwd();

        char linebuf[1024];
        u64 res = tb_getline(linebuf, 1024);
        if (res <= 0) {
            tb_printf("Encounterd EOF. Exiting.\n");
            tb_exit(0);
        }
        if (parse_cmd(linebuf)) {
            tb_printf("Unable to understand input \"%s\"\n", linebuf);
        }
    }
}