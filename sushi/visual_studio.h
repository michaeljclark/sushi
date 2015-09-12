//
//  visual_studio.h
//

#ifndef visual_studio_h
#define visual_studio_h

struct VSSolution;
struct VSSolutionProperty;
struct VSSolutionProject;
struct VSSolutionProjectConfiguration;
struct VSProject;

typedef std::shared_ptr<VSSolution> VSSolutionPtr;
typedef std::shared_ptr<VSSolutionProperty> VSSolutionPropertyPtr;
typedef std::shared_ptr<VSSolutionProject> VSSolutionProjectPtr;
typedef std::shared_ptr<VSSolutionProjectConfiguration> VSSolutionProjectConfigurationPtr;
typedef std::shared_ptr<VSProject> VSProjectPtr;

struct VSObject;
struct VSImport;
struct VSImportGroup;
struct VSItemGroup;
struct VSItemDefinitionGroup;
struct VSPropertyGroup;
struct VSProjectConfiguration;
struct VSClCompile;
struct VSClInclude;
struct VSLink;

typedef std::shared_ptr<VSObject> VSObjectPtr;
typedef std::shared_ptr<VSItemGroup> VSItemGroupPtr;
typedef std::shared_ptr<VSImport> VSImportPtr;
typedef std::shared_ptr<VSImportGroup> VSImportGroupPtr;
typedef std::shared_ptr<VSItemDefinitionGroup> VSItemDefinitionGroupPtr;
typedef std::shared_ptr<VSPropertyGroup> VSPropertyGroupPtr;
typedef std::shared_ptr<VSProjectConfiguration> VSProjectConfigurationPtr;
typedef std::shared_ptr<VSClCompile> VSClCompilePtr;
typedef std::shared_ptr<VSClInclude> VSClIncludePtr;
typedef std::shared_ptr<VSLink> VSLinkPtr;

struct VSSolutionProject
{
	std::string type_guid;
	std::string name;
	std::string path;
	std::string guid;
	std::vector<std::string> dependencies;
	VSProjectPtr project;

	std::vector<std::string> dependenciesToResolve;
};

struct VSSolutionProjectConfiguration
{
	std::string guid;
	std::string config;
	std::string property;
	std::string value;
};

struct VSSolutionProperty
{
	std::string name;
	std::string value;
};

struct VSSolution : VisualStudioParser
{
	static const std::string VisualCPPProjectGUID;

	static const bool debug;

	std::string format_version;
	std::string comment_version;
	std::string visual_studio_version;
	std::string minimum_visual_studio_version;

	std::vector<VSSolutionProjectPtr> projects;
	std::set<std::string> configurations;
	std::vector<VSSolutionProjectConfigurationPtr> projectConfigurations;
	std::vector<VSSolutionPropertyPtr> properties;

	VSSolution();

	void setDefaultVersion();
	void createDefaultConfigurations();
	VSProjectPtr createProject(std::string project_name, std::string project_type,
		std::vector<std::string> depends, std::vector<std::string> libs, std::vector<std::string> source);

	VSProjectConfigurationPtr legacyConfig(std::string config);
	std::string findGuidForProject(std::string project_name);
	void resolveDependencies();

	void read(std::string solution_file);
	void write(std::string solution_file);

	void FormatVersion(const char *value, size_t length);
	void CommentVersion(const char *value, size_t length);
	void VisualStudioVersion(const char *value, size_t length);
	void MinimumVisualStudioVersion(const char *value, size_t length);
	void ProjectTypeGUID(const char *value, size_t length);
	void ProjectName(const char *value, size_t length);
	void ProjectPath(const char *value, size_t length);
	void ProjectGUID(const char *value, size_t length);
	void ProjectDependsGUID(const char *value, size_t length);
	void SolutionConfigPlatform(const char *value, size_t length);
	void ProjectConfigPlatformGUID(const char *value, size_t length);
	void ProjectConfigPlatformConfig(const char *value, size_t length);
	void ProjectConfigPlatformProp(const char *value, size_t length);
	void ProjectConfigPlatformValue(const char *value, size_t length);
	void SolutionPropertiesKey(const char *value, size_t length);
	void SolutionPropertiesValue(const char *value, size_t length);
	void Done();
};


