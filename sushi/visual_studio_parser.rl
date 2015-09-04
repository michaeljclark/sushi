//
//  project_parser.rl
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

    action done { 
        Done();
        fbreak;
    }

    utf8_bom = 0xef 0xbb 0xbf;
    newline = '\r' '\n';
    ws = (' ' | '\t' | '\r' | '\n' )+;

    VersionNumber = [0123456789.]+;
    MinimumVisualStudioVersion = "MinimumVisualStudioVersion = " %mark VersionNumber %w_MinimumVisualStudioVersion newline;
    VisualStudioVersion = "VisualStudioVersion = " %mark VersionNumber %w_VisualStudioVersion newline;
    CommentVersions = VersionNumber; # ( "2012" | "2013" | "14" );
    CommentVersion = "# Visual Studio " %mark CommentVersions %w_CommentVersion newline;
    FormatVersions = VersionNumber; # ( "12.00" );
    FormatVersion = "Microsoft Visual Studio Solution File, Format Version " %mark FormatVersions %w_FormatVersion newline;
    Header = utf8_bom newline FormatVersion CommentVersion VisualStudioVersion? MinimumVisualStudioVersion?;
    Solution = Header %done;

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
