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


/* PBX primitives */

struct Xcodeproj;
struct PBXValue;
struct PBXObject;
struct PBXMap;
struct PBXArray;

typedef std::shared_ptr<Xcodeproj> XcodeprojPtr;
typedef std::shared_ptr<PBXValue> PBXValuePtr;
typedef std::shared_ptr<PBXObject> PBXObjectPtr;
typedef std::shared_ptr<PBXMap> PBXMapPtr;
typedef std::shared_ptr<PBXArray> PBXArrayPtr;

enum PBXType {
	PBXTypeXcodeproj,
	PBXTypeId,
	PBXTypeMap, 
	PBXTypeArray,
	PBXTypeLiteral,
	PBXTypeObject
};

struct PBXKey {
	std::string key_val;
	std::string comment_val;

	PBXKey(const std::string &key_val) : key_val(key_val) {}
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

	void put(std::string key, std::string comment, PBXValuePtr val);
	void replace(std::string key, PBXValuePtr val);

	PBXId getId(std::string key);
	std::string getString(std::string key, std::string default_str = "");
	int getInteger(std::string key, int default_int = 0);
	bool getBoolean(std::string key, bool default_bool = false);
	PBXArrayPtr getArray(std::string key, bool default_create = true);
	PBXMapPtr getMap(std::string key, bool default_create = true);
	PBXObjectPtr getObject(PBXId id);

	void setId(std::string key, PBXId id);
	void setString(std::string key, std::string str_val);
	void setInteger(std::string key, int int_val);
	void setBoolean(std::string key, bool bool_val);
	void setArray(std::string key, PBXArrayPtr arr);
	void setMap(std::string key, PBXMapPtr map);
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

struct PBXObject : PBXMap {
	PBXId object_id;

	virtual ~PBXObject() {}

	virtual PBXType type() { return PBXTypeObject; }

	virtual std::string type_name() { return std::string(); };

	virtual void sync_from_map() {}
	virtual void sync_to_map() {}
	virtual std::string to_string() {
		std::stringstream ss;
		ss << type_name() << "-" << object_id.id_val;
		return ss.str();
	}
};


/* PBX classes */

struct PBXProject;
typedef std::shared_ptr<PBXProject> PBXProjectPtr;

template <typename T> struct PBXObjectImpl : PBXObject {
	std::string type_name() { return T::type_name; }
};

struct Xcodeproj : PBXObjectImpl<Xcodeproj> {
	static const std::string type_name;
	virtual PBXType type() { return PBXTypeXcodeproj; }

	int archiveVersion;
	PBXMapPtr classes;
	int objectVersion;
	PBXMapPtr objects;
	PBXId rootObject;

	void sync_from_map();
	void sync_to_map();

	PBXProjectPtr getProject() {
		return std::static_pointer_cast<PBXProject>(objects->getObject(rootObject));
	}

	template<typename T> std::shared_ptr<T> getObject(PBXId id) {
		return std::static_pointer_cast<T>(objects->getObject(id));
	}
};

struct PBXAggregateTarget : PBXObjectImpl<PBXAggregateTarget> {
	static const std::string type_name;
};

struct PBXAppleScriptBuildPhase : PBXObjectImpl<PBXAppleScriptBuildPhase> {
	static const std::string type_name;
};

struct PBXBuildFile : PBXObjectImpl<PBXBuildFile> {
	static const std::string type_name;

	PBXId fileRef;

	void sync_from_map();
	void sync_to_map();
};

struct PBXBuildRule : PBXObjectImpl<PBXBuildRule> {
	static const std::string type_name;
};

struct PBXBuildStyle : PBXObjectImpl<PBXBuildStyle> {
	static const std::string type_name;
};

struct PBXContainerItemProxy : PBXObjectImpl<PBXContainerItemProxy> {
	static const std::string type_name;
};

struct PBXCopyFilesBuildPhase : PBXObjectImpl<PBXCopyFilesBuildPhase> {
	static const std::string type_name;

