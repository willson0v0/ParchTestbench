ELF_FILE = target/riscv64gc-unknown-none-elf/debug/shell

$(ELF_FILE): $(shell find src -type f) $(shell find ../../usrlib_rust/src -type f)
	cargo build --target-dir ./target

./shell: $(ELF_FILE)
	cp $(ELF_FILE) ./shell

all: ./shell

clean:
	cargo clean
	rm -rf target

.PHONY: all clean