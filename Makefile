all: pbx_parser

clean:
	rm -f pbx_parser

pbx_parser: pbx_parser.cpp
	c++ -O3 -Wall -Wpedantic -std=c++11 $< -o $@