struct VSObjectFactory;
typedef std::shared_ptr<VSObjectFactory> VSObjectFactoryPtr;
template <typename T> struct VSObjectFactoryImpl;

struct VSObjectFactory
{
	virtual ~VSObjectFactory() {}
	virtual VSObjectPtr create() = 0;
};

template <typename T>
struct VSObjectFactoryImpl : VSObjectFactory
{
	VSObjectPtr create() { return std::make_shared<T>(); }
};

struct VSObject
{
	virtual ~VSObject() {}

	virtual void fromXML(tinyxml2::XMLElement *element) = 0;
	virtual void toXML(tinyxml2::XMLElement *parent) = 0;
};

template <typename T> struct VSObjectImpl : VSObject
{
	const std::string& type_name() { return T::type_name; }
};

struct VSProject
{
	static const std::string xmlns;

	static std::once_flag factoryInit;
	static std::map<std::string,VSObjectFactoryPtr> factoryMap;

	template <typename T> static void registerFactory() {
		factoryMap.insert(std::pair<std::string,VSObjectFactoryPtr>
			(T::type_name, VSObjectFactoryPtr(new VSObjectFactoryImpl<T>())));
	}

	static void init();

	static VSObjectPtr createObject(std::string class_name) {
		init();
		auto it = factoryMap.find(class_name);
		return it != factoryMap.end() ? it->second->create() : VSObjectPtr();
	}

	VSProject();

	tinyxml2::XMLDocument doc;

	std::string defaultTargets;
	std::string toolsVersion;
	std::vector<VSObjectPtr> objectList;

	void read(std::string project_file);
	void write(std::string project_file);

	void xmlToProject();
	void projectToXml();
};

struct VSImport : VSObjectImpl<VSImport>
{
	static const std::string type_name;

	std::string project;
	std::string condition;
	std::string label;

	void fromXML(tinyxml2::XMLElement *element);
	void toXML(tinyxml2::XMLElement *parent);
};

struct VSImportGroup : VSObjectImpl<VSImportGroup>
{
	static const std::string type_name;

	std::string label;
	std::string condition;
	std::vector<VSObjectPtr> objectList;

	void fromXML(tinyxml2::XMLElement *element);
	void toXML(tinyxml2::XMLElement *parent);
};

struct VSItemGroup : VSObjectImpl<VSItemGroup>
{
	static const std::string type_name;

	std::string label;
	std::vector<VSObjectPtr> objectList;

	void fromXML(tinyxml2::XMLElement *element);
	void toXML(tinyxml2::XMLElement *parent);
};

struct VSItemDefinitionGroup : VSObjectImpl<VSItemDefinitionGroup>
{
	static const std::string type_name;

	std::string condition;
	std::vector<VSObjectPtr> objectList;

	void fromXML(tinyxml2::XMLElement *element);
	void toXML(tinyxml2::XMLElement *parent);
};

struct VSPropertyGroup : VSObjectImpl<VSPropertyGroup>
{
	static const std::string type_name;

	std::string condition;
	std::string label;
	std::map<std::string,std::string> properties;

	void fromXML(tinyxml2::XMLElement *element);
	void toXML(tinyxml2::XMLElement *parent);
};

struct VSProjectConfiguration : VSObjectImpl<VSProjectConfiguration>
{
	static const std::string type_name;

	std::string include;
	std::string configuration;
	std::string platform;

	void fromXML(tinyxml2::XMLElement *element);
	void toXML(tinyxml2::XMLElement *parent);
};

struct VSClCompile : VSObjectImpl<VSClCompile>
{
	static const std::string type_name;

	std::string include;
	std::map<std::string,std::string> properties;

	void fromXML(tinyxml2::XMLElement *element);
	void toXML(tinyxml2::XMLElement *parent);
};

struct VSClInclude : VSObjectImpl<VSClInclude>
{
	static const std::string type_name;

	std::string include;

	void fromXML(tinyxml2::XMLElement *element);
	void toXML(tinyxml2::XMLElement *parent);
};

struct VSLink : VSObjectImpl<VSLink>
{
	static const std::string type_name;

	std::map<std::string,std::string> properties;

	void fromXML(tinyxml2::XMLElement *element);
	void toXML(tinyxml2::XMLElement *parent);
};

#endif
