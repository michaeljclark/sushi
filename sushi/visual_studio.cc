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

#include "tinyxml2.h"

#include "sushi.h"

#include "util.h"
#include "project_parser.h"
#include "project.h"
#include "visual_studio_parser.h"
#include "visual_studio.h"


/* VSSolution */

const bool VSSolution::debug = false;

const std::string VSSolution::VisualCPPProjectGUID = "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942";

VSSolution::VSSolution() {}

VSSolutionPtr VSSolution::createSolution(project_root_ptr root)
{
	// construct empty solution
	auto config = root->get_config("*");
	VSSolutionPtr solution = std::make_shared<VSSolution>();
	solution->createEmptySolution(root, config->vars);

	// create library targets
	for (auto lib_name : root->get_lib_list()) {
		auto lib = root->get_lib(lib_name);
		solution->createProject(
			root,
			config->vars,
			lib->lib_name,
			lib->lib_type == "static" ? "StaticLibrary" : "DynamicLibrary",
			lib->libs,
			std::vector<std::string>(),
			std::vector<std::string>(),
			std::vector<std::string>(),
			util::globre_list(lib->source)
		);
	}

	// create tool targets
	for (auto tool_name : root->get_tool_list()) {
		auto tool = root->get_tool(tool_name);
		solution->createProject(
			root,
			config->vars,
			tool->tool_name,
			"Application",
			tool->libs,
			std::vector<std::string>(),
			std::vector<std::string>(),
			std::vector<std::string>(),
			util::globre_list(tool->source)
		);
	}

	return solution;
}

void VSSolution::createEmptySolution(project_root_ptr root, std::map<std::string,std::string> vars)
{
	format_version = "12.00";
	comment_version = "14";
	visual_studio_version = "14.0.23107.0";
	minimum_visual_studio_version = "10.0.40219.1";

	// Create Build Configurations
	configurations.clear();
	for (auto config_name : root->get_config_list()) {
		auto config = root->get_config(config_name);
		configurations.insert(config_name + "|x64");
		configurations.insert(config_name + "|Win32");
	}

	auto hideSolutionNodeProperty = std::make_shared<VSSolutionProperty>();
	hideSolutionNodeProperty->name = "HideSolutionNode";
	hideSolutionNodeProperty->value = "FALSE";
	properties.clear();
	properties.push_back(hideSolutionNodeProperty);
}

