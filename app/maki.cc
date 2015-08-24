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
#include <mutex>

#include "log.h"
#include "util.h"
#include "xcode.h"
#include "project_parser.h"
#include "project.h"

/* main */

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <xcodeproj>\n", argv[0]);
		exit(1);
	}

	project proj;
	proj.read(argv[1]);
}
