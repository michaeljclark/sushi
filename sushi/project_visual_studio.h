//
//  project_visual_studio.h
//

#ifndef prject_visual_studio_h
#define prject_visual_studio_h

struct vs_lib_output_data
{
	std::string target_type;
	std::string output_file;

	vs_lib_output_data(std::string target_type, std::string output_file)
		: target_type(target_type), output_file(output_file) {}
};

struct project_visual_studio
{
	static vs_lib_output_data lib_output(project_lib_ptr lib);
	static std::vector<std::string> lib_deps(project_root_ptr root, std::vector<std::string> libs);
	static VSSolutionPtr create_solution(project_root_ptr root);
};

#endif
