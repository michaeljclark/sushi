//
//  project_parser.h
//

#ifndef prject_parser_h
#define prject_parser_h

struct project_parser
{
    int cs;
    int eof;
    size_t mark;
    
    virtual ~project_parser() {}
    
    bool parse(const char *buffer, size_t len);
    
    virtual void symbol(const char *value, size_t length) = 0;
    virtual void start_block() = 0;
    virtual void end_block() = 0;
    virtual void end_statement() = 0;
    virtual void project_done() = 0;
};

#endif