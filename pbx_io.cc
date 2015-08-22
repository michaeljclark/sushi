#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdint>
#include <ctime>
#include <cerrno>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <random>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "pbx_io.h"


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

int PBXUtil::canonicalize_path(char *path)
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

std::vector<std::string> PBXUtil::path_components(std::string path)
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

std::string PBXUtil::hex_encode(const unsigned char *buf, size_t len)
{
	std::string hex;
	for (size_t i = 0; i < len; i++) {
		char b = buf[i];
		hex.append(HEX_DIGITS + ((b >> 4) & 0x0F), 1);
		hex.append(HEX_DIGITS + (b & 0x0F), 1);
	}
	return hex;
}

void PBXUtil::hex_decode(std::string hex, unsigned char *buf, size_t len)
{
	for (size_t i = 0; i < hex.length()/2 && i < len; i++) {
		const char tmp[3] = { hex[i*2], hex[i*2+1], 0 };
		*buf++ = (char)strtoul(tmp, NULL, 16);
	}
}

void PBXUtil::generate_random(unsigned char *buf, size_t len)
{
	static std::default_random_engine generator;
	static std::uniform_int_distribution<unsigned char> distribution(0, 255);
	generator.seed(time(NULL));
	for (size_t i = 0; i < len; i++) {
		buf[i] = distribution(generator);
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
		log_fatal_exit("error short read: %s", filename.c_str());
	}
	::close(fd);

	return buf;
}

/* PBX Id */

uint32_t PBXId::next_id = 0;


/* PBX Array */

void PBXArray::add(PBXValuePtr val) {
	array_val.push_back(val);
}

void PBXArray::addIdRef(PBXObjectPtr obj) {
	array_val.push_back(std::make_shared<PBXId>(obj->id));
}


/* PBX Map */

void PBXMap::put(std::string key, std::string comment, PBXValuePtr val)
{
	if (object_val.find(key) == object_val.end()) {
		object_val[key] = val;
		key_order.push_back(PBXKey(key, comment));
	} else {
		log_fatal_exit("duplicate key \"%s\" in object", key.c_str());
	}
}

void PBXMap::putObject(PBXObjectPtr obj)
{
	put(obj->id.str(), obj->id.comment, obj);
}

void PBXMap::replace(std::string key, PBXValuePtr val) {
	if (object_val.find(key) != object_val.end()) {
		object_val[key] = val;
	} else {
		log_fatal_exit("missing key \"%s\" in object", key.c_str());
	}
}

PBXId PBXMap::getId(std::string key)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		return PBXId();
	} else if (i->second->type() == PBXTypeId) {
		return static_cast<const PBXId&>(*i->second);
	} else {
		return PBXId();
	}
}

std::string PBXMap::getString(std::string key, std::string default_str)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		return default_str;
	} else if (i->second->type() == PBXTypeLiteral) {
		return static_cast<const PBXLiteral&>(*i->second).literal_val;
	} else {
		return std::string();
	}
}

int PBXMap::getInteger(std::string key, int default_int)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		return default_int;
	} else if (i->second->type() == PBXTypeLiteral) {
		return (int)strtoul(static_cast<const PBXLiteral&>(*i->second).literal_val.c_str(), nullptr, 10);
	} else {
		log_fatal_exit("value is not a literal");
		return 0;
	}
}

bool PBXMap::getBoolean(std::string key, bool default_bool)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		return default_bool;
	} else if (i->second->type() == PBXTypeLiteral) {
		std::string val = static_cast<const PBXLiteral&>(*i->second).literal_val;
		return (val == "0" || val == "NO") ? false : true;
	} else {
		return false;
	}
}

PBXArrayPtr PBXMap::getArray(std::string key, bool default_create)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		if (default_create) {
			auto valptr = std::make_shared<PBXArray>();
			put(key, "", valptr);
			return valptr;
		}
		return PBXArrayPtr();
	} else if (i->second->type() == PBXTypeArray) {
		return std::static_pointer_cast<PBXArray>(i->second);
	} else {
		return PBXArrayPtr();
	}
}

PBXMapPtr PBXMap::getMap(std::string key, bool default_create)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		if (default_create) {
			auto valptr = std::make_shared<PBXMap>();
			put(key, "", valptr);
			return valptr;
		}
		return PBXMapPtr();
	} else if (i->second->type() == PBXTypeMap) {
		return std::static_pointer_cast<PBXMap>(i->second);
	} else {
		return PBXMapPtr();
	}
}

PBXObjectPtr PBXMap::getObject(PBXId id)
{
	auto i = object_val.find(id.str());
	if (i == object_val.end()) {
		return PBXObjectPtr();
	} else if (i->second->type() == PBXTypeObject) {
		return std::static_pointer_cast<PBXObject>(i->second);
	} else {
		return PBXObjectPtr();
	}
}


void PBXMap::setId(std::string key, PBXId id)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		key_order.push_back(PBXKey(key));
	}
	object_val[key] = PBXValuePtr(new PBXId(id.str(), id.comment));
}

void PBXMap::setString(std::string key, std::string str_val)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		key_order.push_back(PBXKey(key));
	}
	object_val[key] = PBXValuePtr(new PBXLiteral(str_val));
}

void PBXMap::setInteger(std::string key, int int_val)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		key_order.push_back(PBXKey(key));
	}
	std::stringstream ss;
	ss << int_val;
	object_val[key] = PBXValuePtr(new PBXLiteral(ss.str()));
}

void PBXMap::setBoolean(std::string key, bool bool_val)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		key_order.push_back(PBXKey(key));
	}
	object_val[key] = PBXValuePtr(new PBXLiteral(bool_val ? "YES" : "NO"));
}

void PBXMap::setArray(std::string key, PBXArrayPtr arr)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		key_order.push_back(PBXKey(key));
	}
	object_val[key] = arr;
}

void PBXMap::setMap(std::string key, PBXMapPtr map)
{
	auto i = object_val.find(key);
	if (i == object_val.end()) {
		key_order.push_back(PBXKey(key));
	}
	object_val[key] = map;
}


