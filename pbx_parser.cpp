#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cerrno>
#include <string>
#include <sstream>
#include <memory>
#include <algorithm>
#include <vector>
#include <map>
#include <mutex>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


/* PBX parsing state */

enum PBXParseState {
	PBXParseStateNone						= 0,
	PBXParseStateSlashBang					= 1,
	PBXParseStateEatWhitespace				= 2,
	PBXParseStateObjectComment				= 3,
	PBXParseStateObjectAttrName				= 4,
	PBXParseStateObjectAttrComment			= 5,
	PBXParseStateObjectAttrEquals			= 6,
	PBXParseStateObjectValue				= 7,
	PBXParseStateObjectValueQuotedLiteral	= 8,
	PBXParseStateObjectValueLiteral			= 9,
	PBXParseStateObjectValueComment			= 10,
	PBXParseStateObjectValueSemicolon		= 11,
	PBXParseStateArrayValue					= 12,
	PBXParseStateArrayValueLiteral			= 13,
	PBXParseStateArrayValueQuotedLiteral	= 14,
	PBXParseStateArrayValueComment			= 15,
	PBXParseStateArrayValueComma			= 16,
	PBXParseStateFinalSemicolon				= 17,
	PBXParseStateTrailingWhitespace			= 18,
};

enum PBXParseError {
	PBXParseErrorNone						= 0,
	PBXParseErrorInvalidSlashBang			= 1,
	PBXParseErrorExpectedEquals				= 2,
	PBXParseErrorExpectedSemicolon			= 3,
	PBXParseErrorUnexpectedBracket			= 4,
	PBXParseErrorUnexpectedParenthesis		= 5,
	PBXParseErrorExpectedArraySeparator		= 6, 
	PBXParseErrorExpectedWhitespace			= 7
};


/* PBX Primitives */

struct PBXComment;
typedef std::shared_ptr<PBXComment> PBXCommentPtr;

struct PBXValue;
typedef std::shared_ptr<PBXValue> PBXValuePtr;

struct PBXObject;
typedef std::shared_ptr<PBXObject> PBXObjectPtr;

struct PBXRoot;
typedef std::shared_ptr<PBXRoot> PBXRootPtr;

struct PBXComment {
	std::string comment;
};

enum PBXType {
	PBXTypeRoot,
	PBXTypeId,
	PBXTypeIdRef,
	PBXTypeMap, 
	PBXTypeArray,
	PBXTypeLiteral,
	PBXTypeObject
};

union PBXIdUnion {
	char id_val[12];
	struct {
		uint32_t id_local;
		uint32_t id_project_1;
		uint32_t id_project_2;
	} id_comp;

	bool operator<(const PBXIdUnion &o) { return memcmp(this, &o, sizeof(*this)) == -1; }
	bool operator==(const PBXIdUnion &o) { return memcmp(this, &o, sizeof(*this)) == 0; }
};

struct PBXValue {
	virtual ~PBXValue() {}
	virtual PBXType type() = 0;
};

struct PBXId : PBXValue {
	PBXIdUnion id;
	PBXCommentPtr comment;
	virtual PBXType type() { return PBXTypeId; }
};

struct PBXIdRef : PBXValue {
	PBXIdUnion id_ref;
	PBXCommentPtr comment;
	virtual PBXType type() { return PBXTypeIdRef; }
};

struct PBXMap : PBXValue {
	std::map<std::string,PBXValuePtr> object_val;
	virtual PBXType type() { return PBXTypeMap; }
};

struct PBXArray : PBXValue {
	std::vector<PBXValuePtr> array_val;
	virtual PBXType type() { return PBXTypeArray; }
};

struct PBXLiteral : PBXValue {
	std::string literal_val;
	virtual PBXType type() { return PBXTypeLiteral; }
};

struct PBXRoot: PBXMap {
	virtual PBXType type() { return PBXTypeRoot; }
};

struct PBXObject : PBXMap {
	PBXId object_id;
	virtual ~PBXObject() {}
	PBXType type() { return PBXTypeObject; }
	virtual std::string class_name() { return std::string(); };
};


