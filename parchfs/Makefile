clean:
	rm -rf parchfs

parchfs: parchfs.cpp
	g++ -std=gnu++17 -g $^ -o $@

all: parchfs

.PHONY: clean all
