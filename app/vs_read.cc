//
//  vs_read.cc
//

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
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <mutex>

#include "log.h"
#include "util.h"
#include "visual_studio_parser.h"
#include "visual_studio.h"

/* main */

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <xcodeproj>\n", argv[0]);
		exit(1);
	}
	VSSolution sol;
	sol.read(argv[1]);
	sol.write(std::cout);
}