/* PBX Classes */

template <typename T> struct PBXObjectImpl : PBXObject {
	std::string class_name() { return T::name; }
};

struct PBXAggregateTarget : PBXObjectImpl<PBXAggregateTarget> {
	static const std::string name;
};

struct PBXAppleScriptBuildPhase : PBXObjectImpl<PBXAppleScriptBuildPhase> {
	static const std::string name;
};

struct PBXBuildFile : PBXObjectImpl<PBXBuildFile> {
	static const std::string name;
};

struct PBXBuildStyle : PBXObjectImpl<PBXBuildStyle> {
	static const std::string name;
};

struct PBXContainerItemProxy : PBXObjectImpl<PBXContainerItemProxy> {
	static const std::string name;
};

struct PBXCopyFilesBuildPhase : PBXObjectImpl<PBXCopyFilesBuildPhase> {
	static const std::string name;
};

struct PBXFileReference : PBXObjectImpl<PBXFileReference> {
	static const std::string name;
};

struct PBXFrameworksBuildPhase : PBXObjectImpl<PBXFrameworksBuildPhase> {
	static const std::string name;
};

struct PBXGroup : PBXObjectImpl<PBXGroup> {
	static const std::string name;
};

struct PBXHeadersBuildPhase : PBXObjectImpl<PBXHeadersBuildPhase> {
	static const std::string name;
};

struct PBXNativeTarget : PBXObjectImpl<PBXNativeTarget> {
	static const std::string name;
};

struct PBXProject : PBXObjectImpl<PBXProject> {
	static const std::string name;
};

struct PBXReferenceProxy : PBXObjectImpl<PBXReferenceProxy> {
	static const std::string name;
};

struct PBXResourcesBuildPhase : PBXObjectImpl<PBXResourcesBuildPhase> {
	static const std::string name;
};

struct PBXShellScriptBuildPhase : PBXObjectImpl<PBXShellScriptBuildPhase> {
	static const std::string name;
};

struct PBXSourcesBuildPhase : PBXObjectImpl<PBXSourcesBuildPhase> {
	static const std::string name;
};

struct PBXTargetDependency : PBXObjectImpl<PBXTargetDependency> {
	static const std::string name;
};

struct XCBuildConfiguration : PBXObjectImpl<XCBuildConfiguration> {
	static const std::string name;
};

struct XCConfigurationList : PBXObjectImpl<XCConfigurationList> {
	static const std::string name;
};


/* PBXObjectFactory */

struct PBXObjectFactory;
typedef std::shared_ptr<PBXObjectFactory> PBXObjectFactoryPtr;
template <typename T> struct PBXObjectFactoryImpl;

struct PBXObjectFactory {
	static std::once_flag factoryInit;
	static std::map<std::string,PBXObjectFactoryPtr> factoryMap;

	template <typename T> static void registerFactory() {
		factoryMap.insert(std::pair<std::string,PBXObjectFactoryPtr>
			(T::name, PBXObjectFactoryPtr(new PBXObjectFactoryImpl<T>())));
	}

	static void init();
	static PBXObjectPtr create(std::string class_name, const PBXId &object_id);

	virtual ~PBXObjectFactory() {}
	virtual PBXObjectPtr make() = 0;
};

template <typename T>
struct PBXObjectFactoryImpl : PBXObjectFactory {
	PBXObjectPtr make() { return std::make_shared<T>(); }
};

void PBXObjectFactory::init()
{
	std::call_once(factoryInit, [](){
		registerFactory<PBXAggregateTarget>();
		registerFactory<PBXAppleScriptBuildPhase>();
		registerFactory<PBXBuildFile>();
		registerFactory<PBXBuildStyle>();
		registerFactory<PBXContainerItemProxy>();
		registerFactory<PBXCopyFilesBuildPhase>();
		registerFactory<PBXFileReference>();
		registerFactory<PBXFrameworksBuildPhase>();
		registerFactory<PBXGroup>();
		registerFactory<PBXHeadersBuildPhase>();
		registerFactory<PBXNativeTarget>();
		registerFactory<PBXProject>();
		registerFactory<PBXReferenceProxy>();
		registerFactory<PBXResourcesBuildPhase>();
		registerFactory<PBXShellScriptBuildPhase>();
		registerFactory<PBXSourcesBuildPhase>();
		registerFactory<PBXTargetDependency>();
		registerFactory<XCBuildConfiguration>();
		registerFactory<XCConfigurationList>();
	});
}

