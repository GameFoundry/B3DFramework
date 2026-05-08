//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuDevice.h"

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
		 * The queue owns a single @c MTLSharedEvent that is signaled on every committed command buffer.
		 * Cross-queue dependencies expressed via @c GpuQueueMask are encoded at submit time by making the
		 * submitted command buffer wait on the target queue's last-committed event value (race-free — see @c GetLastCommittedEventValue), and every submission
		 * increments this queue's own event value once the command buffer finishes its work.
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
			 * Performs a monotonic update — if @p value is older than the current committed value
			 * (rare, but possible if two threads commit on this queue in reverse reservation order),
			 * the stored value is left unchanged. Release-store pairs with acquire-loads inside
			 * @c GetLastCommittedEventValue and the cross-queue wait encoders.
			 */
			void NotifySubmissionCommitted(u64 value);

			/**
			 * Returns the shared event listener cached on this queue. Used by callers that need to block
			 * on the queue's event — notably @c MetalGpuQueryPool::TryResolve — without allocating a
			 * fresh listener on every wait.
			 */
			MTLSharedEventListener* GetSharedEventListener() const;
#endif
			~MetalGpuQueue();

			void SubmitCommandBuffer(const GpuSubmissionInformation& information, bool flushTransferCommandBuffer = true) override;
			void WaitUntilIdle() override;
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;

		private:
			struct Impl;
			UPtr<Impl> mImpl;
		};

		/** @} */
	} // namespace render
} // namespace b3d
