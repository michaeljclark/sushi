/*
 *  pluginPriv.h
 *  plugin
 *
 *  Created by Michael Clark on 16/8/15.
 *  Copyright (c) 2015 Michael Clark. All rights reserved.
 *
 */

#include <CoreFoundation/CoreFoundation.h>

#pragma GCC visibility push(hidden)

class Cplugin {
	public:
		CFStringRef UUID(void);
};

#pragma GCC visibility pop
