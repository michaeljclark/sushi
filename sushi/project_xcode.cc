//
//  project_xcode.cc
//

#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdint>
#include <ctime>
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

#include "log.h"
#include "util.h"
#include "xcode.h"
#include "project_parser.h"
#include "project.h"
#include "project_xcode.h"


/* project_xcode */

xcode_lib_output_data project_xcode::lib_output(project_lib_ptr lib)
{
	if (lib->lib_type == "static") {
		return xcode_lib_output_data(
			PBXFileReference::type_library_archive,
			PBXNativeTarget::type_library_static,
			"lib" + lib->lib_name + ".a"
		);
	} else {
		return xcode_lib_output_data(
			PBXFileReference::type_library_dylib,
			PBXNativeTarget::type_library_dynamic,
			lib->lib_name + ".dylib"
		);
	}
}

std::vector<std::string> project_xcode::lib_deps(project_root_ptr root, std::vector<std::string> libs)
{
	std::vector<std::string> lib_deps;
	for (auto lib_name : libs) {
		auto lib = root->get_lib(lib_name);
		lib_deps.push_back(lib_output(lib).output_file);
	}
	return lib_deps;
}

XcodeprojPtr project_xcode::create_project(project_root_ptr root)
{
	XcodeprojPtr xcodeproj = std::make_shared<Xcodeproj>();

	// construct empty Xcode project
	xcodeproj->createEmptyProject(root->project_name, "macosx");

	// create library targets
	for (auto lib_name : root->get_lib_list()) {
		auto lib = root->get_lib(lib_name);
		auto lib_data = lib_output(lib);
		xcodeproj->createNativeTarget(
			lib->lib_name,
			lib_data.output_file,
			lib_data.file_type,
			lib_data.target_type,
			std::vector<std::string>(),
			lib->source);
	}

	// create tool targets
	for (auto tool_name : root->get_tool_list()) {
		auto tool = root->get_tool(tool_name);
		xcodeproj->createNativeTarget(
			tool->tool_name,
			tool->tool_name,
			PBXFileReference::type_executable,
			PBXNativeTarget::type_tool,
			lib_deps(root, tool->libs),
			tool->source);
	}

	return xcodeproj;
}
