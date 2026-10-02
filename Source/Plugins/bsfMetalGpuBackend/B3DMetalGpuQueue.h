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
		 * Metal implementation of a GPU queue, wrapping an @c MTLCommandQueue. Metal queues accept every encoder type, so
		 * queue types only exist to match the engine's abstraction.
		 *
		 * Every committed submission signals the queue's @c MTLSharedEvent with a new value. Cross-queue dependencies wait on
		 * the other queue's last committed value.
		 */
		class MetalGpuQueue : public GpuQueue
		{
		public:
			MetalGpuQueue(GpuDevice& device, GpuQueueType type, u32 index, id<MTLCommandQueue> commandQueue, id<MTLSharedEvent> sharedEvent);

			/** Returns the underlying MTLCommandQueue. */
			id<MTLCommandQueue> GetMetalQueue() const { return mCommandQueue; }

			/** Returns the shared event signaled by submissions on this queue. */
			id<MTLSharedEvent> GetSharedEvent() const { return mSharedEvent; }

			/** Returns the highest event value whose command buffer has been committed on this queue. */
			u64 GetLastCommittedEventValue() const { return mLastCommittedEventValue.load(std::memory_order_acquire); }

			/** Reserves and returns the event value the next submission on this queue will signal. */
			u64 ReserveNextEventValue() { return mLastReservedEventValue.fetch_add(1, std::memory_order_acq_rel) + 1; }

			/**
			 * Records that the command buffer signaling @p eventValue has been committed. Submissions may commit out of
			 * reservation order, so the last committed event value only ever increases.
			 *
			 * @param	eventValue			Event value reserved through ReserveNextEventValue().
			 * @param	commandBuffer		Committed command buffer, waited on by RefreshCompletionState().
			 * @param	ownerCompletion		Optional wait group signaled once the owner-side cleanup for the submission has run.
			 */
			void NotifySubmissionCommitted(u64 eventValue, id<MTLCommandBuffer> commandBuffer, const TShared<WaitGroup>& ownerCompletion = nullptr);

			/**
			 * Records a submission that failed before its command buffer was committed. It signals no event value, and
			 * retires together with the previously committed submission.
			 */
			void NotifySubmissionFailed(const TShared<WaitGroup>& ownerCompletion);

			/** @name Submit thread
			 *  Native halves of the device's IGpuSubmitThreadBackend implementation.
			 *  @{
			 */

			/**
			 * Blocks until every submission committed on this queue has finished executing, and their completion handlers
			 * have run. Unlike WaitUntilIdle() this never goes through the submit thread, so it can also be used during device
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
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;

		private:
			/**
			 * Commits an empty command buffer and blocks until it completes. Completion handlers run in submission order, so
			 * once this returns the handlers of every earlier submission have run. Waiting on the shared event is not enough,
			 * since it is signaled from within the command buffer, before its completion handler runs.
			 */
			void FenceCompletionHandlers();

			/** Submission on this queue that has not been retired yet. */
			struct SubmissionRecord
			{
				u64 EventValue = 0; /**< Signaled event value, or the last committed value for a failed submission. */
				id<MTLCommandBuffer> CommandBuffer = nil; /**< Committed command buffer, or nil for a failed submission. */
				TShared<WaitGroup> OwnerCompletion; /**< Signaled after the owner-side cleanup runs. */
			};

			id<MTLCommandQueue> mCommandQueue = nil;
			id<MTLSharedEvent> mSharedEvent = nil;

			/** Event value reserved by the most recent submission. The shared event's signaled value tracks completion. */
			std::atomic<u64> mLastReservedEventValue { 0 };

			/** Highest event value whose command buffer has been committed. */
			std::atomic<u64> mLastCommittedEventValue { 0 };

			/** Submissions not yet retired, in ascending event value order. Guarded by mSubmissionMutex. */
			Vector<SubmissionRecord> mActiveSubmissions;
			Mutex mSubmissionMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
