.global tb_write
tb_write:
	li a7, 0
	ecall
	ret

.global tb_read
tb_read:
	li a7, 1
	ecall
	ret

.global tb_open
tb_open:
	li a7, 2
	ecall
	ret

.global tb_openat
tb_openat:
	li a7, 3
	ecall
	ret

.global tb_fork
tb_fork:
	li a7, 4
	ecall
	ret

.global tb_exec
tb_exec:
	li a7, 5
	ecall
	ret

.global tb_exit
tb_exit:
	li a7, 6
	ecall
	ret

