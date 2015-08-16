CFLAGS = -g -O3 -Wall -Wpedantic -std=c++11

all: bin obj bin/pbx_read bin/pbx_create

.phony: bin obj clean

clean: ; rm -rf obj bin

bin: ; mkdir -p bin

obj: ; mkdir -p obj

obj/pbx_io.o: pbx_io.cc pbx_io.h
	c++ -c $(CFLAGS) $< -o $@

obj/pbx_read.o: pbx_read.cc pbx_io.h
	c++ -c $(CFLAGS) $< -o $@

obj/pbx_create.o: pbx_create.cc pbx_io.h
	c++ -c $(CFLAGS) $< -o $@

bin/pbx_read: obj/pbx_read.o obj/pbx_io.o
	c++ $(CFLAGS) $^ -o $@

bin/pbx_create: obj/pbx_create.o obj/pbx_io.o
	c++ $(CFLAGS) $^ -o $@
