#ifndef pbx_io_h
#define pbx_io_h


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
	std::string str;
	std::string comment;

	PBXKey(const std::string &str) : str(str) {}
	PBXKey(const std::string &str, const std::string &comment) : str(str), comment(comment) {}

	bool operator<(const PBXKey &o) const { return str < o.str; }
	bool operator==(const PBXKey &o) const { return str == o.str; }
};

struct PBXValue {
	virtual ~PBXValue() {}
	virtual PBXType type() = 0;
};

union PBXIdUnion {
        unsigned char id_val[12];
        struct {
                unsigned char id_local[4];
                unsigned char id_project[8];
        } id_comp;
        uint32_t id_obj;

        bool operator<(const PBXIdUnion &o) { return memcmp(this, &o, sizeof(*this)) == -1; }
        bool operator==(const PBXIdUnion &o) { return memcmp(this, &o, sizeof(*this)) == 0; }
};

struct PBXId : PBXValue {
	PBXIdUnion id;
	std::string comment;

	static uint32_t next_id;

	PBXId() : id(), comment() {}

	PBXId(std::string id_str) : comment() {
		PBXUtil::hex_decode(id_str, id.id_val, sizeof(id.id_val));
	}

	PBXId(std::string id_str, std::string comment) : comment(comment) {
		PBXUtil::hex_decode(id_str, id.id_val, sizeof(id.id_val));
	}

	PBXId(const PBXId& o) : comment(o.comment) {
		memcpy(id.id_val, o.id.id_val, sizeof(id.id_val));
	}

	std::string str() {
		return PBXUtil::hex_encode(id.id_val, sizeof(id.id_val));
	}

	static PBXId createRootId() {
		PBXId newid;
		newid.id.id_obj = next_id++;
		PBXUtil::generate_random(newid.id.id_comp.id_project, sizeof(newid.id.id_comp.id_project));
		return newid;
	}

	static PBXId createId(const PBXId &o) {
		PBXId newid;
		newid.id.id_obj = next_id++;
		memcpy(newid.id.id_comp.id_project, o.id.id_comp.id_project, sizeof(newid.id.id_comp.id_project));;
		return newid;
	}

	virtual PBXType type() { return PBXTypeId; }

	bool operator<(const PBXId &o) { return this->id < o.id; }
	bool operator==(const PBXId &o) { return this->id == o.id; }
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
	void putObject(PBXObjectPtr obj);
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

	void add(PBXValuePtr val);
	void addIdRef(PBXObjectPtr obj);
};

struct PBXLiteral : PBXValue {
	std::string literal_val;

	PBXLiteral(std::string literal_val) : literal_val(literal_val) {}

	virtual PBXType type() { return PBXTypeLiteral; }
};

struct PBXObject : PBXMap {
	PBXId id;
	Xcodeproj *xcodeproj;

	static const std::string default_type_name;

	virtual ~PBXObject() {}

	virtual PBXType type() { return PBXTypeObject; }

	virtual const std::string& type_name() { return default_type_name; };

	virtual void syncFromMap() {}
	virtual void syncToMap() {
		setString("isa", type_name());
	}

	virtual std::string to_string() {
		std::stringstream ss;
		ss << type_name() << "-" << id.str();
		return ss.str();
	}
};


/* PBX classes */

struct PBXAggregateTarget;
struct PBXAppleScriptBuildPhase;
struct PBXBuildFile;
struct PBXBuildRule;
struct PBXBuildStyle;
struct PBXContainerItemProxy;
struct PBXCopyFilesBuildPhase;
struct PBXFileReference;
struct PBXFrameworksBuildPhase;
struct PBXGroup;
struct PBXHeadersBuildPhase;
struct PBXLegacyTarget;
struct PBXNativeTarget;
struct PBXProject;
struct PBXReferenceProxy;
struct PBXResourcesBuildPhase;
struct PBXShellScriptBuildPhase;
struct PBXSourcesBuildPhase;
struct PBXTargetDependency;
struct PBXVariantGroup;
struct XCBuildConfiguration;
struct XCConfigurationList;
struct XCVersionGroup;

