//
//  project_xcode.h
//

#ifndef prject_xcode_h
#define prject_xcode_h

struct project_xcode
{
	static std::string lib_output(project_root_ptr root, project_lib_ptr lib);
	static std::vector<std::string> lib_deps(project_root_ptr root, std::vector<std::string> libs);
	static XcodeprojPtr create_project(project_root_ptr root);
};

#endif