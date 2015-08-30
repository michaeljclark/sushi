#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "log.h"
#include "util.h"
#include "filesystem.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

/* filesystem */

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
	files.push_back({entry.cFileName, entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ?
		directory_entry_type_dir : directory_entry_type_file});
	
	BOOL ret;
	do {
		ret = FindNextFile(dir, &entry);
		if (ret) {
			files.push_back({entry.cFileName, entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ?
				directory_entry_type_dir : directory_entry_type_file});
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
		files.push_back({entry.d_name, entry.d_type & DT_DIR ?
			directory_entry_type_dir : directory_entry_type_file});
	} while (result != NULL);
	
	closedir(dir);
	return true;
}

#endif