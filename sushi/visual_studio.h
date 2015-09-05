//
//  visual_studio.h
//

#ifndef visual_studio_h
#define visual_studio_h

struct VSSolution;
struct VSSolutionProperty;
struct VSSolutionProjectConfiguration;
struct VSProject;

typedef std::shared_ptr<VSSolution> VSSolutionPtr;
typedef std::shared_ptr<VSSolutionProperty> VSSolutionPropertyPtr;
typedef std::shared_ptr<VSSolutionProjectConfiguration> VSSolutionProjectConfigurationPtr;
typedef std::shared_ptr<VSProject> VSProjectPtr;

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

	std::vector<VSProjectPtr> projects;
	std::set<std::string> configurations;
	std::vector<VSSolutionProjectConfigurationPtr> projectConfigurations;
	std::vector<VSSolutionPropertyPtr> properties;

	VSSolution();

	void read(std::string solution_file);
	void write(std::ostream &out);

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

struct VSProject
{
	std::string type_guid;
	std::string name;
	std::string path;
	std::string guid;
	std::vector<std::string> dependencies;
};

#endif
