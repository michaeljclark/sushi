#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdint>
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
	std::default_random_engine generator;
	std::uniform_int_distribution<unsigned char> distribution(0, 255);
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
	auto i = object_val.find(id.id_val());
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
	object_val[key] = PBXValuePtr(new PBXId(id.id_val(), id.comment_val));
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
	PBXProjectPtr project = std::make_shared<PBXProject>();
	project->object_id = PBXId::create_root_id();
	rootObject = project->object_id;
	rootObject.comment_val = "Project Object";
	objects->put(project->object_id.id_val(), rootObject.comment_val, project);
}

void Xcodeproj::sync_from_map()
{
	archiveVersion = getInteger("archiveVersion");
	classes = getMap("classes");
	objectVersion = getInteger("objectVersion");
	objects = getMap("objects");
	rootObject = getId("rootObject");
}

void Xcodeproj::sync_to_map()
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

void PBXAggregateTarget::sync_from_map()
{
	PBXObject::sync_from_map();

	buildConfigurationList = getId("buildConfigurationList");
	buildPhases = getArray("buildPhases");
	dependencies = getArray("dependencies");
	name = getString("name");
	productName = getString("productName");
}

void PBXAggregateTarget::sync_to_map()
{
	PBXObject::sync_to_map();

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

void PBXAppleScriptBuildPhase::sync_from_map()
{
	PBXObject::sync_from_map();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXAppleScriptBuildPhase::sync_to_map()
{
	PBXObject::sync_to_map();

	setInteger("buildActionMask", buildActionMask);
	setArray("files", files);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
}


/* PBXBuildFile */

PBXBuildFile::PBXBuildFile()
{

}

void PBXBuildFile::sync_from_map()
{
	PBXObject::sync_from_map();

	fileRef = getId("fileRef");
}

void PBXBuildFile::sync_to_map()
{
	PBXObject::sync_to_map();

	setId("fileRef", fileRef);
}


/* PBXBuildRule */

PBXBuildRule::PBXBuildRule()
{
	outputFiles = std::make_shared<PBXArray>();
}

void PBXBuildRule::sync_from_map()
{
	PBXObject::sync_from_map();

	compilerSpec = getString("compilerSpec");
	filePatterns = getString("filePatterns");
	fileType = getString("fileType");
	isEditable = getInteger("isEditable");
	outputFiles = getArray("outputFiles");
	script = getString("script");
}

void PBXBuildRule::sync_to_map()
{
	PBXObject::sync_to_map();

	setString("compilerSpec", compilerSpec);
	setString("filePatterns", filePatterns);
	setString("fileType", fileType);
	setInteger("isEditable", isEditable);
	setArray("outputFiles", outputFiles);
	setString("script", script);
}


/* PBXBuildStyle */

PBXBuildStyle::PBXBuildStyle()
{
	buildSettings = std::make_shared<PBXMap>();
}

void PBXBuildStyle::sync_from_map()
{
	PBXObject::sync_from_map();

	buildSettings = getMap("buildSettings");
	name = getString("name");
}

void PBXBuildStyle::sync_to_map()
{
	PBXObject::sync_to_map();

	setMap("buildSettings", buildSettings);
	setString("name", name);
}


/* PBXContainerItemProxy */

PBXContainerItemProxy::PBXContainerItemProxy()
{

}

void PBXContainerItemProxy::sync_from_map()
{
	PBXObject::sync_from_map();

	containerPortal = getId("containerPortal");
	proxyType = getInteger("proxyType");
	remoteGlobalIDString = getId("remoteGlobalIDString");
	remoteInfo = getString("remoteInfo");
}

void PBXContainerItemProxy::sync_to_map()
{
	PBXObject::sync_to_map();

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

void PBXCopyFilesBuildPhase::sync_from_map()
{
	PBXObject::sync_from_map();

	buildActionMask = getInteger("buildActionMask");
	dstPath = getString("dstPath");
	dstSubfolderSpec = getInteger("dstSubfolderSpec");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXCopyFilesBuildPhase::sync_to_map()
{
	PBXObject::sync_to_map();

	setInteger("buildActionMask", buildActionMask);	
	if (dstPath.length() > 0) {
		setString("dstPath", dstPath);
	}
	setInteger("dstSubfolderSpec", dstSubfolderSpec);
	setArray("files", files);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
}


/* PBXFileReference */

PBXFileReference::PBXFileReference()
{

}

void PBXFileReference::sync_from_map()
{
	PBXObject::sync_from_map();

	explicitFileType = getString("explicitFileType");
	lastKnownFileType = getString("lastKnownFileType");
	includeInIndex = getInteger("includeInIndex", 1);
	path = getString("path");
	sourceTree = getString("sourceTree");
}

void PBXFileReference::sync_to_map()
{
	PBXObject::sync_to_map();

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

void PBXFrameworksBuildPhase::sync_from_map()
{
	PBXObject::sync_from_map();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXFrameworksBuildPhase::sync_to_map()
{
	PBXObject::sync_to_map();

	setInteger("buildActionMask", buildActionMask);	
	setArray("files", files);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
}


/* PBXGroup */

PBXGroup::PBXGroup()
{
	children = std::make_shared<PBXArray>();
}

void PBXGroup::sync_from_map()
{
	PBXObject::sync_from_map();

	children = getArray("children");
	name = getString("name");
	path = getString("path");
	sourceTree = getString("sourceTree");
}

void PBXGroup::sync_to_map()
{
	PBXObject::sync_to_map();

	setArray("children", children);
	if (name.length() > 0) {
		setString("name", name);
	}
	if (path.length() > 0) {
		setString("path", path);
	}
	setString("sourceTree", sourceTree);
}


/* PBXHeadersBuildPhase */

PBXHeadersBuildPhase::PBXHeadersBuildPhase()
{
	files = std::make_shared<PBXArray>();
}

void PBXHeadersBuildPhase::sync_from_map()
{
	PBXObject::sync_from_map();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXHeadersBuildPhase::sync_to_map()
{
	PBXObject::sync_to_map();

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

void PBXLegacyTarget::sync_from_map()
{
	PBXObject::sync_from_map();

	buildArgumentsString = getString("buildArgumentsString");
	buildConfigurationList = getId("buildConfigurationList");
	buildPhases = getArray("buildPhases");
	buildToolPath = getString("buildToolPath");
	dependencies = getArray("dependencies");
	name = getString("name");
	passBuildSettingsInEnvironment = getInteger("passBuildSettingsInEnvironment");
	productName = getString("productName");
}

void PBXLegacyTarget::sync_to_map()
{
	PBXObject::sync_to_map();

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

PBXNativeTarget::PBXNativeTarget()
{
	buildPhases = std::make_shared<PBXArray>();
	buildRules = std::make_shared<PBXArray>();
	dependencies = std::make_shared<PBXArray>();
}

void PBXNativeTarget::sync_from_map()
{
	PBXObject::sync_from_map();

	buildConfigurationList = getId("buildConfigurationList");
	buildPhases = getArray("buildPhases");
	buildRules = getArray("buildRules");
	dependencies = getArray("dependencies");
	name = getString("name");
	productName = getString("productName");
	productReference = getId("productReference");
	productType = getString("productType");
}

void PBXNativeTarget::sync_to_map()
{
	PBXObject::sync_to_map();

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
	knownRegions = std::make_shared<PBXArray>();
	projectReferences = std::make_shared<PBXArray>();
	targets = std::make_shared<PBXArray>();
}

void PBXProject::sync_from_map()
{
	PBXObject::sync_from_map();

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

void PBXProject::sync_to_map()
{
	PBXObject::sync_to_map();

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

void PBXReferenceProxy::sync_from_map()
{
	PBXObject::sync_from_map();

	fileType = getString("fileType");
	path = getString("path");
	remoteRef = getId("remoteRef");
	sourceTree = getString("sourceTree");
}

void PBXReferenceProxy::sync_to_map()
{
	PBXObject::sync_to_map();

	setString("fileType", fileType);
	setString("path", path);
	setId("remoteRef", remoteRef);
	setString("sourceTree", sourceTree);
}


/* PBXResourcesBuildPhase */

PBXResourcesBuildPhase::PBXResourcesBuildPhase()
{
	files = std::make_shared<PBXArray>();
}

void PBXResourcesBuildPhase::sync_from_map()
{
	PBXObject::sync_from_map();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXResourcesBuildPhase::sync_to_map()
{
	PBXObject::sync_to_map();

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

void PBXShellScriptBuildPhase::sync_from_map()
{
	PBXObject::sync_from_map();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	inputPaths = getArray("inputPaths");
	outputPaths = getArray("outputPaths");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
	shellPath = getString("shellPath");
	shellScript = getString("shellScript");
}

void PBXShellScriptBuildPhase::sync_to_map()
{
	PBXObject::sync_to_map();

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

void PBXSourcesBuildPhase::sync_from_map()
{
	PBXObject::sync_from_map();

	buildActionMask = getInteger("buildActionMask");
	files = getArray("files");
	runOnlyForDeploymentPostprocessing = getInteger("runOnlyForDeploymentPostprocessing");
}

void PBXSourcesBuildPhase::sync_to_map()
{
	PBXObject::sync_to_map();

	setInteger("buildActionMask", buildActionMask);	
	setArray("files", files);
	setInteger("runOnlyForDeploymentPostprocessing", runOnlyForDeploymentPostprocessing);
}


/* PBXTargetDependency */

PBXTargetDependency::PBXTargetDependency()
{

}

void PBXTargetDependency::sync_from_map()
{
	PBXObject::sync_from_map();

	target = getId("target");
	targetProxy = getId("targetProxy");
}

void PBXTargetDependency::sync_to_map()
{
	PBXObject::sync_to_map();

	setId("target", target);
	setId("targetProxy", targetProxy);
}


/* PBXVariantGroup */

PBXVariantGroup::PBXVariantGroup()
{
	children = std::make_shared<PBXArray>();
}

void PBXVariantGroup::sync_from_map()
{
	PBXObject::sync_from_map();

	children = getArray("children");
	name = getString("name");
	path = getString("path");
	sourceTree = getString("sourceTree");
}

void PBXVariantGroup::sync_to_map()
{
	PBXObject::sync_to_map();

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

void XCBuildConfiguration::sync_from_map()
{
	PBXObject::sync_from_map();

	buildSettings = getMap("buildSettings");
	name = getString("name");
}

void XCBuildConfiguration::sync_to_map()
{
	PBXObject::sync_to_map();

	setMap("buildSettings", buildSettings);
	setString("name", name);
}


/* XCConfigurationList */

XCConfigurationList::XCConfigurationList()
{
	buildConfigurations = std::make_shared<PBXArray>();
}

void XCConfigurationList::sync_from_map()
{
	PBXObject::sync_from_map();

	buildConfigurations = getArray("buildConfigurations");
	defaultConfigurationIsVisible = getInteger("defaultConfigurationIsVisible");
	defaultConfigurationName = getString("defaultConfigurationName");
}

void XCConfigurationList::sync_to_map()
{
	PBXObject::sync_to_map();

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
		static_cast<PBXObject&>(*value_stack.back()).sync_from_map();
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
		PBXId id(last_key.key_val, last_key.comment_val);
		valptr = PBXValuePtr(PBXObjectFactory::create(str, id, old_map));
		parent_map.replace(last_key.key_val, valptr);
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
		static_cast<PBXId&>(*valptr).comment_val = str;
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
		static_cast<PBXId&>(*valptr).comment_val = str;
	}
}


/* PBX writer */

void PBXWriter::write(PBXValuePtr value, std::ostream &out, int indent) {
	switch (value->type()) {
		case PBXTypeXcodeproj:
			out << pbxproj_slash_bang << std::endl;
		case PBXTypeObject:
			static_cast<PBXObject&>(*value).sync_to_map();
		case PBXTypeMap:
		{
			out << "{" << std::endl;
			PBXMap &map = static_cast<PBXMap&>(*value);
			for (const PBXKey &key : map.key_order) {
				PBXValuePtr &val = map.object_val[key.key_val];
				for (int i = 0; i <= indent; i++) out << "\t";
				out << key.key_val;
				if (key.comment_val.length() > 0) {
					out << " /* " << key.comment_val << " */";
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
			out << id.id_val();
			if (id.comment_val.length() > 0) {
				out << " /* " << id.comment_val << " */";
			}
			break;
		}
	}
}
