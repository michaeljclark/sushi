all: pbx_io

clean:
	rm -f pbx_io

pbx_io: pbx_io.cpp
	c++ -O3 -Wall -Wpedantic -std=c++11 $< -o $@
