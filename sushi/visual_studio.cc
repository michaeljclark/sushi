//
//  visual_studio.cc
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
#include "visual_studio_parser.h"
#include "visual_studio.h"


/* VSSolution */

const bool VSSolution::debug = false;

const std::string VSSolution::VisualCPPProjectGUID = "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942";

VSSolution::VSSolution()
{
	format_version = "12.00";
}

void VSSolution::read(std::string solution_file)
{
	std::vector<char> buf = util::read_file(solution_file);
	if (!parse(buf.data(), buf.size())) {
		log_fatal_exit("VSSolution: parse error");
	}
}

void VSSolution::write(std::ostream &out)
{
	std::cout << "\xef\xbb\xbf\r\n";
	std::cout << "Microsoft Visual Studio Solution File, Format Version " << format_version << "\r\n";
	if (comment_version.size() > 0) {
		std::cout << "# Visual Studio " << comment_version << "\r\n";
	}
	if (visual_studio_version.size() > 0) {
		std::cout << "VisualStudioVersion = " << visual_studio_version << "\r\n";
	}
	if (minimum_visual_studio_version.size() > 0) {
		std::cout << "MinimumVisualStudioVersion = " << minimum_visual_studio_version << "\r\n";
	}
	for (auto project : projects) {
		std::cout << "Project(\"{" << project->type_guid << "}\") = \"" << project->name
			<< "\", \"" << project->path << "\", \"{" << project->guid << "}\"\r\n";
		if (project->dependencies.size() > 0) {
			std::cout << "\tProjectSection(ProjectDependencies) = postProject\r\n";
			for (auto dependency : project->dependencies) {
				std::cout << "\t\t{" << dependency << "} = {" << dependency << "}\r\n";
			}
			std::cout << "\tEndProjectSection\r\n";
		}
		std::cout << "EndProject\r\n";
	}
	std::cout << "Global\r\n";
	std::cout << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\r\n";
	for (auto config : configurations) {
		std::cout << "\t\t" << config << " = " << config << "\r\n";
	}
	std::cout << "\tEndGlobalSection\r\n";
	std::cout << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\r\n";
	for (auto projectConfig : projectConfigurations) {
		std::cout << "\t\t{" << projectConfig->guid << "}." << projectConfig->config
			<< "." << projectConfig->property << " = " << projectConfig->value << "\r\n";
	}
	std::cout << "\tEndGlobalSection\r\n";
	std::cout << "\tGlobalSection(SolutionProperties) = preSolution\r\n";
	for (auto property : properties) {
		std::cout << "\t\t" << property->name << " = " << property->value << "\r\n";
	}
	std::cout << "\tEndGlobalSection\r\n";
	std::cout << "EndGlobal\r\n";
}

void VSSolution::FormatVersion(const char *value, size_t length)
{
	if (debug) log_debug("FormatVersion: %s", std::string(value, length).c_str());
	format_version = std::string(value, length);
}

void VSSolution::CommentVersion(const char *value, size_t length)
{
	if (debug) log_debug("CommentVersion: %s", std::string(value, length).c_str());
	comment_version = std::string(value, length);
}

void VSSolution::VisualStudioVersion(const char *value, size_t length)
{
	if (debug) log_debug("VisualStudioVersion: %s", std::string(value, length).c_str());
	visual_studio_version = std::string(value, length);
}

void VSSolution::MinimumVisualStudioVersion(const char *value, size_t length)
{
	if (debug) log_debug("MinimumVisualStudioVersion: %s", std::string(value, length).c_str());
	minimum_visual_studio_version = std::string(value, length);
}

void VSSolution::ProjectTypeGUID(const char *value, size_t length)
{
	if (debug) log_debug("ProjectTypeGUID: %s", std::string(value, length).c_str());
	projects.push_back(std::make_shared<VSProject>());
	projects.back()->type_guid = std::string(value, length);
}

void VSSolution::ProjectName(const char *value, size_t length)
{
	if (debug) log_debug("ProjectName: %s", std::string(value, length).c_str());
	projects.back()->name = std::string(value, length);
}

void VSSolution::ProjectPath(const char *value, size_t length)
{
	if (debug) log_debug("ProjectPath: %s", std::string(value, length).c_str());
	projects.back()->path = std::string(value, length);
}

void VSSolution::ProjectGUID(const char *value, size_t length)
{
	if (debug) log_debug("ProjectGUID: %s", std::string(value, length).c_str());
	projects.back()->guid = std::string(value, length);
}

void VSSolution::ProjectDependsGUID(const char *value, size_t length)
{
	if (debug) log_debug("ProjectDependsGUID: %s", std::string(value, length).c_str());
	projects.back()->dependencies.push_back(std::string(value, length));
}

void VSSolution::SolutionConfigPlatform(const char *value, size_t length)
{
	if (debug) log_debug("SolutionConfigPlatform: %s", std::string(value, length).c_str());
	configurations.insert(std::string(value, length));
}

void VSSolution::ProjectConfigPlatformGUID(const char *value, size_t length)
{
	if (debug) log_debug("ProjectConfigPlatformGUID: %s", std::string(value, length).c_str());
	projectConfigurations.push_back(std::make_shared<VSSolutionProjectConfiguration>());
	projectConfigurations.back()->guid = std::string(value, length);
}

void VSSolution::ProjectConfigPlatformConfig(const char *value, size_t length)
{
	if (debug) log_debug("ProjectConfigPlatformConfig: %s", std::string(value, length).c_str());
	projectConfigurations.back()->config = std::string(value, length);
}

void VSSolution::ProjectConfigPlatformProp(const char *value, size_t length)
{
	if (debug) log_debug("ProjectConfigPlatformProp: %s", std::string(value, length).c_str());
	projectConfigurations.back()->property = std::string(value, length);
}

void VSSolution::ProjectConfigPlatformValue(const char *value, size_t length)
{
	if (debug) log_debug("ProjectConfigPlatformValue: %s", std::string(value, length).c_str());
	projectConfigurations.back()->value = std::string(value, length);
}

void VSSolution::SolutionPropertiesKey(const char *value, size_t length)
{
	if (debug) log_debug("SolutionPropertiesKey: %s", std::string(value, length).c_str());
	properties.push_back(std::make_shared<VSSolutionProperty>());
	properties.back()->name = std::string(value, length);
}

void VSSolution::SolutionPropertiesValue(const char *value, size_t length)
{
	if (debug) log_debug("SolutionPropertiesValue: %s", std::string(value, length).c_str());
	properties.back()->value = std::string(value, length);
}

void VSSolution::Done()
{
	if (debug) log_debug("Done");
}

