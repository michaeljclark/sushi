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
	    statement_fn_map["cflags"] = {2,  -1, "*", &statement_cflags };
	    statement_fn_map["source"] = {2,  -1, "*", &statement_source };
	    statement_fn_map["libs"] = {2,  -1, "*", &statement_libs };
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
    line.push_back(std::string(value, length));
}

void project::begin_block()
{
	if (debug) log_debug("begin_block");
	if (line.size() < 1) return;
	auto bi = block_fn_map.find(line[0]);
	if (bi != block_fn_map.end()) {
		block_record &rec = bi->second;
		// TODO - check parent block
		if (rec.minargs == rec.maxargs && (int)line.size() != rec.minargs) {
			log_fatal_exit("%s requires %d argument(s)", line[0].c_str(), rec.minargs);
		} else if (rec.minargs > 0 && (int)line.size() < rec.minargs) {
			log_fatal_exit("%s requires at least %d argument(s)", line[0].c_str(), rec.minargs);
		} else if (rec.maxargs > 0 && (int)line.size() > rec.maxargs) {
			log_fatal_exit("%s requires no more than %d argument(s)", line[0].c_str(), rec.maxargs);
		}
		rec.begin_block_fn(this, line);
	} else {
		log_fatal_exit("unrecognized block: %s", line[0].c_str());
	}
    line.clear();
}

void project::end_block()
{
	if (debug) log_debug("end_block");
	// TODO - call end block fn
}

void project::end_statement()
{
	if (debug) log_debug("end_statement");
	if (line.size() < 1) return;
	auto si = statement_fn_map.find(line[0]);
	if (si != statement_fn_map.end()) {
		statement_record &rec = si->second;
		// TODO - check parent block
		if (rec.minargs == rec.maxargs && (int)line.size() != rec.minargs) {
			log_fatal_exit("%s requires %d argument(s)", line[0].c_str(), rec.minargs);
		} else if (rec.minargs > 0 && (int)line.size() < rec.minargs) {
			log_fatal_exit("%s requires at least %d argument(s)", line[0].c_str(), rec.minargs);
		} else if (rec.maxargs > 0 && (int)line.size() > rec.maxargs) {
			log_fatal_exit("%s requires no more than %d argument(s)", line[0].c_str(), rec.maxargs);
		}
		rec.statement_fn(this, line);
	} else {
		log_fatal_exit("unrecognized statement: %s", line[0].c_str());
	}
    line.clear();
}

void project::project_done()
{
	if (debug) log_debug("project_done");
}

