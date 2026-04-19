//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuQueue.h"
#include "B3DNullGpuDevice.h"

namespace b3d
{
	namespace render
	{
		NullGpuQueue::NullGpuQueue(GpuDevice& device, GpuQueueType type, u32 index)
			: GpuQueue(device, type, index)
		{ }
	} // namespace render
} // namespace b3d
