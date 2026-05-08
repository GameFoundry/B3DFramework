//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuCommandBufferPool.h"
#include "B3DNullGpuCommandBuffer.h"
#include "B3DNullGpuDevice.h"

namespace b3d
{
	namespace render
	{
		NullGpuCommandBufferPool::NullGpuCommandBufferPool(NullGpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation)
			: Base(device, createInformation)
		{ }

		NullGpuCommandBufferPool::~NullGpuCommandBufferPool()
		{
			NullGpuCommandBufferPool::Destroy();
		}

		TShared<GpuCommandBuffer> NullGpuCommandBufferPool::Create(const GpuCommandBufferCreateInformation& createInformation)
		{
			const u32 id = mNextCommandBufferId++;
			TShared<NullGpuCommandBuffer> commandBuffer = B3DMakeShared<NullGpuCommandBuffer>(
				static_cast<NullGpuDevice&>(mGpuDevice), *this, id, mInformation.Thread, mInformation.Type, createInformation);

			mCommandBuffers[id] = commandBuffer;
			return commandBuffer;
		}

		TShared<GpuCommandBuffer> NullGpuCommandBufferPool::FindOrCreate(const GpuCommandBufferCreateInformation& createInformation)
		{
			// For simplicity, always create a new one
			return Create(createInformation);
		}

		void NullGpuCommandBufferPool::Destroy()
		{
			if(mIsDestroyed)
				return;

			GetMessageQueue().PostRequestShutdownCommand(true);
			mCommandBuffers.clear();
			Base::Destroy();
		}
	} // namespace render
} // namespace b3d
