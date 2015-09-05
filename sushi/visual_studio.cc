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

void VSSolution::ProjectTypeGUID(const char *value, size_t length)
{
	log_debug("ProjectTypeGUID: %s", std::string(value, length).c_str());
}

void VSSolution::ProjectName(const char *value, size_t length)
{
	log_debug("ProjectName: %s", std::string(value, length).c_str());
}

void VSSolution::ProjectPath(const char *value, size_t length)
{
	log_debug("ProjectPath: %s", std::string(value, length).c_str());
}

void VSSolution::ProjectGUID(const char *value, size_t length)
{
	log_debug("ProjectGUID: %s", std::string(value, length).c_str());
}

void VSSolution::ProjectDependsGUID(const char *value, size_t length)
{
	log_debug("ProjectDependsGUID: %s", std::string(value, length).c_str());
}

void VSSolution::SolutionConfigPlatformKey(const char *value, size_t length)
{
	log_debug("SolutionConfigPlatformKey: %s", std::string(value, length).c_str());
}

void VSSolution::SolutionConfigPlatformValue(const char *value, size_t length)
{
	log_debug("SolutionConfigPlatformValue: %s", std::string(value, length).c_str());
}

void VSSolution::ProjectConfigPlatformGUID(const char *value, size_t length)
{
	log_debug("ProjectConfigPlatformGUID: %s", std::string(value, length).c_str());
}

void VSSolution::ProjectConfigPlatformConfig(const char *value, size_t length)
{
	log_debug("ProjectConfigPlatformConfig: %s", std::string(value, length).c_str());
}

void VSSolution::ProjectConfigPlatformProp(const char *value, size_t length)
{
	log_debug("ProjectConfigPlatformProp: %s", std::string(value, length).c_str());
}

void VSSolution::ProjectConfigPlatformValue(const char *value, size_t length)
{
	log_debug("ProjectConfigPlatformValue: %s", std::string(value, length).c_str());
}

void VSSolution::SolutionPropertiesKey(const char *value, size_t length)
{
	log_debug("SolutionPropertiesKey: %s", std::string(value, length).c_str());
}

void VSSolution::SolutionPropertiesValue(const char *value, size_t length)
{
	log_debug("SolutionPropertiesValue: %s", std::string(value, length).c_str());
}
void VSSolution::Done()
{
	log_debug("Done");
}

