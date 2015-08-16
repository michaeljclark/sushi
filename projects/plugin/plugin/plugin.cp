/*
 *  plugin.cp
 *  plugin
 *
 *  Created by Michael Clark on 16/8/15.
 *  Copyright (c) 2015 Michael Clark. All rights reserved.
 *
 */

#include "plugin.h"
#include "pluginPriv.h"

CFStringRef pluginUUID(void)
{
	Cplugin* theObj = new Cplugin;
	return theObj->UUID();
}

CFStringRef Cplugin::UUID()
{
	return CFSTR("0001020304050607");
}
