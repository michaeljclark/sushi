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
	void Done();
};

struct VSProject
{

};

#endif
