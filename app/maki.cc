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
#include "project_xcode.h"

/* main */

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <makiproj>\n", argv[0]);
		exit(1);
	}

	project proj;
	proj.read(argv[1]);
	XcodeprojPtr xcodeproj = project_xcode::create_project(proj.root);
	PBXWriter::write(xcodeproj, std::cout, 0);
	std::cout << std::endl;
}
