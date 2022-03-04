#include <syscall.h>
#include <basic_io.h>

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
    return tb_chdir(cmd);
}

void strcpy(char* src, char* dst) {
    while(*src) {
        *(dst++) = *(src++);
    }
}

void memcpy(char* src, char* dst, u64 len) {
    while(len --> 0) {
        *(dst++) = *(src++);
    }
}

void memset(char* ptr, u8 b, u64 len) {
    while(len --> 0) 
        *(ptr++) = b;
}

int exec_cmd(char* cmd) {
    char name[128] = {0};
    char arg_str[16][32];
    // first, find program name
    char* p = cmd;
    while(*p != ' ' && *p != '\0') {
        p++;
    }
    memcpy(cmd, name, (p - cmd));
    u64 argv_idx = 0;
    while (*p != '\0') {
        char* arg = p;
        while(*p != ' ' && *p != '\0') {
            p++;
        }
        memcpy(arg, arg_str[argv_idx++], p - arg);
    }
    char* argv[16] = {0};
    for (int i = 0; i < argv_idx++; i++) {
        argv[i] = arg_str[i];
    }

    u64 fork_res = tb_fork();
    if (fork_res == 0) {
        u64 res = tb_exec(name, argv);
        tb_printf("exec failed with errno %d. \r\n", res);
        tb_exit(-1);
    } else {
        u64 exec_res;
        tb_waitpid(fork_res, &exec_res);
        tb_printf("\r\nProgram exited with code %d\r\n", exec_res);
        return 0;
    }
}

int parse_cmd(char* cmd) {
    if(start_with(cmd, "cd ")) {
        return serve_cd(cmd);
    } else if(start_with(cmd, "help")) {
        tb_printf("No help for you.");
    } else {
        exec_cmd(cmd);
    }
}

int main() {
    tb_printf("[shell] Hello world!\n");
    while(1) {
        print_cwd();

        char linebuf[1024];
        tb_getline(linebuf, 1024);
        if (!parse_cmd(linebuf)) {
            tb_printf("Unable to understand input \"%s\"\r\n", linebuf);
        }
    }
}