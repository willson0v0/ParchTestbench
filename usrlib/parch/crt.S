
    .globl _start
_start:
    la sp, crt_stack_top
    call main

    .section .bss.stack
    .globl boot_stack
crt_stack:
    .space 4096 * 2
    .globl crt_stack_top
crt_stack_top: