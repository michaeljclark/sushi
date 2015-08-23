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

#include "log.h"
#include "util.h"
#include "pbx.h"
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