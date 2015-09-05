//
//  visual_studio_parser.h
//

#ifndef visual_studio_parser_h
#define visual_studio_parser_h

struct VisualStudioParser
{
    int cs;
    int eof;
    size_t mark;
    
    virtual ~VisualStudioParser() {}
    
    bool parse(const char *buffer, size_t len);
    
    virtual void FormatVersion(const char *value, size_t length) = 0;
    virtual void CommentVersion(const char *value, size_t length) = 0;
    virtual void VisualStudioVersion(const char *value, size_t length) = 0;
    virtual void MinimumVisualStudioVersion(const char *value, size_t length) = 0;
    virtual void ProjectTypeGUID(const char *value, size_t length) = 0;
    virtual void ProjectName(const char *value, size_t length) = 0;
    virtual void ProjectPath(const char *value, size_t length) = 0;
    virtual void ProjectGUID(const char *value, size_t length) = 0;
    virtual void ProjectDependsGUID(const char *value, size_t length) = 0;
    virtual void SolutionConfigPlatformKey(const char *value, size_t length) = 0;
    virtual void SolutionConfigPlatformValue(const char *value, size_t length) = 0;
    virtual void ProjectConfigPlatformGUID(const char *value, size_t length) = 0;
    virtual void ProjectConfigPlatformConfig(const char *value, size_t length) = 0;
    virtual void ProjectConfigPlatformProp(const char *value, size_t length) = 0;
    virtual void ProjectConfigPlatformValue(const char *value, size_t length) = 0;
    virtual void SolutionPropertiesKey(const char *value, size_t length) = 0;
    virtual void SolutionPropertiesValue(const char *value, size_t length) = 0;
    virtual void Done() = 0;
};

#endif