PBXObjectPtr PBXObjectFactory::create(std::string class_name, const PBXId &object_id)
{
	init();
	auto it = factoryMap.find(class_name);
	PBXObjectPtr ptr = it != factoryMap.end() ?
		it->second->make() : std::make_shared<PBXObject>();
	ptr->object_id = object_id;
	return ptr;
}


/* static initialization */

const std::string pbxproj_slash_bang = "// !$*UTF8*$!";

const std::string PBXAggregateTarget::name =			"PBXAggregateTarget";
const std::string PBXAppleScriptBuildPhase::name =		"PBXAppleScriptBuildPhase";
const std::string PBXBuildFile::name =					"PBXBuildFile";
const std::string PBXBuildStyle::name =					"PBXBuildStyle";
const std::string PBXContainerItemProxy::name =			"PBXContainerItemProxy";
const std::string PBXCopyFilesBuildPhase::name =		"PBXCopyFilesBuildPhase";
const std::string PBXFileReference::name =				"PBXFileReference";
const std::string PBXFrameworksBuildPhase::name =		"PBXFrameworksBuildPhase";
const std::string PBXGroup::name =						"PBXGroup";
const std::string PBXHeadersBuildPhase::name =			"PBXHeadersBuildPhase";
const std::string PBXNativeTarget::name =				"PBXNativeTarget";
const std::string PBXProject::name =					"PBXProject";
const std::string PBXReferenceProxy::name =				"PBXReferenceProxy";
const std::string PBXResourcesBuildPhase::name =		"PBXResourcesBuildPhase";
const std::string PBXShellScriptBuildPhase::name =		"PBXShellScriptBuildPhase";
const std::string PBXSourcesBuildPhase::name =			"PBXSourcesBuildPhase";
const std::string PBXTargetDependency::name =			"PBXTargetDependency";
const std::string XCBuildConfiguration::name =			"XCBuildConfiguration";
const std::string XCConfigurationList::name =			"XCConfigurationList";

std::once_flag PBXObjectFactory::factoryInit;
std::map<std::string,PBXObjectFactoryPtr> PBXObjectFactory::factoryMap;


/* logging */

std::string format_string(const char* fmt, ...);
void log_prefix(const char* prefix, const char* fmt, va_list arg);
void log_fatal_exit(const char* fmt, ...);
void log_error(const char* fmt, ...);
void log_info(const char* fmt, ...);
void log_debug(const char* fmt, ...);

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


/* utility */

struct PBXUtil {
	static const char* HEX_DIGITS;
	static const char* LITERAL_CHARS;

	static std::string ltrim(std::string s);
	static std::string rtrim(std::string s);
	static std::string trim(std::string s);
	static std::string hex_encode(const unsigned char *buf, size_t len);
	static void hex_decode(std::string hex, char *buf, size_t len);
	static bool literal_requires_quotes(std::string str);
	static bool literal_is_hex_id(std::string str);
	static std::vector<char> read_file(std::string filename);
};

const char* PBXUtil::HEX_DIGITS = "0123456789ABCDEF";
const char* PBXUtil::LITERAL_CHARS = "/._";