/* PBX object factory */

void PBXObjectFactory::init()
{
	std::call_once(factoryInit, [](){
		registerFactory<PBXAggregateTarget>();
		registerFactory<PBXAppleScriptBuildPhase>();
		registerFactory<PBXBuildFile>();
		registerFactory<PBXBuildRule>();
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
		registerFactory<PBXVariantGroup>();
		registerFactory<XCBuildConfiguration>();
		registerFactory<XCConfigurationList>();
		registerFactory<XCVersionGroup>();
	});
}

PBXObject* PBXObjectFactory::create(std::string class_name, const PBXId &id, const PBXMap &map)
{
	init();
	auto it = factoryMap.find(class_name);
	PBXObject *ptr = it != factoryMap.end() ? it->second->create() : new PBXObject();
	ptr->id = id;
	ptr->object_val = map.object_val;
	ptr->key_order = map.key_order;
	return ptr;
}

std::once_flag PBXObjectFactory::factoryInit;
std::map<std::string,PBXObjectFactoryPtr> PBXObjectFactory::factoryMap;


/* PBX classes */

const std::string Xcodeproj::type_name =                     "Xcodeproj";
const std::string PBXAggregateTarget::type_name =            "PBXAggregateTarget";
const std::string PBXAppleScriptBuildPhase::type_name =      "PBXAppleScriptBuildPhase";
const std::string PBXBuildFile::type_name =                  "PBXBuildFile";
const std::string PBXBuildRule::type_name =                  "PBXBuildRule";
const std::string PBXBuildStyle::type_name =                 "PBXBuildStyle";
const std::string PBXContainerItemProxy::type_name =         "PBXContainerItemProxy";
const std::string PBXCopyFilesBuildPhase::type_name =        "PBXCopyFilesBuildPhase";
const std::string PBXFileReference::type_name =              "PBXFileReference";
const std::string PBXFrameworksBuildPhase::type_name =       "PBXFrameworksBuildPhase";
const std::string PBXGroup::type_name =                      "PBXGroup";
const std::string PBXHeadersBuildPhase::type_name =          "PBXHeadersBuildPhase";
const std::string PBXLegacyTarget::type_name =               "PBXLegacyTarget";
const std::string PBXNativeTarget::type_name =               "PBXNativeTarget";
const std::string PBXProject::type_name =                    "PBXProject";
const std::string PBXReferenceProxy::type_name =             "PBXReferenceProxy";
const std::string PBXResourcesBuildPhase::type_name =        "PBXResourcesBuildPhase";
const std::string PBXShellScriptBuildPhase::type_name =      "PBXShellScriptBuildPhase";
const std::string PBXSourcesBuildPhase::type_name =          "PBXSourcesBuildPhase";
const std::string PBXTargetDependency::type_name =           "PBXTargetDependency";
const std::string PBXVariantGroup::type_name =               "PBXVariantGroup";
const std::string XCBuildConfiguration::type_name =          "XCBuildConfiguration";
const std::string XCConfigurationList::type_name =           "XCConfigurationList";
const std::string XCVersionGroup::type_name =                "XCVersionGroup";


/* Xcodeproj */

Xcodeproj::Xcodeproj()
{
	archiveVersion = 1;
	classes = std::make_shared<PBXMap>();
	objectVersion = 46;
	objects = std::make_shared<PBXMap>();
}

PBXFileReferencePtr Xcodeproj::getFileReferenceForPath(std::string path, bool create)
{
	auto project = getProject();
	auto mainGroup = getObject<PBXGroup>(project->mainGroup);
	auto pathComponents = PBXUtil::path_components(path);
	if (pathComponents.size() == 0) {
		return PBXFileReferencePtr();
	}

	// find or create group
	auto currentGroup = mainGroup;
	for (size_t i = 0; i < pathComponents.size() - 1; i++) {
		PBXGroupPtr foundGroup;
		for (auto child : currentGroup->children->array_val) {
			if (child->type() != PBXTypeId) continue;
			auto childId = std::static_pointer_cast<PBXId>(child);
			auto childObject = getObject<PBXObject>(*childId);
			if (childObject->type_name() != PBXGroup::type_name) continue;
			auto group = std::static_pointer_cast<PBXGroup>(childObject);
			if (group->path == pathComponents[i]) {
				foundGroup = group;
				break;
			}
		}
		if (!foundGroup && !create) {
			return PBXFileReferencePtr();
		}
		if (!foundGroup) {
			foundGroup = createObject<PBXGroup>(pathComponents[i]);
			foundGroup->name = foundGroup->path = pathComponents[i];
			foundGroup->sourceTree = "<group>";
			currentGroup->children->addIdRef(foundGroup);
		}
		currentGroup = foundGroup;
	}

	// find or create file reference
	PBXFileReferencePtr foundFileRef;
	for (auto child : currentGroup->children->array_val) {
		if (child->type() != PBXTypeId) continue;
		auto childId = std::static_pointer_cast<PBXId>(child);
		auto childObject = getObject<PBXObject>(*childId);
		if (childObject->type_name() != PBXFileReference::type_name) continue;
		auto fileRef = std::static_pointer_cast<PBXFileReference>(childObject);
		if (fileRef->path == pathComponents.back()) {
			foundFileRef = fileRef;
			break;
		}
	}
	if (!foundFileRef && !create) {
		return PBXFileReferencePtr();
	}
	if (!foundFileRef) {
		foundFileRef = createObject<PBXFileReference>(pathComponents.back());
		foundFileRef->path = pathComponents.back();
		foundFileRef->sourceTree = "<group>";
		currentGroup->children->addIdRef(foundFileRef);
	}
	return foundFileRef;
}

