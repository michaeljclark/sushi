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
#include <map>
#include <set>

#include "log.h"
#include "util.h"
#include "filesystem.h"
#include "project_parser.h"
#include "project.h"


/* project */

const bool project::debug = false;
bool project::function_map_init = false;
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

void project::statement_depends(project *project, statement &line)
{
	auto target = std::static_pointer_cast<project_target>(project->item_stack.back());
	for (size_t i = 1; i < line.size(); i++) {
		target->depends.push_back(line[i]);
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
	if (!function_map_init) {
		block_fn_map["project"] = block_record(2,  2, "<root>", &block_project_begin);
		block_fn_map["config"] = block_record(2,  2, "project", &block_config_begin);
		block_fn_map["lib"] = block_record(2,  2, "project", &block_lib_begin);
		block_fn_map["tool"] = block_record(2,  2, "project", &block_tool_begin);
		statement_fn_map["type"] = statement_record(2,  2, "lib", &statement_type);
		statement_fn_map["define"] = statement_record(3,  3, "lib|tool|config", &statement_define);
		statement_fn_map["cflags"] = statement_record(2,  -1, "lib|tool|config", &statement_cflags);
		statement_fn_map["depends"] = statement_record(2,  2, "lib|tool", &statement_depends);
		statement_fn_map["source"] = statement_record(2,  -1, "lib|tool", &statement_source);
		statement_fn_map["libs"] = statement_record(2,  -1, "tool", &statement_libs);
		function_map_init = true;
	};
}

project::project() { init(); }

void project::read(std::string project_file)
{
	std::vector<char> buf = filesystem::read_file(project_file);
	if (!parse(buf.data(), buf.size())) {
		log_fatal_exit("project: parse error");
	}
}

bool project::check_parent(std::string allowed_parent_spec)
{
	std::vector<std::string> allowed_parent_list = util::split(allowed_parent_spec, "|");
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


/* project_root */

std::vector<std::string> project_root::get_config_list()
{
	std::set<std::string> set;
	for (auto config : config_list) {
		if (config->config_name == "*") continue;
		set.insert(config->config_name);
	}
	std::vector<std::string> list;
	for (std::string item : set) {
		list.push_back(item);
	}
	return list;
}

std::vector<std::string> project_root::get_lib_list()
{
	std::set<std::string> set;
	for (auto lib : lib_list) {
		if (lib->lib_name == "*") continue;
		set.insert(lib->lib_name);
	}
	std::vector<std::string> list;
	for (std::string item : set) {
		list.push_back(item);
	}
	return list;
}

std::vector<std::string> project_root::get_tool_list()
{
	std::set<std::string> set;
	for (auto tool : tool_list) {
		if (tool->tool_name == "*") continue;
		set.insert(tool->tool_name);
	}
	std::vector<std::string> list;
	for (std::string item : set) {
		list.push_back(item);
	}
	return list;
}

project_config_ptr project_root::get_config(std::string name)
{
	project_config_ptr merged_config = std::make_shared<project_config>();
	merged_config->config_name = name;
	for (auto config : config_list) {
		if (config->config_name != "*") continue;
		for (auto ent : config->defines) merged_config->defines[ent.first] = ent.second;
		merged_config->cflags.insert(merged_config->cflags.end(), config->cflags.begin(), config->cflags.end());
	}
	for (auto config : config_list) {
		if (config->config_name != name) continue;
		for (auto ent : config->defines) merged_config->defines[ent.first] = ent.second;
		merged_config->cflags.insert(merged_config->cflags.end(), config->cflags.begin(), config->cflags.end());
	}
	return merged_config;
}

project_lib_ptr project_root::get_lib(std::string name)
{
	project_lib_ptr merged_lib = std::make_shared<project_lib>();
	merged_lib->lib_name = name;
	for (auto lib : lib_list) {
		if (lib->lib_name != "*") continue;
		if (lib->lib_type.size() > 0) merged_lib->lib_type = lib->lib_type;
		for (auto ent : lib->defines) merged_lib->defines[ent.first] = ent.second;
		merged_lib->cflags.insert(merged_lib->cflags.end(), lib->cflags.begin(), lib->cflags.end());
		merged_lib->depends.insert(merged_lib->depends.end(), lib->depends.begin(), lib->depends.end());
	}
	for (auto lib : lib_list) {
		if (lib->lib_name != name) continue;
		if (lib->lib_type.size() > 0) merged_lib->lib_type = lib->lib_type;
		for (auto ent : lib->defines) merged_lib->defines[ent.first] = ent.second;
		merged_lib->cflags.insert(merged_lib->cflags.end(), lib->cflags.begin(), lib->cflags.end());
		merged_lib->depends.insert(merged_lib->depends.end(), lib->depends.begin(), lib->depends.end());
		merged_lib->source.insert(merged_lib->source.end(), lib->source.begin(), lib->source.end());
	}
	return merged_lib;
}

project_tool_ptr project_root::get_tool(std::string name)
{
	project_tool_ptr merged_tool = std::make_shared<project_tool>();
	merged_tool->tool_name = name;
	for (auto tool : tool_list) {
		if (tool->tool_name != "*") continue;
		for (auto ent : tool->defines) merged_tool->defines[ent.first] = ent.second;
		merged_tool->cflags.insert(merged_tool->cflags.end(), tool->cflags.begin(), tool->cflags.end());
		merged_tool->libs.insert(merged_tool->libs.end(), tool->libs.begin(), tool->libs.end());
	}
	for (auto tool : tool_list) {
		if (tool->tool_name != name) continue;
		for (auto ent : tool->defines) merged_tool->defines[ent.first] = ent.second;
		merged_tool->cflags.insert(merged_tool->cflags.end(), tool->cflags.begin(), tool->cflags.end());
		merged_tool->depends.insert(merged_tool->depends.end(), tool->depends.begin(), tool->depends.end());
		merged_tool->source.insert(merged_tool->source.end(), tool->source.begin(), tool->source.end());
		merged_tool->libs.insert(merged_tool->libs.end(), tool->libs.begin(), tool->libs.end());
	}
	return merged_tool;
}
