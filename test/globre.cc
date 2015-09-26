//
//  globre.cc
//

#include "sushi.h"

/* main */

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <globre>\n", argv[0]);
		exit(1);
	}

	std::vector<std::string> files = util::globre(argv[1]);
	for (std::string file : files) {
		std::cout << "result: " << file << std::endl;
	}
}