PBXFileReferencePtr Xcodeproj::getProductReference(std::string path)
{
	auto project = getProject();
	auto productsGroup = getObject<PBXGroup>(project->productRefGroup);
	for (auto child : productsGroup->children->array_val) {
		if (child->type() != PBXTypeId) continue;
		auto childId = std::static_pointer_cast<PBXId>(child);
		auto childObject = getObject<PBXObject>(*childId);
		if (childObject->type_name() != PBXFileReference::type_name) continue;
		auto fileRef = std::static_pointer_cast<PBXFileReference>(childObject);
		if (fileRef->path == path) {
			return fileRef;
		}
	}
	return PBXFileReferencePtr();
}

PBXBuildFilePtr Xcodeproj::getBuildFile(PBXFileReferencePtr &fileRef, std::string comment)
{
	// TODO - we shouldn't use a linear scan
	for (auto &keyval : objects->object_val) {
		auto &objKey = keyval.second;
		if (objKey->type() != PBXTypeId) continue;
		auto objId = std::static_pointer_cast<PBXId>(objKey);
		auto obj = getObject<PBXObject>(*objId);
		if (obj->type_name() != PBXBuildFile::type_name) continue;
		auto buildFile = std::static_pointer_cast<PBXBuildFile>(obj);
		if (buildFile->fileRef == fileRef->id) {
			return buildFile;
		}
	}
	auto buildFile = createObject<PBXBuildFile>(comment);
	buildFile->fileRef = fileRef->id;
	return buildFile;
}

void Xcodeproj::createEmptyProject(std::string projectName, std::string sdkRoot)
{
	// Create Project
	rootObject = PBXId::createRootId();
	auto project = createObject<PBXProject>("Project Object");
	rootObject = project->id;

	// Create Build Configuration List
	auto configurationList = createObject<XCConfigurationList>
		("Build configuration list for PBXProject \"" + projectName + "\"");
	project->buildConfigurationList = configurationList->id;

	// Create Debug Build Configuration
	auto debugConfiguration = createObject<XCBuildConfiguration>("Debug");
	debugConfiguration->name = "Debug";
	debugConfiguration->buildSettings->setString("SDKROOT", sdkRoot);
	debugConfiguration->buildSettings->setString("MACOSX_DEPLOYMENT_TARGET", "10.10");
	configurationList->buildConfigurations->addIdRef(debugConfiguration);

	// Create Release Build Configuration
	auto releaseConfiguration = createObject<XCBuildConfiguration>("Release");
	releaseConfiguration->name = "Release";
	releaseConfiguration->buildSettings->setString("SDKROOT", sdkRoot);
	releaseConfiguration->buildSettings->setString("MACOSX_DEPLOYMENT_TARGET", "10.10");
	configurationList->buildConfigurations->addIdRef(releaseConfiguration);

	// Create main group
	auto mainGroup = createObject<PBXGroup>("");
	mainGroup->sourceTree = "<group>";
	project->mainGroup = mainGroup->id;

	// Create products group
	auto productsGroup = createObject<PBXGroup>("Products");
	productsGroup->sourceTree = "<group>";
	productsGroup->name = "Products";
	mainGroup->children->addIdRef(productsGroup);
	project->productRefGroup = productsGroup->id;
}

void Xcodeproj::createNativeTarget(std::string targetName, std::string targetProduct,
                            std::string targetType, std::string targetProductType,
                            std::vector<std::string> libraries,
                            std::vector<std::string> source)
{
	auto project = getProject();
	auto mainGroup = getObject<PBXGroup>(project->mainGroup);
	auto productsGroup = getObject<PBXGroup>(project->productRefGroup);

	// Create Build Configuration List
	auto configurationList = createObject<XCConfigurationList>
		("Build configuration list for PBXNativeTarget \"" + targetName + "\"");

	// Create Debug Build Configuration
	auto debugConfiguration = createObject<XCBuildConfiguration>("Debug");
	debugConfiguration->name = "Debug";
	debugConfiguration->buildSettings->setString("PRODUCT_NAME", "$(TARGET_NAME)");
	configurationList->buildConfigurations->addIdRef(debugConfiguration);

	// Create Release Build Configuration
	auto releaseConfiguration = createObject<XCBuildConfiguration>("Release");
	releaseConfiguration->name = "Release";
	releaseConfiguration->buildSettings->setString("PRODUCT_NAME", "$(TARGET_NAME)");
	configurationList->buildConfigurations->addIdRef(releaseConfiguration);

	// Create PBXFrameworksBuildPhase
	auto frameworkBuildPhase = createObject<PBXFrameworksBuildPhase>("Frameworks");
	frameworkBuildPhase->buildActionMask = 2147483647;
	frameworkBuildPhase->runOnlyForDeploymentPostprocessing = 0;

	// Create PBXBuildFiles for target link libraries
	for (std::string library : libraries) {
		auto libraryFileRef = getProductReference(library);
		if (libraryFileRef) {
			auto libraryBuildFileRef = getBuildFile(libraryFileRef, libraryFileRef->id.comment + " in Frameworks");
			frameworkBuildPhase->files->addIdRef(libraryBuildFileRef);
		}
	}

	// Create PBXSourcesBuildPhase
	auto sourceBuildPhase = createObject<PBXSourcesBuildPhase>("Sources");
	sourceBuildPhase->buildActionMask = 2147483647;
	sourceBuildPhase->runOnlyForDeploymentPostprocessing = 0;

	// Create PBXFileReferences for target source
	for (auto sourceFile : source) {
		FileTypeMetaData *meta = PBXFileReference::getFileMetaForPath(sourceFile);
		auto sourceFileRef = getFileReferenceForPath(sourceFile);
		sourceFileRef->lastKnownFileType = meta->type;
		sourceFileRef->includeInIndex = 1;
		if (!(meta && (meta->flags & FileTypeFlag_Compiler))) continue;
		auto sourceBuildFileRef = getBuildFile(sourceFileRef, sourceFileRef->id.comment + " in Sources");
		sourceBuildPhase->files->addIdRef(sourceBuildFileRef);
	}

	// Create PBXFileReference for target output and add to Products
	auto targetProductFileRef = createObject<PBXFileReference>(targetProduct);
	targetProductFileRef->explicitFileType = targetType;
	targetProductFileRef->includeInIndex = 0;
	targetProductFileRef->path = targetProduct;
	targetProductFileRef->sourceTree = "BUILT_PRODUCTS_DIR";
	productsGroup->children->addIdRef(targetProductFileRef);

	// Create PBXNativeTarget
	auto nativeTarget = createObject<PBXNativeTarget>(targetName);
	nativeTarget->name = targetName;
	nativeTarget->productName = targetName;
	nativeTarget->productReference = targetProductFileRef->id;
	nativeTarget->productType = targetProductType;
	nativeTarget->buildConfigurationList = configurationList->id;
	nativeTarget->buildPhases->addIdRef(sourceBuildPhase);
	nativeTarget->buildPhases->addIdRef(frameworkBuildPhase);
	project->targets->addIdRef(nativeTarget);
}

