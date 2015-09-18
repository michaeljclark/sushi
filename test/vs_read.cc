//
//  vs_read.cc
//

#include "sushi.h"

/* main */

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <vssolution>\n", argv[0]);
		exit(1);
	}
	std::string solution_file = argv[1];
	VSSolution sol;
	sol.read(solution_file);

	std::string new_solution_file = solution_file + ".new";
	std::cout << "writing " << new_solution_file << std::endl;
	sol.write(new_solution_file);
	for (auto project : sol.projects) {
		std::string new_project_file_path = util::path_relative_to_path(project->path, solution_file) + ".new";
		std::cout << "writing " << new_project_file_path << std::endl;
		project->project->write(new_project_file_path);
	}
}
