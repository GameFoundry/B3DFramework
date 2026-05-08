//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuQueue.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuCommandBuffer.h"
#include "GpuBackend/B3DRenderWindow.h"

namespace b3d
{
	namespace render
	{
		struct MetalGpuQueue::Impl
		{
			id<MTLCommandQueue> CommandQueue = nil;
			id<MTLSharedEvent> SharedEvent = nil;

			/**
			 * Shared event listener used by @c WaitUntilIdle (and by @c MetalGpuQueryPool::TryResolve
			 * through the same queue). Apple recommends reusing a single listener instance per queue
			 * rather than allocating one per wait; this listener is retained for the queue's lifetime.
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
			 * @c [SharedEvent signaledValue] tracks completion. All mutation happens under the queue's
			 * implicit submission serialization (callers serialize submissions per queue).
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
				// pattern used in @c WaitUntilIdle for @c dispatch_semaphore_t.
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

		void MetalGpuQueue::NotifySubmissionCommitted(u64 value)
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
					return;
				}
			}
		}

		void MetalGpuQueue::SubmitCommandBuffer(const GpuSubmissionInformation& information, bool flushTransferCommandBuffer)
		{
			if(flushTransferCommandBuffer)
				mGpuDevice.SubmitTransferCommandBuffers();

			const TShared<GpuCommandBuffer>& commandBuffer = information.CommandBuffer;
			if (!commandBuffer)
				return;

			auto metalCB = std::static_pointer_cast<MetalGpuCommandBuffer>(commandBuffer);

			// OR in any per-command-buffer sync mask accumulated via AddQueueSyncMask. Per the base
			// class contract (B3DGpuCommandBuffer.h: AddQueueSyncMask): "The provided mask is OR-ed
			// with existing mask ... utilized at the time the command buffer is submitted to a queue."
			// The Vulkan backend does the same fold in VulkanSubmitThread::QueueSubmit (see
			// B3DVulkanSubmitThread.cpp: "syncMask |= commandBuffer->GetQueueSyncMask();").
			GpuQueueMask syncMask = information.SyncMask;
			syncMask |= metalCB->GetQueueSyncMask();

			// Encode cross-queue waits *before* the command buffer's own work, then append a signal on
			// this queue's event at the end of the buffer, then encode any user-provided timeline fence
			// signals after that so they observe the same FIFO ordering. See
			// MetalGpuCommandBuffer::CommitInternal for the encoding logic. @c CommitInternal also clears
			// the buffer-level sync mask after consumption, matching the Vulkan cleanup pattern
			// (B3DVulkanGpuCommandBuffer.cpp resets mQueueSyncMask so subsequent Add/Submit cycles start fresh).
			metalCB->CommitInternal(*this, syncMask, information.SignalFences);
		}

		void MetalGpuQueue::WaitUntilIdle()
		{
			if (mImpl->CommandQueue == nil)
				return;

			// Prefer waiting on the shared event's signaled value: it tracks GPU completion of every
			// submission on this queue without committing an empty buffer. Fall back to an empty commit
			// if the queue has never been submitted to (event value still 0).
			const u64 waitValue = mImpl->LastReservedValue.load(std::memory_order_acquire);
			if (waitValue == 0 || mImpl->SharedEvent == nil)
			{
				id<MTLCommandBuffer> flushBuffer = [mImpl->CommandQueue commandBuffer];
				[flushBuffer commit];
				[flushBuffer waitUntilCompleted];
				return;
			}

			// MTLSharedEvent has no native blocking wait from the CPU; use a listener + semaphore.
			// The listener is allocated once per queue (see Impl::EventListener) and reused across waits
			// — Apple documents one listener per queue as the preferred pattern.
			if ([mImpl->SharedEvent signaledValue] >= waitValue)
				return;

			dispatch_semaphore_t sem = dispatch_semaphore_create(0);
			[mImpl->SharedEvent notifyListener:mImpl->EventListener atValue:waitValue block:^(id<MTLSharedEvent>, uint64_t)
			{
				dispatch_semaphore_signal(sem);
			}];
			dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);

			// Under MRC, @c dispatch_object_t is not ARC-managed and leaks if not explicitly released.
			// Under ARC (the default for this plugin), the compiler inserts the release automatically
			// and calling @c dispatch_release here is prohibited. Feature-guard accordingly.
#if !__has_feature(objc_arc)
			dispatch_release(sem);
#endif
		}

		void MetalGpuQueue::PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask)
		{
			if (!renderWindow)
				return;

			const TShared<IRenderWindowSurface>& surface = renderWindow->GetRenderWindowSurface();
			if (!surface)
				return;

			renderWindow->NotifySwapBuffersRequested();
			surface->SwapBuffers(*this, syncMask);
		}
	} // namespace render
} // namespace b3d
