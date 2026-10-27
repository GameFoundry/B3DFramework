//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuCommandBuffer.h"
#include "B3DNullGpuDevice.h"
#include "B3DNullGpuCommandBufferPool.h"
#include "GpuBackend/B3DGpuBuffer.h"

namespace b3d
{
	namespace render
	{
		NullGpuCommandBuffer::NullGpuCommandBuffer(NullGpuDevice& device, NullGpuCommandBufferPool& pool, u32 id, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation)
			: GpuCommandBuffer(device, pool, queueType, createInformation)
			, mId(id)
		{ }

		void NullGpuCommandBuffer::End()
		{
			// Transition from Ready to Done (null backend executes instantly)
			mState = GpuCommandBufferState::Done;
		}

		void NullGpuCommandBuffer::CopyWindowToBuffer(const TShared<RenderWindow>& source, const TShared<GpuBuffer>& destination)
		{
			GpuCommandBuffer::CopyWindowToBuffer(source, destination);

			if(destination == nullptr)
				return;

			GpuBufferMappedScope mapping = destination->Map(GpuMapOption::Write);
			memset(mapping.GetMappedMemory(), 0, destination->GetTotalSize());
		}
	} // namespace render
} // namespace b3d
