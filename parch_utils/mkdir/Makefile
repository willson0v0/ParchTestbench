ELF_FILE = target/riscv64gc-unknown-none-elf/debug/mkdir

$(ELF_FILE): $(shell find src -type f) $(shell find ../../usrlib_rust/src -type f)
	cargo build --target-dir ./target

./mkdir: $(ELF_FILE)
	cp $(ELF_FILE) ./mkdir

all: ./mkdir

clean:
	cargo clean
	rm -rf target

.PHONY: all clean