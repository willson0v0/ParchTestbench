TEST_PROGRAMS = 00_helloworld 01_write
TP_FOLDER = $(addprefix ../test_programs/, $(TEST_PROGRAMS))
TP_BINARIES = $(join $(TP_FOLDER), $(addprefix /build/parch/, $(TEST_PROGRAMS)) )

$(TP_BINARIES): $(TP_FOLDER)
	for i in $(TEST_PROGRAMS); do \
		make -C ../test_programs/$$i; \
	done

files: $(TP_BINARIES) rootfs/
	for i in $(TEST_PROGRAMS); do \
		cp ../test_programs/$$i/build/parch/$$i rootfs/$$i; \
	done

rootfs/:
	mkdir -p $@

clean:
	rm -rf rootfs parchfs

parchfs: parchfs.cpp
	g++ -std=gnu++17 -g $^ -o $@

all: parchfs files

.PHONY: clean files $(TP_BINARIES)
