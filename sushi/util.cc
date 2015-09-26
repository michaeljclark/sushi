//
//  util.cc
//

#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cerrno>
#include <ctime>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <regex>

#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define fileno _fileno
#define mkdir(file,mode) _mkdir(file)
#else
#include <dirent.h>
#endif

#include "sushi.h"

#include "util.h"


/* logging */

static const int INITIAL_LOG_BUFFER_SIZE = 256;

static const char* FATAL_PREFIX = "FATAL";
static const char* ERROR_PREFIX = "ERROR";
static const char* DEBUG_PREFIX = "DEBUG";
static const char* INFO_PREFIX = "INFO";

std::string format_string(const char* fmt, ...)
{
	std::vector<char> buf(INITIAL_LOG_BUFFER_SIZE);
	va_list ap;

	va_start(ap, fmt);
	int len = vsnprintf(buf.data(), buf.capacity(), fmt, ap);
	va_end(ap);

	std::string str;
	if (len >= (int)buf.capacity()) {
		buf.resize(len + 1);
		va_start(ap, fmt);
		vsnprintf(buf.data(), buf.capacity(), fmt, ap);
		va_end(ap);
	}
	str = buf.data();

	return str;
}

void log_prefix(const char* prefix, const char* fmt, va_list arg)
{
	std::vector<char> buf(INITIAL_LOG_BUFFER_SIZE);

	int len = vsnprintf(buf.data(), buf.capacity(), fmt, arg);

	if (len >= (int)buf.capacity()) {
		buf.resize(len + 1);
		vsnprintf(buf.data(), buf.capacity(), fmt, arg);
	}

	fprintf(stderr, "%s: %s\n", prefix, buf.data());
}

void log_fatal_exit(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	log_prefix(FATAL_PREFIX, fmt, ap);
	va_end(ap);
	exit(9);
}

void log_error(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	log_prefix(ERROR_PREFIX, fmt, ap);
	va_end(ap);
}

void log_info(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	log_prefix(INFO_PREFIX, fmt, ap);
	va_end(ap);
}

void log_debug(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	log_prefix(DEBUG_PREFIX, fmt, ap);
	va_end(ap);
}


/* util */

std::vector<char> util::read_file(std::string filename)
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

int util::canonicalize_path(char *path)
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

std::vector<std::string> util::path_components(std::string path)
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

void util::make_directories(std::string path)
{
	std::vector<std::string> comps = path_components(path);
	if (comps.size() > 1) {
		comps.pop_back();
		for (size_t i = 1; i <= comps.size(); i++) {
			std::vector<std::string> dirComps;
			for (size_t j = 0; j < i; j++) dirComps.push_back(comps[j]);
			std::string path = util::join(dirComps, "/");
			mkdir(path.c_str(), 0777);
		}
	}
}

std::string util::path_relative_to_path(std::string path, std::string relative_to)
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

bool util::list_files(std::vector<directory_entry> &files, std::string path_name)
{
	HANDLE dir;
	WIN32_FIND_DATA entry;
	memset(&entry, 0, sizeof(entry));
	files.clear();

	path_name = path_name + "\\*";
	if ((dir = FindFirstFile(path_name.c_str(), &entry)) == INVALID_HANDLE_VALUE) {
		return false;
	}

	BOOL ret;
	for (;;) {
		files.push_back(directory_entry(entry.cFileName, entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ?
			directory_entry_type_dir : directory_entry_type_file));
		if (!(ret = FindNextFile(dir, &entry))) break;
	}

	DWORD dwError = GetLastError();
	FindClose(dir);
	return (dwError != ERROR_NO_MORE_FILES);
}

#else

bool util::list_files(std::vector<directory_entry> &files, std::string path_name)
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
	for (;;) {
		if ((ret = readdir_r(dir, &entry, &result)) < 0 || !result) break;
		files.push_back(directory_entry(entry.d_name, entry.d_type & DT_DIR ?
			directory_entry_type_dir : directory_entry_type_file));
	}

	closedir(dir);
	return (ret == 0);
}

#endif

struct globre_component
{
	static const std::string GLOBRE_CHARS;

	std::string comp;
	bool compiled;
	bool has_re;
	std::regex comp_regex;

	globre_component() : comp(), compiled(false), has_re(false) {}
	globre_component(std::string comp) : comp(comp), compiled(false), has_re(false) {}
	globre_component(const globre_component &o) : comp(o.comp), compiled(false), has_re(false) {}

	void compile()
	{
		if (!compiled) {
			if ((has_re = test_globre_chars())) {
				compile_globre();
			}
			compiled = true;
		}
	}

	bool has_regex()
	{
		compile();
		return has_re;
	}

