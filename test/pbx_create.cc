//
//  pbx_create.cc
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

/* main */

int main(int argc, char **argv) {
	XcodeprojPtr xcodeproj = std::make_shared<Xcodeproj>();
	xcodeproj->createEmptyProject("simple", "macosx");
	std::vector<std::string> simple_lib_source;
	simple_lib_source.push_back("simple/lib.h");
	simple_lib_source.push_back("simple/lib.cpp");
	xcodeproj->createNativeTarget("static", "libstatic.a",
		                          PBXFileReference::type_library_archive,
		                          PBXNativeTarget::type_library_static,
		                          std::vector<std::string>(),
		                          simple_lib_source);
	std::vector<std::string> simple_lib_deps;
	simple_lib_deps.push_back("libstatic.a");
	std::vector<std::string> simple_tool_source;
	simple_tool_source.push_back("simple/main.cpp");
	xcodeproj->createNativeTarget("simple", "simple",
		                          PBXFileReference::type_executable,
		                          PBXNativeTarget::type_tool,
		                          simple_lib_deps,
		                          simple_tool_source);
	PBXWriter::write(xcodeproj, std::cout, 0);
	std::cout << std::endl;
}
