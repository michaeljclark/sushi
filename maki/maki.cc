//
//  maki.cc
//

#include "sushi.h"

/* main */

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: %s <project.sushi> (xcode|vs)\n", argv[0]);
		exit(1);
	}

	project proj;
	proj.read(argv[1]);

	if (strcmp(argv[2], "xcode") == 0) {
		XcodeprojPtr xcodeproj = Xcodeproj::createProject(proj.root);
		std::string project_file = proj.root->project_name + ".xcodeproj/project.pbxproj";
		std::cout << "writing " << project_file << std::endl;
		util::make_directories(project_file);
		std::ofstream out(project_file.c_str());
		PBXWriter::write(xcodeproj, out, 0);
		out << "\n";
	} else if (strcmp(argv[2], "vs") == 0) {
		VSSolutionPtr sol = VSSolution::createSolution(proj.root);
		std::string solution_file = proj.root->project_name + ".vsproj/" + proj.root->project_name + ".sln";
		std::cout << "writing " << solution_file << std::endl;
		util::make_directories(solution_file);
		sol->write(solution_file);
		for (auto project : sol->projects) {
			std::string project_file_path = util::path_relative_to_path(project->path, solution_file);
			std::cout << "writing " << project_file_path << std::endl;
			util::make_directories(project_file_path);
			project->project->write(project_file_path);
		}
	} else {
		std::cerr << "unknown project format: " << argv[2] << std::endl;
	}
}
