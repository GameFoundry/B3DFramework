//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPrerequisitesUtil.h"

namespace bs
{
	UINT64 BS_THREADLOCAL MemoryCounter::Allocs = 0;
	UINT64 BS_THREADLOCAL MemoryCounter::Frees = 0;
}