typedef std::shared_ptr<PBXAggregateTarget> PBXAggregateTargetPtr;
typedef std::shared_ptr<PBXAppleScriptBuildPhase> PBXAppleScriptBuildPhasePtr;
typedef std::shared_ptr<PBXBuildFile> PBXBuildFilePtr;
typedef std::shared_ptr<PBXBuildRule> PBXBuildRulePtr;
typedef std::shared_ptr<PBXBuildStyle> PBXBuildStylePtr;
typedef std::shared_ptr<PBXContainerItemProxy> PBXContainerItemProxyPtr;
typedef std::shared_ptr<PBXCopyFilesBuildPhase> PBXCopyFilesBuildPhasePtr;
typedef std::shared_ptr<PBXFileReference> PBXFileReferencePtr;
typedef std::shared_ptr<PBXFrameworksBuildPhase> PBXFrameworksBuildPhasePtr;
typedef std::shared_ptr<PBXGroup> PBXGroupPtr;
typedef std::shared_ptr<PBXHeadersBuildPhase> PBXHeadersBuildPhasePtr;
typedef std::shared_ptr<PBXLegacyTarget> PBXLegacyTargetPtr;
typedef std::shared_ptr<PBXNativeTarget> PBXNativeTargetPtr;
typedef std::shared_ptr<PBXProject> PBXProjectPtr;
typedef std::shared_ptr<PBXReferenceProxy> PBXReferenceProxyPtr;
typedef std::shared_ptr<PBXResourcesBuildPhase> PBXResourcesBuildPhasePtr;
typedef std::shared_ptr<PBXShellScriptBuildPhase> PBXShellScriptBuildPhasePtr;
typedef std::shared_ptr<PBXSourcesBuildPhase> PBXSourcesBuildPhasePtr;
typedef std::shared_ptr<PBXTargetDependency> PBXTargetDependencyPtr;
typedef std::shared_ptr<PBXVariantGroup> PBXVariantGroupPtr;
typedef std::shared_ptr<XCBuildConfiguration> XCBuildConfigurationPtr;
typedef std::shared_ptr<XCConfigurationList> XCConfigurationListPtr;
typedef std::shared_ptr<XCVersionGroup> XCVersionGroupPtr;

template <typename T> struct PBXObjectImpl : PBXObject {
	const std::string& type_name() { return T::type_name; }
};

struct Xcodeproj : PBXObjectImpl<Xcodeproj> {
	static const std::string type_name;
	virtual PBXType type() { return PBXTypeXcodeproj; }

	int archiveVersion;
	PBXMapPtr classes;
	int objectVersion;
	PBXMapPtr objects;
	PBXId rootObject;

	Xcodeproj();

	PBXFileReferencePtr getFileReferenceForPath(std::string path, bool create = true);
	PBXFileReferencePtr getProductReference(std::string path);
	PBXBuildFilePtr getBuildFile(PBXFileReferencePtr &fileRef, std::string comment);
	
	void createEmptyProject(std::string projectName, std::string sdkRoot);
	void createNativeTarget(std::string targetName, std::string targetProduct,
                            std::string targetType, std::string targetProductType,
                            std::vector<std::string> libraries,
                            std::vector<std::string> source);

	void syncFromMap();
	void syncToMap();

	PBXProjectPtr getProject() {
		return std::static_pointer_cast<PBXProject>(objects->getObject(rootObject));
	}

	template<typename T> std::shared_ptr<T> getObject(PBXId id) {
		return std::static_pointer_cast<T>(objects->getObject(id));
	}

	template<typename T> std::shared_ptr<T> createObject(std::string comment) {
		auto obj = std::make_shared<T>();
		obj->id = PBXId::createId(rootObject);
		obj->id.comment = comment;
		obj->xcodeproj = this;
		objects->putObject(obj);
		return obj;
	}
};

struct PBXAggregateTarget : PBXObjectImpl<PBXAggregateTarget> {
	static const std::string type_name;

	PBXId buildConfigurationList;
	PBXArrayPtr buildPhases;
	PBXArrayPtr dependencies;
	std::string name;
	std::string productName;

	PBXAggregateTarget();

	void syncFromMap();
	void syncToMap();
};

struct PBXAppleScriptBuildPhase : PBXObjectImpl<PBXAppleScriptBuildPhase> {
	static const std::string type_name;

	int buildActionMask;
	PBXArrayPtr files;
	bool runOnlyForDeploymentPostprocessing;

	PBXAppleScriptBuildPhase();

	void syncFromMap();
	void syncToMap();
};

struct PBXBuildFile : PBXObjectImpl<PBXBuildFile> {
	static const std::string type_name;

	PBXId fileRef;

	PBXBuildFile();

	void syncFromMap();
	void syncToMap();
};

