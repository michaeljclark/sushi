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
#include <functional>

#include "sushi.h"

#include "util.h"
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

void project::statement_set(project *project, statement &line)
{
	auto config = std::static_pointer_cast<project_config>(project->item_stack.back());
	config->vars[line[1]] = line[2];
}

void project::statement_depends(project *project, statement &line)
{
	auto target = std::static_pointer_cast<project_target>(project->item_stack.back());
	for (size_t i = 1; i < line.size(); i++) {
		target->depends.push_back(line[i]);
	}
}

void project::statement_defines(project *project, statement &line)
{
	auto config = std::static_pointer_cast<project_config>(project->item_stack.back());
	for (size_t i = 1; i < line.size(); i++) {
		config->defines.push_back(line[i]);
	}
}

void project::statement_includes(project *project, statement &line)
{
	auto target = std::static_pointer_cast<project_target>(project->item_stack.back());
	for (size_t i = 1; i < line.size(); i++) {
		target->includes.push_back(line[i]);
	}
}

void project::statement_export_defines(project *project, statement &line)
{
	auto target = std::static_pointer_cast<project_target>(project->item_stack.back());
	for (size_t i = 1; i < line.size(); i++) {
		target->export_defines.push_back(line[i]);
	}
}

void project::statement_export_includes(project *project, statement &line)
{
	auto target = std::static_pointer_cast<project_target>(project->item_stack.back());
	for (size_t i = 1; i < line.size(); i++) {
		target->export_includes.push_back(line[i]);
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
	auto target = std::static_pointer_cast<project_target>(project->item_stack.back());
	for (size_t i = 1; i < line.size(); i++) {
		target->libs.push_back(line[i]);
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
		statement_fn_map["set"] = statement_record(3,  3, "lib|tool|config", &statement_set);
		statement_fn_map["defines"] = statement_record(2,  -1, "lib|tool|config", &statement_defines);
		statement_fn_map["depends"] = statement_record(2,  -1, "lib|tool", &statement_depends);
		statement_fn_map["includes"] = statement_record(2,  -1, "lib|tool", &statement_includes);
		statement_fn_map["export_defines"] = statement_record(2,  2, "lib", &statement_export_defines);
		statement_fn_map["export_includes"] = statement_record(2,  -1, "lib", &statement_export_includes);
		statement_fn_map["source"] = statement_record(2,  -1, "lib|tool", &statement_source);
		statement_fn_map["libs"] = statement_record(2,  -1, "lib|tool", &statement_libs);
		function_map_init = true;
	};
}

project::project() { init(); }

void project::read(std::string project_file)
{
	std::vector<char> buf = util::read_file(project_file);
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

static void add_unique(std::vector<std::string> &vec, std::vector<std::string> &add)
{
	for (std::string str : add) if (std::find(vec.begin(), vec.end(), str) == vec.end()) vec.push_back(str);
}

static void merge_config(project_config_ptr merged_config, project_config_ptr config)
{
	for (auto ent : config->vars) merged_config->vars[ent.first] = ent.second;
	add_unique(merged_config->defines, config->defines);
}

static void merge_target(project_target_ptr merged_target, project_target_ptr target)
{
	merge_config(merged_target, target);
	add_unique(merged_target->depends, target->depends);
	add_unique(merged_target->includes, target->includes);
	add_unique(merged_target->export_defines, target->export_defines);
	add_unique(merged_target->export_includes, target->export_includes);
	add_unique(merged_target->source, target->source);
	add_unique(merged_target->libs, target->libs);
}

project_config_ptr project_root::get_config(std::string name, bool inherit)
{
	project_config_ptr merged_config = std::make_shared<project_config>();
	merged_config->config_name = name;
	if (inherit) {
		for (auto config : config_list) {
			if (config->config_name != "*") continue;
			merge_config(merged_config, config);
		}
	}
	for (auto config : config_list) {
		if (config->config_name != name) continue;
		merge_config(merged_config, config);
	}
	return merged_config;
}

project_lib_ptr project_root::get_lib(std::string name, bool inherit)
{
	project_lib_ptr merged_lib = std::make_shared<project_lib>();
	merged_lib->lib_name = name;
	if (inherit) {
		for (auto lib : lib_list) {
			if (lib->lib_name != "*") continue;
			if (lib->lib_type.size() > 0) merged_lib->lib_type = lib->lib_type;
			merge_target(merged_lib, lib);
		}
	}
	for (auto lib : lib_list) {
		if (lib->lib_name != name) continue;
		if (lib->lib_type.size() > 0) merged_lib->lib_type = lib->lib_type;
		merge_target(merged_lib, lib);
	}
	return merged_lib;
}

project_tool_ptr project_root::get_tool(std::string name, bool inherit)
{
	project_tool_ptr merged_tool = std::make_shared<project_tool>();
	merged_tool->tool_name = name;
	if (inherit) {
		for (auto tool : tool_list) {
			if (tool->tool_name != "*") continue;
			merge_target(merged_tool, tool);
		}
	}
	for (auto tool : tool_list) {
		if (tool->tool_name != name) continue;
		merge_target(merged_tool, tool);
	}
	return merged_tool;
}

void project_root::resolve_target_libs(std::vector<std::string> &stack,
		std::vector<std::string> &libs, project_target_ptr target)
{
	for (std::string lib : target->libs) {
		if (std::find(stack.begin(), stack.end(), lib) != stack.end()) {
			log_fatal_exit("resolve_target_libs: circular dependency");
		}
		if (std::find(libs.begin(), libs.end(), lib) == libs.end()) {
			libs.push_back(lib);
		}
		stack.push_back(lib);
		resolve_target_libs(stack, libs, get_lib(lib));
		stack.pop_back();
	}
}

std::vector<std::string> project_root::get_libs(project_target_ptr target)
{
	std::vector<std::string> stack, libs;
	resolve_target_libs(stack, libs, target);
	return libs;
}
