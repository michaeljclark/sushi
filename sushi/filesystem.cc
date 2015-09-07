//
//  filesystem.cc
//

#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cerrno>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#define fileno _fileno
#else
#include <dirent.h>
#endif

#include "log.h"
#include "util.h"
#include "filesystem.h"


/* filesystem */

std::vector<char> filesystem::read_file(std::string filename)
{
	std::vector<char> buf;
	struct stat stat_buf;

	FILE *file = fopen(filename.c_str(), "r");
	if (!file) {
		log_fatal_exit("error fopen: %s: %s", filename.c_str(), strerror(errno));
	}

	if (fstat(fileno(file), &stat_buf) < 0) {
		log_fatal_exit("error fstat: %s: %s", filename.c_str(), strerror(errno));
	}

	buf.resize(stat_buf.st_size);
	size_t bytes_read = fread(buf.data(), 1, stat_buf.st_size, file);
	if (bytes_read != (size_t)stat_buf.st_size) {
		log_fatal_exit("error fread: %s", filename.c_str());
	}
	fclose(file);

	return buf;
}

int filesystem::canonicalize_path(char *path)
{
	char *r, *w;
	int last_was_slash = 0;
	r = w = path;
	while(*r != 0)
	{
		/* convert backslash to foward slash */
		if (*r == '\\') *r = '/';
		/* Ignore duplicate /'s */
		if (*r == '/' && last_was_slash) {
			r++;
			continue;
		}
		/* Calculate /../ in a secure way */
		if (last_was_slash && *r == '.') {
			if (*(r+1) == '.') {
				/* skip past .. or ../ with read pointer */
				if (*(r+2) == '/') r += 3;
				else if (*(r+2) == 0) r += 2;
				/* skip back to last / with write pointer */
				if (w > path+1) {
					w--;
					while(*(w-1) != '/') { w--; }
					continue;
				} else {
					return -1;
				}
			} else if (*(r+1) == '/') {
				r += 2;
				continue;
			}
		}
		*w = *r;
		last_was_slash = (*r == '/');
		r++;
		w++;
	}
	*w = 0;

	return 0;
}

std::vector<std::string> filesystem::path_components(std::string path)
{
	std::vector<char> buf;
	buf.resize(path.size() + 1);
	memcpy(buf.data(), path.c_str(), path.size());
	if (canonicalize_path(buf.data()) < 0) {
		return std::vector<std::string>();
	}
	path = buf.data();
	return util::split(path, "/", false);
}

std::string filesystem::path_relative_to_path(std::string path, std::string relative_to)
{
	std::vector<std::string> relative_comps = path_components(relative_to);
	if (relative_comps.size() > 0) {
		relative_comps.pop_back();
	}
	std::vector<std::string> path_comps = path_components(path);
	relative_comps.insert(relative_comps.end(), path_comps.begin(), path_comps.end());
	return util::join(relative_comps, "/");
}

#ifdef _WIN32

bool filesystem::list_files(std::vector<directory_entry> &files, std::string path_name)
{
	HANDLE dir;
	WIN32_FIND_DATA entry;
	memset(&entry, 0, sizeof(entry));
	files.clear();
	
	if ((dir = FindFirstFile(path_name.c_str(), &entry)) == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_NO_MORE_FILES) {
			return true;
		} else {
			return false;
		}
	}
	files.push_back(directory_entry(entry.cFileName, entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ?
		directory_entry_type_dir : directory_entry_type_file));
	
	BOOL ret;
	do {
		ret = FindNextFile(dir, &entry);
		if (ret) {
			files.push_back(directory_entry(entry.cFileName, entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ?
				directory_entry_type_dir : directory_entry_type_file));
		} else {
			if (GetLastError() == ERROR_NO_MORE_FILES) {
				break;
			} else {
				CloseHandle(dir);
				return false;
			}
		}
	} while (ret);
	
	CloseHandle(dir);
	return true;
}

#else

bool filesystem::list_files(std::vector<directory_entry> &files, std::string path_name)
{
	DIR *dir;
	struct dirent entry;
	struct dirent *result;
	memset(&entry, 0, sizeof(entry));
	files.clear();
	
	if ((dir = opendir(path_name.c_str())) == NULL) {
		return false;
	}
	
	int ret;
	do {
		if ((ret = readdir_r(dir, &entry, &result)) < 0) {
			closedir(dir);
			return false;
		}
		files.push_back(directory_entry(entry.d_name, entry.d_type & DT_DIR ?
			directory_entry_type_dir : directory_entry_type_file));
	} while (result != NULL);
	
	closedir(dir);
	return true;
}

#endif
