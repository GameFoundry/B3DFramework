//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuCommandBuffer.h"
#include "B3DNullGpuDevice.h"
#include "B3DNullGpuCommandBufferPool.h"
#include "B3DNullResource.h"
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

		void NullGpuCommandBuffer::CopyBufferToBuffer(IGpuBufferResource& source, IGpuBufferResource& destination, TArrayView<const GpuBufferCopyRange> ranges)
		{
			// Virtual buffers have no memory to copy
			const u8* const sourceMemory = static_cast<NullBufferResource&>(source).GetMemory();
			u8* const destinationMemory = static_cast<NullBufferResource&>(destination).GetMemory();
			if(sourceMemory == nullptr || destinationMemory == nullptr)
				return;

			for(const GpuBufferCopyRange& range : ranges)
				memcpy(destinationMemory + range.DestinationOffset, sourceMemory + range.SourceOffset, range.Size);
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
