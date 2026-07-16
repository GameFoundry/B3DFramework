//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuQueue.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "Debug/B3DLog.h"
#include "Profiling/B3DRenderStats.h"
#include "Utility/Threading/B3DThreading.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			void LogQueueCommandBufferError(id<MTLCommandBuffer> commandBuffer)
			{
				if ([commandBuffer status] != MTLCommandBufferStatusError)
					return;

				NSError* error = [commandBuffer error];
				B3D_LOG(Fatal, LogRenderBackend, "Metal queue synchronization command buffer failed ({0}, code {1}): {2}",
					error ? String([[error domain] UTF8String]) : String("<unknown domain>"),
					error ? (i64)[error code] : 0,
					error ? String([[error localizedDescription] UTF8String]) : String("No error details were provided."));
			}
		}

		struct MetalGpuQueue::Impl
		{
			id<MTLCommandQueue> CommandQueue = nil;
			id<MTLSharedEvent> SharedEvent = nil;

			/**
			 * Shared event listener used by the queue's blocking waits (and by
			 * @c MetalGpuQueryPool::TryResolve through the same queue). Retaining one listener for the
			 * queue's lifetime avoids allocating a listener for each wait.
			 *
			 * Initialized with a dedicated concurrent dispatch queue (@c ListenerDispatchQueue below).
			 * The default @c [MTLSharedEventListener init] posts blocks on the main dispatch queue,
			 * which deadlocks @c MetalGpuQueryPool::TryResolve(wait=true) when it is invoked on the
			 * main thread (main blocks on a semaphore that would only be signaled by a listener block
			 * that never runs because main is blocked). The dedicated queue sidesteps this.
			 */
			MTLSharedEventListener* EventListener = nil;

			/**
			 * Dispatch queue used to deliver @c MTLSharedEventListener notifications. Created once per
			 * @c MetalGpuQueue. Concurrent because listener blocks are independent and semantically
			 * order-free; Apple's guidance permits a concurrent queue here.
			 */
			dispatch_queue_t ListenerDispatchQueue = nullptr;

			/**
			 * Monotonically increasing counter used as the event value for the next submission on this
			 * queue. Under Metal, @c MTLSharedEvent.signaledValue only moves forward when the GPU side of
			 * the command buffer actually runs, so @c LastReservedValue tracks the *scheduled* value while
			 * @c [SharedEvent signaledValue] tracks completion. Submissions are serialized on the device's
			 * submit thread (command buffers) or on the present path, so mutation is effectively ordered;
			 * the atomic keeps cross-thread reads well-defined.
			 */
			std::atomic<u64> LastReservedValue { 0 };

			/**
			 * Value last committed to the GPU via @c [cmdBuffer commit] on this queue. Bumped inside
			 * @c MetalGpuCommandBuffer::CommitInternal *after* the commit returns. Used by cross-queue
			 * waits: a consumer queue waits on the producer's committed value so it never waits on a
			 * value that has been reserved but not yet encoded/committed on the producer side (the
			 * previous "last reserved" semantics could deadlock in that race window).
			 */
			std::atomic<u64> LastCommittedValue { 0 };

			/** One engine-visible submission on this queue that may not have retired yet. */
			struct SubmissionRecord
			{
				u32 SubmitIndex = 0; /**< Engine submit index (monotonic, starts at 1). */
				u64 EventValue = 0;  /**< Shared-event value, or zero for a pre-commit failure. */
				id<MTLCommandBuffer> CommandBuffer = nil; /**< Native buffer, or nil for a pre-commit failure. */
				TShared<WaitGroup> OwnerCompletion; /**< Signaled after the owner-side cleanup callback runs. */
			};

			/**
			 * Engine submissions that have not yet been observed complete, in ascending
			 * SubmitIndex / EventValue order. Appended by NotifySubmissionCommitted, pruned by
			 * RefreshCompletionState. Guarded by @c SubmissionMutex so diagnostic reads remain safe
			 * while completion state is refreshed.
			 */
			Vector<SubmissionRecord> ActiveSubmissions;

			/** Engine submit index handed to the next committed submission. Guarded by @c SubmissionMutex. */
			u32 NextSubmitIndex = 1;

			/** Guards @c ActiveSubmissions / @c NextSubmitIndex. */
			Mutex SubmissionMutex;
		};

		MetalGpuQueue::MetalGpuQueue(GpuDevice& device, GpuQueueType type, u32 index, id<MTLCommandQueue> commandQueue, id<MTLSharedEvent> sharedEvent)
			: GpuQueue(device, type, index)
			, mImpl(B3DMakeUnique<Impl>())
		{
			mImpl->CommandQueue = commandQueue;
			mImpl->SharedEvent = sharedEvent;

			// Allocate a dedicated dispatch queue for the MTLSharedEventListener. Passing nil (the
			// default initializer) would route listener blocks through the main dispatch queue, which
			// deadlocks any main-thread caller of TryResolve(wait=true) — the semaphore it blocks on
			// would only be signaled by a listener block that can never run while main is blocked.
			// A concurrent queue is appropriate: listener callbacks are independent and must not
			// serialize on each other.
			mImpl->ListenerDispatchQueue = dispatch_queue_create("b3d.metal.eventlistener", DISPATCH_QUEUE_CONCURRENT);
			if (mImpl->ListenerDispatchQueue != nullptr)
				mImpl->EventListener = [[MTLSharedEventListener alloc] initWithDispatchQueue:mImpl->ListenerDispatchQueue];
		}

		MetalGpuQueue::~MetalGpuQueue()
		{
			if (mImpl)
			{
				mImpl->EventListener = nil;
				mImpl->SharedEvent = nil;
				mImpl->CommandQueue = nil;

				// Under MRC, dispatch_queue_t is not ARC-managed and leaks if not released explicitly.
				// Under ARC it is toll-free bridged as an Obj-C object and released automatically;
				// calling @c dispatch_release under ARC is prohibited. Feature-guard to match the
				// pattern used by the query and timeline-fence wait paths for dispatch objects.
#if !__has_feature(objc_arc)
				if (mImpl->ListenerDispatchQueue != nullptr)
					dispatch_release(mImpl->ListenerDispatchQueue);
#endif
				mImpl->ListenerDispatchQueue = nullptr;
			}
		}

		id<MTLCommandQueue> MetalGpuQueue::GetMetalQueue() const
		{
			return mImpl->CommandQueue;
		}

		id<MTLSharedEvent> MetalGpuQueue::GetSharedEvent() const
		{
			return mImpl->SharedEvent;
		}

		MTLSharedEventListener* MetalGpuQueue::GetSharedEventListener() const
		{
			return mImpl->EventListener;
		}

		u64 MetalGpuQueue::GetLastReservedEventValue() const
		{
			return mImpl->LastReservedValue.load(std::memory_order_acquire);
		}

		u64 MetalGpuQueue::GetLastCommittedEventValue() const
		{
			return mImpl->LastCommittedValue.load(std::memory_order_acquire);
		}

		u64 MetalGpuQueue::GetLastSignaledEventValue() const
		{
			if (mImpl->SharedEvent == nil)
				return 0;
			return [mImpl->SharedEvent signaledValue];
		}

		u64 MetalGpuQueue::ReserveNextEventValue()
		{
			return mImpl->LastReservedValue.fetch_add(1, std::memory_order_acq_rel) + 1;
		}

		void MetalGpuQueue::NotifySubmissionCommitted(u64 value, id<MTLCommandBuffer> commandBuffer,
			const TShared<WaitGroup>& ownerCompletion)
		{
			// Monotonic max-CAS loop. Two threads may commit submissions on this queue in an order that
			// differs from reservation order (thread A reserves N, thread B reserves N+1, then B's
			// [commit] returns before A's). We want @c LastCommittedValue to stay the high-water mark so
			// cross-queue waits don't observe it regressing.
			u64 previous = mImpl->LastCommittedValue.load(std::memory_order_relaxed);
			while (value > previous)
			{
				if (mImpl->LastCommittedValue.compare_exchange_weak(previous, value,
					std::memory_order_release, std::memory_order_relaxed))
				{
					break;
				}
			}

			// Record the submission for frame pacing: GpuSubmitThread snapshots GetLastSubmitIndex()
			// at frame boundaries and later waits on it through RefreshCompletionState().
			Lock lock(mImpl->SubmissionMutex);
			mImpl->ActiveSubmissions.push_back({ mImpl->NextSubmitIndex++, value, commandBuffer, ownerCompletion });
		}

		void MetalGpuQueue::NotifySubmissionFailed(const TShared<WaitGroup>& ownerCompletion)
		{
			Lock lock(mImpl->SubmissionMutex);
			mImpl->ActiveSubmissions.push_back({ mImpl->NextSubmitIndex++, 0, nil, ownerCompletion });
		}

		void MetalGpuQueue::SubmitCommandBuffer(const GpuSubmissionInformation& information)
		{
			if (!B3D_ENSURE(information.CommandBuffer))
				return;

			auto metalCB = std::static_pointer_cast<MetalGpuCommandBuffer>(information.CommandBuffer);
			if (!B3D_ENSURE(metalCB->GetQueueType() == mType))
				return;

			if (metalCB->GetState() == GpuCommandBufferState::Executing)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot submit a command buffer that's still executing.");
				return;
			}

			if (!B3D_ENSURE(!metalCB->IsInRenderPass()))
				metalCB->EndRenderPass();

			if (metalCB->IsRecording())
				metalCB->End();

			// Mark the buffer Executing on the owner thread before handing it off, so external
			// observers of GetState() never see a submitted buffer in a recording state (mirrors
			// VulkanGpuQueue::SubmitCommandBuffer's SetIsSubmitted).
			metalCB->SetState(GpuCommandBufferState::Executing);

			// Route through the submit thread. GpuSubmitThread::QueueSubmit invokes
			// MetalGpuDevice::NotifyWillQueueForSubmit on this thread (releasing recording state that
			// must not be touched off-thread), folds the buffer's accumulated queue sync mask into
			// @p information.SyncMask, and calls MetalGpuDevice::ExecuteSubmit ->
			// MetalGpuCommandBuffer::CommitInternal on the submit thread.
			mGpuDevice.GetSubmitThread().QueueSubmit(information.CommandBuffer, *this, information.SyncMask, information.SignalFences);
		}

		void MetalGpuQueue::WaitUntilIdle()
		{
			// Route through the submit thread so the wait is ordered after every queued submit and the
			// backend's completion refresh runs (GpuSubmitThread::WaitUntilIdle(queue) calls
			// ExecuteWaitUntilIdle + RefreshCompletionState on the submit thread). The submit thread
			// lives from the end of MetalGpuDevice::Initialize to the start of its destruction; in the
			// remaining windows the native wait suffices. Mirrors VulkanGpuDevice::WaitUntilIdle's
			// null-check pattern.
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
			// No AssertIfNotSubmitThread here: this is also the fallback for teardown windows where the
			// submit thread no longer exists (see WaitUntilIdle), matching how Vulkan's native
			// ExecuteWaitUntilIdle is assert-free.
			if (mImpl->CommandQueue == nil)
				return;

			// A failed command buffer may never execute its final shared-event signal. A trailing native
			// buffer reaches either Completed or Error without relying on that signal and also covers
			// presentation buffers.
			FenceCompletionHandlers();
		}

		void MetalGpuQueue::RefreshCompletionState(bool forceWait, bool queueEmpty, u32 lastSubmitIndex)
		{
			AssertIfNotSubmitThread();

			// Resolve the native command buffer covering every submission up to @p lastSubmitIndex.
			// Records are appended in ascending order, so the last qualifying record is the boundary.
			id<MTLCommandBuffer> waitCommandBuffer = nil;
			TInlineArray<TShared<WaitGroup>, 16> ownerCompletions;
			{
				Lock lock(mImpl->SubmissionMutex);
				for (const Impl::SubmissionRecord& record : mImpl->ActiveSubmissions)
				{
					if (lastSubmitIndex != ~0u && record.SubmitIndex > lastSubmitIndex)
						break;

					if (record.CommandBuffer != nil)
						waitCommandBuffer = record.CommandBuffer;
					if (record.OwnerCompletion != nullptr)
						ownerCompletions.Add(record.OwnerCompletion);
				}
			}

			if (forceWait && waitCommandBuffer != nil)
			{
				// This waits for the native completion handler as well as the GPU work. The handler posts
				// the Done transition before returning, so the owner can safely pump and reset its pool.
				[waitCommandBuffer waitUntilCompleted];
			}

			if (forceWait)
			{
				for (const TShared<WaitGroup>& ownerCompletion : ownerCompletions)
					ownerCompletion->Wait();
			}

			Lock lock(mImpl->SubmissionMutex);
			if (queueEmpty)
			{
				mImpl->ActiveSubmissions.clear();
				return;
			}

			// Do not prune from a poll-only refresh: a terminal MTLCommandBuffer status is observable
			// before its completion handler returns. Forced frame/idle refreshes are the retirement point.
			if (forceWait)
			{
				size_t retiredCount = 0;
				while (retiredCount < mImpl->ActiveSubmissions.size()
					&& (lastSubmitIndex == ~0u || mImpl->ActiveSubmissions[retiredCount].SubmitIndex <= lastSubmitIndex))
				{
					retiredCount++;
				}

				mImpl->ActiveSubmissions.erase(mImpl->ActiveSubmissions.begin(), mImpl->ActiveSubmissions.begin() + retiredCount);
			}
		}

		u32 MetalGpuQueue::GetLastSubmitIndex() const
		{
			Lock lock(mImpl->SubmissionMutex);
			return mImpl->NextSubmitIndex - 1;
		}

		void MetalGpuQueue::FenceCompletionHandlers()
		{
			// Completion handlers on an MTLCommandQueue execute in submission order, so waiting for an
			// empty trailing buffer guarantees every earlier submission's addCompletedHandler block —
			// and therefore its message-queue post — has already run. The commit path allocates
			// autoreleased transients; drain them locally (fiber threads may never hit a runloop).
			@autoreleasepool
			{
				id<MTLCommandBuffer> flushBuffer = [mImpl->CommandQueue commandBuffer];
				if (flushBuffer == nil)
					return;

				[flushBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedBuffer)
				{
					LogQueueCommandBufferError(completedBuffer);
				}];
				[flushBuffer commit];
				[flushBuffer waitUntilCompleted];
			} // @autoreleasepool
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
