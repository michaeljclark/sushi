all: bin bin/pbx_io

.phony: bin clean

clean:
	rm -rf bin

bin:
	mkdir -p bin

bin/pbx_io: pbx_io.cpp
	c++ -g -O3 -Wall -Wpedantic -std=c++11 $< -o $@
