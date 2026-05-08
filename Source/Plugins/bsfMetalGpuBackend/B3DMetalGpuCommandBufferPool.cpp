//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuCommandBufferPool.h"
#include "B3DMetalGpuCommandBuffer.h"
#include "B3DMetalGpuDevice.h"
#include <thread>

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
			const u32 id = mNextCommandBufferId++;
			TShared<MetalGpuCommandBuffer> commandBuffer = B3DMakeShared<MetalGpuCommandBuffer>(
				static_cast<MetalGpuDevice&>(mGpuDevice), *this, id, mInformation.Thread, mInformation.Type, createInformation);

			mCommandBuffers[id] = commandBuffer;
			return commandBuffer;
		}

		TShared<GpuCommandBuffer> MetalGpuCommandBufferPool::FindOrCreate(const GpuCommandBufferCreateInformation& createInformation)
		{
			// B12: @c mReadyIds free-list is touched only on the pool's owner thread. Both
			// @c FindOrCreate (render thread) and @c NotifyCommandBufferReady (posted via the pool's
			// @c SingleConsumerQueue from Metal's completion-handler thread and drained on the owner
			// thread) run here, so the free-list push/pop pair never races and no lock is needed. The
			// assert below cements that contract so a future refactor that accidentally calls either
			// API off-thread fails loudly under debug.
			B3D_ASSERT(std::this_thread::get_id() == mInformation.Thread);

			// Pop the most recently-released id off the free-list. The completion handler in
			// CommitInternal pushes ids via NotifyCommandBufferReady once the GPU has finished with them,
			// so popping here is O(1) with no hash-map scan. Validate the state defensively: the buffer
			// should be Done (completed) or Ready (never submitted) — a stale entry is skipped so the
			// caller never receives a buffer that's still Executing.
			while (!mReadyIds.empty())
			{
				const u32 id = mReadyIds.back();
				mReadyIds.pop_back();

				auto existing = mCommandBuffers.find(id);
				if (existing == mCommandBuffers.end())
					continue;

				auto metalCB = std::static_pointer_cast<MetalGpuCommandBuffer>(existing->second);
				const GpuCommandBufferState state = metalCB->GetState();
				if (state != GpuCommandBufferState::Done && state != GpuCommandBufferState::Ready)
					continue;

				metalCB->SetState(GpuCommandBufferState::Ready);
				metalCB->SetName(createInformation.Name);
				return metalCB;
			}

			return Create(createInformation);
		}

		void MetalGpuCommandBufferPool::NotifyCommandBufferReady(u32 id)
		{
			// B12: see the invariant note in @c FindOrCreate. @c SingleConsumerQueue delivery thread
			// equals the pool's owner thread, so the free-list mutation below is race-free without
			// a lock.
			B3D_ASSERT(std::this_thread::get_id() == mInformation.Thread);
			mReadyIds.push_back(id);
		}

		void MetalGpuCommandBufferPool::Destroy()
		{
			if(mIsDestroyed)
				return;

			// Drain every GPU queue before tearing down the pool. MetalGpuCommandBuffer installs a
			// completion handler on each commit that calls back into mPool.GetMessageQueue() — that
			// callback fires on Metal's own completion thread, so if we destroy the pool while any
			// command buffer is still in flight the handler will dereference freed memory. Waiting
			// for all queues to go idle guarantees the handlers have already run.
			mGpuDevice.WaitUntilIdle();

			GetMessageQueue().PostRequestShutdownCommand(true);
			mCommandBuffers.clear();
			mReadyIds.clear();
			Base::Destroy();
		}
	} // namespace render
} // namespace b3d
