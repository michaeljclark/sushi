#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdint>
#include <cerrno>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <random>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "pbx_io.h"

/* main */

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <xcodeproj>\n", argv[0]);
		exit(1);
	}
	PBXParserImpl pbx;
	std::vector<char> buf = PBXUtil::read_file(argv[1]);
	PBXParseError error = pbx.parse(buf);
	if (error != PBXParseErrorNone) {
		log_fatal_exit("error parsing project: %d", error);
	}

	PBXWriter::write(pbx.xcodeproj, std::cout, 0);
	std::cout << std::endl;
}
