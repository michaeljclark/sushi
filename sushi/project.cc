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

#include "log.h"
#include "util.h"
#include "project_parser.h"
#include "project.h"


/* project */

project::project() {}

void project::read(std::string project_file)
{
    std::vector<char> buf = util::read_file(project_file);
    if (!parse(buf.data(), buf.size())) {
        log_fatal_exit("config: parse error");
    }
}

void project::symbol(const char *value, size_t length)
{
	log_debug("symbol: %s", std::string(value, length).c_str());
}

void project::start_block()
{
	log_debug("start_block");
}

void project::end_block()
{
	log_debug("end_block");
}

void project::end_statement()
{
	log_debug("end_statement");
}

void project::project_done()
{
	log_debug("project_done");
}

