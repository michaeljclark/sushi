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
#include <fstream>
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
	for (auto project : projects) {
		std::string project_file_path = filesystem::path_relative_to_path(project->path, solution_file);
		project->project = std::make_shared<VSProject>();
		project->project->read(project_file_path);
	}
}

void VSSolution::write(std::string solution_file)
{
	std::ofstream out(solution_file.c_str());
	out << "\xef\xbb\xbf\r\n";
	out << "Microsoft Visual Studio Solution File, Format Version " << format_version << "\r\n";
	if (comment_version.size() > 0) {
		out << "# Visual Studio " << comment_version << "\r\n";
	}
	if (visual_studio_version.size() > 0) {
		out << "VisualStudioVersion = " << visual_studio_version << "\r\n";
	}
	if (minimum_visual_studio_version.size() > 0) {
		out << "MinimumVisualStudioVersion = " << minimum_visual_studio_version << "\r\n";
	}
	for (auto project : projects) {
		out << "Project(\"{" << project->type_guid << "}\") = \"" << project->name
			<< "\", \"" << project->path << "\", \"{" << project->guid << "}\"\r\n";
		if (project->dependencies.size() > 0) {
			out << "\tProjectSection(ProjectDependencies) = postProject\r\n";
			for (auto dependency : project->dependencies) {
				out << "\t\t{" << dependency << "} = {" << dependency << "}\r\n";
			}
			out << "\tEndProjectSection\r\n";
		}
		out << "EndProject\r\n";
	}
	out << "Global\r\n";
	out << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\r\n";
	for (auto config : configurations) {
		out << "\t\t" << config << " = " << config << "\r\n";
	}
	out << "\tEndGlobalSection\r\n";
	out << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\r\n";
	for (auto projectConfig : projectConfigurations) {
		out << "\t\t{" << projectConfig->guid << "}." << projectConfig->config
			<< "." << projectConfig->property << " = " << projectConfig->value << "\r\n";
	}
	out << "\tEndGlobalSection\r\n";
	out << "\tGlobalSection(SolutionProperties) = preSolution\r\n";
	for (auto property : properties) {
		out << "\t\t" << property->name << " = " << property->value << "\r\n";
	}
	out << "\tEndGlobalSection\r\n";
	out << "EndGlobal\r\n";
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
	projects.push_back(std::make_shared<VSSolutionProject>());
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


/* VSProject */

const std::string VSProject::xmlns = "http://schemas.microsoft.com/developer/msbuild/2003";

std::once_flag VSProject::factoryInit;
std::map<std::string,VSObjectFactoryPtr> VSProject::factoryMap;

void VSProject::init()
{
	std::call_once(factoryInit, [](){
		registerFactory<VSImport>();
		registerFactory<VSImportGroup>();
		registerFactory<VSItemGroup>();
		registerFactory<VSItemDefinitionGroup>();
		registerFactory<VSPropertyGroup>();
		registerFactory<VSProjectConfiguration>();
		registerFactory<VSClCompile>();
		registerFactory<VSClInclude>();
		registerFactory<VSLink>();
	});
}

VSProject::VSProject() : doc(false) {}

void VSProject::read(std::string project_file)
{
	doc.Clear();
	std::vector<char> buf = util::read_file(project_file);
	tinyxml2::XMLError err = doc.Parse(buf.data());
	if (err != tinyxml2::XML_NO_ERROR) {
		log_fatal_exit("VSProject: error reading: %s: xml_error=%d", project_file.c_str(), err);
	}
	xmlToProject();
}

void VSProject::write(std::string project_file)
{
	doc.Clear();
	doc.SetBOM(true);
	projectToXml();
	tinyxml2::XMLError err = doc.SaveFile(project_file.c_str(), false);
	if (err != tinyxml2::XML_NO_ERROR) {
		log_fatal_exit("VSProject: error writing: xml_error=%d",  err);
	}
}

void VSProject::xmlToProject()
{
	tinyxml2::XMLElement *root = doc.RootElement();
	if (!root) log_fatal_exit("root element not present");

	const char* defaultTargets = root->Attribute("DefaultTargets");
	if (defaultTargets) this->defaultTargets = defaultTargets;

	const char* toolsVersion = root->Attribute("ToolsVersion");
	if (toolsVersion) this->toolsVersion = toolsVersion;

	tinyxml2::XMLNode *node = root->FirstChild();
	do {
		if (!node) break;
		tinyxml2::XMLElement *element = node->ToElement();
		if (!element) continue;
		VSObjectPtr object = createObject(element->Name());
		if (!object) continue;
		object->fromXML(element);
		objectList.push_back(object);
	} while ((node = node->NextSibling()));
}

void VSProject::projectToXml()
{
	tinyxml2::XMLDeclaration *decl = doc.NewDeclaration();
	tinyxml2::XMLElement *root = doc.NewElement("Project");
	doc.InsertFirstChild(root);
	doc.InsertFirstChild(decl);
	root->SetAttribute("DefaultTargets", defaultTargets.c_str());
	root->SetAttribute("ToolsVersion", toolsVersion.c_str());
	root->SetAttribute("xmlns", xmlns.c_str());
	for (auto obj : objectList) {
		obj->toXML(root);
	}
}

/* VS classes */

const std::string VSImport::type_name               = "Import";
const std::string VSImportGroup::type_name          = "ImportGroup";
const std::string VSItemGroup::type_name            = "ItemGroup";
const std::string VSItemDefinitionGroup::type_name  = "ItemDefinitionGroup";
const std::string VSPropertyGroup::type_name        = "PropertyGroup";
const std::string VSProjectConfiguration::type_name = "ProjectConfiguration";
const std::string VSClCompile::type_name            = "ClCompile";
const std::string VSClInclude::type_name            = "ClInclude";
const std::string VSLink::type_name                 = "Link";


/* VSImport */

void VSImport::fromXML(tinyxml2::XMLElement *element)
{
	const char* project = element->Attribute("Project");
	if (project) this->project = project;
	const char* condition = element->Attribute("Condition");
	if (condition) this->condition = condition;
	const char* label = element->Attribute("Label");
	if (label) this->label = label;
}

void VSImport::toXML(tinyxml2::XMLElement *parent)
{
	tinyxml2::XMLElement *element = parent->GetDocument()->NewElement(type_name.c_str());
	parent->InsertEndChild(element);

	if (project.length() > 0) {
		element->SetAttribute("Project", project.c_str());
	}
	if (condition.length() > 0) {
		element->SetAttribute("Condition", condition.c_str());
	}
	if (label.length() > 0) {
		element->SetAttribute("Label", label.c_str());
	}
}


/* VSImportGroup */

void VSImportGroup::fromXML(tinyxml2::XMLElement *element)
{
	const char* label = element->Attribute("Label");
	if (label) this->label = label;
	const char* condition = element->Attribute("Condition");
	if (condition) this->condition = condition;

	tinyxml2::XMLNode *node = element->FirstChild();
	do {
		if (!node) break;
		tinyxml2::XMLElement *element = node->ToElement();
		if (!element) continue;
		VSObjectPtr object = VSProject::createObject(element->Name());
		if (!object) continue;
		object->fromXML(element);
		objectList.push_back(object);
	} while ((node = node->NextSibling()));
}

void VSImportGroup::toXML(tinyxml2::XMLElement *parent)
{
	tinyxml2::XMLElement *element = parent->GetDocument()->NewElement(type_name.c_str());
	parent->InsertEndChild(element);

	if (label.length() > 0) {
		element->SetAttribute("Label", label.c_str());
	}
	if (condition.length() > 0) {
		element->SetAttribute("Condition", condition.c_str());
	}
	for (VSObjectPtr object : objectList) {
		object->toXML(element);
	}
}


/* VSItemGroup */

void VSItemGroup::fromXML(tinyxml2::XMLElement *element)
{
	const char* label = element->Attribute("Label");
	if (label) this->label = label;

	tinyxml2::XMLNode *node = element->FirstChild();
	do {
		if (!node) break;
		tinyxml2::XMLElement *element = node->ToElement();
		if (!element) continue;
		VSObjectPtr object = VSProject::createObject(element->Name());
		if (!object) continue;
		object->fromXML(element);
		objectList.push_back(object);
	} while ((node = node->NextSibling()));
}

void VSItemGroup::toXML(tinyxml2::XMLElement *parent)
{
	tinyxml2::XMLElement *element = parent->GetDocument()->NewElement(type_name.c_str());
	parent->InsertEndChild(element);

	if (label.length() > 0) {
		element->SetAttribute("Label", label.c_str());
	}
	for (VSObjectPtr object : objectList) {
		object->toXML(element);
	}
}


/* VSItemDefinitionGroup */

void VSItemDefinitionGroup::fromXML(tinyxml2::XMLElement *element)
{
	const char* condition = element->Attribute("Condition");
	if (condition) this->condition = condition;

	tinyxml2::XMLNode *node = element->FirstChild();
	do {
		if (!node) break;
		tinyxml2::XMLElement *element = node->ToElement();
		if (!element) continue;
		VSObjectPtr object = VSProject::createObject(element->Name());
		if (!object) continue;
		object->fromXML(element);
		objectList.push_back(object);
	} while ((node = node->NextSibling()));
}

void VSItemDefinitionGroup::toXML(tinyxml2::XMLElement *parent)
{
	tinyxml2::XMLElement *element = parent->GetDocument()->NewElement(type_name.c_str());
	parent->InsertEndChild(element);

	if (condition.length() > 0) {
		element->SetAttribute("Condition", condition.c_str());
	}
	for (VSObjectPtr object : objectList) {
		object->toXML(element);
	}
}


/* VSPropertyGroup */

void VSPropertyGroup::fromXML(tinyxml2::XMLElement *element)
{
	const char* condition = element->Attribute("Condition");
	if (condition) this->condition = condition;
	const char* label = element->Attribute("Label");
	if (label) this->label = label;

	tinyxml2::XMLNode *node = element->FirstChild();
	do {
		if (!node) break;
		tinyxml2::XMLElement *element = node->ToElement();
		if (!element) continue;
		std::string key = element->Name();
		std::string value = element->GetText();
		properties[key] = value;
	} while ((node = node->NextSibling()));
}

void VSPropertyGroup::toXML(tinyxml2::XMLElement *parent)
{
	tinyxml2::XMLElement *element = parent->GetDocument()->NewElement(type_name.c_str());
	parent->InsertEndChild(element);

	if (condition.length() > 0) {
		element->SetAttribute("Condition", condition.c_str());
	}
	if (label.length() > 0) {
		element->SetAttribute("Label", label.c_str());
	}

	for (auto ent : properties) {
		const std::string &key = ent.first;
		const std::string &value = ent.second;
		tinyxml2::XMLElement *propertyElement = parent->GetDocument()->NewElement(key.c_str());
		propertyElement->SetText(value.c_str());
		element->InsertEndChild(propertyElement);
	}
}


/* VSProjectConfiguration */

void VSProjectConfiguration::fromXML(tinyxml2::XMLElement *element)
{
	const char* include = element->Attribute("Include");
	if (include) this->include = include;

	tinyxml2::XMLNode *node = element->FirstChild();
	do {
		if (!node) break;
		tinyxml2::XMLElement *element = node->ToElement();
		if (!element) continue;
		if (strcmp(element->Name(), "Configuration") == 0) {
			configuration = element->GetText();
		} else if (strcmp(element->Name(), "Platform") == 0) {
			platform = element->GetText();
		}
	} while ((node = node->NextSibling()));
}

void VSProjectConfiguration::toXML(tinyxml2::XMLElement *parent)
{
	tinyxml2::XMLElement *element = parent->GetDocument()->NewElement(type_name.c_str());
	parent->InsertEndChild(element);

	if (include.length() > 0) {
		element->SetAttribute("Include", include.c_str());
	}
	if (configuration.length() > 0) {
		tinyxml2::XMLElement *configurationElement = parent->GetDocument()->NewElement("Configuration");
		configurationElement->SetText(configuration.c_str());
		element->InsertEndChild(configurationElement);
	}
	if (platform.length() > 0) {
		tinyxml2::XMLElement *platformElement = parent->GetDocument()->NewElement("Platform");
		platformElement->SetText(platform.c_str());
		element->InsertEndChild(platformElement);
	}
}


/* VSClCompile */

void VSClCompile::fromXML(tinyxml2::XMLElement *element)
{
	const char* include = element->Attribute("Include");
	if (include) this->include = include;

	tinyxml2::XMLNode *node = element->FirstChild();
	do {
		if (!node) break;
		tinyxml2::XMLElement *element = node->ToElement();
		if (!element) continue;
		std::string key = element->Name();
		std::string value = element->GetText();
		properties[key] = value;
	} while ((node = node->NextSibling()));
}

void VSClCompile::toXML(tinyxml2::XMLElement *parent)
{
	tinyxml2::XMLElement *element = parent->GetDocument()->NewElement(type_name.c_str());
	parent->InsertEndChild(element);

	if (include.length() > 0) {
		element->SetAttribute("Include", include.c_str());
	}

	for (auto ent : properties) {
		const std::string &key = ent.first;
		const std::string &value = ent.second;
		tinyxml2::XMLElement *propertyElement = parent->GetDocument()->NewElement(key.c_str());
		propertyElement->SetText(value.c_str());
		element->InsertEndChild(propertyElement);
	}
}


/* VSClInclude */

void VSClInclude::fromXML(tinyxml2::XMLElement *element)
{
	const char* include = element->Attribute("Include");
	if (include) this->include = include;
}

void VSClInclude::toXML(tinyxml2::XMLElement *parent)
{
	tinyxml2::XMLElement *element = parent->GetDocument()->NewElement(type_name.c_str());
	parent->InsertEndChild(element);

	if (include.length() > 0) {
		element->SetAttribute("Include", include.c_str());
	}
}


/* VSLink */

void VSLink::fromXML(tinyxml2::XMLElement *element)
{
	tinyxml2::XMLNode *node = element->FirstChild();
	do {
		if (!node) break;
		tinyxml2::XMLElement *element = node->ToElement();
		if (!element) continue;
		std::string key = element->Name();
		std::string value = element->GetText();
		properties[key] = value;
	} while ((node = node->NextSibling()));
}

void VSLink::toXML(tinyxml2::XMLElement *parent)
{
	tinyxml2::XMLElement *element = parent->GetDocument()->NewElement(type_name.c_str());
	parent->InsertEndChild(element);

	for (auto ent : properties) {
		const std::string &key = ent.first;
		const std::string &value = ent.second;
		tinyxml2::XMLElement *propertyElement = parent->GetDocument()->NewElement(key.c_str());
		propertyElement->SetText(value.c_str());
		element->InsertEndChild(propertyElement);
	}
}
