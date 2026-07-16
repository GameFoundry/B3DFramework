//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "Threading/B3DWaitGroup.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of a GPU queue.
		 *
		 * Wraps a @c MTLCommandQueue. One queue is created per GpuQueueType by the device. Metal queues
		 * accept any kind of encoder (graphics, compute, or blit); the type-based split exists purely to
		 * mirror the engine's abstraction and to allow future per-type submission ordering.
		 *
		 * Command buffer submission is routed through the device's GpuSubmitThread:
		 * SubmitCommandBuffer() validates and hands off to GpuSubmitThread::QueueSubmit, which calls back
		 * into MetalGpuDevice::ExecuteSubmit -> MetalGpuCommandBuffer::CommitInternal on the submit
		 * thread. The submit-thread-facing half of the queue (ExecuteWaitUntilIdle,
		 * RefreshCompletionState, GetLastSubmitIndex) backs the device's IGpuSubmitThreadBackend
		 * implementation.
		 *
		 * The queue owns a single @c MTLSharedEvent that is signaled on every committed command buffer.
		 * Cross-queue dependencies expressed via @c GpuQueueMask are encoded at submit time by making the
		 * submitted command buffer wait on the target queue's last-committed event value (race-free — see
		 * @c GetLastCommittedEventValue), and every submission increments this queue's own event value
		 * once the command buffer finishes its work.
		 */
		class MetalGpuQueue : public GpuQueue
		{
		public:
#ifdef __OBJC__
			MetalGpuQueue(GpuDevice& device, GpuQueueType type, u32 index, id<MTLCommandQueue> commandQueue, id<MTLSharedEvent> sharedEvent);

			/** Returns the underlying MTLCommandQueue. */
			id<MTLCommandQueue> GetMetalQueue() const;

			/** Returns the shared event used to signal submissions on this queue. */
			id<MTLSharedEvent> GetSharedEvent() const;

			/**
			 * Returns the event value reserved for the most recent submission on this queue — i.e. the
			 * value the upcoming @c encodeSignalEvent:value: has latched. This is a CPU-side reservation
			 * counter bumped synchronously inside @c ReserveNextEventValue; the GPU has not necessarily
			 * committed or begun work for this value when the accessor returns. Used by the producer
			 * side to encode its own signal value inside @c CommitInternal. Cross-queue waits must use
			 * @c GetLastCommittedEventValue instead — waiting on the reserved value can deadlock if a
			 * concurrent submission reserved N+1 but has not yet reached @c [cmdBuffer commit].
			 */
			u64 GetLastReservedEventValue() const;

			/**
			 * Returns the largest event value that has been reserved *and* committed to the GPU on this
			 * queue (i.e. @c [cmdBuffer commit] has returned for that submission). Used by cross-queue
			 * waits: encoding @c encodeWaitForEvent:value: with the producer queue's committed value
			 * ensures the waiter never blocks on a value that has been reserved by a concurrent submit
			 * but not yet handed to the Metal driver.
			 */
			u64 GetLastCommittedEventValue() const;

			/**
			 * @deprecated Use @c GetLastCommittedEventValue for cross-queue waits (race-free) or
			 * @c GetLastReservedEventValue for encoding the queue's own next signal. This alias forwards
			 * to the committed value because every existing caller is a cross-queue waiter that wants
			 * commit-order semantics.
			 */
			u64 GetLastScheduledEventValue() const { return GetLastCommittedEventValue(); }

			/**
			 * Returns the event value most recently signaled on the GPU side of this queue (i.e. the
			 * completion frontier). Reads from @c [SharedEvent signaledValue], so the value changes
			 * asynchronously as the GPU retires submissions. Used by @c MetalGpuQueryPool to decide
			 * whether a submitted query's event value has been reached without blocking.
			 */
			u64 GetLastSignaledEventValue() const;

			/** Returns the next event value that will be signaled by the upcoming submission on this queue. */
			u64 ReserveNextEventValue();

			/**
			 * Records that a submission on this queue with @p value has reached @c [cmdBuffer commit].
			 * Performs a monotonic update of the committed high-water mark — if @p value is older than
			 * the current committed value (rare, but possible if two threads commit on this queue in
			 * reverse reservation order), the stored mark is left unchanged. Release-store pairs with
			 * acquire-loads inside @c GetLastCommittedEventValue and the cross-queue wait encoders.
			 *
			 * Additionally assigns the submission the next engine submit index and records the
			 * (submit index, event value) pair for RefreshCompletionState / GetLastSubmitIndex
			 * (frame pacing via GpuSubmitThread).
			 */
			void NotifySubmissionCommitted(u64 value, id<MTLCommandBuffer> commandBuffer,
				const TShared<WaitGroup>& ownerCompletion = nullptr);

			/**
			 * Records owner-side completion for a submission that failed before any native command buffer
			 * was committed. This advances only the engine submit index; no shared-event value is published.
			 */
			void NotifySubmissionFailed(const TShared<WaitGroup>& ownerCompletion);

			/**
			 * Returns the shared event listener cached on this queue. Used by callers that need to block
			 * on the queue's event — notably @c MetalGpuQueryPool::TryResolve — without allocating a
			 * fresh listener on every wait.
			 */
			MTLSharedEventListener* GetSharedEventListener() const;
#endif
			~MetalGpuQueue();

			void SubmitCommandBuffer(const GpuSubmissionInformation& information) override;
			void WaitUntilIdle() override;
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;

			/** @name Submit thread
			 *  Native halves of the device's IGpuSubmitThreadBackend implementation.
			 *  @{
			 */

			/**
			 * Blocks until every submission committed on this queue has finished executing on the GPU,
			 * using a trailing native command buffer that also fences completion-handler execution so all
			 * command-buffer completion callbacks have been posted to their pools' message queues
			 * before returning. Backs MetalGpuDevice::ExecuteWaitUntilIdle. Unlike WaitUntilIdle()
			 * this never routes through the submit thread, so it is also the device-teardown fallback.
			 */
			void ExecuteWaitUntilIdle();

			/**
			 * Checks which submissions on this queue have finished executing and prunes the internal
			 * submission records accordingly. A forced frame-boundary wait blocks directly on the native
			 * command buffer associated with @p lastSubmitIndex, avoiding an extra empty submission and
			 * guaranteeing that command buffer's completion handler has returned.
			 *
			 * @param	forceWait		If true, blocks until every submission up to @p lastSubmitIndex has
			 *							finished executing, and waits until the finished buffers' owner-side
			 *							completion and cleanup callbacks have executed.
			 * @param	queueEmpty		If true, the caller guarantees the queue will be empty (e.g. on
			 *							shutdown); all submission records are dropped.
			 * @param	lastSubmitIndex	Index of the last submission to check. If ~0u, all submissions are
			 *							checked.
			 *
			 * @note	Submit thread only.
			 */
			void RefreshCompletionState(bool forceWait, bool queueEmpty = false, u32 lastSubmitIndex = ~0u);

			/**
			 * Returns the submit index of the most recently committed submission on this queue, or 0 if
			 * nothing has been committed yet. Captured at a frame boundary by GpuSubmitThread and passed
			 * back to RefreshCompletionState() to wait for all of that frame's work.
			 */
			u32 GetLastSubmitIndex() const;

			/** @} */

		private:
			/**
			 * Commits an empty MTLCommandBuffer and blocks until it completes. Completion handlers on a
			 * queue execute in submission order, so once this returns every earlier submission's
			 * addCompletedHandler block has run — i.e. all completion messages have been posted to their
			 * pools' message queues. Used by ExecuteWaitUntilIdle and RefreshCompletionState(forceWait)
			 * because the shared-event signal is encoded *inside* the command buffer and can be observed
			 * slightly before the buffer's completion handler executes.
			 */
			void FenceCompletionHandlers();

			struct Impl;
			TUnique<Impl> mImpl;
		};

		/** @} */
	} // namespace render
} // namespace b3d
