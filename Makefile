CFLAGS = -g -O3 -Wall -Wpedantic -std=c++11 -Isushi -Ipbx_io

all: bin obj bin/pbx_read bin/pbx_create

.phony: bin obj clean

clean: ; rm -rf obj bin

bin: ; mkdir -p bin

obj: ; mkdir -p obj

obj/log.o: sushi/log.cc sushi/log.h ; c++ -c $(CFLAGS) $< -o $@

obj/util.o: sushi/util.cc sushi/util.h ; c++ -c $(CFLAGS) $< -o $@

obj/project.o: sushi/project.cc sushi/project.h sushi/project_parser.h sushi/log.h sushi/util.h ; c++ -c $(CFLAGS) $< -o $@

obj/project_parser.o: sushi/project_parser.cc sushi/project_parser.h ; c++ -c $(CFLAGS) $< -o $@

obj/pbx.o: sushi/pbx.cc sushi/pbx.h sushi/log.h sushi/util.h ; c++ -c $(CFLAGS) $< -o $@

obj/pbx_read.o: pbx_read/pbx_read.cc sushi/pbx.h sushi/log.h sushi/util.h ; c++ -c $(CFLAGS) $< -o $@

obj/pbx_create.o: pbx_create/pbx_create.cc sushi/pbx.h sushi/log.h sushi/util.h ; c++ -c $(CFLAGS) $< -o $@

bin/pbx_read: obj/pbx_read.o obj/pbx.o obj/log.o obj/util.o ; c++ $(CFLAGS) $^ -o $@

bin/pbx_create: obj/pbx_create.o obj/pbx.o obj/log.o obj/util.o ; c++ $(CFLAGS) $^ -o $@
