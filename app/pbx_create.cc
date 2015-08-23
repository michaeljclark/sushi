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

/* main */

int main(int argc, char **argv) {
	XcodeprojPtr xcodeproj = std::make_shared<Xcodeproj>();
	xcodeproj->createEmptyProject("simple", "macosx");
	xcodeproj->createNativeTarget("static", "libstatic.a",
		                          PBXFileReference::type_library_archive,
		                          PBXNativeTarget::type_library_static,
		                          { },
		                          { "simple/lib.h", "simple/lib.cpp" });
	xcodeproj->createNativeTarget("simple", "simple",
		                          PBXFileReference::type_executable,
		                          PBXNativeTarget::type_tool,
		                          { "libstatic.a" },
		                          { "simple/main.cpp" });
	PBXWriter::write(xcodeproj, std::cout, 0);
	std::cout << std::endl;
}