std::string PBXUtil::ltrim(std::string s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
			std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

std::string PBXUtil::rtrim(std::string s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
			std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

std::string PBXUtil::trim(std::string s) {
	return ltrim(rtrim(s));
}


std::string PBXUtil::hex_encode(const unsigned char *buf, size_t len)
{
    std::string hex;
    for (size_t i = 0; i < len; i++) {
        unsigned char b = buf[i];
        hex.append(HEX_DIGITS + ((b >> 4) & 0x0F), 1);
        hex.append(HEX_DIGITS + (b & 0x0F), 1);
    }
    return hex;
}

void PBXUtil::hex_decode(std::string hex, char *buf, size_t len)
{
    for (size_t i = 0; i < hex.length()/2 && i < len; i++) {
        const char tmp[3] = { hex[i*2], hex[i*2+1], 0 };
        *buf++ = (char)strtoul(tmp, NULL, 16);
    }
}

bool PBXUtil::literal_requires_quotes(std::string str)
{
	if (str.size() == 0) return true;
    for (size_t i = 0; i < str.length(); i++) {
    	char c = str[i];
    	if (!isalnum(c) && strchr(LITERAL_CHARS, c) == NULL) return true;
    }
    return false;
}

bool PBXUtil::literal_is_hex_id(std::string str)
{
	if (str.size() != 24) return false;
    for (size_t i = 0; i < str.length(); i++) {
    	if (strchr(HEX_DIGITS, str[i]) == NULL) return false;
    }
    return true;
}

std::vector<char> PBXUtil::read_file(std::string filename)
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
		log_fatal_exit("error short read: %s: %s", filename.c_str());
	}
	::close(fd);

	return buf;
}

/* PBX project parser */

struct PBXParser {
	PBXParseError parse(std::vector<char> &buf);

	virtual void begin_object() = 0;
	virtual void end_object() = 0;
	virtual void object_comment(std::string str) = 0;
	virtual void object_attr(std::string str) = 0;
	virtual void object_attr_comment(std::string str) = 0;
	virtual void object_value_literal(std::string str) = 0;
	virtual void object_value_comment(std::string str) = 0;
	virtual void begin_array() = 0;
	virtual void end_array() = 0;
	virtual void array_value_literal(std::string str) = 0;
	virtual void array_value_comment(std::string str) = 0;
};

