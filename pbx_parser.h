#ifndef pbxparser_h
#define pbxparser_h


/* logging */

std::string format_string(const char* fmt, ...);
void log_prefix(const char* prefix, const char* fmt, va_list arg);
void log_fatal_exit(const char* fmt, ...);
void log_error(const char* fmt, ...);
void log_info(const char* fmt, ...);
void log_debug(const char* fmt, ...);


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
	static std::string escape_quotes(std::string str);
	static bool literal_is_hex_id(std::string str);
	static std::vector<char> read_file(std::string filename);
};


/* PBX Primitives */

struct PBXValue;
typedef std::shared_ptr<PBXValue> PBXValuePtr;

struct PBXObject;
typedef std::shared_ptr<PBXObject> PBXObjectPtr;

struct PBXRoot;
typedef std::shared_ptr<PBXRoot> PBXRootPtr;

enum PBXType {
	PBXTypeRoot,
	PBXTypeId,
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

struct PBXKey {
	std::string key_val;
	std::string comment_val;

	PBXKey(const std::string &key_val, const std::string &comment_val) : key_val(key_val), comment_val(comment_val) {}

	bool operator<(const PBXKey &o) const { return key_val < o.key_val; }
	bool operator==(const PBXKey &o) const { return key_val == o.key_val; }
};

struct PBXValue {
	virtual ~PBXValue() {}
	virtual PBXType type() = 0;
};

struct PBXId : PBXValue {
	std::string id_val;
	std::string comment_val;

	PBXId() {}
	PBXId(std::string id_val) : id_val(id_val) {}
	PBXId(std::string id_val, std::string comment_val) : id_val(id_val), comment_val(comment_val) {}
	PBXId(const PBXId& o) : id_val(o.id_val), comment_val(o.comment_val) {}

	virtual PBXType type() { return PBXTypeId; }
};

struct PBXMap : PBXValue {
	std::map<std::string,PBXValuePtr> object_val;
	std::vector<PBXKey> key_order;

	virtual PBXType type() { return PBXTypeMap; }

	void clear() {
		object_val.clear();
		key_order.clear();
	}

	void put(std::string key, std::string comment, PBXValuePtr &val) {
		if (object_val.find(key) == object_val.end()) {
			object_val[key] = val;
			key_order.push_back(PBXKey(key, comment));
		} else {
			log_fatal_exit("duplicate key \"%s\" in object", key.c_str());
		}
	}

	void replace(std::string key, PBXValuePtr &val) {
		if (object_val.find(key) != object_val.end()) {
			object_val[key] = val;
		} else {
			log_fatal_exit("missing key \"%s\" in object", key.c_str());
		}
	}
};

struct PBXArray : PBXValue {
	std::vector<PBXValuePtr> array_val;

	virtual PBXType type() { return PBXTypeArray; }

	void add(PBXValuePtr &val) {
		array_val.push_back(val);
	}
};

struct PBXLiteral : PBXValue {
	std::string literal_val;

	PBXLiteral(std::string literal_val) : literal_val(literal_val) {}

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


/* PBX classes */

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


/* PBX object factory */

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
	static PBXObject* create(std::string class_name, const PBXId &object_id, const PBXMap &map);

	virtual ~PBXObjectFactory() {}
	virtual PBXObject* create() = 0;
};

template <typename T>
struct PBXObjectFactoryImpl : PBXObjectFactory {
	PBXObject* create() { return new T(); }
};


/* PBX project parser */

enum PBXParseState {
	PBXParseStateNone                       = 0,
	PBXParseStateSlashBang                  = 1,
	PBXParseStateEatWhitespace              = 2,
	PBXParseStateObjectComment              = 3,
	PBXParseStateObjectAttrName             = 4,
	PBXParseStateObjectAttrComment          = 5,
	PBXParseStateObjectAttrEquals           = 6,
	PBXParseStateObjectValue                = 7,
	PBXParseStateObjectValueQuotedLiteral   = 8,
	PBXParseStateObjectValueLiteral         = 9,
	PBXParseStateObjectValueComment         = 10,
	PBXParseStateObjectValueSemicolon       = 11,
	PBXParseStateArrayValue                 = 12,
	PBXParseStateArrayValueLiteral          = 13,
	PBXParseStateArrayValueQuotedLiteral    = 14,
	PBXParseStateArrayValueComment          = 15,
	PBXParseStateArrayValueComma            = 16,
	PBXParseStateFinalSemicolon             = 17,
	PBXParseStateTrailingWhitespace         = 18,
};

enum PBXParseError {
	PBXParseErrorNone                       = 0,
	PBXParseErrorInvalidSlashBang           = 1,
	PBXParseErrorExpectedEquals             = 2,
	PBXParseErrorExpectedSemicolon          = 3,
	PBXParseErrorUnexpectedBracket          = 4,
	PBXParseErrorUnexpectedParenthesis      = 5,
	PBXParseErrorExpectedArraySeparator     = 6,
	PBXParseErrorExpectedWhitespace         = 7
};

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


/* PBX parser implementation */

struct PBXParserImpl : PBXParser {
	static const bool debug = false;

	PBXRootPtr root;
	PBXValuePtr valptr;
	std::vector<PBXValuePtr> value_stack;
	std::string current_attr_name;
	std::string current_attr_comment;

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


/* PBX writer */

struct PBXWriter {
	static void write(PBXValuePtr value, std::stringstream &ss, int indent);
};

#endif