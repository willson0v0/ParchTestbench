.DEFAULT_GOAL := all

C_OBJECT_NAME = basic_io.o
ASM_OBJECT_NAME = syscall.o crt.o
C_OBJECTS = $(addprefix build/, $(C_OBJECT_NAME))
ASM_OBJECTS = $(addprefix build/, $(ASM_OBJECT_NAME))
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)
CC = riscv64-unknown-elf-gcc
CFLAGS = -c -I ../header -nodefaultlibs --freestanding -nostdlib -g -mcmodel=medany
ASMFLAGS = -c -nodefaultlibs --freestanding -nostdlib -g -mcmodel=medany

syscall.S: syscall_gen.py ../../../syscall_num.csv
	python syscall_gen.py

$(C_OBJECTS): build/%.o: %.c | build/
	$(CC) $(CFLAGS) -o $@ $^

$(ASM_OBJECTS): build/%.o: %.S | build/
	$(CC) $(ASMFLAGS) $^ -o $@ 

build/usrlib.a: $(OBJECTS) | build/
	ar rcs $@ $^

build/:
	mkdir -p $@

all: build/usrlib.a

clean:
	rm -rf build

.PHONY: all clean