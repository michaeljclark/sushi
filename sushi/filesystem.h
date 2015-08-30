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
};

struct filesystem
{
    bool list_files(std::vector<directory_entry> &files, std::string path_name);
};

#endif