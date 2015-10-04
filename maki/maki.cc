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
		xcodeproj->write(proj.root);
	} else if (strcmp(argv[2], "vs") == 0) {
		VSSolutionPtr solution = VSSolution::createSolution(proj.root);
		solution->write(proj.root);
	} else if (strcmp(argv[2], "ninja") == 0) {
		NinjaPtr ninja = Ninja::createBuild(proj.root);
		ninja->write(proj.root);
	} else {
		fprintf(stderr, "unknown project format: %s\n", argv[2]);
	}
}