void Xcodeproj::syncFromMap()
{
	archiveVersion = getInteger("archiveVersion");
	classes = getMap("classes");
	objectVersion = getInteger("objectVersion");
	objects = getMap("objects");
	rootObject = getId("rootObject");
}

void Xcodeproj::syncToMap()
{
	setInteger("archiveVersion", archiveVersion);
	setMap("classes", classes);
	setInteger("objectVersion", objectVersion);
	setMap("objects", objects);
	setId("rootObject", rootObject);
}


/* PBXAggregateTarget */

PBXAggregateTarget::PBXAggregateTarget()
{
	buildPhases = std::make_shared<PBXArray>();
	dependencies = std::make_shared<PBXArray>();
}

void PBXAggregateTarget::syncFromMap()
{
	PBXObject::syncFromMap();

	buildConfigurationList = getId("buildConfigurationList");
	buildPhases = getArray("buildPhases");
	dependencies = getArray("dependencies");
	name = getString("name");
	productName = getString("productName");
}

void PBXAggregateTarget::syncToMap()
{
	PBXObject::syncToMap();

	setId("buildConfigurationList", buildConfigurationList);
	setArray("buildPhases", buildPhases);
	setArray("dependencies", dependencies);
	setString("name", name);
	setString("productName", productName);
}


/* PBXAppleScriptBuildPhase */

PBXAppleScriptBuildPhase::PBXAppleScriptBuildPhase()
{
	files = std::make_shared<PBXArray>();
}

