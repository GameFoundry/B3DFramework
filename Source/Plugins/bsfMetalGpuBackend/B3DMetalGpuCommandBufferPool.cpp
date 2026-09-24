//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuCommandBufferPool.h"
#include "B3DMetalGpuCommandBuffer.h"
#include "B3DMetalGpuDevice.h"

namespace b3d
{
	namespace render
	{
		MetalGpuCommandBufferPool::MetalGpuCommandBufferPool(MetalGpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation)
			: Base(device, createInformation)
		{ }

		MetalGpuCommandBufferPool::~MetalGpuCommandBufferPool()
		{
			MetalGpuCommandBufferPool::Destroy();
		}

		TShared<GpuCommandBuffer> MetalGpuCommandBufferPool::Create(const GpuCommandBufferCreateInformation& createInformation)
		{
			const u32 commandBufferId = mNextCommandBufferId++;
			TShared<MetalGpuCommandBuffer> commandBuffer = B3DMakeShared<MetalGpuCommandBuffer>(
				static_cast<MetalGpuDevice&>(mGpuDevice), *this, commandBufferId, mInformation.Thread, mInformation.Type, createInformation);

			mCommandBuffers[commandBufferId] = commandBuffer;

			// The completion handlers installed by CommitInternal keep the buffer alive via GetShared()
			commandBuffer->SetShared(commandBuffer);
			return commandBuffer;
		}

		TShared<GpuCommandBuffer> MetalGpuCommandBufferPool::FindOrCreate(const GpuCommandBufferCreateInformation& createInformation)
		{
			EnsureValidThread();

			while (!mReadyIds.empty())
			{
				const u32 commandBufferId = mReadyIds.back();
				mReadyIds.pop_back();

				auto existing = mCommandBuffers.find(commandBufferId);
				if (existing == mCommandBuffers.end())
					continue;

				auto metalCommandBuffer = std::static_pointer_cast<MetalGpuCommandBuffer>(existing->second);
				const GpuCommandBufferState state = metalCommandBuffer->GetState();
				if (state != GpuCommandBufferState::Done && state != GpuCommandBufferState::Ready)
					continue;

				metalCommandBuffer->SetState(GpuCommandBufferState::Ready);
				metalCommandBuffer->SetName(createInformation.Name);

				return metalCommandBuffer;
			}

			return Create(createInformation);
		}

		void MetalGpuCommandBufferPool::NotifyCommandBufferReady(u32 commandBufferId)
		{
			EnsureValidThread();

			mReadyIds.push_back(commandBufferId);
		}

		void MetalGpuCommandBufferPool::Reset()
		{
			EnsureValidThread();

			// Rebuild the free list from scratch, since NotifyCommandBufferReady may already have queued some of
			// these ids and none may appear twice
			mReadyIds.clear();

			for (auto& commandBufferPair : mCommandBuffers)
			{
				auto* metalCommandBuffer = static_cast<MetalGpuCommandBuffer*>(commandBufferPair.second.get());
				metalCommandBuffer->NotifyParentPoolReset();

				if (metalCommandBuffer->GetState() == GpuCommandBufferState::Ready)
					mReadyIds.push_back(commandBufferPair.first);
			}
		}

		void MetalGpuCommandBufferPool::Destroy()
		{
			if (mIsDestroyed)
				return;

			EnsureValidThread();

			// Reset first so command buffers in the Done state transition to Ready
			if (mInformation.UsePoolReset)
				Reset();

			// Ready and Done command buffers can no longer call back into the pool. Any other state may still run
			// a completion handler that references it, so the GPU must be waited on first
			bool requiresIdleWait = false;
			for (const auto& commandBufferPair : mCommandBuffers)
			{
				const GpuCommandBufferState state = commandBufferPair.second->GetState();
				if (state != GpuCommandBufferState::Ready && state != GpuCommandBufferState::Done)
				{
					requiresIdleWait = true;
					break;
				}
			}

			// Also waits for Metal's completion handlers, so every completion message is queued once this returns
			if (requiresIdleWait)
				mGpuDevice.WaitUntilIdle();

			// Process the pending completion messages (moving their command buffers to Done) before shutting the queue down
			GetMessageQueue().PostRequestShutdownCommand(true);

			// Destroy all command buffers before destroying the pool
			for (const auto& commandBufferPair : mCommandBuffers)
				static_cast<MetalGpuCommandBuffer*>(commandBufferPair.second.get())->Destroy();

			mCommandBuffers.clear();
			mReadyIds.clear();
			Base::Destroy();
		}
	} // namespace render
} // namespace b3d
