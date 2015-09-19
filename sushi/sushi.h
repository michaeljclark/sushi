//
//  sushi.h
//

#ifndef sushi_h
#define sushi_h

#ifdef _WIN32
#   ifdef SUSHI_EXPORT
#       define SUSHI_LIB __declspec(dllexport)
#   elif defined(SUSHI_IMPORT)
#       define SUSHI_LIB __declspec(dllimport)
#   else
#       define SUSHI_LIB
#   endif
#else
#   define SUSHI_LIB
#endif

#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdint>
#include <ctime>
#include <cerrno>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <random>
#include <functional>

#include "util.h"
#include "visual_studio_parser.h"
#include "visual_studio.h"
#include "xcode.h"
#include "project_parser.h"
#include "project.h"
#include "project_visual_studio.h"
#include "project_xcode.h"

#endif
