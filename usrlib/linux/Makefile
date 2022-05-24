.DEFAULT_GOAL := all

OBJECT_NAME = $(addsuffix .o, $(basename $(shell find . -name '*.c')))
OBJECTS = $(addprefix build/, $(OBJECT_NAME))
CC = riscv64-linux-gnu-gcc
CFLAGS = -c -I ../header -g

$(OBJECTS): build/%.o: %.c | build/
	$(CC) $(CFLAGS) -o $@ $^

build/usrlib.a: $(OBJECTS) | build/
	ar rcs $@ $^

build/:
	mkdir -p $@

all: build/usrlib.a

clean:
	rm -rf build

.PHONY: all clean