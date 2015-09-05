//
//  visual_studio.h
//

#ifndef visual_studio_h
#define visual_studio_h

struct VSSolution : VisualStudioParser
{
	void read(std::string solution_file);

	void FormatVersion(const char *value, size_t length);
	void CommentVersion(const char *value, size_t length);
	void VisualStudioVersion(const char *value, size_t length);
	void MinimumVisualStudioVersion(const char *value, size_t length);
    void ProjectTypeGUID(const char *value, size_t length);
    void ProjectName(const char *value, size_t length);
    void ProjectPath(const char *value, size_t length);
    void ProjectGUID(const char *value, size_t length);
    void ProjectDependsGUID(const char *value, size_t length);
    void SolutionConfigPlatformKey(const char *value, size_t length);
    void SolutionConfigPlatformValue(const char *value, size_t length);
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

};

#endif