	int buildActionMask;
	std::string dstPath;
	bool dstSubfolderSpec;
	PBXArrayPtr files;
	bool runOnlyForDeploymentPostprocessing;

	void sync_from_map();
	void sync_to_map();
};

struct PBXFileReference : PBXObjectImpl<PBXFileReference> {
	static const std::string type_name;

	std::string explicitFileType;
	std::string lastKnownFileType;
	bool includeInIndex;
	std::string path;
	std::string sourceTree;

	void sync_from_map();
	void sync_to_map();
};

struct PBXFrameworksBuildPhase : PBXObjectImpl<PBXFrameworksBuildPhase> {
	static const std::string type_name;

	int buildActionMask;
	PBXArrayPtr files;
	bool runOnlyForDeploymentPostprocessing;

	void sync_from_map();
	void sync_to_map();
};

struct PBXGroup : PBXObjectImpl<PBXGroup> {
	static const std::string type_name;

	PBXArrayPtr children;
	std::string name;
	std::string path;
	std::string sourceTree;

	void sync_from_map();
	void sync_to_map();
};

struct PBXHeadersBuildPhase : PBXObjectImpl<PBXHeadersBuildPhase> {
	static const std::string type_name;
};

struct PBXLegacyTarget : PBXObjectImpl<PBXLegacyTarget> {
	static const std::string type_name;
};

struct PBXNativeTarget : PBXObjectImpl<PBXNativeTarget> {
	static const std::string type_name;

	PBXArrayPtr buildPhases;
	PBXArrayPtr buildRules;
	PBXArrayPtr dependencies;
	std::string name;
	std::string productName;
	PBXId productReference;
	std::string productType;

	void sync_from_map();
	void sync_to_map();
};

struct PBXProject : PBXObjectImpl<PBXProject> {
	static const std::string type_name;

	PBXMapPtr attributes;
	PBXId buildConfigurationList;
	std::string compatibilityVersion;
	std::string developmentRegion;
	bool hasScannedForEncodings;
	PBXArrayPtr knownRegions;
	PBXId mainGroup;
	PBXId productRefGroup;
	std::string projectDirPath;
	std::string projectRoot;
	PBXArrayPtr targets;

	void sync_from_map();
	void sync_to_map();
};

struct PBXReferenceProxy : PBXObjectImpl<PBXReferenceProxy> {
	static const std::string type_name;
};

struct PBXResourcesBuildPhase : PBXObjectImpl<PBXResourcesBuildPhase> {
	static const std::string type_name;
};

struct PBXShellScriptBuildPhase : PBXObjectImpl<PBXShellScriptBuildPhase> {
	static const std::string type_name;
};

struct PBXSourcesBuildPhase : PBXObjectImpl<PBXSourcesBuildPhase> {
	static const std::string type_name;

	int buildActionMask;
	PBXArrayPtr files;
	bool runOnlyForDeploymentPostprocessing;

	void sync_from_map();
	void sync_to_map();
};

struct PBXTargetDependency : PBXObjectImpl<PBXTargetDependency> {
	static const std::string type_name;
};

struct PBXVariantGroup : PBXObjectImpl<PBXVariantGroup> {
	static const std::string type_name;
};

struct XCBuildConfiguration : PBXObjectImpl<XCBuildConfiguration> {
	static const std::string type_name;

	PBXArrayPtr buildSettings;
	std::string name;

	void sync_from_map();
	void sync_to_map();
};

struct XCConfigurationList : PBXObjectImpl<XCConfigurationList> {
	static const std::string type_name;

	PBXArrayPtr buildConfigurations;
	int defaultConfigurationIsVisible;
	std::string defaultConfigurationName;

	void sync_from_map();
	void sync_to_map();
};

struct XCVersionGroup : PBXObjectImpl<XCVersionGroup> {
	static const std::string type_name;
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
			(T::type_name, PBXObjectFactoryPtr(new PBXObjectFactoryImpl<T>())));
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

	XcodeprojPtr xcodeproj;
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