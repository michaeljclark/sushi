//
//  project.h
//

#ifndef project_h
#define project_h

struct project;
struct project_block;
struct project_config;
struct project_lib;
struct project_tool;
struct statement_record;
struct block_record;
typedef std::shared_ptr<project> project_ptr;
typedef std::shared_ptr<project_block> project_block_ptr;
typedef std::shared_ptr<project_config> project_config_ptr;
typedef std::shared_ptr<project_lib> project_lib_ptr;
typedef std::shared_ptr<project_tool> project_tool_ptr;
typedef std::vector<std::string> statement;
typedef std::function<void(project*,statement&)>statement_function;
typedef std::map<std::string,statement_record> statement_function_map;
typedef std::function<void(project*,statement&)>block_begin_function;
typedef std::function<void(project*)>block_end_function;
typedef std::map<std::string,block_record> block_function_map;

struct statement_record
{
    int minargs;
    int maxargs;
    std::string parent_block;
    statement_function statement_fn;
};

struct block_record
{
    int minargs;
    int maxargs;
    std::string parent_block;
    block_begin_function begin_block_fn;
    block_end_function end_block_fn;
};

struct project_block
{
	std::string type;
	std::vector<std::string> libs;
	std::vector<std::string> source;
	std::vector<std::string> cflags;
	std::map<std::string,std::string> defines;

	virtual ~project_block() {}
};

struct project_config : project_block
{

};

struct project_lib : project_block
{

};

struct project_tool : project_block
{

};

struct project : project_parser
{
	static const bool debug;

	static std::once_flag function_map_init;
	static statement_function_map statement_fn_map;
	static block_function_map block_fn_map;

	static void block_project_begin(project *project, statement &st);
	static void block_project_end(project *project);
	static void block_config_begin(project *project, statement &st);
	static void block_config_end(project *project);
	static void block_lib_begin(project *project, statement &st);
	static void block_lib_end(project *project);
	static void block_tool_begin(project *project, statement &st);
	static void block_tool_end(project *project);
	static void statement_type(project *project, statement &st);
	static void statement_define(project *project, statement &st);
	static void statement_cflags(project *project, statement &st);
	static void statement_source(project *project, statement &st);
	static void statement_libs(project *project, statement &st);

	static void init();

	statement line;
	std::vector<project_block_ptr> block_stack;

	std::vector<project_config_ptr> config_list;
	std::vector<project_lib_ptr> lib_list;
	std::vector<project_tool_ptr> tool_list;

    project();

    void read(std::string project_file);
    
    void symbol(const char *value, size_t length);
    void end_statement();
    void begin_block();
    void end_block();
    void project_done();
};

#endif
