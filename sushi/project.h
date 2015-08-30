//
//  project.h
//

#ifndef project_h
#define project_h

struct project;
struct project_root;
struct project_item;
struct project_config;
struct project_target;
struct project_lib;
struct project_tool;
typedef std::shared_ptr<project_root> project_root_ptr;
typedef std::shared_ptr<project> project_ptr;
typedef std::shared_ptr<project_item> project_item_ptr;
typedef std::shared_ptr<project_config> project_config_ptr;
typedef std::shared_ptr<project_target> project_target_ptr;
typedef std::shared_ptr<project_lib> project_lib_ptr;
typedef std::shared_ptr<project_tool> project_tool_ptr;

struct statement_record;
struct block_record;
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
	std::string parent_block_spec;
	statement_function statement_fn;
};

struct block_record
{
	int minargs;
	int maxargs;
	std::string parent_block_spec;
	block_begin_function begin_block_fn;
};

struct project_item
{
	virtual ~project_item() {}
	virtual std::string block_name() = 0;
	virtual bool validate() { return true; }
};

struct project_root : project_item
{
	virtual std::string block_name() { return "project"; }

	std::string project_name;
	std::vector<project_config_ptr> config_list;
	std::vector<project_lib_ptr> lib_list;
	std::vector<project_tool_ptr> tool_list;
};

struct project_config : project_item
{
	virtual std::string block_name() { return "config"; }

	std::string config_name;
	std::map<std::string,std::string> defines;
	std::vector<std::string> cflags;
};

struct project_target : project_config
{
	virtual std::string target_name() = 0;

	std::vector<std::string> source;
};

struct project_lib : project_target
{
	virtual std::string block_name() { return "lib"; }
	virtual std::string target_name() { return lib_name; }

	std::string lib_name;
	std::string lib_type;
};

struct project_tool : project_target
{
	virtual std::string block_name() { return "tool"; }
	virtual std::string target_name() { return tool_name; }

	std::string tool_name;
	std::vector<std::string> libs;
};

struct project : project_parser
{
	static const bool debug;

	static std::once_flag function_map_init;
	static statement_function_map statement_fn_map;
	static block_function_map block_fn_map;

	static void block_project_begin(project *project, statement &line);
	static void block_config_begin(project *project, statement &line);
	static void block_lib_begin(project *project, statement &line);
	static void block_tool_begin(project *project, statement &line);
	static void statement_type(project *project, statement &line);
	static void statement_define(project *project, statement &line);
	static void statement_cflags(project *project, statement &line);
	static void statement_source(project *project, statement &line);
	static void statement_libs(project *project, statement &line);

	static void init();

	statement line;
	project_root_ptr root;
	std::vector<project_item_ptr> item_stack;

	project();

	void read(std::string project_file);
	bool check_parent(std::string allowed_parent_spec);
	
	void symbol(const char *value, size_t length);
	void end_statement();
	void begin_block();
	void end_block();
	void project_done();
};

#endif
