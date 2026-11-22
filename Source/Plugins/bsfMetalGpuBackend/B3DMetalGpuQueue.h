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
		 * Metal implementation of a GPU queue, wrapping an @c MTL4CommandQueue. Metal queues accept every encoder type, so
		 * queue types only exist to match the engine's abstraction.
		 *
		 * Work committed to the queue is not ordered by Metal, only by the barriers it encodes. Every submission ends with
		 * a signal of the queue's @c MTLSharedEvent with a new value, which Metal performs once all previously committed work
		 * has finished. Other queues wait on the last committed value.
		 *
		 * @note	Commit, wait and signal methods may only be called on the submit thread.
		 */
		class MetalGpuQueue : public GpuQueue
		{
		public:
			MetalGpuQueue(GpuDevice& device, GpuQueueType type, u32 index, id<MTL4CommandQueue> commandQueue, id<MTLSharedEvent> sharedEvent);

			/** Returns the underlying MTL4CommandQueue. */
			id<MTL4CommandQueue> GetMetalQueue() const { return mCommandQueue; }

			/** Returns the shared event signaled by submissions on this queue. */
			id<MTLSharedEvent> GetSharedEvent() const { return mSharedEvent; }

			/** Returns the highest event value whose submission has been committed on this queue. */
			u64 GetLastCommittedEventValue() const { return mLastCommittedEventValue.load(std::memory_order_acquire); }

			/** Returns the event value the next submission on this queue signals, see SignalSubmission(). Submit thread only. */
			u64 GetNextSubmissionEventValue() const { return mLastReservedEventValue + 1; }

			/**
			 * Commits @p commandBuffers, in order. Metal does not order their execution, the barriers they encode do. Execution
			 * errors are reported once the GPU finishes them.
			 */
			void Commit(const id<MTL4CommandBuffer>* commandBuffers, u32 count);

			/**
			 * Makes work committed afterwards wait until each other queue's event reaches its value in @p waits. Zero values and the
			 * entry of this queue are ignored.
			 */
			void WaitForQueues(const GpuQueueFenceValues& waits);

			/** Makes work committed afterwards wait until @p event reaches @p value. */
			void WaitForEvent(id<MTLSharedEvent> event, u64 value);

			/** Signals @p event with @p value once all work committed before has finished. */
			void SignalEvent(id<MTLSharedEvent> event, u64 value);

			/**
			 * Makes work committed afterwards wait until @p drawable can be rendered to. Must precede the commit of the first
			 * command buffer that accesses the drawable's texture. Waits issued earlier for the same drawable are not repeated.
			 */
			void WaitForDrawable(id<CAMetalDrawable> drawable);

			/** Releases @p drawable for presentation once all work committed before has finished. */
			void SignalDrawable(id<CAMetalDrawable> drawable);

			/**
			 * Ends a submission by signaling the queue's event with a new value, once all work committed before has finished.
			 *
			 * @param	onCompleted			Called on a Metal callback thread once the GPU has finished the submission.
			 * @param	ownerCompletion		Optional wait group signaled once the owner-side cleanup for the submission has run.
			 * @return						Signaled event value.
			 */
			u64 SignalSubmission(Function<void()> onCompleted, const TShared<WaitGroup>& ownerCompletion = nullptr);

			/**
			 * Records a submission that failed before its work was committed. It still signals the event value a successful
			 * submission would, once all work committed before has finished, so work waiting on that value doesn't wait forever.
			 */
			void NotifySubmissionFailed(const TShared<WaitGroup>& ownerCompletion);

			/** @name Submit thread
			 *  Native halves of the device's IGpuSubmitThreadBackend implementation.
			 *  @{
			 */

			/**
			 * Blocks until every submission signaled on this queue has finished executing, and their completion callbacks have
			 * run. Unlike WaitUntilIdle() this never goes through the submit thread, so it can also be used during device
			 * teardown.
			 */
			void ExecuteWaitUntilIdle();

			/**
			 * Checks which submissions on this queue have finished executing and retires them.
			 *
			 * @param	forceWait		If true, blocks until every submission up to @p lastEventValue has finished
			 *							executing, and their owner-side cleanup has run. Submissions are only retired
			 *							by forced waits.
			 * @param	lastEventValue	Committed event value of the last submission to check, as returned by
			 *							GetLastCommittedEventValue(). If ~0, all submissions are checked.
			 *
			 * @note	Submit thread only.
			 */
			void RefreshCompletionState(bool forceWait, u64 lastEventValue = ~0ull);

			/** @} */

			void SubmitCommandBuffer(const GpuSubmissionInformation& information) override;
			void WaitUntilIdle() override;

		private:
			/** Submission on this queue that has not been retired yet. */
			struct SubmissionRecord
			{
				u64 EventValue = 0; /**< Signaled event value. */
				TShared<WaitGroup> OwnerCompletion; /**< Signaled after the owner-side cleanup runs. */
			};

			id<MTL4CommandQueue> mCommandQueue = nil;
			id<MTLSharedEvent> mSharedEvent = nil;

			/** Event value reserved by the most recent submission. The shared event's signaled value tracks completion. */
			u64 mLastReservedEventValue = 0;

			/** Highest event value whose submission has been committed. */
			std::atomic<u64> mLastCommittedEventValue { 0 };

			/** Submissions not yet retired, in ascending event value order. Guarded by mSubmissionMutex. */
			Vector<SubmissionRecord> mActiveSubmissions;
			Mutex mSubmissionMutex;

			/** Number of signaled submissions whose completion callback has not run yet. Guarded by mCompletionMutex. */
			u32 mPendingCompletionCount = 0;
			Mutex mCompletionMutex;
			ConditionVariable mCompletionCondition;

			/** Drawables waited on and not signaled yet. Entries become nil once their drawable is released. */
			Vector<__weak id<CAMetalDrawable>> mWaitedDrawables;
		};

		/** @} */
	} // namespace render
} // namespace b3d