PBXParseError PBXParser::parse(std::vector<char> &buf)
{
	PBXParseState state = PBXParseStateSlashBang;
	PBXParseState saved_state = PBXParseStateNone;
	std::vector<PBXParseState> stack;
	std::stringbuf token;
	size_t offset = 0;
	char c, peek, last = 0;
	while (offset < buf.size()) {
		c = buf[offset];
		peek = offset < buf.size() - 1 ? buf[offset + 1] : 0;

		switch (state) {
			case PBXParseStateSlashBang:
				if (offset != 0 &&
					buf.size() < pbxproj_slash_bang.size() &&
					memcmp(&buf[offset], pbxproj_slash_bang.c_str(),
							pbxproj_slash_bang.size()) != 0) {
					return PBXParseErrorInvalidSlashBang;
				} else {
					offset += pbxproj_slash_bang.size();
					saved_state = PBXParseStateObjectValue;
					state = PBXParseStateEatWhitespace;
				}
				break;

			case PBXParseStateEatWhitespace:
				if (isspace(c)) {
					offset++;
				} else {
					state = saved_state;
				}
				break;

			case PBXParseStateObjectComment:
				if (c == '*' && peek == '/') {
					object_comment(PBXUtil::rtrim(token.str()));
					token.str(std::string());
					saved_state = PBXParseStateObjectAttrName;
					state = PBXParseStateEatWhitespace;
					offset += 2;
				} else {
					token.sputc(c);
					offset++;
				}
				break;

			case PBXParseStateObjectAttrComment:
				if (c == '*' && peek == '/') {
					object_attr_comment(PBXUtil::rtrim(token.str()));
					token.str(std::string());
					saved_state = PBXParseStateObjectAttrEquals;
					state = PBXParseStateEatWhitespace;
					offset += 2;
				} else {
					token.sputc(c);
					offset++;
				}
				break;

			case PBXParseStateObjectValueComment:
				if (c == '*' && peek == '/') {
					object_value_comment(PBXUtil::rtrim(token.str()));
					token.str(std::string());
					saved_state = PBXParseStateObjectValueSemicolon;
					state = PBXParseStateEatWhitespace;
					offset += 2;
				} else {
					token.sputc(c);
					offset++;
				}
				break;

			case PBXParseStateObjectAttrName:
				if (c == '/' && peek == '*') {
					saved_state = PBXParseStateObjectComment;
					state = PBXParseStateEatWhitespace;
					offset += 2;
				} else if (c == '}') {
					end_object();
					if (stack.size() == 0) {
						return PBXParseErrorUnexpectedBracket;
					} else {
						saved_state = stack.back();
						state = PBXParseStateEatWhitespace;
						offset++;
						stack.pop_back();
					}
				} else if (isspace(c)) {
					object_attr(token.str());
					token.str(std::string());
					saved_state = PBXParseStateObjectAttrEquals;
					state = PBXParseStateEatWhitespace;
				} else {
					token.sputc(c);
					offset++;
				}
				break;

			case PBXParseStateObjectAttrEquals:
				if (c == '/' && peek == '*') {
					saved_state = PBXParseStateObjectAttrComment;
					state = PBXParseStateEatWhitespace;
					offset += 2;
				} else if (c == '=') {
					saved_state = PBXParseStateObjectValue;
					state = PBXParseStateEatWhitespace;
					offset++;
				} else {
					return PBXParseErrorExpectedEquals;
				}
				break;

			case PBXParseStateObjectValue:
				if (c == '{') {
					begin_object();
					saved_state = PBXParseStateObjectAttrName;
					state = PBXParseStateEatWhitespace;
					stack.push_back(PBXParseStateObjectValueSemicolon);
					offset++;
				} else if (c == '(') {
					begin_array();
					saved_state = PBXParseStateArrayValue;
					state = PBXParseStateEatWhitespace;
					stack.push_back(PBXParseStateObjectValueSemicolon);
					offset++;
				} else if (c == '"') {
					offset++;
					state = PBXParseStateObjectValueQuotedLiteral;
				} else {
					state = PBXParseStateObjectValueLiteral;
				}
				break;

			case PBXParseStateObjectValueQuotedLiteral:
				if (c == '\\' && last != '\\') {
					offset++;
				} else if (c == '"' && last != '\\') {
					object_value_literal(token.str());
					token.str(std::string());
					saved_state = PBXParseStateObjectValueSemicolon;
					state = PBXParseStateEatWhitespace;
					offset++;
				} else {
					token.sputc(c);
					offset++;
				}
				break;

			case PBXParseStateObjectValueLiteral:
				if (isspace(c) || c == ';') {
					object_value_literal(token.str());
					token.str(std::string());
					saved_state = PBXParseStateObjectValueSemicolon;
					state = PBXParseStateEatWhitespace;
				} else {
					token.sputc(c);
					offset++;
				}
				break;

			case PBXParseStateObjectValueSemicolon:
				if (c == '/' && peek == '*') {
					saved_state = PBXParseStateObjectValueComment;
					state = PBXParseStateEatWhitespace;
					offset += 2;
				} else if (c == ';') {
					saved_state = PBXParseStateObjectAttrName;
					state = PBXParseStateEatWhitespace;
					offset++;
				} else {
					return PBXParseErrorExpectedSemicolon;
				}
				break;

			case PBXParseStateArrayValue:
				if (c == '"') {
					offset++;
					state = PBXParseStateArrayValueQuotedLiteral;
				} else if (c == ')') {
					end_array();
					if (stack.size() == 0) {
						return PBXParseErrorUnexpectedParenthesis;
					} else {
						saved_state = stack.back();
						state = PBXParseStateEatWhitespace;
						offset++;
						stack.pop_back();
					}
				} else {
					state = PBXParseStateArrayValueLiteral;
				}
				break;

			case PBXParseStateArrayValueComment:
				if (c == '*' && peek == '/') {
					array_value_comment(PBXUtil::rtrim(token.str()));
					token.str(std::string());
					saved_state = PBXParseStateArrayValueComma;
					state = PBXParseStateEatWhitespace;
					offset += 2;
				} else {
					token.sputc(c);
					offset++;
				}
				break;

			case PBXParseStateArrayValueComma:
				if (c == '/' && peek == '*') {
					saved_state = PBXParseStateArrayValueComment;
					state = PBXParseStateEatWhitespace;
					offset += 2;
				} else if (c == ',' || c == ')') {
					saved_state = PBXParseStateArrayValue;
					state = PBXParseStateEatWhitespace;
					offset++;
				} else {
					return PBXParseErrorExpectedArraySeparator;
				}
				break;

			case PBXParseStateArrayValueQuotedLiteral:
				if (c == '\\' && last != '\\') {
					offset++;
				} else if (c == '"' && last != '\\') {
					array_value_literal(token.str());
					token.str(std::string());
					saved_state = PBXParseStateArrayValueComma;
					state = PBXParseStateEatWhitespace;
					offset++;
				} else {
					token.sputc(c);
					offset++;
				}
				break;

			case PBXParseStateArrayValueLiteral:
				if (isspace(c) || c == ',') {
					array_value_literal(token.str());
					token.str(std::string());
					saved_state = PBXParseStateArrayValueComma;
					state = PBXParseStateEatWhitespace;
				} else {
					token.sputc(c);
					offset++;
				}
				break;

			case PBXParseStateFinalSemicolon:
				if (c == ';') {
					offset++;
					state = PBXParseStateTrailingWhitespace;
				} else {
					return PBXParseErrorExpectedSemicolon;
				}
				break;

			case PBXParseStateTrailingWhitespace:
				if (isspace(c)) {
					offset++;
				} else {
					return PBXParseErrorExpectedWhitespace;
				}
				break;

			case PBXParseStateNone:
				break;
		}
		last = c;
	}

	return PBXParseErrorNone;
}


