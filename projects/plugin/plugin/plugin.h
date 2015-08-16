/*
 *  plugin.h
 *  plugin
 *
 *  Created by Michael Clark on 16/8/15.
 *  Copyright (c) 2015 Michael Clark. All rights reserved.
 *
 */

extern "C" {
#include <CoreFoundation/CoreFoundation.h>

#pragma GCC visibility push(default)

/* External interface to the plugin, C-based */

CFStringRef pluginUUID(void);

#pragma GCC visibility pop
}