struct PBXBuildRule : PBXObjectImpl<PBXBuildRule> {
	static const std::string type_name;

	std::string compilerSpec;
	std::string filePatterns;
	std::string type;
	bool isEditable;
	PBXArrayPtr outputFiles;
	std::string script;

	PBXBuildRule();

	void syncFromMap();
	void syncToMap();
};

struct PBXBuildStyle : PBXObjectImpl<PBXBuildStyle> {
	static const std::string type_name;

	PBXMapPtr buildSettings;
	std::string name;

	PBXBuildStyle();

	void syncFromMap();
	void syncToMap();
};

struct PBXContainerItemProxy : PBXObjectImpl<PBXContainerItemProxy> {
	static const std::string type_name;

	PBXId containerPortal;
	int proxyType;
	PBXId remoteGlobalIDString;
	std::string remoteInfo;

	PBXContainerItemProxy();

	void syncFromMap();
	void syncToMap();
};

struct PBXCopyFilesBuildPhase : PBXObjectImpl<PBXCopyFilesBuildPhase> {
	static const std::string type_name;

	int buildActionMask;
	std::string dstPath;
	bool dstSubfolderSpec;
	PBXArrayPtr files;
	bool runOnlyForDeploymentPostprocessing;

	PBXCopyFilesBuildPhase();

	void syncFromMap();
	void syncToMap();
};

enum FileTypeFlag {
	FileTypeFlag_None           = 0x0000,
	FileTypeFlag_Compiler       = 0x0001,
	FileTypeFlag_Assembler      = 0x0002,
	FileTypeFlag_Header         = 0x0004,
	FileTypeFlag_LinkLibrary    = 0x0008,
	FileTypeFlag_LinkFramework  = 0x0010,
	FileTypeFlag_Resource       = 0x0020,
	FileTypeFlag_Application    = 0x0040
};

struct FileTypeMetaData {
	std::vector<std::string> extensions;
	std::string type;
	uint64_t flags;
};

struct PBXFileReference : PBXObjectImpl<PBXFileReference> {
	static const std::string type_name;

	static const std::string ext_c_source;
	static const std::string ext_c_header;
	static const std::string ext_objc_source;
	static const std::string ext_objcpp_source;
	static const std::string ext_cpp_source_1;
	static const std::string ext_cpp_source_2;
	static const std::string ext_cpp_header_1;
	static const std::string ext_cpp_header_2;
	static const std::string ext_plist;
	static const std::string ext_library_archive;
	static const std::string ext_application;
	static const std::string ext_bundle;
	static const std::string ext_framework;

	static const std::string type_c_source;
	static const std::string type_c_header;
	static const std::string type_objc_source;
	static const std::string type_objccpp_source;
	static const std::string type_cpp_source;
	static const std::string type_cpp_header;
	static const std::string type_plist;
	static const std::string type_library_archive;
	static const std::string type_library_dylib;
	static const std::string type_application;
	static const std::string type_bundle;
	static const std::string type_framework;
	static const std::string type_executable;

	static std::once_flag extTypeMapInit;
	static std::map<std::string,FileTypeMetaData*> extTypeMap;
	static FileTypeMetaData typeMetaData[];

	static std::string getExtensionFromPath(std::string path);
	static FileTypeMetaData* getFileMetaForPath(std::string path);
	static FileTypeMetaData* getFileMetaForExtension(std::string ext);

	std::string explicitFileType;
	std::string lastKnownFileType;
	bool includeInIndex;
	std::string path;
	std::string sourceTree;

	PBXFileReference();

	void syncFromMap();
	void syncToMap();
};

struct PBXFrameworksBuildPhase : PBXObjectImpl<PBXFrameworksBuildPhase> {
	static const std::string type_name;

	int buildActionMask;
	PBXArrayPtr files;
	bool runOnlyForDeploymentPostprocessing;

	PBXFrameworksBuildPhase();

	void syncFromMap();
	void syncToMap();
};

struct PBXGroup : PBXObjectImpl<PBXGroup> {
	static const std::string type_name;

	PBXArrayPtr children;
	std::string name;
	std::string path;
	std::string sourceTree;

	PBXGroup();

	void syncFromMap();
	void syncToMap();
	void sortChildren();
};

struct PBXHeadersBuildPhase : PBXObjectImpl<PBXHeadersBuildPhase> {
	static const std::string type_name;

	int buildActionMask;
	PBXArrayPtr files;
	bool runOnlyForDeploymentPostprocessing;

