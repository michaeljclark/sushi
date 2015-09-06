//
//  filesystem.h
//

#ifndef filesystem_h
#define filesystem_h

struct directory;
struct directory_entry;
typedef std::shared_ptr<directory> directory_ptr;
typedef std::shared_ptr<directory_entry> directory_entry_ptr;

enum directory_entry_type {
	directory_entry_type_file,
	directory_entry_type_dir
};

struct directory_entry
{
	std::string name;
	directory_entry_type type;

	directory_entry(std::string name, directory_entry_type type) : name(name), type(type) {}
};

struct filesystem
{
	static int canonicalize_path(char *path);
	static std::vector<std::string> path_components(std::string path);
	static std::string path_relative_to_path(std::string path, std::string relative_to);
	static bool list_files(std::vector<directory_entry> &files, std::string path_name);
};

#endif