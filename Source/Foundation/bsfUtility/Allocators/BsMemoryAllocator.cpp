//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsUtilityPrerequisites.h"

using namespace bs;

u64 B3D_THREADLOCAL MemoryCounter::Allocs = 0;
u64 B3D_THREADLOCAL MemoryCounter::Frees = 0;
