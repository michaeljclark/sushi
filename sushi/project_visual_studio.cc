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
#include <mutex>
#include <random>

#include "tinyxml2.h"

#include "log.h"
#include "util.h"
#include "filesystem.h"
#include "visual_studio_parser.h"
#include "visual_studio.h"
#include "project_parser.h"
#include "project.h"
#include "project_visual_studio.h"


/* project_visual_studio */

VSSolutionPtr project_visual_studio::create_solution(project_root_ptr root)
{
	VSSolutionPtr solution = std::make_shared<VSSolution>();
	solution->setDefaultVersion();
	solution->createDefaultConfigurations();

	// create library targets
	for (auto lib_name : root->get_lib_list()) {
		auto lib = root->get_lib(lib_name);
		solution->createProject(lib->lib_name, "StaticLibrary", lib->depends, std::vector<std::string>(), lib->source);
	}

	// create tool targets
	for (auto tool_name : root->get_tool_list()) {
		auto tool = root->get_tool(tool_name);
		solution->createProject(tool->tool_name, "Application", tool->depends, tool->libs, tool->source);
	}

	return solution;
}