VSProjectPtr VSSolution::createProject(project_root_ptr root, std::map<std::string,std::string> vars,
	std::string project_name, std::string project_type,
	std::vector<std::string> depends,
	std::vector<std::string> defines,
	std::vector<std::string> lib_dirs,
	std::vector<std::string> lib_files,
	std::vector<std::string> source)
{
	// find deployment target and sdk
	std::string platformToolset = "v110";
	std::string platformVersion = "8.1";
	auto platformToolset_i = vars.find("x_ms_platform_toolset");
	auto platformVersion_i = vars.find("x_ms_platform_version");
	if (platformToolset_i != vars.end()) platformToolset = platformToolset_i->second;
	if (platformVersion_i != vars.end()) platformVersion = platformVersion_i->second;

	VSSolutionProjectPtr solutionProject = std::make_shared<VSSolutionProject>();

	uuid project_uuid;
	util::generate_uuid(project_uuid);

	solutionProject->type_guid = VSSolution::VisualCPPProjectGUID;
	solutionProject->name = project_name;
	solutionProject->path = project_name + "\\" + project_name + ".vcxproj";
	solutionProject->guid = util::format_uuid(project_uuid);
	for (std::string dependency : depends) {
		if (std::find(solutionProject->dependenciesToResolve.begin(), solutionProject->dependenciesToResolve.end(),
				dependency) == solutionProject->dependenciesToResolve.end()) {
			solutionProject->dependenciesToResolve.push_back(dependency);
		}
	}
	projects.push_back(solutionProject);

	VSProjectPtr project = std::make_shared<VSProject>();
	project->toolsVersion = "14.0";
	project->defaultTargets = "Build";
	solutionProject->project = project;

	VSItemGroupPtr projectConfigItemGroup = std::make_shared<VSItemGroup>();
	projectConfigItemGroup->label = "ProjectConfigurations";
	project->objectList.push_back(projectConfigItemGroup);

	for (auto config : configurations) {
		VSProjectConfigurationPtr projectConfig = legacyConfig(config);

		VSSolutionProjectConfigurationPtr activeConfig = std::make_shared<VSSolutionProjectConfiguration>();
		activeConfig->guid = solutionProject->guid;
		activeConfig->config = config;
		activeConfig->property = "ActiveCfg";
		activeConfig->value = projectConfig->include;
		projectConfigurations.push_back(activeConfig);

		VSSolutionProjectConfigurationPtr build0Config = std::make_shared<VSSolutionProjectConfiguration>();
		build0Config->guid = solutionProject->guid;
		build0Config->config = config;
		build0Config->property = "Build.0";
		build0Config->value = projectConfig->include;
		projectConfigurations.push_back(build0Config);

		projectConfigItemGroup->objectList.push_back(projectConfig);
	}

	VSPropertyGroupPtr globalProperties = std::make_shared<VSPropertyGroup>();
	globalProperties->label = "Globals";
	globalProperties->properties["ProjectGuid"] = std::string("{") + solutionProject->guid + std::string("}");
	globalProperties->properties["RootNamespace"] = project_name;
	globalProperties->properties["WindowsTargetPlatformVersion"] = platformVersion;
	project->objectList.push_back(globalProperties);

	VSImportPtr defaultsImport = std::make_shared<VSImport>();
	defaultsImport->project = "$(VCTargetsPath)\\Microsoft.Cpp.Default.props";
	project->objectList.push_back(defaultsImport);

	for (auto config_name : configurations) {
		VSProjectConfigurationPtr projectConfig = legacyConfig(config_name);
		auto config = root->get_config(projectConfig->configuration);
		std::string optimizationLevel = "3";
		auto optimizationLevel_i = config->vars.find("optimization");
		if (optimizationLevel_i != config->vars.end()) optimizationLevel = optimizationLevel_i->second;

		VSPropertyGroupPtr propertyGroup = std::make_shared<VSPropertyGroup>();
		propertyGroup->label = "Configuration";
		propertyGroup->condition = format_string("'$(Configuration)|$(Platform)'=='%s'", projectConfig->include.c_str());
		propertyGroup->properties["ConfigurationType"] = project_type;
		propertyGroup->properties["PlatformToolset"] = platformToolset;
		propertyGroup->properties["CharacterSet"] = "MultiByte";
		if (optimizationLevel == "0") {
			propertyGroup->properties["UseDebugLibraries"] = "true"; // TODO - use different property
		} else if (optimizationLevel == "3") {
			propertyGroup->properties["WholeProgramOptimization"] = "true";
			propertyGroup->properties["UseDebugLibraries"] = "false"; // TODO - use different property
		}
		project->objectList.push_back(propertyGroup);
	}

	VSImportPtr cppImport = std::make_shared<VSImport>();
	cppImport->project = "$(VCTargetsPath)\\Microsoft.Cpp.props";
	project->objectList.push_back(cppImport);

	VSImportGroupPtr extensionSettingsImportGroup = std::make_shared<VSImportGroup>();
	extensionSettingsImportGroup->label = "ExtensionSettings";
	project->objectList.push_back(extensionSettingsImportGroup);

	VSImportGroupPtr sharedImportGroup = std::make_shared<VSImportGroup>();
	sharedImportGroup->label = "Shared";
	project->objectList.push_back(sharedImportGroup);

	for (auto config_name : configurations) {
		VSProjectConfigurationPtr projectConfig = legacyConfig(config_name);
		VSImportGroupPtr userConfigImportGroup = std::make_shared<VSImportGroup>();
		userConfigImportGroup->label = "PropertySheets";
		userConfigImportGroup->condition = format_string("'$(Configuration)|$(Platform)'=='%s'", projectConfig->include.c_str());
		VSImportPtr userConfigImport = std::make_shared<VSImport>();
		userConfigImport->project = "$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props";
		userConfigImport->condition = "exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')";
		userConfigImport->label = "LocalAppDataPlatform";
		userConfigImportGroup->objectList.push_back(userConfigImport);
		project->objectList.push_back(userConfigImportGroup);
	}

	VSPropertyGroupPtr userMacros = std::make_shared<VSPropertyGroup>();
	userMacros->label = "UserMacros";
	project->objectList.push_back(userMacros);

	VSPropertyGroupPtr empty = std::make_shared<VSPropertyGroup>();
	project->objectList.push_back(empty);

	std::string additionalIncludes;
	for (auto dependency : depends) {
		// NOTE - this works because the sushi convention is that the library
		//        directory name is the same as the library name
		// TODO - use export_includes
		if (additionalIncludes.size() > 0) additionalIncludes.append(";");
		additionalIncludes.append(format_string("$(ProjectDir)\\..\\..\\%s", dependency.c_str()));
	}
	
	std::string additionalLibraryDirectories;
	for (auto lib_dir : lib_dirs) {
		if (additionalLibraryDirectories.size() > 0) additionalLibraryDirectories.append(";");
		additionalLibraryDirectories.append(lib_dir);
	}

	std::string additionalDependencies;
	for (auto lib_file : lib_files) {
		if (additionalDependencies.size() > 0) additionalDependencies.append(";");
		additionalDependencies.append(lib_file);
	}

	for (auto config_name : configurations) {
		VSProjectConfigurationPtr projectConfig = legacyConfig(config_name);
		auto config = root->get_config(projectConfig->configuration);
		std::string optimizationLevel = "3";
		auto optimizationLevel_i = config->vars.find("optimization");
		if (optimizationLevel_i != config->vars.end()) optimizationLevel = optimizationLevel_i->second;

		std::string preprocessorDefinitions;
		for (auto define : config->defines) {
			if (preprocessorDefinitions.size() > 0) preprocessorDefinitions.append(";");
			preprocessorDefinitions.append(define);
		}
		
		// TODO - target specific defines

		VSItemDefinitionGroupPtr compileAndLink = std::make_shared<VSItemDefinitionGroup>();
		compileAndLink->condition = format_string("'$(Configuration)|$(Platform)'=='%s'", projectConfig->include.c_str());

		VSClCompilePtr compile = std::make_shared<VSClCompile>();
		if (preprocessorDefinitions.size() > 0) {
			compile->properties["PreprocessorDefinitions"] = preprocessorDefinitions + ";%(PreprocessorDefinitions)";
		}
		if (additionalIncludes.size() > 0) {
			compile->properties["AdditionalIncludeDirectories"] = additionalIncludes + ";%(AdditionalIncludeDirectories)";
		}
		if (optimizationLevel == "0") {
			compile->properties["Optimization"] = "Disabled";
		} else if (optimizationLevel == "3") {
			compile->properties["Optimization"] = "MaxSpeed";
			compile->properties["IntrinsicFunctions"] = "true";
		}
		compileAndLink->objectList.push_back(compile);

		VSLinkPtr link = std::make_shared<VSLink>();
		link->properties["GenerateDebugInformation"] = "true";
		if (additionalLibraryDirectories.size() > 0) {
			link->properties["AdditionalLibraryDirectories"] = additionalLibraryDirectories + ";%(AdditionalLibraryDirectories)";
		}
		if (additionalDependencies.size() > 0) {
			link->properties["AdditionalDependencies"] = additionalDependencies + ";%(AdditionalDependencies)";
		}
		if (optimizationLevel == "3") {
			link->properties["EnableCOMDATFolding"] = "true";
			link->properties["OptimizeReferences"] = "true";
		}
		compileAndLink->objectList.push_back(link);

		project->objectList.push_back(compileAndLink);
	}

	// TODO - lookup file extension metadata
	project->headerItemGroup = std::make_shared<VSItemGroup>();
	for (std::string source_file : source) {
		if (source_file.find(".h") == source_file.length() - 2)
		{
			std::vector<std::string> comps = util::path_components(source_file);
			comps.insert(comps.begin(), "..");
			comps.insert(comps.begin(), "..");
			VSClIncludePtr include = std::make_shared<VSClInclude>();
			include->include = util::join(comps, "\\");
			project->headerItemGroup->objectList.push_back(include);
		}
	}
	project->objectList.push_back(project->headerItemGroup);

	// TODO - lookup file extension metadata
	project->sourceItemGroup = std::make_shared<VSItemGroup>();
	for (std::string source_file : source) {
		if (source_file.find(".cc") == source_file.length() - 3 ||
				source_file.find(".cpp") == source_file.length() - 4)
		{
			std::vector<std::string> comps = util::path_components(source_file);
			comps.insert(comps.begin(), "..");
			comps.insert(comps.begin(), "..");
			VSClCompilePtr compile = std::make_shared<VSClCompile>();
			compile->include = util::join(comps, "\\");
			project->sourceItemGroup->objectList.push_back(compile);
		}
	}
	project->objectList.push_back(project->sourceItemGroup);

	project->dependsItemGroup = std::make_shared<VSItemGroup>();
	project->objectList.push_back(project->dependsItemGroup);

	VSImportPtr targetsImport = std::make_shared<VSImport>();
	targetsImport->project = "$(VCTargetsPath)\\Microsoft.Cpp.targets";
	project->objectList.push_back(targetsImport);

	VSImportGroupPtr extensionTargetsImportGroup = std::make_shared<VSImportGroup>();
	extensionTargetsImportGroup->label = "ExtensionTargets";
	project->objectList.push_back(extensionTargetsImportGroup);

	return project;
}

