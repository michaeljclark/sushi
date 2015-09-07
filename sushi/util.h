//
//  util.h
//

#ifndef util_h
#define util_h

/* utility */

struct util
{
	static const char* HEX_DIGITS;

	static std::string ltrim(std::string s);
	static std::string rtrim(std::string s);
	static std::string trim(std::string s);
	static std::vector<std::string> split(std::string str, std::string separator,
		bool includeEmptyElements = true, bool includeSeparators = false);
	static std::string join(std::vector<std::string> list, std::string separator);
	static std::string hex_encode(const unsigned char *buf, size_t len);
	static void hex_decode(std::string hex, unsigned char *buf, size_t len);
	static void generate_random(unsigned char *buf, size_t len);
};

#endif
