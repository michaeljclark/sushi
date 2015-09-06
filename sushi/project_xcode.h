//
//  project_xcode.h
//

#ifndef prject_xcode_h
#define prject_xcode_h

struct lib_output_data
{
	std::string file_type;
	std::string target_type;
	std::string output_file;

	lib_output_data(std::string file_type, std::string target_type, std::string output_file)
		: file_type(file_type), target_type(target_type), output_file(output_file) {}
};

struct project_xcode
{
	static lib_output_data lib_output(project_root_ptr root, project_lib_ptr lib);
	static std::vector<std::string> lib_deps(project_root_ptr root, std::vector<std::string> libs);
	static XcodeprojPtr create_project(project_root_ptr root);
};

#endif