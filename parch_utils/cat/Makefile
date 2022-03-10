ELF_FILE = target/riscv64gc-unknown-none-elf/debug/cat

$(ELF_FILE): $(shell find src -type f) $(shell find ../../usrlib_rust/src -type f)
	cargo build --target-dir ./target

./cat: $(ELF_FILE)
	cp $(ELF_FILE) ./cat

all: ./cat

clean:
	cargo clean
	rm -rf target

.PHONY: all clean