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

#include "pbx_parser.h"


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


/* utility */

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

std::string PBXUtil::escape_quotes(std::string str)
{
	std::stringstream ss;
	for (size_t i = 0; i < str.length(); i++) {
		char c = str[i];
		if (c == '"') ss << "\\";
		ss << c;
	}
	return ss.str();
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


/* PBX object factory */

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

PBXObject* PBXObjectFactory::create(std::string class_name, const PBXId &object_id, const PBXMap &map)
{
	init();
	auto it = factoryMap.find(class_name);
	PBXObject *ptr = it != factoryMap.end() ? it->second->create() : new PBXObject();
	ptr->object_id = object_id;
	ptr->object_val = map.object_val;
	ptr->key_order = map.key_order;
	return ptr;
}

std::once_flag PBXObjectFactory::factoryInit;
std::map<std::string,PBXObjectFactoryPtr> PBXObjectFactory::factoryMap;


/* PBX classes */

const std::string PBXAggregateTarget::name =            "PBXAggregateTarget";
const std::string PBXAppleScriptBuildPhase::name =      "PBXAppleScriptBuildPhase";
const std::string PBXBuildFile::name =                  "PBXBuildFile";
const std::string PBXBuildStyle::name =                 "PBXBuildStyle";
const std::string PBXContainerItemProxy::name =         "PBXContainerItemProxy";
const std::string PBXCopyFilesBuildPhase::name =        "PBXCopyFilesBuildPhase";
const std::string PBXFileReference::name =              "PBXFileReference";
const std::string PBXFrameworksBuildPhase::name =       "PBXFrameworksBuildPhase";
const std::string PBXGroup::name =                      "PBXGroup";
const std::string PBXHeadersBuildPhase::name =          "PBXHeadersBuildPhase";
const std::string PBXNativeTarget::name =               "PBXNativeTarget";
const std::string PBXProject::name =                    "PBXProject";
const std::string PBXReferenceProxy::name =             "PBXReferenceProxy";
const std::string PBXResourcesBuildPhase::name =        "PBXResourcesBuildPhase";
const std::string PBXShellScriptBuildPhase::name =      "PBXShellScriptBuildPhase";
const std::string PBXSourcesBuildPhase::name =          "PBXSourcesBuildPhase";
const std::string PBXTargetDependency::name =           "PBXTargetDependency";
const std::string XCBuildConfiguration::name =          "XCBuildConfiguration";
const std::string XCConfigurationList::name =           "XCConfigurationList";


/* PBX project parser */

const std::string pbxproj_slash_bang = "// !$*UTF8*$!";

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


/* PBX parser implementation */

void PBXParserImpl::begin_object() {
	if (debug) {
		log_debug("begin_object");
	}
	if (!root) {
		value_stack.push_back((root = std::make_shared<PBXRoot>()));
	}
	else if (value_stack.size() == 0)
	{
		log_fatal_exit("value stack empty");
	}
	else if (value_stack.back()->type() == PBXTypeRoot ||
			 value_stack.back()->type() == PBXTypeMap)
	{
		PBXMap *map = new PBXMap();
		valptr = PBXValuePtr(map);
		static_cast<PBXMap&>(*value_stack.back()).put(current_attr_name, current_attr_comment, valptr);
		value_stack.push_back(valptr);
	}
	else if (value_stack.back()->type() == PBXTypeArray)
	{
		PBXMap *map = new PBXMap();
		valptr = PBXValuePtr(map);
		static_cast<PBXArray&>(*value_stack.back()).add(valptr);
		value_stack.push_back(valptr);
	}
}

void PBXParserImpl::end_object() {
	if (debug) {
		log_debug("end_object");
	}
	value_stack.pop_back();
}

void PBXParserImpl::object_comment(std::string str) {
	if (debug) {
		log_debug("object_comment: \"%s\"", str.c_str());
	}
	current_attr_comment = str;
}

void PBXParserImpl::object_attr(std::string str) {
	if (debug) {
		log_debug("object_attr: \"%s\"", str.c_str());
	}
	current_attr_name = str;
	current_attr_comment = std::string();
}

void PBXParserImpl::object_attr_comment(std::string str) {
	if (debug) {
		log_debug("object_attr_comment: \"%s\"", str.c_str());
	}
	current_attr_comment = str;
}

void PBXParserImpl::object_value_literal(std::string str) {
	if (debug) {
		log_debug("object_value_literal: \"%s\"", str.c_str());
	}
	bool is_id = PBXUtil::literal_is_hex_id(str);
	if (value_stack.size() == 0)
	{
		log_fatal_exit("value stack empty");
	}
	else if (value_stack.back()->type() == PBXTypeRoot ||
			 value_stack.back()->type() == PBXTypeMap ||
			 value_stack.back()->type() == PBXTypeObject)
	{
		if (is_id) {
			PBXId *id = new PBXId(str);
			valptr = PBXValuePtr(id);
		} else {
			PBXLiteral *lit = new PBXLiteral(str);
			valptr = PBXValuePtr(lit);
		}
		static_cast<PBXMap&>(*value_stack.back()).put(current_attr_name, current_attr_comment, valptr);
	}
	else if (value_stack.back()->type() == PBXTypeArray)
	{
		if (is_id) {
			PBXId *id = new PBXId(str);
			valptr = PBXValuePtr(id);
		} else {
			PBXLiteral *lit = new PBXLiteral(str);
			valptr = PBXValuePtr(lit);
		}
		static_cast<PBXArray&>(*value_stack.back()).add(valptr);
	}
}

void PBXParserImpl::object_value_comment(std::string str) {
	if (debug) {
		log_debug("object_value_comment: \"%s\"", str.c_str());
	}
	if (valptr->type() == PBXTypeId) {
		static_cast<PBXId&>(*valptr).comment_val = str;
	}
}

void PBXParserImpl::begin_array() {
	if (debug) {
		log_debug("begin_array");
	}
	if (value_stack.size() == 0)
	{
		log_fatal_exit("value stack empty");
	}
	else if (value_stack.back()->type() == PBXTypeRoot ||
			 value_stack.back()->type() == PBXTypeMap ||
			 value_stack.back()->type() == PBXTypeObject)
	{
		PBXArray *arr = new PBXArray();
		valptr = PBXValuePtr(arr);
		static_cast<PBXMap&>(*value_stack.back()).put(current_attr_name, current_attr_comment, valptr);
		value_stack.push_back(valptr);
	}
	else if (value_stack.back()->type() == PBXTypeArray)
	{
		PBXArray *arr = new PBXArray();
		valptr = PBXValuePtr(arr);
		static_cast<PBXArray&>(*value_stack.back()).add(valptr);
		value_stack.push_back(valptr);
	}
}

void PBXParserImpl::end_array() {
	if (debug) {
		log_debug("end_array");
	}
	value_stack.pop_back();
}

void PBXParserImpl::array_value_literal(std::string str) {
	if (debug) {
		log_debug("array_value_literal: \"%s\"", str.c_str());
	}
	bool is_id = PBXUtil::literal_is_hex_id(str);
	if (value_stack.size() == 0)
	{
		log_fatal_exit("value stack empty");
	}
	else if (value_stack.back()->type() == PBXTypeArray)
	{
		if (is_id) {
			PBXId *id = new PBXId(str);
			valptr = PBXValuePtr(id);
		} else {
			PBXLiteral *lit = new PBXLiteral(str);
			valptr = PBXValuePtr(lit);
		}
		static_cast<PBXArray&>(*value_stack.back()).array_val.push_back(valptr);
	}
}

void PBXParserImpl::array_value_comment(std::string str) {
	if (debug) {
		log_debug("array_value_comment: \"%s\"", str.c_str());
	}
	if (valptr->type() == PBXTypeId) {
		static_cast<PBXId&>(*valptr).comment_val = str;
	}
}


/* PBX writer */

void PBXWriter::write(PBXValuePtr value, std::stringstream &ss, int indent) {
	switch (value->type()) {
		case PBXTypeRoot:
		{
			ss << pbxproj_slash_bang << std::endl;
			ss << "{" << std::endl;
			PBXMap &map = static_cast<PBXMap&>(*value);
			for (const PBXKey &key : map.key_order) {
				PBXValuePtr &val = map.object_val[key.key_val];
				ss << "\t" << key.key_val;
				if (key.comment_val.length() > 0) {
					ss << " /* " << key.comment_val << " */";
				}
				ss << " = ";
				write(val, ss, indent + 1);
				ss << ";" << std::endl;
			}
			ss << "}" << std::endl;
			break;
		}
		case PBXTypeMap:
		case PBXTypeObject:
		{
			ss << "{" << std::endl;
			PBXMap &map = static_cast<PBXMap&>(*value);
			for (const PBXKey &key : map.key_order) {
				PBXValuePtr &val = map.object_val[key.key_val];
				for (int i = 0; i <= indent; i++) ss << "\t";
				ss << key.key_val;
				if (key.comment_val.length() > 0) {
					ss << " /* " << key.comment_val << " */";
				}
				ss << " = ";
				write(val, ss, indent + 1);
				ss << ";" << std::endl;
			}
			for (int i = 0; i < indent; i++) ss << "\t";
			ss << "}";
			break;
		}
		case PBXTypeArray:
		{
			ss << "(" << std::endl;
			PBXArray &arr = static_cast<PBXArray&>(*value);
			for (PBXValuePtr &val : arr.array_val) {
				for (int i = 0; i <= indent; i++) ss << "\t";
				write(val, ss, indent + 1);
				ss << "," << std::endl;
			}
			for (int i = 0; i < indent; i++) ss << "\t";
			ss << ")";
			break;
		}
		case PBXTypeLiteral:
		{
			PBXLiteral &lit = static_cast<PBXLiteral&>(*value);
			if (PBXUtil::literal_requires_quotes(lit.literal_val)) {
				ss << "\"" << PBXUtil::escape_quotes(lit.literal_val) << "\"";
			} else {
				ss << lit.literal_val;
			}
			break;
		}
		case PBXTypeId:
		{
			PBXId &id = static_cast<PBXId&>(*value);
			ss << id.id_val;
			if (id.comment_val.length() > 0) {
				ss << " /* " << id.comment_val << " */";
			}
			break;
		}
	}
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
		log_fatal_exit("error parsing project: %d", error);
	}

	std::stringstream ss;
	PBXWriter::write(pbx.root, ss, 0);
	printf("%s", ss.str().c_str());
}
