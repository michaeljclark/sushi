#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <functional>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "log.h"
#include "util.h"


/* utility */

const char* util::HEX_DIGITS = "0123456789ABCDEF";
const char* util::LITERAL_CHARS = "/._";

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

	std::vector<std::string> path_components;
	if (canonicalize_path(buf.data()) < 0) {
		return path_components;
	}

	char *token, *p = buf.data();
	while ((token = strsep(&p, "/")) != NULL) {
		if(strlen(token)) {
			path_components.push_back(token);
		}
	}
	return path_components;
}

std::string util::hex_encode(const unsigned char *buf, size_t len)
{
	std::string hex;
	for (size_t i = 0; i < len; i++) {
		char b = buf[i];
		hex.append(HEX_DIGITS + ((b >> 4) & 0x0F), 1);
		hex.append(HEX_DIGITS + (b & 0x0F), 1);
	}
	return hex;
}

void util::hex_decode(std::string hex, unsigned char *buf, size_t len)
{
	for (size_t i = 0; i < hex.length()/2 && i < len; i++) {
		const char tmp[3] = { hex[i*2], hex[i*2+1], 0 };
		*buf++ = (char)strtoul(tmp, NULL, 16);
	}
}

void util::generate_random(unsigned char *buf, size_t len)
{
	static std::default_random_engine generator;
	static std::uniform_int_distribution<unsigned char> distribution(0, 255);
	generator.seed(time(NULL));
	for (size_t i = 0; i < len; i++) {
		buf[i] = distribution(generator);
	}
}

bool util::literal_requires_quotes(std::string str)
{
	if (str.size() == 0) return true;
	for (size_t i = 0; i < str.length(); i++) {
		char c = str[i];
		if (!isalnum(c) && strchr(LITERAL_CHARS, c) == NULL) return true;
	}
	return false;
}

std::string util::escape_quotes(std::string str)
{
	std::stringstream ss;
	for (size_t i = 0; i < str.length(); i++) {
		char c = str[i];
		if (c == '"') ss << "\\";
		ss << c;
	}
	return ss.str();
}

bool util::literal_is_hex_id(std::string str)
{
	if (str.size() != 24) return false;
	for (size_t i = 0; i < str.length(); i++) {
		if (strchr(HEX_DIGITS, str[i]) == NULL) return false;
	}
	return true;
}

std::vector<char> util::read_file(std::string filename)
{
	std::vector<char> buf;
	struct stat stat_buf;

	int fd = ::open(filename.c_str(), O_RDONLY);
	if (fd < 0) {
		log_fatal_exit("error open: %s: %s", filename.c_str(), strerror(errno));
	}

	if (fstat(fd, &stat_buf) < 0) {
		log_fatal_exit("error fstat: %s: %s", filename.c_str(), strerror(errno));
	}

	buf.resize(stat_buf.st_size);
	ssize_t bytes_read = ::read(fd, buf.data(), stat_buf.st_size);
	if (bytes_read < 0) {
		log_fatal_exit("error read: %s: %s", filename.c_str(), strerror(errno));
	} else if (bytes_read != stat_buf.st_size) {
		log_fatal_exit("error short read: %s", filename.c_str());
	}
	::close(fd);

	return buf;
}