VSProjectConfigurationPtr VSSolution::legacyConfig(std::string config)
{
	VSProjectConfigurationPtr projectConfig = std::make_shared<VSProjectConfiguration>();
	std::vector<std::string> configParts = util::split(config, "|");
	if (configParts[1] == "x86") configParts[1] = "Win32";
	projectConfig->configuration = configParts[0];
	projectConfig->platform = configParts[1];
	projectConfig->include = configParts[0] + "|" + configParts[1];
	return projectConfig;
}

std::string VSSolution::findGuidForProject(std::string project_name)
{
	for (auto solutionProject : projects) {
		if (solutionProject->name == project_name) return solutionProject->guid;
	}
	return "";
}

void VSSolution::resolveDependencies()
{
	for (auto solutionProject : projects) {
		for (std::string dependency : solutionProject->dependenciesToResolve) {
			std::string dependencyGuid = findGuidForProject(dependency);
			if (dependencyGuid.size() > 0) {
				if (std::find(solutionProject->dependencies.begin(), solutionProject->dependencies.end(),
						dependencyGuid) == solutionProject->dependencies.end()) {
					solutionProject->dependencies.push_back(dependencyGuid);
				}
			}
			VSProjectPtr project = solutionProject->project;
			VSProjectReferencePtr projectReference = std::make_shared<VSProjectReference>();
			projectReference->include = std::string("..\\") + dependency + "\\" + dependency + ".vcxproj";
			projectReference->properties["Project"] = std::string("{") + dependencyGuid + std::string("}");
			project->dependsItemGroup->objectList.push_back(projectReference);
		}
	}
}

