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
	XcodeprojPtr xcodeproj = std::make_shared<Xcodeproj>();
	xcodeproj->createEmptyProject("simple", "macosx");
	xcodeproj->createNativeTarget("static", "libstatic.a",
		                          PBXFileReference::type_library_archive,
		                          PBXNativeTarget::type_library_static,
		                          { "simple/lib.h", "simple/lib.cpp" });
	/* todo : depend on libstatic.a */
	xcodeproj->createNativeTarget("simple", "simple",
		                          PBXFileReference::type_executable,
		                          PBXNativeTarget::type_tool,
		                          { "simple/main.cpp" });
	PBXWriter::write(xcodeproj, std::cout, 0);
	std::cout << std::endl;
}
