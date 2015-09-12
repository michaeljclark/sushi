//
//  vs_read.cc
//

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include <vector>

#include "log.h"
#include "util.h"

/* main */

int main(int argc, char **argv) {
    uuid u;
    util::generate_uuid(u);
    std::cout << util::format_uuid(u) << std::endl;
}
