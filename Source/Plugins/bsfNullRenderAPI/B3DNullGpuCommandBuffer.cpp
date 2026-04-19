//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuCommandBuffer.h"
#include "B3DNullGpuDevice.h"
#include "B3DNullGpuCommandBufferPool.h"

namespace b3d
{
	namespace render
	{
		NullGpuCommandBuffer::NullGpuCommandBuffer(NullGpuDevice& device, NullGpuCommandBufferPool& pool, u32 id, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation)
			: GpuCommandBuffer(device, ownerThread, queueType, createInformation)
			, mId(id)
		{ }

		void NullGpuCommandBuffer::End()
		{
			// Transition from Ready to Done (null backend executes instantly)
			mState = GpuCommandBufferState::Done;
		}
	} // namespace render
} // namespace b3d
