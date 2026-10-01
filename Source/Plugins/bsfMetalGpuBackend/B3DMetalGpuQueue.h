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
#ifdef __OBJC__
			MetalGpuQueue(GpuDevice& device, GpuQueueType type, u32 index, id<MTLCommandQueue> commandQueue, id<MTLSharedEvent> sharedEvent);
#endif
			~MetalGpuQueue() override;

#ifdef __OBJC__
			/** Returns the underlying MTLCommandQueue. */
			id<MTLCommandQueue> GetMetalQueue() const;

			/** Returns the shared event signaled by submissions on this queue. */
			id<MTLSharedEvent> GetSharedEvent() const;

			/** Returns the highest event value whose command buffer has been committed on this queue. */
			u64 GetLastCommittedEventValue() const;

			/** Reserves and returns the event value the next submission on this queue will signal. */
			u64 ReserveNextEventValue();

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
#endif

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
			struct Impl;

			/**
			 * Commits an empty command buffer and blocks until it completes. Completion handlers run in submission order, so
			 * once this returns the handlers of every earlier submission have run. Waiting on the shared event is not enough,
			 * since it is signaled from within the command buffer, before its completion handler runs.
			 */
			void FenceCompletionHandlers();

			TUnique<Impl> mImpl;
		};

		/** @} */
	} // namespace render
} // namespace b3d
