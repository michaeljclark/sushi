//
//  project.h
//

#ifndef project_h
#define project_h

struct project : project_parser
{
    project();

    void read(std::string project_file);
    
    void symbol(const char *value, size_t length);
    void end_statement();
    void start_block();
    void end_block();
    void config_done();
};

#endif