void PBXAppleScriptBuildPhase::syncFromMap()
{
	PBXObject::syncFromMap();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXAppleScriptBuildPhase::syncToMap()
{
	PBXObject::syncToMap();

	setInteger("buildActionMask", buildActionMask);
	setArray("files", files);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
}


/* PBXBuildFile */

PBXBuildFile::PBXBuildFile()
{

}

void PBXBuildFile::syncFromMap()
{
	PBXObject::syncFromMap();

	fileRef = getId("fileRef");
}

void PBXBuildFile::syncToMap()
{
	PBXObject::syncToMap();

	setId("fileRef", fileRef);
}


/* PBXBuildRule */

PBXBuildRule::PBXBuildRule()
{
	outputFiles = std::make_shared<PBXArray>();
}

void PBXBuildRule::syncFromMap()
{
	PBXObject::syncFromMap();

	compilerSpec = getString("compilerSpec");
	filePatterns = getString("filePatterns");
	type = getString("type");
	isEditable = getInteger("isEditable");
	outputFiles = getArray("outputFiles");
	script = getString("script");
}

void PBXBuildRule::syncToMap()
{
	PBXObject::syncToMap();

	setString("compilerSpec", compilerSpec);
	setString("filePatterns", filePatterns);
	setString("type", type);
	setInteger("isEditable", isEditable);
	setArray("outputFiles", outputFiles);
	setString("script", script);
}


/* PBXBuildStyle */

PBXBuildStyle::PBXBuildStyle()
{
	buildSettings = std::make_shared<PBXMap>();
}

void PBXBuildStyle::syncFromMap()
{
	PBXObject::syncFromMap();

	buildSettings = getMap("buildSettings");
	name = getString("name");
}

void PBXBuildStyle::syncToMap()
{
	PBXObject::syncToMap();

	setMap("buildSettings", buildSettings);
	setString("name", name);
}


/* PBXContainerItemProxy */

PBXContainerItemProxy::PBXContainerItemProxy()
{

}

void PBXContainerItemProxy::syncFromMap()
{
	PBXObject::syncFromMap();

	containerPortal = getId("containerPortal");
	proxyType = getInteger("proxyType");
	remoteGlobalIDString = getId("remoteGlobalIDString");
	remoteInfo = getString("remoteInfo");
}

void PBXContainerItemProxy::syncToMap()
{
	PBXObject::syncToMap();

	setId("containerPortal", containerPortal);
	setInteger("proxyType", proxyType);
	setId("remoteGlobalIDString", remoteGlobalIDString);
	setString("remoteInfo", remoteInfo);
}


/* PBXCopyFilesBuildPhase */

PBXCopyFilesBuildPhase::PBXCopyFilesBuildPhase()
{
	files = std::make_shared<PBXArray>();
}

void PBXCopyFilesBuildPhase::syncFromMap()
{
	PBXObject::syncFromMap();

	buildActionMask = getInteger("buildActionMask");
	dstPath = getString("dstPath");
	dstSubfolderSpec = getInteger("dstSubfolderSpec");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXCopyFilesBuildPhase::syncToMap()
{
	PBXObject::syncToMap();

	setInteger("buildActionMask", buildActionMask);	
	if (dstPath.length() > 0) {
		setString("dstPath", dstPath);
	}
	setInteger("dstSubfolderSpec", dstSubfolderSpec);
	setArray("files", files);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
}


/* PBXFileReference */

const std::string PBXFileReference::ext_c_source          = "c";
const std::string PBXFileReference::ext_c_header          = "h";
const std::string PBXFileReference::ext_objc_source       = "m";
const std::string PBXFileReference::ext_objcpp_source     = "mm";
const std::string PBXFileReference::ext_cpp_source_1      = "cc";
const std::string PBXFileReference::ext_cpp_source_2      = "cpp";
const std::string PBXFileReference::ext_cpp_header_1      = "hh";
const std::string PBXFileReference::ext_cpp_header_2      = "hpp";
const std::string PBXFileReference::ext_plist             = "plist";
const std::string PBXFileReference::ext_library_archive   = "a";
const std::string PBXFileReference::ext_application       = "app";
const std::string PBXFileReference::ext_bundle            = "bundle";
const std::string PBXFileReference::ext_framework         = "framework"; 

const std::string PBXFileReference::type_c_source         = "sourcecode.c.c";
const std::string PBXFileReference::type_c_header         = "sourcecode.c.h";
const std::string PBXFileReference::type_objc_source      = "sourcecode.c.objc";
const std::string PBXFileReference::type_objccpp_source   = "sourcecode.cpp.objcpp";
const std::string PBXFileReference::type_cpp_source       = "sourcecode.cpp.cpp";
const std::string PBXFileReference::type_cpp_header       = "sourcecode.cpp.h";
const std::string PBXFileReference::type_plist            = "text.plist.xml";
const std::string PBXFileReference::type_library_archive  = "archive.ar";
const std::string PBXFileReference::type_library_dylib    = "compiled.mach-o.dylib";
const std::string PBXFileReference::type_application      = "wrapper.application";
const std::string PBXFileReference::type_bundle           = "wrapper.cfbundle";
const std::string PBXFileReference::type_framework        = "wrapper.framework";
const std::string PBXFileReference::type_executable       = "compiled.mach-o.executable";

std::once_flag PBXFileReference::extTypeMapInit;
std::map<std::string,FileTypeMetaData*> PBXFileReference::extTypeMap;

FileTypeMetaData PBXFileReference::typeMetaData[] = {
	{{ext_c_source}, type_c_source, FileTypeFlag_Compiler},
	{{ext_c_header}, type_c_header, FileTypeFlag_Header},
	{{ext_objc_source}, type_objc_source, FileTypeFlag_Compiler},
	{{ext_objcpp_source}, type_objccpp_source, FileTypeFlag_Compiler},
	{{ext_cpp_source_1, ext_cpp_source_2}, type_cpp_source, FileTypeFlag_Compiler},
	{{ext_cpp_header_1, ext_cpp_header_2}, type_cpp_header, FileTypeFlag_Header},
	{{ext_plist}, type_plist, FileTypeFlag_Resource},
	{{ext_library_archive}, type_library_archive, FileTypeFlag_LinkLibrary},
	{{ext_application}, type_application, FileTypeFlag_Application},
	{{ext_bundle}, type_bundle, FileTypeFlag_Resource},
	{{ext_framework}, type_framework, FileTypeFlag_LinkFramework},
	{{}, std::string(), FileTypeFlag_None}
};

std::string PBXFileReference::getExtensionFromPath(std::string path)
{
	size_t lastDotIndex = path.find_last_of(".");
	if (lastDotIndex == std::string::npos) return std::string();
	return path.substr(lastDotIndex + 1);
}

FileTypeMetaData* PBXFileReference::getFileMetaForPath(std::string path)
{
	return getFileMetaForExtension(getExtensionFromPath(path));
}

FileTypeMetaData* PBXFileReference::getFileMetaForExtension(std::string extension)
{
	std::call_once(extTypeMapInit, [](){
		FileTypeMetaData *meta = typeMetaData;
		while (meta->flags != FileTypeFlag_None) {
			for (std::string ext : meta->extensions) {
				extTypeMap[ext] = meta;
			}
			meta++;
		}
	});
	auto it = extTypeMap.find(extension);
	return (it != extTypeMap.end()) ? it->second : nullptr;
}

PBXFileReference::PBXFileReference()
{

}

void PBXFileReference::syncFromMap()
{
	PBXObject::syncFromMap();

	explicitFileType = getString("explicitFileType");
	lastKnownFileType = getString("lastKnownFileType");
	includeInIndex = getInteger("includeInIndex", 1);
	path = getString("path");
	sourceTree = getString("sourceTree");
}

void PBXFileReference::syncToMap()
{
	PBXObject::syncToMap();

	if (explicitFileType.length() > 0) {
		setString("explicitFileType", explicitFileType);
	}
	if (lastKnownFileType.length() > 0) {
		setString("lastKnownFileType", lastKnownFileType);
	}
	if (includeInIndex == 0) {
		setInteger("includeInIndex", includeInIndex);
	}
	setString("path", path);
	setString("sourceTree", sourceTree);
}


/* PBXFrameworksBuildPhase */

PBXFrameworksBuildPhase::PBXFrameworksBuildPhase()
{
	files = std::make_shared<PBXArray>();
}

void PBXFrameworksBuildPhase::syncFromMap()
{
	PBXObject::syncFromMap();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXFrameworksBuildPhase::syncToMap()
{
	PBXObject::syncToMap();

	setInteger("buildActionMask", buildActionMask);	
	setArray("files", files);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
}


/* PBXGroup */

PBXGroup::PBXGroup()
{
	children = std::make_shared<PBXArray>();
}

void PBXGroup::syncFromMap()
{
	PBXObject::syncFromMap();

	children = getArray("children");
	name = getString("name");
	path = getString("path");
	sourceTree = getString("sourceTree");
}

void PBXGroup::syncToMap()
{
	PBXObject::syncToMap();

	sortChildren();

	setArray("children", children);
	if (name.length() > 0) {
		setString("name", name);
	}
	if (path.length() > 0) {
		setString("path", path);
	}
	setString("sourceTree", sourceTree);
}

void PBXGroup::sortChildren()
{
	if (!xcodeproj) return;
	sort(children->array_val.begin(), children->array_val.end(), 
			[&](const PBXValuePtr &a, const PBXValuePtr &b)
	{ 
		if (a->type() != PBXTypeId || b->type() != PBXTypeId) return false;
		auto aId = std::static_pointer_cast<PBXId>(a);
		auto bId = std::static_pointer_cast<PBXId>(b);
		auto aObj = xcodeproj->getObject<PBXObject>(*aId);
		auto bObj = xcodeproj->getObject<PBXObject>(*bId);
		std::string aName, bName;
		if (aObj->type_name() == PBXGroup::type_name) {
			aName = std::static_pointer_cast<PBXGroup>(aObj)->name;
		}
		else if (aObj->type_name() == PBXFileReference::type_name) {
			aName = std::static_pointer_cast<PBXFileReference>(aObj)->path;
		}
		if (bObj->type_name() == PBXGroup::type_name) {
			bName = std::static_pointer_cast<PBXGroup>(bObj)->name;
		}
		else if (bObj->type_name() == PBXFileReference::type_name) {
			bName = std::static_pointer_cast<PBXFileReference>(bObj)->path;
		}
		return aName < bName; 
	});
}


/* PBXHeadersBuildPhase */

PBXHeadersBuildPhase::PBXHeadersBuildPhase()
{
	files = std::make_shared<PBXArray>();
}

void PBXHeadersBuildPhase::syncFromMap()
{
	PBXObject::syncFromMap();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXHeadersBuildPhase::syncToMap()
{
	PBXObject::syncToMap();

	setInteger("buildActionMask", buildActionMask);	
	setArray("files", files);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
}


/* PBXLegacyTarget */

PBXLegacyTarget::PBXLegacyTarget()
{
	buildPhases = std::make_shared<PBXArray>();
	dependencies = std::make_shared<PBXArray>();
}

void PBXLegacyTarget::syncFromMap()
{
	PBXObject::syncFromMap();

	buildArgumentsString = getString("buildArgumentsString");
	buildConfigurationList = getId("buildConfigurationList");
	buildPhases = getArray("buildPhases");
	buildToolPath = getString("buildToolPath");
	dependencies = getArray("dependencies");
	name = getString("name");
	passBuildSettingsInEnvironment = getInteger("passBuildSettingsInEnvironment");
	productName = getString("productName");
}

void PBXLegacyTarget::syncToMap()
{
	PBXObject::syncToMap();

	setString("buildArgumentsString", buildArgumentsString);
	setId("buildConfigurationList", buildConfigurationList);
	setArray("buildPhases", buildPhases);
	setString("buildToolPath", buildToolPath);
	setArray("dependencies", dependencies);
	setString("name", name);
	setInteger("passBuildSettingsInEnvironment", passBuildSettingsInEnvironment);
	setString("productName", productName);
}


/* PBXNativeTarget */

const std::string PBXNativeTarget::type_application     = "com.apple.product-type.application";
const std::string PBXNativeTarget::type_bundle          = "com.apple.product-type.bundle";
const std::string PBXNativeTarget::type_framework       = "com.apple.product-type.framework";
const std::string PBXNativeTarget::type_library_dynamic = "com.apple.product-type.library.dynamic";
const std::string PBXNativeTarget::type_library_static  = "com.apple.product-type.library.static";
const std::string PBXNativeTarget::type_tool            = "com.apple.product-type.tool";

PBXNativeTarget::PBXNativeTarget()
{
	buildPhases = std::make_shared<PBXArray>();
	buildRules = std::make_shared<PBXArray>();
	dependencies = std::make_shared<PBXArray>();
}

void PBXNativeTarget::syncFromMap()
{
	PBXObject::syncFromMap();

	buildConfigurationList = getId("buildConfigurationList");
	buildPhases = getArray("buildPhases");
	buildRules = getArray("buildRules");
	dependencies = getArray("dependencies");
	name = getString("name");
	productName = getString("productName");
	productReference = getId("productReference");
	productType = getString("productType");
}

void PBXNativeTarget::syncToMap()
{
	PBXObject::syncToMap();

	setId("buildConfigurationList", buildConfigurationList);
	setArray("buildPhases", buildPhases);
	setArray("buildRules", buildRules);
	setArray("dependencies", dependencies);
	setString("name", name);
	setString("productName", productName);
	setId("productReference", productReference);
	setString("productType", productType);
}

/* PBXProject */

PBXProject::PBXProject()
{
	attributes = std::make_shared<PBXMap>();
	compatibilityVersion = "Xcode 3.2";
	developmentRegion = "English";
	knownRegions = std::make_shared<PBXArray>();
	knownRegions->add(std::make_shared<PBXLiteral>("en"));
	projectReferences = std::make_shared<PBXArray>();
	targets = std::make_shared<PBXArray>();
}

void PBXProject::syncFromMap()
{
	PBXObject::syncFromMap();

	attributes = getMap("attributes");
	buildConfigurationList = getId("buildConfigurationList");
	compatibilityVersion = getString("compatibilityVersion");
	developmentRegion = getString("developmentRegion");
	hasScannedForEncodings = getInteger("hasScannedForEncodings");
	knownRegions = getArray("knownRegions");
	mainGroup = getId("mainGroup");
	productRefGroup = getId("productRefGroup");
	projectReferences = getArray("projectReferences");
	projectDirPath = getString("projectDirPath");
	projectRoot = getString("projectRoot");
	targets = getArray("targets");
}

void PBXProject::syncToMap()
{
	PBXObject::syncToMap();

	setMap("attributes", attributes);
	setId("buildConfigurationList", buildConfigurationList);
	setString("compatibilityVersion", compatibilityVersion);
	setString("developmentRegion", developmentRegion);
	setInteger("hasScannedForEncodings", hasScannedForEncodings);
	setArray("knownRegions", knownRegions);
	setId("mainGroup", mainGroup);
	setId("productRefGroup", productRefGroup);
	setString("projectDirPath", projectDirPath);
	setArray("projectReferences", projectReferences);
	setString("projectRoot", projectRoot);
	setArray("targets", targets);
}


/* PBXReferenceProxy */

PBXReferenceProxy::PBXReferenceProxy()
{

}

void PBXReferenceProxy::syncFromMap()
{
	PBXObject::syncFromMap();

	type = getString("type");
	path = getString("path");
	remoteRef = getId("remoteRef");
	sourceTree = getString("sourceTree");
}

void PBXReferenceProxy::syncToMap()
{
	PBXObject::syncToMap();

	setString("type", type);
	setString("path", path);
	setId("remoteRef", remoteRef);
	setString("sourceTree", sourceTree);
}


/* PBXResourcesBuildPhase */

PBXResourcesBuildPhase::PBXResourcesBuildPhase()
{
	files = std::make_shared<PBXArray>();
}

void PBXResourcesBuildPhase::syncFromMap()
{
	PBXObject::syncFromMap();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXResourcesBuildPhase::syncToMap()
{
	PBXObject::syncToMap();

	setInteger("buildActionMask", buildActionMask);	
	setArray("files", files);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
}


/* PBXShellScriptBuildPhase */

PBXShellScriptBuildPhase::PBXShellScriptBuildPhase()
{
	files = std::make_shared<PBXArray>();
	inputPaths = std::make_shared<PBXArray>();
	outputPaths = std::make_shared<PBXArray>();
}

void PBXShellScriptBuildPhase::syncFromMap()
{
	PBXObject::syncFromMap();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	inputPaths = getArray("inputPaths");
	outputPaths = getArray("outputPaths");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
	shellPath = getString("shellPath");
	shellScript = getString("shellScript");
}

void PBXShellScriptBuildPhase::syncToMap()
{
	PBXObject::syncToMap();

	setInteger("buildActionMask", buildActionMask);	
	setArray("files", files);
	setArray("inputPaths", inputPaths);
	setArray("outputPaths", outputPaths);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
	setString("shellPath", shellPath);
	setString("shellScript", shellScript);
}


/* PBXSourcesBuildPhase */

PBXSourcesBuildPhase::PBXSourcesBuildPhase()
{
	files = std::make_shared<PBXArray>();
}

void PBXSourcesBuildPhase::syncFromMap()
{
	PBXObject::syncFromMap();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXSourcesBuildPhase::syncToMap()
{
	PBXObject::syncToMap();

	setInteger("buildActionMask", buildActionMask);	
	setArray("files", files);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
}


/* PBXTargetDependency */

PBXTargetDependency::PBXTargetDependency()
{

}

void PBXTargetDependency::syncFromMap()
{
	PBXObject::syncFromMap();

	target = getId("target");
	targetProxy = getId("targetProxy");
}

void PBXTargetDependency::syncToMap()
{
	PBXObject::syncToMap();

	setId("target", target);
	setId("targetProxy", targetProxy);
}


/* PBXVariantGroup */

PBXVariantGroup::PBXVariantGroup()
{
	children = std::make_shared<PBXArray>();
}

void PBXVariantGroup::syncFromMap()
{
	PBXObject::syncFromMap();

	children = getArray("children");
	name = getString("name");
	path = getString("path");
	sourceTree = getString("sourceTree");
}

void PBXVariantGroup::syncToMap()
{
	PBXObject::syncToMap();

	setArray("children", children);
	if (name.length() > 0) {
		setString("name", name);
	}
	if (path.length() > 0) {
		setString("path", path);
	}
	setString("sourceTree", sourceTree);
}


/* XCBuildConfiguration */

XCBuildConfiguration::XCBuildConfiguration()
{
	buildSettings = std::make_shared<PBXMap>();
}

void XCBuildConfiguration::syncFromMap()
{
	PBXObject::syncFromMap();

	buildSettings = getMap("buildSettings");
	name = getString("name");
}

void XCBuildConfiguration::syncToMap()
{
	PBXObject::syncToMap();

	setMap("buildSettings", buildSettings);
	setString("name", name);
}


/* XCConfigurationList */

XCConfigurationList::XCConfigurationList()
{
	buildConfigurations = std::make_shared<PBXArray>();
	defaultConfigurationName = "Release";
}

void XCConfigurationList::syncFromMap()
{
	PBXObject::syncFromMap();

	buildConfigurations = getArray("buildConfigurations");
	defaultConfigurationIsVisible = getInteger("defaultConfigurationIsVisible");
	defaultConfigurationName = getString("defaultConfigurationName");
}

void XCConfigurationList::syncToMap()
{
	PBXObject::syncToMap();

	setArray("buildConfigurations", buildConfigurations);
	setInteger("defaultConfigurationIsVisible", defaultConfigurationIsVisible);
	if (defaultConfigurationName.size() > 0) {
		setString("defaultConfigurationName", defaultConfigurationName);
	}
}


/* PBX parser state machine */

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
				} else if (c == '{') {
					begin_object();
					saved_state = PBXParseStateObjectAttrName;
					state = PBXParseStateEatWhitespace;
					stack.push_back(PBXParseStateArrayValueComma);
					offset++;
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
	if (!xcodeproj) {
		value_stack.push_back((xcodeproj = std::make_shared<Xcodeproj>()));
	}
	else if (value_stack.size() == 0) {
		log_fatal_exit("value stack empty");
	}
	else if (value_stack.back()->type() == PBXTypeXcodeproj ||
			 value_stack.back()->type() == PBXTypeMap ||
			 value_stack.back()->type() == PBXTypeObject)
	{
		valptr = PBXValuePtr(new PBXMap());
		static_cast<PBXMap&>(*value_stack.back()).put(current_attr_name, current_attr_comment, valptr);
		value_stack.push_back(valptr);
	}
	else if (value_stack.back()->type() == PBXTypeArray)
	{
		valptr = PBXValuePtr(new PBXMap());
		static_cast<PBXArray&>(*value_stack.back()).add(valptr);
		value_stack.push_back(valptr);
	}
}

void PBXParserImpl::end_object() {
	if (debug) {
		log_debug("end_object");
	}
	if (value_stack.size() == 0) {
		log_fatal_exit("value stack empty");
	}
	if (value_stack.back()->type() == PBXTypeObject ||
		value_stack.back()->type() == PBXTypeXcodeproj) {
		static_cast<PBXObject&>(*value_stack.back()).syncFromMap();
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
	if (value_stack.size() == 0) {
		log_fatal_exit("value stack empty");
	}
	else if (value_stack.back()->type() == PBXTypeMap && current_attr_name == "isa")
	{
		// pop the previous map off the value stack
		PBXValuePtr old_map_ptr = value_stack.back();
		value_stack.pop_back();

		// sanity checking
		if (value_stack.size() == 0) {
			log_fatal_exit("value stack empty");
		} else if (value_stack.back()->type() != PBXTypeMap) {
			log_fatal_exit("parent is not a map");
		}

		// reinstantiate with a concrete type
		const PBXMap &old_map = static_cast<const PBXMap&>(*old_map_ptr);
		PBXMap &parent_map = static_cast<PBXMap&>(*value_stack.back());
		PBXKey &last_key = parent_map.key_order.back();
		PBXId id(last_key.str, last_key.comment);
		valptr = PBXValuePtr(PBXObjectFactory::create(str, id, old_map));
		parent_map.replace(last_key.str, valptr);
		value_stack.push_back(valptr);

		// add isa
		static_cast<PBXMap&>(*value_stack.back()).put(current_attr_name, current_attr_comment, PBXValuePtr(new PBXLiteral(str)));
	}
	else if (value_stack.back()->type() == PBXTypeXcodeproj ||
			 value_stack.back()->type() == PBXTypeMap ||
			 value_stack.back()->type() == PBXTypeObject)
	{
		valptr = PBXValuePtr(is_id ?
			(PBXValue*)new PBXId(str) : (PBXValue*)new PBXLiteral(str));
		static_cast<PBXMap&>(*value_stack.back()).put(current_attr_name, current_attr_comment, valptr);
	}
	else if (value_stack.back()->type() == PBXTypeArray)
	{
		valptr = PBXValuePtr(is_id ?
			(PBXValue*)new PBXId(str) : (PBXValue*)new PBXLiteral(str));
		static_cast<PBXArray&>(*value_stack.back()).add(valptr);
	}
}

void PBXParserImpl::object_value_comment(std::string str) {
	if (debug) {
		log_debug("object_value_comment: \"%s\"", str.c_str());
	}
	if (valptr->type() == PBXTypeId) {
		static_cast<PBXId&>(*valptr).comment = str;
	}
}

void PBXParserImpl::begin_array() {
	if (debug) {
		log_debug("begin_array");
	}
	if (value_stack.size() == 0) {
		log_fatal_exit("value stack empty");
	}
	else if (value_stack.back()->type() == PBXTypeXcodeproj ||
			 value_stack.back()->type() == PBXTypeMap ||
			 value_stack.back()->type() == PBXTypeObject)
	{
		valptr = PBXValuePtr(new PBXArray());
		static_cast<PBXMap&>(*value_stack.back()).put(current_attr_name, current_attr_comment, valptr);
		value_stack.push_back(valptr);
	}
	else if (value_stack.back()->type() == PBXTypeArray)
	{
		valptr = PBXValuePtr(new PBXArray());
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
	if (value_stack.size() == 0) {
		log_fatal_exit("value stack empty");
	}
	else if (value_stack.back()->type() == PBXTypeArray)
	{
		valptr = PBXValuePtr(is_id ?
			(PBXValue*)new PBXId(str) : (PBXValue*)new PBXLiteral(str));
		static_cast<PBXArray&>(*value_stack.back()).array_val.push_back(valptr);
	}
}

void PBXParserImpl::array_value_comment(std::string str) {
	if (debug) {
		log_debug("array_value_comment: \"%s\"", str.c_str());
	}
	if (valptr->type() == PBXTypeId) {
		static_cast<PBXId&>(*valptr).comment = str;
	}
}


/* PBX writer */

void PBXWriter::write(PBXValuePtr value, std::ostream &out, int indent) {
	switch (value->type()) {
		case PBXTypeXcodeproj:
			out << pbxproj_slash_bang << std::endl;
		case PBXTypeObject:
			static_cast<PBXObject&>(*value).syncToMap();
		case PBXTypeMap:
		{
			out << "{" << std::endl;
			PBXMap &map = static_cast<PBXMap&>(*value);
			for (const PBXKey &key : map.key_order) {
				PBXValuePtr &val = map.object_val[key.str];
				for (int i = 0; i <= indent; i++) out << "\t";
				out << key.str;
				if (key.comment.length() > 0) {
					out << " /* " << key.comment << " */";
				}
				out << " = ";
				write(val, out, indent + 1);
				out << ";" << std::endl;
			}
			for (int i = 0; i < indent; i++) out << "\t";
			out << "}";
			break;
		}
		case PBXTypeArray:
		{
			out << "(" << std::endl;
			PBXArray &arr = static_cast<PBXArray&>(*value);
			for (PBXValuePtr &val : arr.array_val) {
				for (int i = 0; i <= indent; i++) out << "\t";
				write(val, out, indent + 1);
				out << "," << std::endl;
			}
			for (int i = 0; i < indent; i++) out << "\t";
			out << ")";
			break;
		}
		case PBXTypeLiteral:
		{
			PBXLiteral &lit = static_cast<PBXLiteral&>(*value);
			if (PBXUtil::literal_requires_quotes(lit.literal_val)) {
				out << "\"" << PBXUtil::escape_quotes(lit.literal_val) << "\"";
			} else {
				out << lit.literal_val;
			}
			break;
		}
		case PBXTypeId:
		{
			PBXId &id = static_cast<PBXId&>(*value);
			out << id.str();
			if (id.comment.length() > 0) {
				out << " /* " << id.comment << " */";
			}
			break;
		}
	}
}

