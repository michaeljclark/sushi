//
//  ninja.cc
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
#include "ninja.h"


/* Ninja */

NinjaVar::NinjaVar(std::string name, std::string value) : name(name), value(value) {}

NinjaRule::NinjaRule(std::string name, std::string command, std::string description) : name(name)
{
	properties["command"] = command;
	properties["description"] = description;
}

NinjaBuild::NinjaBuild(std::string output, std::string rule, std::string input) : output(output), rule(rule), input(input)
{

}

static std::vector<std::string> lib_deps(project_root_ptr root, std::vector<std::string> libs)
{
	std::vector<std::string> lib_deps;
	for (auto lib_name : libs) {
		auto lib = root->get_lib(lib_name);
		lib_deps.push_back(std::string("lib") + lib->lib_name + "$lib");
	}
	return lib_deps;
}

NinjaPtr Ninja::createBuild(project_root_ptr root)
{
	// construct empty solution
	auto config = root->get_config("*");
	NinjaPtr ninja = std::make_shared<Ninja>();
	ninja->createEmptyBuild(root, config->vars);

	// create library targets
	for (auto lib_name : root->get_lib_list()) {
		auto lib = root->get_lib(lib_name);
		ninja->createTarget(
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
		ninja->createTarget(
			root,
			config->vars,
			tool->tool_name,
			"Application",
			tool->libs,
			std::vector<std::string>(),
			std::vector<std::string>(),
			lib_deps(root, root->get_libs(tool)),
			util::globre_list(tool->source)
		);
	}

	return ninja;
}

void Ninja::createEmptyBuild(project_root_ptr root, std::map<std::string,std::string> vars)
{
	// TODO - add detection: currently hard coded to MSC and GCC
	NinjaVarPtr arch_var = std::make_shared<NinjaVar>("arch", arch::get().literal());
	NinjaVarPtr sourcedir_var = std::make_shared<NinjaVar>("sourcedir", ".");
	NinjaVarPtr builddir_var = std::make_shared<NinjaVar>("builddir", "build");
#if defined (_WIN32)
	NinjaVarPtr obj_var = std::make_shared<NinjaVar>("obj", ".obj");
	NinjaVarPtr lib_var = std::make_shared<NinjaVar>("lib", ".lib");
	NinjaVarPtr exe_var = std::make_shared<NinjaVar>("exe", ".exe");
	NinjaVarPtr ar_var = std::make_shared<NinjaVar>("ar", "link");
	NinjaVarPtr cc_var = std::make_shared<NinjaVar>("cc", "cl");
	NinjaVarPtr cxx_var = std::make_shared<NinjaVar>("cxx", "cl");	
	NinjaVarPtr cflags_var = std::make_shared<NinjaVar>("cflags",
		"/nologo "  /* Supress banner */
		"/Zi "      /* Enable Debug Information */
		"/FS "      /* Write to program database */
		"/Ox "      /* Maximum optimization */
		"/GL "      /* Whole program optimization */
		"/MT "      /* link with LIBCMT.LIB */
		"/DNDEBUG"  /* Define NDEBUG */
	);
	NinjaVarPtr cxxflags_var = std::make_shared<NinjaVar>("cxxflags",
		"/EHsc"     /* Enable C++ exception */
	);
	NinjaVarPtr ldflags_var = std::make_shared<NinjaVar>("ldflags",
		"/DEBUG "   /* Creates debugging information */
		"/OPT:REF " /* Eliminate unreferenced code and data */
		"/OPT:ICF " /* Perform identical COMDAT folding */
		"/LTCG"     /* Enable Link Time Code Generation */
	);
	NinjaRulePtr cc_rule = std::make_shared<NinjaRule>("cc", "$cxx $cflags -c $in /Fo$out", "CC $out");
	cc_rule->properties["deps"] = "msvc";
	NinjaRulePtr cxx_rule = std::make_shared<NinjaRule>("cxx", "$cxx $cflags $cxxflags -c $in /Fo$out", "CXX $out");
	cxx_rule->properties["deps"] = "msvc";
	NinjaRulePtr ar_rule = std::make_shared<NinjaRule>("ar", "lib /nologo /ltcg /out:$out $in", "LIB $out");
	NinjaRulePtr link_rule = std::make_shared<NinjaRule>("link", "$cxx $in $libs /nologo /link $ldflags /out:$out", "LINK $out");
#else
	NinjaVarPtr obj_var = std::make_shared<NinjaVar>("obj", ".o");
	NinjaVarPtr lib_var = std::make_shared<NinjaVar>("lib", ".a");
	NinjaVarPtr exe_var = std::make_shared<NinjaVar>("exe", "");
	NinjaVarPtr ar_var = std::make_shared<NinjaVar>("ar", "ar");
	NinjaVarPtr cc_var = std::make_shared<NinjaVar>("cc", "gcc");
	NinjaVarPtr cxx_var = std::make_shared<NinjaVar>("cxx", "g++");	
	NinjaVarPtr cflags_var = std::make_shared<NinjaVar>("cflags", "-Wall -Wpedantic");
	NinjaVarPtr cxxflags_var = std::make_shared<NinjaVar>("cxxflags", "-std=c++11");
	NinjaVarPtr ldflags_var = std::make_shared<NinjaVar>("ldflags", "-L$builddir");
	NinjaRulePtr cc_rule = std::make_shared<NinjaRule>("cc", "$cc -MMD -MT $out -MF $out.d $cflags -c $in -o $out", "CC $out");
	cc_rule->properties["depfile"] = "$out.d";
	cc_rule->properties["deps"] = "gcc";
	NinjaRulePtr cxx_rule = std::make_shared<NinjaRule>("cxx", "$cxx -MMD -MT $out -MF $out.d $cxxflags $cflags -c $in -o $out", "CXX $out");
	cxx_rule->properties["depfile"] = "$out.d";
	cxx_rule->properties["deps"] = "gcc";
	NinjaRulePtr ar_rule = std::make_shared<NinjaRule>("ar", "rm -f $out && $ar crs $out $in", "AR $out");
	NinjaRulePtr link_rule = std::make_shared<NinjaRule>("link", "$cxx $ldflags -o $out $in $libs", "LINK $out");
#endif
	ninjaVarList.push_back(arch_var);
	ninjaVarList.push_back(sourcedir_var);
	ninjaVarList.push_back(builddir_var);
	ninjaVarList.push_back(obj_var);
	ninjaVarList.push_back(lib_var);
	ninjaVarList.push_back(exe_var);
	ninjaVarList.push_back(ar_var);
	ninjaVarList.push_back(cc_var);
	ninjaVarList.push_back(cxx_var);
	ninjaVarList.push_back(cflags_var);
	ninjaVarList.push_back(cxxflags_var);
	ninjaVarList.push_back(ldflags_var);
	ninjaRuleList.push_back(cc_rule);
	ninjaRuleList.push_back(cxx_rule);
	ninjaRuleList.push_back(ar_rule);
	ninjaRuleList.push_back(link_rule);
}

static std::pair<std::string,std::string> file_ext(std::string filename)
{
	size_t offset = filename.find_last_of(".");
	if (offset == std::string::npos) {
		return std::pair<std::string,std::string>(filename,std::string());
	} else {
		return std::pair<std::string,std::string>(filename.substr(0, offset), filename.substr(offset + 1));
	}
}

void Ninja::createTarget(project_root_ptr root, std::map<std::string,std::string> vars,
		std::string target_name, std::string target_type,
		std::vector<std::string> depends,
		std::vector<std::string> defines,
		std::vector<std::string> lib_dirs,
		std::vector<std::string> lib_files,
		std::vector<std::string> source)
{
	std::string additionalIncludes;
	for (auto dependency : depends) {
		// NOTE - this works because the sushi convention is that the library
		//        directory name is the same as the library name
		// TODO - use export_includes
		if (additionalIncludes.size() > 0) additionalIncludes.append(";");
		additionalIncludes.append(format_string("-I%s", dependency.c_str()));
	}

	std::vector<std::string> objectFiles;
	for (auto sourceFile : source) {
		auto nameExt = file_ext(sourceFile);
		if (nameExt.second == "c") {			
			std::string outputFile = "$builddir/$arch/obj/" + nameExt.first + "$obj";
			NinjaBuildPtr buildFile = std::make_shared<NinjaBuild>(outputFile, "cc", sourceFile);
			if (additionalIncludes.size() > 0) {
				buildFile->properties["cflags"] = "$cflags " + additionalIncludes;
			}
			ninjaBuildList.push_back(buildFile);
			objectFiles.push_back(outputFile);
		} else if (nameExt.second == "cc" || nameExt.second == "cpp") {
			std::string outputFile = "$builddir/$arch/obj/" + nameExt.first + "$obj";
			NinjaBuildPtr buildFile = std::make_shared<NinjaBuild>(outputFile, "cxx", sourceFile);
			if (additionalIncludes.size() > 0) {
				buildFile->properties["cflags"] = "$cflags " + additionalIncludes;
			}
			ninjaBuildList.push_back(buildFile);
			objectFiles.push_back(outputFile);
		}
	}
	if (target_type == "Application") {
		for (std::string lib_file : lib_files) {
			objectFiles.push_back(std::string("$builddir/$arch/lib/") + lib_file);
		}
		std::string outputFile = "$builddir/$arch/bin/" + target_name + "$exe";
		NinjaBuildPtr buildFile = std::make_shared<NinjaBuild>(outputFile, "link", util::join(objectFiles, " "));
		ninjaBuildList.push_back(buildFile);
	} else if (target_type == "StaticLibrary") {
		std::string outputFile = std::string("$builddir/$arch/lib/") + std::string("lib") + target_name + "$lib";
		NinjaBuildPtr buildFile = std::make_shared<NinjaBuild>(outputFile, "ar", util::join(objectFiles, " "));
		ninjaBuildList.push_back(buildFile);
	} else if (target_type == "DynamicLibrary") {
		// TODO
	}
}

void Ninja::write(project_root_ptr root)
{
	write("build.ninja");
}

void Ninja::write(std::string build_file)
{
	std::ofstream out(build_file.c_str());
	for (auto var : ninjaVarList) {
		out << var->name << " = " << var->value << '\n';
	}
	out << '\n';
	for (auto rule : ninjaRuleList) {
		out << "rule " << rule->name << '\n';
		for (auto ent : rule->properties) {
			out << "    " << ent.first << " = " << ent.second << '\n';
		}
		out << '\n';
	}
	for (auto build : ninjaBuildList) {
		out << "build " << build->output << ": " << build->rule << " " << build->input << "\n";
		if (build->properties.size() > 0) {
			for (auto ent : build->properties) {
				out << "    " << ent.first << " = " << ent.second << '\n';
			}
		}
		out << '\n';
	}
}