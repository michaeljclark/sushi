//
//  project_xcode.h
//

#ifndef prject_xcode_h
#define prject_xcode_h

struct SUSHI_LIB xcode_lib_output_data
{
	std::string file_type;
	std::string target_type;
	std::string output_file;

	xcode_lib_output_data(std::string file_type, std::string target_type, std::string output_file)
		: file_type(file_type), target_type(target_type), output_file(output_file) {}
};

struct SUSHI_LIB project_xcode
{
	static xcode_lib_output_data lib_output(project_lib_ptr lib);
	static std::vector<std::string> lib_deps(project_root_ptr root, std::vector<std::string> libs);
	static XcodeprojPtr create_project(project_root_ptr root);
};

#endif
