//
//  project_visual_studio.cc
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
#include <set>
#include <functional>

#include "sushi.h"

#include "util.h"
#include "visual_studio_parser.h"
#include "visual_studio.h"
#include "project_parser.h"
#include "project.h"
#include "project_visual_studio.h"


/* project_visual_studio */

vs_lib_output_data project_visual_studio::lib_output(project_lib_ptr lib)
{
	if (lib->lib_type == "static") {
		return vs_lib_output_data(
			"StaticLibrary",
			lib->lib_name + ".lib"
		);
	} else {
		return vs_lib_output_data(
			"DynamicLibrary",
			lib->lib_name + ".lib" // DLL import library
		);
	}
}

std::vector<std::string> project_visual_studio::lib_deps(project_root_ptr root, std::vector<std::string> libs)
{
	std::vector<std::string> lib_deps;
	for (auto lib_name : libs) {
		auto lib = root->get_lib(lib_name);
		lib_deps.push_back(lib_output(lib).output_file);
	}
	return lib_deps;
}

VSSolutionPtr project_visual_studio::create_solution(project_root_ptr root)
{
	// construct empty solution
	auto config = root->get_config("*");
	VSSolutionPtr solution = std::make_shared<VSSolution>();
	solution->createEmptySolution(config->vars);

	// create library targets
	for (auto lib_name : root->get_lib_list()) {
		auto lib = root->get_lib(lib_name);
		auto lib_data = lib_output(lib);
		solution->createProject(
			config->vars,
			lib->lib_name,
			lib_data.target_type,
			lib->libs,
			std::vector<std::string>(),
			lib->source
		);
	}

	// create tool targets
	for (auto tool_name : root->get_tool_list()) {
		auto tool = root->get_tool(tool_name);
		solution->createProject(
			config->vars,
			tool->tool_name,
			"Application",
			tool->libs,
			std::vector<std::string>(),
			tool->source
		);
	}

	return solution;
}
