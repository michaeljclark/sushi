//
//  project.cc
//

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <map>

#include "log.h"
#include "util.h"
#include "project_parser.h"
#include "project.h"


/* project */

const bool project::debug = true;
std::once_flag project::function_map_init;
statement_function_map project::statement_fn_map;
block_function_map project::block_fn_map;

void project::block_project_begin(project *project, statement &st)
{

}

void project::block_project_end(project *project)
{

}

void project::block_config_begin(project *project, statement &st)
{

}

void project::block_config_end(project *project)
{
	
}

void project::block_lib_begin(project *project, statement &st)
{

}

void project::block_lib_end(project *project)
{
	
}

void project::block_tool_begin(project *project, statement &st)
{

}

void project::block_tool_end(project *project)
{
	
}

void project::statement_type(project *project, statement &st)
{

}

void project::statement_define(project *project, statement &st)
{

}

void project::statement_cflags(project *project, statement &st)
{

}

void project::statement_source(project *project, statement &st)
{

}

void project::statement_libs(project *project, statement &st)
{

}

void project::init()
{
	std::call_once(function_map_init, []()
	{
	    block_fn_map["project"] = { 2,  2, "", &block_project_begin, &block_project_end };
	    block_fn_map["config"] = {2,  2, "config", &block_config_begin, &block_config_end };
	    block_fn_map["lib"] = {2,  2, "project", &block_lib_begin, &block_lib_end };
	    block_fn_map["tool"] = {2,  2, "project", &block_tool_begin, &block_tool_end };
	    statement_fn_map["type"] = {2,  2, "*", &statement_type };
	    statement_fn_map["define"] = {3,  -1, "*", &statement_define };
	    statement_fn_map["cflags"] = {3,  -1, "*", &statement_cflags };
	    statement_fn_map["source"] = {3,  -1, "*", &statement_source };
	    statement_fn_map["libs"] = {3,  -1, "*", &statement_libs };
	});
}

project::project() { init(); }

void project::read(std::string project_file)
{
    std::vector<char> buf = util::read_file(project_file);
    if (!parse(buf.data(), buf.size())) {
        log_fatal_exit("config: parse error");
    }
}

void project::symbol(const char *value, size_t length)
{
	if (debug) log_debug("symbol: %s", std::string(value, length).c_str());
    current_statement.push_back(std::string(value, length));
}

void project::begin_block()
{
	if (debug) log_debug("begin_block");
}

void project::end_block()
{
	if (debug) log_debug("end_block");
}

void project::end_statement()
{
	if (debug) log_debug("end_statement");
}

void project::project_done()
{
	if (debug) log_debug("project_done");
}

