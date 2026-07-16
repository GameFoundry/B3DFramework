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
			// check below cements that contract so a future refactor that accidentally calls either
			// API off-thread fails loudly under debug.
			EnsureValidThread();

			// Pop the most recently-released id off the free-list. The completion handler in
			// CommitInternal pushes ids via NotifyCommandBufferReady once the GPU has finished with them
			// (their cached state was already cleaned via Cleanup on the same message-queue lambda), so
			// popping here is O(1) with no hash-map scan. Validate the state defensively: the buffer
			// should be Done (completed) or Ready (never submitted, or pool-reset) — a stale entry is
			// skipped so the caller never receives a buffer that's still Executing.
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
			EnsureValidThread();
			mReadyIds.push_back(id);
		}

		void MetalGpuCommandBufferPool::Reset()
		{
			EnsureValidThread();

			// Rebuild the free-list from scratch so ids never appear twice: completion-driven recycling
			// (NotifyCommandBufferReady) may already have queued some of these ids, and the caller's
			// message-queue pump (GpuCommandBufferPoolRing::AdvanceFrame posts a blocking no-op before
			// calling Reset) guarantees all pending completion messages were consumed before this runs.
			mReadyIds.clear();

			for (auto& commandBufferPair : mCommandBuffers)
			{
				auto* metalCB = static_cast<MetalGpuCommandBuffer*>(commandBufferPair.second.get());
				metalCB->NotifyParentPoolReset();

				if (metalCB->GetState() == GpuCommandBufferState::Ready)
					mReadyIds.push_back(commandBufferPair.first);
			}
		}

		void MetalGpuCommandBufferPool::Destroy()
		{
			if (mIsDestroyed)
				return;

			EnsureValidThread();

			// Reset the pool before destroying it, so any command buffers in Done state transition to
			// Ready state (mirrors VulkanGpuCommandBufferPool::Destroy).
			if (mInformation.UsePoolReset)
				Reset();

			// Wait only when something can still reach the GPU or its completion handler. Ready buffers
			// were never submitted (or were fully recycled); Done buffers have already executed their
			// completion handler AND had the resulting message consumed (the Done transition happens
			// inside the message-queue lambda on this thread), so neither can call back into the pool
			// after it is gone. Anything else (Recording / RecordingDone / Executing) may still have —
			// or may still install — a completion handler that dereferences this pool.
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

			// The device-level wait routes through the submit thread while it is alive and falls back
			// to the native per-queue wait during teardown windows; either path also fences Metal's
			// completion-handler execution (see MetalGpuQueue::ExecuteWaitUntilIdle), so by the time it
			// returns every pending completion post already sits in this pool's message queue.
			if (requiresIdleWait)
				mGpuDevice.WaitUntilIdle();

			// Drains any pending completion messages (flipping their buffers to Done and running their
			// Cleanup) before shutting the queue down.
			GetMessageQueue().PostRequestShutdownCommand(true);

			// Destroy all command buffers before destroying the pool.
			for (const auto& commandBufferPair : mCommandBuffers)
				static_cast<MetalGpuCommandBuffer*>(commandBufferPair.second.get())->Destroy();

			mCommandBuffers.clear();
			mReadyIds.clear();
			Base::Destroy();
		}
	} // namespace render
} // namespace b3d
