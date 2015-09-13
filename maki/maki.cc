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
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <functional>

#include "tinyxml2.h"

#include "log.h"
#include "util.h"
#include "filesystem.h"
#include "visual_studio_parser.h"
#include "visual_studio.h"
#include "xcode.h"
#include "project_parser.h"
#include "project.h"
#include "project_visual_studio.h"
#include "project_xcode.h"

/* main */

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: %s <project.sushi> (xcode|vstudio)\n", argv[0]);
		exit(1);
	}

	project proj;
	proj.read(argv[1]);

	if (strcmp(argv[2], "xcode") == 0) {
		XcodeprojPtr xcodeproj = project_xcode::create_project(proj.root);
		std::string project_file = proj.root->project_name + ".xcodeproj/project.pbxproj";
		std::cout << "writing " << project_file << std::endl;
		filesystem::make_directories(project_file);
		std::ofstream out(project_file.c_str());
		PBXWriter::write(xcodeproj, out, 0);
		out << "\n";
	} else if (strcmp(argv[2], "vstudio") == 0) {
		VSSolutionPtr sol = project_visual_studio::create_solution(proj.root);
		std::string solution_file = proj.root->project_name + ".vsproj/" + proj.root->project_name + ".sln";
		std::cout << "writing " << solution_file << std::endl;
		filesystem::make_directories(solution_file);
		sol->write(solution_file);
		for (auto project : sol->projects) {
			std::string project_file_path = filesystem::path_relative_to_path(project->path, solution_file);
			std::cout << "writing " << project_file_path << std::endl;
			filesystem::make_directories(project_file_path);
			project->project->write(project_file_path);
		}
	} else {
		std::cerr << "unknown project format: " << argv[2] << std::endl;
	}
}