	bool match(std::string s)
	{
		compile();
		if (has_re) {
			return (std::regex_match(s, comp_regex)); 
		} else {
			return s == comp;
		}
	}

	bool test_globre_chars()
	{
		for (char c : comp) {
			if (GLOBRE_CHARS.find(c) != std::string::npos) {
				return true;
			}
		}
		return false;
	}

	void compile_globre()
	{
		/*
		 * globre is a hybrid glob using several regular expression features
		 * globre is designed so that simple .* glob expressions are compatible 
		 *
		 * The following transformations are applied to each path component
		 * if any of the following characters are present: ()[]{}*?\
		 *
		 *   add anchors at start and end ^ $
		 *   translate . into \.
		 *   translate * into .*
		 *   translate ? into .?
		 *   translate \. into .
		 *   translate \* into *
		 *   translate \? into ?
		 *
		 * e.g.
		 *
		 *   globre                  Regular Expression
		 *
		 *   foo.*             =     ^foo\..*$
		 *   foo.?             =     ^foo\..?$
		 *   foo.(c|cc|h)      =     ^foo\.(c|cc|h)$
		 *   *.(c|cc|h)        =     ^.*\.(c|cc|h)$
		 *   foo(_x86)\?.cc    =     ^foo(_x86)?\.cc$
		 *
		 */
		std::stringstream ss;
		ss << "^";
		char last_c = 0;
		for (char c : comp) {
			if (last_c == '\\' && c == '.') {
				ss << ".";
			} else if (last_c == '\\' && c == '*') {
				ss << "*";
			} else if (last_c == '\\' && c == '?') {
				ss << "?";
			} else if (last_c == '\\') {
				ss << "\\" << c;
			} else if (c == '\\') {

			} else if (c == '.') {
				ss << "\\.";
			} else if (c == '*') {
				ss << ".*";
			} else if (c == '?') {
				ss << ".?";
			} else {
				ss << c;
			}
			last_c = c;
		}
		ss << "$";
		comp_regex = std::regex(ss.str());
	}
};

const std::string globre_component::GLOBRE_CHARS = "()[]{}*?\\";

struct globre_matcher
{
	std::vector<globre_component> globre_comps;

	globre_matcher(std::string globre_expression)
	{
		std::vector<std::string> path_comps = util::split(globre_expression, "/", true);
		for (std::string comp : path_comps) {
			globre_comps.push_back(globre_component(comp));
		}
	}

	void accumlate_matches_scan(std::vector<std::string> &prefix,
		size_t depth, std::vector<std::string> &results)
	{
		globre_component &globre_comp = globre_comps[depth];

		// reconstruct directory name from current prefix
		std::vector<std::string> dir_comps = prefix;
		dir_comps.push_back(".");
		std::string dir = util::join(dir_comps, "/");

		// read directory contents
		std::vector<directory_entry> dents;
		util::list_files(dents, dir);

		// check for matches in this directory
		for (const directory_entry &dent : dents) {
			if (dent.name == "." || dent.name == "..") continue;
			if (depth < globre_comps.size() - 1 &&
					dent.type == directory_entry_type_dir &&
					globre_comp.match(dent.name))
			{
				// recurse to next level deep
				prefix.push_back(dent.name);
				accumlate_matches(prefix, depth + 1, results);
				prefix.pop_back();
			} else if (depth == globre_comps.size() - 1 &&
					globre_comp.match(dent.name))
			{
				// full depth, accumulate results
				std::vector<std::string> file_comps = prefix;
				file_comps.push_back(dent.name);
				std::string file = util::join(file_comps, "/");
				results.push_back(file);
			}
		}
	}

	void accumlate_matches_static(std::vector<std::string> &prefix,
		size_t depth, std::vector<std::string> &results)
	{
		globre_component &globre_comp = globre_comps[depth];

		// reconstruct file or directory name from current prefix
		std::vector<std::string> file_comps = prefix;
		file_comps.push_back(globre_comp.comp);
		std::string file = util::join(file_comps, "/");

		// handle absolute directory paths
		if (depth == 0 && globre_comp.comp.size() == 0) {
			file = "/";
		}
#ifdef _WIN32
		if (depth == 0 && globre_comp.comp.size() == 2 && globre_comp.comp[1] == ':') {
			file = file + "/";
		}
#endif
		// check this path component exists
		struct stat stat_buf;
		int ret = stat(file.c_str(), &stat_buf);
		if (ret < 0) return;
		if (depth < globre_comps.size() - 1 && stat_buf.st_mode & S_IFDIR) {
			// recurse to next level deep
			prefix.push_back(globre_comp.comp);
			accumlate_matches(prefix, depth + 1, results);
			prefix.pop_back();
		} else if (depth == globre_comps.size() - 1) {
			// full depth, accumulate results
			results.push_back(file);
		}
	}

