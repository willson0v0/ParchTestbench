.DEFAULT_GOAL := all

LINUX_USRLIB = ../../usrlib/linux
PARCH_USRLIB = ../../usrlib/parch
LIB_PATH = /build/usrlib.a
FILE_NAME = $(notdir $(shell pwd))

build/linux/$(FILE_NAME): $(FILE_NAME).c ../parch_linker.ld  $(LINUX_USRLIB)$(LIB_PATH) | build/linux/
	riscv64-linux-gnu-gcc -g -static  -I $(LINUX_USRLIB)/../header -I ../../../gem5/include $(FILE_NAME).c $(LINUX_USRLIB)$(LIB_PATH) ../../../gem5/util/m5/build/riscv/out/libm5.a -o $@

build/parch/$(FILE_NAME): $(FILE_NAME).c ../parch_linker.ld  $(PARCH_USRLIB)$(LIB_PATH) | build/parch/
	riscv64-unknown-elf-gcc -T../parch_linker.ld -mcmodel=medany -g -static -nostdlib -nodefaultlibs --freestanding -I ../../../gem5/include -I$(PARCH_USRLIB)/../header $(FILE_NAME).c $(PARCH_USRLIB)$(LIB_PATH) ../../../gem5/util/m5/build/riscv/out/libm5.a -o $@

build/linux/:
	mkdir -p $@

build/parch/:
	mkdir -p $@

all: build/linux/$(FILE_NAME) build/parch/$(FILE_NAME)

clean:
	rm -rf build

.PHONY: all clean