/* PBX Parser Implementation */

struct PBXParserImpl : PBXParser {
	PBXRootPtr root;
	std::vector<PBXValuePtr> value_stack;
	std::string current_attr_name;

	void begin_object();
	void end_object();
	void object_comment(std::string str);
	void object_attr(std::string str);
	void object_attr_comment(std::string str);
	void object_value_literal(std::string str);
	void object_value_comment(std::string str);
	void begin_array();
	void end_array();
	void array_value_literal(std::string str);
	void array_value_comment(std::string str);
};

void PBXParserImpl::begin_object() {
	printf("begin_object\n");
}

void PBXParserImpl::end_object() {
	printf("end_object\n");
}

void PBXParserImpl::object_comment(std::string str) {
	printf("object_comment: \"%s\"\n", str.c_str());
}

void PBXParserImpl::object_attr(std::string str) {
	printf("object_attr: \"%s\" id=%d\n", str.c_str(),
		PBXUtil::literal_is_hex_id(str));
}

void PBXParserImpl::object_attr_comment(std::string str) {
	printf("object_attr_comment: \"%s\"\n", str.c_str());
}

void PBXParserImpl::object_value_literal(std::string str) {
	printf("object_value_literal: \"%s\" quote=%d id=%d\n", str.c_str(),
		PBXUtil::literal_requires_quotes(str),
		PBXUtil::literal_is_hex_id(str));
}

void PBXParserImpl::object_value_comment(std::string str) {
	printf("object_value_comment: \"%s\"\n", str.c_str());
}

void PBXParserImpl::begin_array() {
	printf("begin_array\n");
}

void PBXParserImpl::end_array() {
	printf("end_array\n");
}

void PBXParserImpl::array_value_literal(std::string str) {
	printf("array_value_literal: \"%s\" quote=%d id=%d\n", str.c_str(),
		PBXUtil::literal_requires_quotes(str),
		PBXUtil::literal_is_hex_id(str));
}

void PBXParserImpl::array_value_comment(std::string str) {
	printf("array_value_comment: \"%s\"\n", str.c_str());
}


/* main */

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <xcodeproj>\n", argv[0]);
		exit(1);
	}
	PBXParserImpl pbx;
	std::vector<char> buf = PBXUtil::read_file(argv[1]);
	PBXParseError error = pbx.parse(buf);
	if (error != PBXParseErrorNone) {
		log_fatal_exit("error parsing project: %d\n", error);
	}
}