	void accumlate_matches(std::vector<std::string> &prefix,
		size_t depth, std::vector<std::string> &results)
	{
		globre_component &globre_comp = globre_comps[depth];
		if (globre_comp.has_regex()) {
			// regular expression path component so we scan files and check for matches
			accumlate_matches_scan(prefix, depth, results);
		} else {
			// fixed path component so we stat the entry to check that it exists
			accumlate_matches_static(prefix, depth, results);
		}
	}
};

std::vector<std::string> util::globre(std::string globre_expression)
{
	std::vector<std::string> results, prefix;
	globre_matcher matcher(globre_expression);
	matcher.accumlate_matches(prefix, 0, results);
	return results;
}

std::vector<std::string> util::globre_list(std::vector<std::string> globre_expression_list)
{
	std::vector<std::string> results;
	for (std::string globre_expression : globre_expression_list) {
		std::vector<std::string> files_to_add = util::globre(globre_expression);
		results.insert(results.end(), files_to_add.begin(), files_to_add.end());
	}
	return results;
}


/* utility */

const char* util::HEX_DIGITS = "0123456789ABCDEF";

std::string util::ltrim(std::string s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
			std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

std::string util::rtrim(std::string s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
			std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

std::string util::trim(std::string s) {
	return ltrim(rtrim(s));
}

std::vector<std::string> util::split(std::string str, std::string separator,
		bool includeEmptyElements, bool includeSeparators)
{
	size_t last_index = 0, index;
	std::vector<std::string> components;
	while ((index = str.find_first_of(separator, last_index)) != std::string::npos) {
		if (includeEmptyElements || index - last_index > 0) {
			components.push_back(str.substr(last_index, index - last_index));
		}
		if (includeSeparators) {
			components.push_back(separator);
		}
		last_index = index + separator.length();
	}
	if (includeEmptyElements || str.size() - last_index > 0) {
		components.push_back(str.substr(last_index, str.size() - last_index));
	}
	return components;
}

std::string util::join(std::vector<std::string> list, std::string separator)
{
	std::stringstream ss;
	for (auto i = list.begin(); i != list.end(); i++) {
		if (i != list.begin()) ss << separator;
		ss << *i;
	}
	return ss.str();
}

std::string util::hex_encode(const unsigned char *buf, size_t len, bool byte_swap)
{
	std::string hex;
	if (byte_swap) {
		for (size_t i = len; i > 0; i--) {
			char b = buf[i-1];
			hex.append(HEX_DIGITS + ((b >> 4) & 0x0F), 1);
			hex.append(HEX_DIGITS + (b & 0x0F), 1);
		}
	} else {
		for (size_t i = 0; i < len; i++) {
			char b = buf[i];
			hex.append(HEX_DIGITS + ((b >> 4) & 0x0F), 1);
			hex.append(HEX_DIGITS + (b & 0x0F), 1);
		}
	}
	return hex;
}

void util::hex_decode(std::string hex, unsigned char *buf, size_t len, bool byte_swap)
{
	if (hex.length() % 2 != 0) return;
	if (byte_swap) {
		for (size_t i = hex.length()/2; i > 0; i--) {
			const char tmp[3] = { hex[(i - 1) << 1], hex[((i - 1) << 1) + 1], 0 };
			*buf++ = (char)strtoul(tmp, NULL, 16);
		}
	} else {
		for (size_t i = 0; i < hex.length()/2 && i < len; i++) {
			const char tmp[3] = { hex[i << 1], hex[(i << 1) + 1], 0 };
			*buf++ = (char)strtoul(tmp, NULL, 16);
		}
	}
}

void util::generate_random(unsigned char *buf, size_t len)
{
	static std::default_random_engine generator;
	static std::uniform_int_distribution<unsigned int> distribution(0, 255);
	for (size_t i = 0; i < len; i++) {
		buf[i] = (unsigned char)distribution(generator);
	}
}

void util::generate_uuid(uuid &u)
{
	generate_random(u.data, 16);
	u.val.data3 = (u.val.data3 & 0x0FFF) | 0x4000; /* random uuid */
}

std::string util::format_uuid(uuid &u)
{
	std::stringstream ss;
	ss << hex_encode(&u.data[0], 4, host_endian.value == endian_little);
	ss << "-";
	ss << hex_encode(&u.data[4], 2, host_endian.value == endian_little);
	ss << "-";
	ss << hex_encode(&u.data[6], 2, host_endian.value == endian_little);
	ss << "-";
	ss << hex_encode(&u.data[8], 2, host_endian.value == endian_little);
	ss << "-";
	ss << hex_encode(&u.data[10], 6, false);
	return ss.str();
}
