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

lib_output_data project_xcode::lib_output(project_root_ptr root, project_lib_ptr lib)
{
	std::string lib_type;
	for (auto lib : root->lib_list) {
		if (lib->lib_name == "*") {
			lib_type = lib->lib_type;
		}
	}
	if (lib->lib_type.length() > 0) {
		lib_type = lib->lib_type;
	}
	if (lib_type == "static") {
		return lib_output_data{ PBXFileReference::type_library_archive,
								PBXNativeTarget::type_library_static,
								"lib" + lib->lib_name + ".a"};
	} else {
		return lib_output_data{ PBXFileReference::type_library_dylib,
								PBXNativeTarget::type_library_dynamic,
								lib->lib_name + ".dylib"};
	}
}

std::vector<std::string> project_xcode::lib_deps(project_root_ptr root, std::vector<std::string> libs)
{
	std::vector<std::string> lib_deps;
	for (auto lib_name : libs) {
		auto li = std::find_if(root->lib_list.begin(), root->lib_list.end(),
							   [&lib_name](project_lib_ptr const& lib) { return lib->lib_name == lib_name; });
		if (li != root->lib_list.end()) {
			lib_deps.push_back(lib_output(root, *li).output_file);
		} else {
			log_fatal_exit("can't find library definition for '%s'", lib_name.c_str());
		}
	}
	return lib_deps;
}

XcodeprojPtr project_xcode::create_project(project_root_ptr root)
{
	XcodeprojPtr xcodeproj = std::make_shared<Xcodeproj>();

	// construct Xcode project
	xcodeproj->createEmptyProject(root->project_name, "macosx");

	// create libs
	for (auto lib : root->lib_list) {
		if (lib->lib_name == "*") continue;
		auto lib_data = lib_output(root, lib);
		xcodeproj->createNativeTarget(lib->lib_name, lib_data.output_file,
									lib_data.file_type,
									lib_data.target_type,
									{ }, lib->source);
	}

	// create tools
	std::vector<std::string> base_libs;
	for (auto tool : root->tool_list) {
		if (tool->tool_name == "*") {
			base_libs = lib_deps(root, tool->libs);
			break;
		}
	}
	for (auto tool : root->tool_list) {
		if (tool->tool_name == "*") continue;
		std::vector<std::string> tool_libs = lib_deps(root, tool->libs);
		tool_libs.insert(tool_libs.end(), base_libs.begin(), base_libs.end());
		xcodeproj->createNativeTarget(tool->tool_name, tool->tool_name,
									PBXFileReference::type_executable,
									PBXNativeTarget::type_tool,
									tool_libs, tool->source);
	}

	return xcodeproj;
}