	PBXHeadersBuildPhase();

	void syncFromMap();
	void syncToMap();
};

struct PBXLegacyTarget : PBXObjectImpl<PBXLegacyTarget> {
	static const std::string type_name;

	std::string buildArgumentsString;
	PBXId buildConfigurationList;
	PBXArrayPtr buildPhases;
	std::string buildToolPath;
	PBXArrayPtr dependencies;
	std::string name;
	bool passBuildSettingsInEnvironment;
	std::string productName;

	PBXLegacyTarget();

	void syncFromMap();
	void syncToMap();
};

struct PBXNativeTarget : PBXObjectImpl<PBXNativeTarget> {
	static const std::string type_name;

	static const std::string type_application;
	static const std::string type_bundle;
	static const std::string type_framework;
	static const std::string type_library_static;
	static const std::string type_library_dynamic;
	static const std::string type_tool;

	PBXId buildConfigurationList;
	PBXArrayPtr buildPhases;
	PBXArrayPtr buildRules;
	PBXArrayPtr dependencies;
	std::string name;
	std::string productName;
	PBXId productReference;
	std::string productType;

	PBXNativeTarget();

	void syncFromMap();
	void syncToMap();
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
	PBXArrayPtr projectReferences;
	std::string projectRoot;
	PBXArrayPtr targets;

	PBXProject();

	void syncFromMap();
	void syncToMap();
};

struct PBXReferenceProxy : PBXObjectImpl<PBXReferenceProxy> {
	static const std::string type_name;

	std::string type;
	std::string path;
	PBXId remoteRef;
	std::string sourceTree;

	PBXReferenceProxy();

	void syncFromMap();
	void syncToMap();
};

struct PBXResourcesBuildPhase : PBXObjectImpl<PBXResourcesBuildPhase> {
	static const std::string type_name;

	int buildActionMask;
	PBXArrayPtr files;
	bool runOnlyForDeploymentPostprocessing;

	PBXResourcesBuildPhase();

	void syncFromMap();
	void syncToMap();
};

struct PBXShellScriptBuildPhase : PBXObjectImpl<PBXShellScriptBuildPhase> {
	static const std::string type_name;

	int buildActionMask;
	PBXArrayPtr files;
	PBXArrayPtr inputPaths;
	PBXArrayPtr outputPaths;
	bool runOnlyForDeploymentPostprocessing;
	std::string shellPath;
	std::string shellScript;

	PBXShellScriptBuildPhase();

	void syncFromMap();
	void syncToMap();
};

struct PBXSourcesBuildPhase : PBXObjectImpl<PBXSourcesBuildPhase> {
	static const std::string type_name;

	int buildActionMask;
	PBXArrayPtr files;
	bool runOnlyForDeploymentPostprocessing;

	PBXSourcesBuildPhase();

	void syncFromMap();
	void syncToMap();
};

struct PBXTargetDependency : PBXObjectImpl<PBXTargetDependency> {
	static const std::string type_name;

	PBXId target;
	PBXId targetProxy;

	PBXTargetDependency();

	void syncFromMap();
	void syncToMap();
};

struct PBXVariantGroup : PBXObjectImpl<PBXVariantGroup> {
	static const std::string type_name;

	PBXArrayPtr children;
	std::string name;
	std::string path;
	std::string sourceTree;

	PBXVariantGroup();

	void syncFromMap();
	void syncToMap();
};

struct XCBuildConfiguration : PBXObjectImpl<XCBuildConfiguration> {
	static const std::string type_name;

	PBXMapPtr buildSettings;
	std::string name;

	XCBuildConfiguration();

	void syncFromMap();
	void syncToMap();
};

struct XCConfigurationList : PBXObjectImpl<XCConfigurationList> {
	static const std::string type_name;

	PBXArrayPtr buildConfigurations;
	bool defaultConfigurationIsVisible;
	std::string defaultConfigurationName;

	XCConfigurationList();

	void syncFromMap();
	void syncToMap();
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
	static PBXObject* create(std::string class_name, const PBXId &id, const PBXMap &map);

	virtual ~PBXObjectFactory() {}
	virtual PBXObject* create() = 0;
};

template <typename T>
struct PBXObjectFactoryImpl : PBXObjectFactory {
	PBXObject* create() { return new T(); }
};


/* PBX parser state machine */

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
	static void write(PBXValuePtr value, std::ostream &out, int indent);
};

#endif
