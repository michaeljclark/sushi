//
//  xcode.cc
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

#include "tinyxml2.h"

#include "log.h"
#include "util.h"
#include "visual_studio_parser.h"
#include "visual_studio.h"


/* VSSolution */

void VSSolution::read(std::string solution_file)
{
	std::vector<char> buf = util::read_file(solution_file);
	if (!parse(buf.data(), buf.size())) {
		log_fatal_exit("VSSolution: parse error");
	}
}

void VSSolution::FormatVersion(const char *value, size_t length)
{
	log_debug("FormatVersion: %s", std::string(value, length).c_str());
}

void VSSolution::CommentVersion(const char *value, size_t length)
{
	log_debug("CommentVersion: %s", std::string(value, length).c_str());
}

void VSSolution::VisualStudioVersion(const char *value, size_t length)
{
	log_debug("VisualStudioVersion: %s", std::string(value, length).c_str());
}

void VSSolution::MinimumVisualStudioVersion(const char *value, size_t length)
{
	log_debug("MinimumVisualStudioVersion: %s", std::string(value, length).c_str());
}

void VSSolution::Done()
{
	log_debug("Done");
}

