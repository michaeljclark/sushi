//
//  visual_studio_parser.rl
//

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cctype>
#include <cstring>

#include "visual_studio_parser.h"

%%{
	
	machine visual_studio_parser;

	action mark                          { mark = fpc; }
	action w_FormatVersion               { FormatVersion(mark, fpc - mark); }
	action w_CommentVersion              { CommentVersion(mark, fpc - mark); }
	action w_VisualStudioVersion         { VisualStudioVersion(mark, fpc - mark); }
	action w_MinimumVisualStudioVersion  { MinimumVisualStudioVersion(mark, fpc - mark); }
	action w_ProjectTypeGUID             { ProjectTypeGUID(mark, fpc - mark); }
	action w_ProjectName                 { ProjectName(mark, fpc - mark); }
	action w_ProjectPath                 { ProjectPath(mark, fpc - mark); }
	action w_ProjectGUID                 { ProjectGUID(mark, fpc - mark); }
	action w_ProjectDependsGUID          { ProjectDependsGUID(mark, fpc - mark); }
	action w_SolutionConfigPlatform      { SolutionConfigPlatform(mark, fpc - mark); }
	action w_ProjectConfigPlatformGUID   { ProjectConfigPlatformGUID(mark, fpc - mark); }
	action w_ProjectConfigPlatformConfig { ProjectConfigPlatformConfig(mark, fpc - mark); }
	action w_ProjectConfigPlatformProp   { ProjectConfigPlatformProp(mark, fpc - mark); }
	action w_ProjectConfigPlatformValue  { ProjectConfigPlatformValue(mark, fpc - mark); }
	action w_SolutionPropertiesKey       { SolutionPropertiesKey(mark, fpc - mark); }
	action w_SolutionPropertiesValue     { SolutionPropertiesValue(mark, fpc - mark); }

	action done { 
		Done();
		fbreak;
	}

	UTF8_BOM = 0xef 0xbb 0xbf;
	Tab = '\t';
	Newline = '\r' '\n';
	WhiteSpace = (Tab | Newline | ' ');

	Hex = [0123456789ABCDEF];
	GUID = Hex{8} '-' Hex{4} '-' Hex{4} '-' Hex{4} '-' Hex{12};
	VersionNumber = [0123456789.]+;

	MinimumVisualStudioVersion = 'MinimumVisualStudioVersion = ' %mark VersionNumber %w_MinimumVisualStudioVersion Newline;
	VisualStudioVersion = 'VisualStudioVersion = ' %mark VersionNumber %w_VisualStudioVersion Newline;
	CommentVersions = VersionNumber; # ( '2012' | '2013' | '14' );
	CommentVersion = '# Visual Studio ' %mark CommentVersions %w_CommentVersion Newline;
	FormatVersions = VersionNumber; # ( '12.00' );
	FormatVersion = 'Microsoft Visual Studio Solution File, Format Version ' %mark FormatVersions %w_FormatVersion Newline;
	Header = UTF8_BOM Newline FormatVersion CommentVersion VisualStudioVersion? MinimumVisualStudioVersion?;

	ProjectTypeGUID = 'Project("{' %mark GUID %w_ProjectTypeGUID '}")';
	ProjectName = '"' %mark (any - '"')* %w_ProjectName '"';
	ProjectPath = '"' %mark (any - '"')* %w_ProjectPath '"';
	ProjectGUID = '"{' %mark GUID %w_ProjectGUID '}"';
	ProjectClause = ProjectTypeGUID ' = ' ProjectName ', ' ProjectPath ', ' ProjectGUID Newline;
	ProjectSectionDepedenciesClause = Tab 'ProjectSection(ProjectDependencies) = postProject' Newline;
	ProjectSectionDepedency = Tab Tab '{' %mark GUID %w_ProjectDependsGUID '} = {' GUID '}' Newline;
	ProjectSectionDepedenciesEnd = Tab 'EndProjectSection' Newline;
	ProjectSectionDepedencies = ProjectSectionDepedenciesClause ProjectSectionDepedency* ProjectSectionDepedenciesEnd;
	ProjectEnd = 'EndProject' Newline;
	Project = ProjectClause ProjectSectionDepedencies* ProjectEnd;

	SolutionConfigurationPlatformKey = Tab Tab %mark (any - WhiteSpace)* %w_SolutionConfigPlatform;
	SolutionConfigurationPlatformValue = ' = ' %mark (any - WhiteSpace)*;
	SolutionConfigurationPlatform = SolutionConfigurationPlatformKey SolutionConfigurationPlatformValue Newline;
	SolutionConfigurationPlatformsClause = Tab 'GlobalSection(SolutionConfigurationPlatforms) = preSolution' Newline;
	SolutionConfigurationPlatformsEnd = Tab 'EndGlobalSection' Newline;
	SolutionConfigurationPlatforms = SolutionConfigurationPlatformsClause SolutionConfigurationPlatform* SolutionConfigurationPlatformsEnd;

	ProjectConfigurationPlatformGUID = Tab Tab '{' %mark GUID %w_ProjectConfigPlatformGUID '}';
	ProjectConfigurationPlatformConfig = '.' %mark (any - '.')+ %w_ProjectConfigPlatformConfig;
	ProjectConfigurationPlatformProperty = '.' %mark (any - WhiteSpace)+ %w_ProjectConfigPlatformProp;
	ProjectConfigurationPlatformKey =  ProjectConfigurationPlatformGUID ProjectConfigurationPlatformConfig ProjectConfigurationPlatformProperty;
	ProjectConfigurationPlatformValue = ' = ' %mark (any - WhiteSpace)* %w_ProjectConfigPlatformValue;
	ProjectConfigurationPlatform = ProjectConfigurationPlatformKey ProjectConfigurationPlatformValue Newline;
	ProjectConfigurationPlatformsClause = Tab 'GlobalSection(ProjectConfigurationPlatforms) = postSolution' Newline;
	ProjectConfigurationPlatformsEnd = Tab 'EndGlobalSection' Newline;
	ProjectConfigurationPlatforms = ProjectConfigurationPlatformsClause ProjectConfigurationPlatform* ProjectConfigurationPlatformsEnd;

	SolutionPropertiesKey = Tab Tab %mark (any - WhiteSpace)* %w_SolutionPropertiesKey;
	SolutionPropertiesValue = ' = ' %mark (any - WhiteSpace)* %w_SolutionPropertiesValue;
	SolutionProperty = SolutionPropertiesKey SolutionPropertiesValue Newline;
	SolutionPropertiesClause = Tab 'GlobalSection(SolutionProperties) = preSolution' Newline;
	SolutionPropertiesEnd = Tab 'EndGlobalSection' Newline;
	SolutionProperties = SolutionPropertiesClause SolutionProperty* SolutionPropertiesEnd;

	GlobalClause = 'Global' Newline;
	GlobalSections = SolutionConfigurationPlatforms* ProjectConfigurationPlatforms* SolutionProperties*;
	GlobalEnd = 'EndGlobal' Newline;
	Global = GlobalClause GlobalSections GlobalEnd;
	Solution = Header Project* Global? %done;

	main := Solution;

}%%

%% write data;

bool VisualStudioParser::parse(const char *buffer, size_t len)
{
	int cs = visual_studio_parser_en_main;
	
	const char *mark = NULL;
	const char *p = buffer;
	const char *pe = buffer + strlen(buffer);
	const char *eof = pe;

	%% write init;
	%% write exec;

	return (cs != visual_studio_parser_error || cs == visual_studio_parser_first_final);
}
