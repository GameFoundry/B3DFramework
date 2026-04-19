//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityPrerequisites.h"

using namespace b3d;

u64 B3D_THREADLOCAL MemoryCounter::Allocs = 0;
u64 B3D_THREADLOCAL MemoryCounter::Frees = 0;
