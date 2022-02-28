.DEFAULT_GOAL := all
TEST_PROGRAMS := $(notdir $(shell find test_programs/ -maxdepth 1 -mindepth 1 -type d))
TP_EXEC_PARCH = $(join $(addprefix test_programs/, $(addsuffix /build/parch/, $(TEST_PROGRAMS))), $(TEST_PROGRAMS))
TP_EXEC_LINUX = $(join $(addprefix test_programs/, $(addsuffix /build/linux/, $(TEST_PROGRAMS))), $(TEST_PROGRAMS))

UTIL_PROGRAMS := $(notdir $(shell find parch_utils/ -maxdepth 1 -mindepth 1 -type d))
UTIL_EXEC_PARCH = $(join $(addprefix parch_utils/, $(addsuffix /, $(UTIL_PROGRAMS))), $(UTIL_PROGRAMS))

root_fs_parch/: test_programs util_programs
	mkdir -p $@
	mkdir -p $@test/
	@for i in $(TP_EXEC_PARCH); do \
		cp $$i $@test/; \
	done
	@for i in $(UTIL_EXEC_PARCH); do \
		cp $$i $@; \
	done

root_fs_linux/: test_programs
	mkdir -p $@
	@for i in $(TP_EXEC_LINUX); do \
		cp $$i $@; \
	done

test_programs: usrlib
	@for i in $(TEST_PROGRAMS); do \
		make -C test_programs/$$i all; \
	done

util_programs: usrlib
	@for i in $(UTIL_PROGRAMS); do \
		make -C parch_utils/$$i all; \
	done

usrlib:
	@make -C usrlib all

parchfs:
	@make -C parchfs all

all: root_fs_parch/ root_fs_linux/ parchfs

clean:
	@rm -rf root_fs_linux
	@rm -rf root_fs_parch
	@for i in $(TEST_PROGRAMS); do \
		make -C test_programs/$$i clean ; \
	done
	@for i in $(UTIL_PROGRAMS); do \
		make -C parch_utils/$$i clean ; \
	done
	@make -C parchfs clean
	@make -C usrlib clean

.PHONY: all clean test_programs parchfs usrlib util_programs