LINUX_USRLIB = ../../usrlib/linux
PARCH_USRLIB = ../../usrlib/parch
LIB_PATH = /build/usrlib.a

$(LINUX_USRLIB):
	make -C $(LINUX_USRLIB) all

$(PARCH_USRLIB):
	make -C $(PARCH_USRLIB) all

build/linux/00_helloworld: $(LINUX_USRLIB) | build/linux/
	riscv64-linux-gnu-gcc -g -static -I $(LINUX_USRLIB)/../header helloworld.c $(LINUX_USRLIB)$(LIB_PATH) -o $@

build/parch/00_helloworld: $(PARCH_USRLIB) | build/parch/
	riscv64-unknown-elf-gcc -Tparch_linker.ld -g -static -nostdlib -nodefaultlibs --freestanding -I$(PARCH_USRLIB)/../header helloworld.c $(PARCH_USRLIB)$(LIB_PATH) -o $@

build/linux/:
	mkdir -p $@

build/parch/:
	mkdir -p $@

all: build/linux/00_helloworld build/parch/00_helloworld

clean:
	rm -rf build

.PHONY: $(LINUX_USRLIB) $(PARCH_USRLIB) all clean