void VSSolution::read(std::string solution_file)
{
	std::vector<char> buf = util::read_file(solution_file);
	if (!parse(buf.data(), buf.size())) {
		log_fatal_exit("VSSolution: parse error");
	}
	for (auto project : projects) {
		std::string project_file_path = util::path_relative_to_path(project->path, solution_file);
		project->project = std::make_shared<VSProject>();
		project->project->read(project_file_path);
	}
}

void VSSolution::write(project_root_ptr root)
{
	std::string solution_file = root->project_name + ".vsproj/" + root->project_name + ".sln";
	write(solution_file);
}

void VSSolution::write(std::string solution_file)
{
	util::make_directories(solution_file);
	write_solution(solution_file);
	for (auto project : projects) {
		std::string project_file_path = util::path_relative_to_path(project->path, solution_file);
		util::make_directories(project_file_path);
		project->project->write(project_file_path);
	}
}

void VSSolution::write_solution(std::string solution_file)
{
	resolveDependencies();
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

bool VSProject::factoryInit = false;
std::map<std::string,VSObjectFactoryPtr> VSProject::factoryMap;

void VSProject::init()
{
	if (!factoryInit) {
		registerFactory<VSImport>();
		registerFactory<VSImportGroup>();
		registerFactory<VSItemGroup>();
		registerFactory<VSItemDefinitionGroup>();
		registerFactory<VSPropertyGroup>();
		registerFactory<VSProjectConfiguration>();
		registerFactory<VSProjectReference>();
		registerFactory<VSClCompile>();
		registerFactory<VSClInclude>();
		registerFactory<VSLink>();
		factoryInit = true;
	};
}

VSProject::VSProject() {}

void VSProject::read(std::string project_file)
{
	tinyxml2::XMLDocument doc(false);
	std::vector<char> buf = util::read_file(project_file);
	tinyxml2::XMLError err = doc.Parse(buf.data());
	if (err != tinyxml2::XML_NO_ERROR) {
		log_fatal_exit("VSProject: error reading: %s: xml_error=%d", project_file.c_str(), err);
	}
	xmlToProject(&doc);
}

void VSProject::write(std::string project_file)
{
	tinyxml2::XMLDocument doc(false);
	doc.SetBOM(true);
	projectToXml(&doc);
	tinyxml2::XMLError err = doc.SaveFile(project_file.c_str(), false);
	if (err != tinyxml2::XML_NO_ERROR) {
		log_fatal_exit("VSProject: error writing: xml_error=%d",  err);
	}
}

void VSProject::xmlToProject(tinyxml2::XMLDocument *doc)
{
	tinyxml2::XMLElement *root = doc->RootElement();
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

void VSProject::projectToXml(tinyxml2::XMLDocument *doc)
{
	tinyxml2::XMLDeclaration *decl = doc->NewDeclaration();
	tinyxml2::XMLElement *root = doc->NewElement("Project");
	doc->InsertFirstChild(root);
	doc->InsertFirstChild(decl);
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
const std::string VSProjectReference::type_name     = "ProjectReference";
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


/* VSProjectReference */

void VSProjectReference::fromXML(tinyxml2::XMLElement *element)
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

void VSProjectReference::toXML(tinyxml2::XMLElement *parent)
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
