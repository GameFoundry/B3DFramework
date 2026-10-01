//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuQueue.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "Debug/B3DLog.h"
#include "Profiling/B3DRenderStats.h"
#include "Threading/B3DThreading.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			/** Logs the error of @p commandBuffer, if it failed. */
			void LogCommandBufferError(id<MTLCommandBuffer> commandBuffer)
			{
				if ([commandBuffer status] != MTLCommandBufferStatusError)
					return;

				NSError* error = [commandBuffer error];
				B3D_LOG(Fatal, LogRenderBackend, "Metal queue synchronization command buffer failed ({0}, code {1}): {2}",
					error ? String([[error domain] UTF8String]) : String("<unknown domain>"),
					error ? (i64)[error code] : 0,
					error ? String([[error localizedDescription] UTF8String]) : String("No error details were provided."));
			}
		} // namespace

		struct MetalGpuQueue::Impl
		{
			/** Submission on this queue that has not been retired yet. */
			struct SubmissionRecord
			{
				u64 EventValue = 0; /**< Signaled event value, or the last committed value for a failed submission. */
				id<MTLCommandBuffer> CommandBuffer = nil; /**< Committed command buffer, or nil for a failed submission. */
				TShared<WaitGroup> OwnerCompletion; /**< Signaled after the owner-side cleanup runs. */
			};

			id<MTLCommandQueue> CommandQueue = nil;
			id<MTLSharedEvent> SharedEvent = nil;

			/** Event value reserved by the most recent submission. The shared event's signaled value tracks completion. */
			std::atomic<u64> LastReservedEventValue { 0 };

			/** Highest event value whose command buffer has been committed. */
			std::atomic<u64> LastCommittedEventValue { 0 };

			/** Submissions not yet retired, in ascending event value order. Guarded by SubmissionMutex. */
			Vector<SubmissionRecord> ActiveSubmissions;
			Mutex SubmissionMutex;
		};

		MetalGpuQueue::MetalGpuQueue(GpuDevice& device, GpuQueueType type, u32 index, id<MTLCommandQueue> commandQueue, id<MTLSharedEvent> sharedEvent)
			: GpuQueue(device, type, index), mImpl(B3DMakeUnique<Impl>())
		{
			mImpl->CommandQueue = commandQueue;
			mImpl->SharedEvent = sharedEvent;
		}

		MetalGpuQueue::~MetalGpuQueue() = default;

		id<MTLCommandQueue> MetalGpuQueue::GetMetalQueue() const
		{
			return mImpl->CommandQueue;
		}

		id<MTLSharedEvent> MetalGpuQueue::GetSharedEvent() const
		{
			return mImpl->SharedEvent;
		}

		u64 MetalGpuQueue::GetLastCommittedEventValue() const
		{
			return mImpl->LastCommittedEventValue.load(std::memory_order_acquire);
		}

		u64 MetalGpuQueue::ReserveNextEventValue()
		{
			return mImpl->LastReservedEventValue.fetch_add(1, std::memory_order_acq_rel) + 1;
		}

		void MetalGpuQueue::NotifySubmissionCommitted(u64 eventValue, id<MTLCommandBuffer> commandBuffer, const TShared<WaitGroup>& ownerCompletion)
		{
			u64 previousEventValue = mImpl->LastCommittedEventValue.load(std::memory_order_relaxed);
			while (eventValue > previousEventValue)
			{
				if (mImpl->LastCommittedEventValue.compare_exchange_weak(previousEventValue, eventValue, std::memory_order_release, std::memory_order_relaxed))
				{
					break;
				}
			}

			Lock lock(mImpl->SubmissionMutex);
			mImpl->ActiveSubmissions.push_back({ eventValue, commandBuffer, ownerCompletion });
		}

		void MetalGpuQueue::NotifySubmissionFailed(const TShared<WaitGroup>& ownerCompletion)
		{
			Lock lock(mImpl->SubmissionMutex);
			mImpl->ActiveSubmissions.push_back({ GetLastCommittedEventValue(), nil, ownerCompletion });
		}

		void MetalGpuQueue::SubmitCommandBuffer(const GpuSubmissionInformation& information)
		{
			if (!B3D_ENSURE(information.CommandBuffer))
				return;

			auto metalCommandBuffer = std::static_pointer_cast<MetalGpuCommandBuffer>(information.CommandBuffer);
			if (!B3D_ENSURE(metalCommandBuffer->GetQueueType() == mType))
				return;

			if (metalCommandBuffer->GetState() == GpuCommandBufferState::Executing)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot submit a command buffer that's still executing.");
				return;
			}

			if (!B3D_ENSURE(!metalCommandBuffer->IsInRenderPass()))
				metalCommandBuffer->EndRenderPass();

			if (metalCommandBuffer->IsRecording())
				metalCommandBuffer->End();

			// Set before the hand-off, so the owner thread never observes a submitted buffer as recording
			metalCommandBuffer->SetState(GpuCommandBufferState::Executing);

			mGpuDevice.GetSubmitThread().QueueSubmit(information.CommandBuffer, *this, information.SyncMask, information.SignalFences);
		}

		void MetalGpuQueue::WaitUntilIdle()
		{
			// The submit thread doesn't exist during device initialization and teardown
			auto& metalDevice = static_cast<MetalGpuDevice&>(mGpuDevice);
			if (!metalDevice.HasSubmitThread())
			{
				ExecuteWaitUntilIdle();
				return;
			}

			metalDevice.GetSubmitThread().WaitUntilIdle(*this);
		}

		void MetalGpuQueue::ExecuteWaitUntilIdle()
		{
			if (mImpl->CommandQueue == nil)
				return;

			// A failed command buffer may never signal its shared event, so wait on a trailing command buffer instead
			FenceCompletionHandlers();
		}

		void MetalGpuQueue::RefreshCompletionState(bool forceWait, u64 lastEventValue)
		{
			AssertIfNotSubmitThread();

			// Records are in ascending order, so the last command buffer up to lastEventValue covers all earlier ones
			id<MTLCommandBuffer> waitCommandBuffer = nil;
			TInlineArray<TShared<WaitGroup>, 16> ownerCompletions;
			{
				Lock lock(mImpl->SubmissionMutex);
				for (const Impl::SubmissionRecord& record : mImpl->ActiveSubmissions)
				{
					if (record.EventValue > lastEventValue)
						break;

					if (record.CommandBuffer != nil)
						waitCommandBuffer = record.CommandBuffer;

					if (record.OwnerCompletion != nullptr)
						ownerCompletions.Add(record.OwnerCompletion);
				}
			}

			// A terminal command buffer status is observable before its completion handler returns, so only forced waits
			// retire submissions
			if (!forceWait)
				return;

			// Also waits for the completion handler, which posts the completion to the owner
			if (waitCommandBuffer != nil)
				[waitCommandBuffer waitUntilCompleted];

			for (const TShared<WaitGroup>& ownerCompletion : ownerCompletions)
				ownerCompletion->Wait();

			Lock lock(mImpl->SubmissionMutex);
			size_t retiredCount = 0;
			while (retiredCount < mImpl->ActiveSubmissions.size() && mImpl->ActiveSubmissions[retiredCount].EventValue <= lastEventValue)
				retiredCount++;

			mImpl->ActiveSubmissions.erase(mImpl->ActiveSubmissions.begin(), mImpl->ActiveSubmissions.begin() + retiredCount);
		}

		void MetalGpuQueue::FenceCompletionHandlers()
		{
			// Drained locally since the calling thread may have no run loop
			@autoreleasepool
			{
				id<MTLCommandBuffer> fenceCommandBuffer = [mImpl->CommandQueue commandBuffer];
				if (fenceCommandBuffer == nil)
					return;

				[fenceCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedCommandBuffer)
				{
					LogCommandBufferError(completedCommandBuffer);
				}];
				[fenceCommandBuffer commit];
				[fenceCommandBuffer waitUntilCompleted];
			}
		}

		void MetalGpuQueue::PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask)
		{
			if (renderWindow == nullptr)
				return;

			IRenderWindowSurface* surface = renderWindow->GetRenderWindowSurface().get();
			if (surface == nullptr)
				return;

			renderWindow->NotifySwapBuffersRequested();
			surface->SwapBuffers(*this, syncMask);

			B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
		}
	} // namespace render
} // namespace b3d
