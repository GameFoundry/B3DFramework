//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/Allocators/B3DGpuTransientHeapPool.h"

namespace b3d
{
	TConfigVariable<u32> gGpuTransientHeapMinimumSize("gpu.TransientHeapMinimumSize",
		"Size of a new transient heap in megabytes, unless the resource that requires the heap is larger.", 128);

	TConfigVariable<u32> gGpuTransientIdleFrames("gpu.TransientIdleFrames",
		"Number of frames a transient heap without references, or a cached transient resource, may go unused before it is released.", 30);
} // namespace b3d
