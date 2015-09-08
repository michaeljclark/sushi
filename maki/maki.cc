//
//  maki.cc
//

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdint>
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

int main(int argc, char **argv)
{
	// TODO - parse command line options
	if (argc != 2) {
		fprintf(stderr, "usage: %s <sushi>\n", argv[0]);
		exit(1);
	}
	// TODO - currently hardcoded to produce Xcode project
	project proj;
	proj.read(argv[1]);
	XcodeprojPtr xcodeproj = project_xcode::create_project(proj.root);
	PBXWriter::write(xcodeproj, std::cout, 0);
	std::cout << std::endl;
}
