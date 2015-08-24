//
//  project_parser.rl
//

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cctype>
#include <cstring>

#include "project_parser.h"

%%{
    
    machine project_parser;

    action mark             { mark = fpc; }
    action w_start_block    { start_block(); }
    action w_end_block      { end_block(); }
    action w_symbol         { symbol(mark, fpc - mark); }
    action w_end_statement  { end_statement(); }

    action done { 
        project_done();
        fbreak;
    }

    Eol = ';' %w_end_statement;
    newline = ('\r' | '\n' ) | '\n';
    ws = (' ' | '\t' | '\r' | '\n' )+;
    comment = '#' ( any - '\n' )* '\n';
    symbol = ( any - ';' - ws - '{' - '}' - '#' )+ >mark %w_symbol;
    statement = ( symbol ( ws symbol)* ) ws* Eol;
    start_block = ( symbol ( ws symbol)* ) ws+ '{' %w_start_block;
    end_block = '}' %w_end_block;
    project = ( comment | start_block | end_block | statement | ws )* %done;

    main := project;

}%%

%% write data;

bool project_parser::parse(const char *buffer, size_t len)
{
    int cs = project_parser_en_main;
    
    const char *mark = NULL;
    const char *p = buffer;
    const char *pe = buffer + strlen(buffer);
    const char *eof = pe;

    %% write init;
    %% write exec;

    return (cs != project_parser_error && cs == project_parser_first_final);
}
