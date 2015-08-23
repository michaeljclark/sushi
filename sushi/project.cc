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

void project::symbol(const char *value, size_t vlen)
{
}

void project::start_block()
{
}

void project::end_block()
{
}

void project::end_statement()
{
}

void project::config_done()
{
}

