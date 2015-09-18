//
//  util.h
//

#ifndef util_h
#define util_h


/* logging */

SUSHI_LIB std::string format_string(const char* fmt, ...);
SUSHI_LIB void log_prefix(const char* prefix, const char* fmt, va_list arg);
SUSHI_LIB void log_fatal_exit(const char* fmt, ...);
SUSHI_LIB void log_error(const char* fmt, ...);
SUSHI_LIB void log_info(const char* fmt, ...);
SUSHI_LIB void log_debug(const char* fmt, ...);


/* utility */

union uuid
{
	unsigned char data[16];
	struct {
		uint32_t data1;
		uint16_t data2;
		uint16_t data3;
		uint64_t data4;
	} val;
};

struct directory;
struct directory_entry;
typedef std::shared_ptr<directory> directory_ptr;
typedef std::shared_ptr<directory_entry> directory_entry_ptr;

enum directory_entry_type {
	directory_entry_type_file,
	directory_entry_type_dir
};

struct SUSHI_LIB directory_entry
{
	std::string name;
	directory_entry_type type;

	directory_entry(std::string name, directory_entry_type type) : name(name), type(type) {}
};

struct SUSHI_LIB util
{
	static const char* HEX_DIGITS;

	static std::vector<char> read_file(std::string filename);
	static int canonicalize_path(char *path);
	static std::vector<std::string> path_components(std::string path);
	static void make_directories(std::string path);
	static std::string path_relative_to_path(std::string path, std::string relative_to);
	static bool list_files(std::vector<directory_entry> &files, std::string path_name);
	static std::string ltrim(std::string s);
	static std::string rtrim(std::string s);
	static std::string trim(std::string s);
	static std::vector<std::string> split(std::string str, std::string separator,
		bool includeEmptyElements = true, bool includeSeparators = false);
	static std::string join(std::vector<std::string> list, std::string separator);
	static std::string hex_encode(const unsigned char *buf, size_t len, bool byte_swap);
	static void hex_decode(std::string hex, unsigned char *buf, size_t len, bool byte_swap);
	static void generate_random(unsigned char *buf, size_t len);
	static void generate_uuid(uuid &u);
	static std::string format_uuid(uuid &u);
};


/* endian */

enum endian {
    endian_little = 0x03020100ul,
    endian_big    = 0x00010203ul,
};

union endianness { unsigned char bytes[4]; uint32_t value; };

static const endianness host_endian = { { 0, 1, 2, 3 } };

#ifndef bswap16
#if defined __GNUC__
#define bswap16(x) __builtin_bswap16(x)
#else
#define bswap16(x) ((uint16_t)((((uint16_t) (x) & 0xff00) >> 8) | \
                    (((uint16_t) (x) & 0x00ff) << 8)))
#endif
#endif

#ifndef bswap32
#if defined __GNUC__
#define bswap32(x) __builtin_bswap32(x)
#else
#define bswap32(x) ((uint32_t)((((uint32_t) (x) & 0xff000000) >> 24) | \
                    (((uint32_t) (x) & 0x00ff0000) >> 8) | \
                    (((uint32_t) (x) & 0x0000ff00) << 8) | \
                    (((uint32_t) (x) & 0x000000ff) << 24)))
#endif
#endif

#ifndef bswap64
#if defined __GNUC__
#define bswap64(x) __builtin_bswap64(x)
#else
#define bswap64(x) ((uint64_t)((((uint64_t) (x) & 0xff00000000000000ull) >> 56) | \
                    (((uint64_t) (x) & 0x00ff000000000000ull) >> 40) | \
                    (((uint64_t) (x) & 0x0000ff0000000000ull) >> 24) | \
                    (((uint64_t) (x) & 0x000000ff00000000ull) >> 8) | \
                    (((uint64_t) (x) & 0x00000000ff000000ull) << 8) | \
                    (((uint64_t) (x) & 0x0000000000ff0000ull) << 24) | \
                    (((uint64_t) (x) & 0x000000000000ff00ull) << 40) | \
                    (((uint64_t) (x) & 0x00000000000000ffull) << 56)))
#endif
#endif

#ifndef htobe16
#define htobe16(x) host_endian.value == endian_little ? bswap16((x)) : ((uint16_t)(x))
#endif
#ifndef htole16
#define htole16(x) host_endian.value == endian_little ? ((uint16_t)(x)) : bswap16((x))
#endif
#ifndef be16toh
#define be16toh(x) host_endian.value == endian_little ? bswap16((x)) : ((uint16_t)(x))
#endif
#ifndef le16toh
#define le16toh(x) host_endian.value == endian_little ? ((uint16_t)(x)) : bswap16((x))
#endif

#ifndef htobe32
#define htobe32(x) host_endian.value == endian_little ? bswap32((x)) : ((uint32_t)(x))
#endif
#ifndef htole32
#define htole32(x) host_endian.value == endian_little ? ((uint32_t)(x)) : bswap32((x))
#endif
#ifndef be32toh
#define be32toh(x) host_endian.value == endian_little ? bswap32((x)) : ((uint32_t)(x))
#endif
#ifndef le32toh
#define le32toh(x) host_endian.value == endian_little ? ((uint32_t)(x)) : bswap32((x))
#endif

#ifndef htobe64
#define htobe64(x) host_endian.value == endian_little ? bswap64((x)) : ((uint64_t)(x))
#endif
#ifndef htole64
#define htole64(x) host_endian.value == endian_little ? ((uint64_t)(x)) : bswap64((x))
#endif
#ifndef be64toh
#define be64toh(x) host_endian.value == endian_little ? bswap64((x)) : ((uint64_t)(x))
#endif
#ifndef le64toh
#define le64toh(x) host_endian.value == endian_little ? ((uint64_t)(x)) : bswap64((x))
#endif

#endif
