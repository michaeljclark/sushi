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

const bool project::debug = false;
std::once_flag project::function_map_init;
statement_function_map project::statement_fn_map;
block_function_map project::block_fn_map;

void project::block_project_begin(project *project, statement &line)
{
	auto root = std::make_shared<project_root>();
	root->project_name = line[1];
	project->root = root;
	project->item_stack.push_back(project->root);
}

void project::block_config_begin(project *project, statement &line)
{
	auto config = std::make_shared<project_config>();
	config->config_name = line[1];
	project->item_stack.push_back(config);
	project->root->config_list.push_back(config);
}

void project::block_lib_begin(project *project, statement &line)
{
	auto lib = std::make_shared<project_lib>();
	lib->lib_name = line[1];
	project->item_stack.push_back(lib);
	project->root->lib_list.push_back(lib);
}

void project::block_tool_begin(project *project, statement &line)
{
	auto tool = std::make_shared<project_tool>();
	tool->tool_name = line[1];
	project->item_stack.push_back(tool);
	project->root->tool_list.push_back(tool);
}

void project::statement_type(project *project, statement &line)
{
	auto lib = std::static_pointer_cast<project_lib>(project->item_stack.back());
	if (line[1] == "static" || line[1] == "dynamic") {
		lib->lib_type = line[1];
	} else {
		log_fatal_exit("type must be 'static' or 'dynamic'");
	}
}

void project::statement_define(project *project, statement &line)
{
    auto config = std::static_pointer_cast<project_config>(project->item_stack.back());
    config->defines[line[1]] = line[2];
}

void project::statement_cflags(project *project, statement &line)
{
    auto config = std::static_pointer_cast<project_config>(project->item_stack.back());
    for (size_t i = 1; i < line.size(); i++) {
    	config->cflags.push_back(line[i]);
    }
}

void project::statement_source(project *project, statement &line)
{
    auto target = std::static_pointer_cast<project_target>(project->item_stack.back());
    for (size_t i = 1; i < line.size(); i++) {
    	target->source.push_back(line[i]);
    }
}

void project::statement_libs(project *project, statement &line)
{
    auto tool = std::static_pointer_cast<project_tool>(project->item_stack.back());
    for (size_t i = 1; i < line.size(); i++) {
    	tool->libs.push_back(line[i]);
    }
}

void project::init()
{
	std::call_once(function_map_init, []()
	{
	    block_fn_map["project"] = { 2,  2, "<root>", &block_project_begin };
	    block_fn_map["config"] = {2,  2, "project", &block_config_begin };
	    block_fn_map["lib"] = {2,  2, "project", &block_lib_begin };
	    block_fn_map["tool"] = {2,  2, "project", &block_tool_begin };
	    statement_fn_map["type"] = {2,  2, "lib", &statement_type };
	    statement_fn_map["define"] = {3,  3, "lib|tool|config", &statement_define };
	    statement_fn_map["cflags"] = {2,  -1, "lib|tool|config", &statement_cflags };
	    statement_fn_map["source"] = {2,  -1, "lib|tool", &statement_source };
	    statement_fn_map["libs"] = {2,  -1, "tool", &statement_libs };
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

bool project::check_parent(std::string allowed_parent_spec)
{
	std::vector<std::string> allowed_parent_list = util::split(allowed_parent_spec, '|');
	std::string parent = item_stack.size() == 0 ? "<root>" : item_stack.back()->block_name();
	for (std::string allowed_parent : allowed_parent_list) {
		if (allowed_parent == parent) return true;
	}
	return false;
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
		if (!check_parent(rec.parent_block_spec)) {
			log_fatal_exit("%s must be defined within %s", line[0].c_str(), rec.parent_block_spec.c_str());
		} else if (rec.minargs == rec.maxargs && (int)line.size() != rec.minargs) {
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
	if (item_stack.size() == 0) {
		log_fatal_exit("unexpected close block");
	}
	if (!item_stack.back()->validate()) {
		log_fatal_exit("invalid block: %s", item_stack.back()->block_name().c_str());
	}
	item_stack.pop_back();
}

void project::end_statement()
{
	if (debug) log_debug("end_statement");
	if (line.size() < 1) return;
	auto si = statement_fn_map.find(line[0]);
	if (si != statement_fn_map.end()) {
		statement_record &rec = si->second;
		if (!check_parent(rec.parent_block_spec)) {
			log_fatal_exit("%s must be defined within %s", line[0].c_str(), rec.parent_block_spec.c_str());
		} else if (rec.minargs == rec.maxargs && (int)line.size() != rec.minargs) {
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

