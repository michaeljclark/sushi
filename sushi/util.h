#ifndef util_h
#define util_h

/* utility */

struct util {
	static const char* HEX_DIGITS;
	static const char* LITERAL_CHARS;

	static std::string ltrim(std::string s);
	static std::string rtrim(std::string s);
	static std::string trim(std::string s);
	static int canonicalize_path(char *path);
	static std::vector<std::string> path_components(std::string path);
	static std::string hex_encode(const unsigned char *buf, size_t len);
	static void hex_decode(std::string hex, unsigned char *buf, size_t len);
	static void generate_random(unsigned char *buf, size_t len);
	static bool literal_requires_quotes(std::string str);
	static std::string escape_quotes(std::string str);
	static bool literal_is_hex_id(std::string str);
	static std::vector<char> read_file(std::string filename);
};

#endif