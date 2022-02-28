.DEFAULT_GOAL := all

PARCH_USRLIB = ../../usrlib/parch
PARCH_USRLIB_OUT = $(PARCH_USRLIB)/build/usrlib.a

$(PARCH_USRLIB_OUT): $(shell find $(PARCH_USRLIB) -type f)
	make -C $(PARCH_USRLIB)

init_proc: init.c linker.ld $(PARCH_USRLIB_OUT)
	riscv64-unknown-elf-gcc -Tlinker.ld -mcmodel=medany -g -static -nostdlib -nodefaultlibs --freestanding -I$(PARCH_USRLIB)/../header init.c $(PARCH_USRLIB_OUT) -o $@

all: init_proc

clean:
	rm -rf init_proc

.PHONY: all