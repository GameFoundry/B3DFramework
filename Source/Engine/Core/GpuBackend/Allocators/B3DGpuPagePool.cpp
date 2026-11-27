//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/Allocators/B3DGpuPagePool.h"

namespace b3d
{
	TConfigVariable<u32> gGpuVirtualMemoryPageHeapSize("gpu.VirtualMemory.PageHeapSize",
		"Size of a heap of a page pool in megabytes, rounded up to a whole number of 64 KiB pages.", 32);

	TConfigVariable<u32> gGpuVirtualMemoryPageHeapIdleFrames("gpu.VirtualMemory.PageHeapIdleFrames",
		"Number of frames a heap of a page pool without allocated pages may go unused before it is destroyed. Heaps a pool keeps as "
		"spares are never destroyed.", 0);
} // namespace b3d
