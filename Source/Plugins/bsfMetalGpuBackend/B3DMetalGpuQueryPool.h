//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuQueries.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Allocators/B3DRangeAllocator.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;
		class MetalGpuQueue;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of a query pool.
		 *
		 * Occlusion pools own a range of the 8-byte slots of the device's visibility result buffer (see
		 * MetalGpuDevice::GetVisibilityResultBuffer()), one per query. Timestamp pools use a range of an @c MTL4CounterHeap shared
		 * with other pools, resolved on the CPU.
		 * Pipeline-statistics pools are not implemented (needs a core API change).
		 */
		class MetalGpuQueryPool final : public GpuQueryPool
		{
		public:
			MetalGpuQueryPool(MetalGpuDevice& gpuDevice, const GpuQueryPoolCreateInformation& createInformation);
			~MetalGpuQueryPool() override;

			GpuQueryId AllocateQuery() override;
			bool TryResolve(bool wait = false) override;
			u64 GetQueryResult(GpuQueryId queryId, u32 elementIndex = 0) override;

			/** Returns the byte range of the pool's slots in the device's visibility result buffer. Empty for other than occlusion pools. */
			NSRange GetVisibilityResultRange() const;

			/** Returns the counter heap the pool's timestamp queries are written to, or nil if unsupported. */
			id<MTL4CounterHeap> GetCounterHeap() const { return mCounterHeap; }

			/** Returns the slot of a timestamp query in the counter heap. */
			u32 GetCounterHeapSlot(GpuQueryId queryId) const { return mFirstCounterHeapSlot + queryId.Id; }

			/** Returns the byte offset of a query's result slot inside the device's visibility result buffer. */
			u32 GetQueryOffset(GpuQueryId queryId) const { return (mFirstVisibilityResultSlot + queryId.Id) * sizeof(u64); }

			/**
			 * Records the first reference from a command buffer that has not entered submission yet.
			 *
			 * @note	Called on the thread recording the command buffer.
			 */
			void MarkRecorded();

			/**
			 * Transitions one recorded reference into the asynchronous submit pipeline.
			 *
			 * @note	Called on the thread recording the command buffer, before it is handed to the submit thread.
			 */
			void MarkQueuedForSubmission();

			/**
			 * Records that the owning command buffer has been submitted on @p queue, with the shared-event
			 * value @p eventValue scheduled as the completion marker. @c TryResolve uses this to poll each
			 * tracked queue's signaled value instead of committing an empty flush.
			 *
			 * A single pool may be written to from multiple queues over its lifetime (e.g. recorded on the
			 * graphics queue in one frame, then on the compute queue in the next). Event values live in a
			 * per-queue namespace and cannot be compared across queues, so each (queue, value) pair is
			 * tracked independently until resolved.
			 *
			 * @note	Submit thread only.
			 */
			void MarkSubmitted(MetalGpuQueue& queue, u64 eventValue);

			/**
			 * Releases a queued marker when submission fails before a completion event can be encoded.
			 *
			 * @note	Thread safe. Called on the submit thread when a submission fails, or on the thread recording the
			 *			command buffer when a queued command buffer is discarded.
			 */
			void MarkSubmissionFailed();

			/**
			 * Releases a recorded marker when its command buffer is discarded without submission.
			 *
			 * @note	Thread safe. Called on the thread recording the command buffer, or on the submit thread when a
			 *			submission fails.
			 */
			void MarkRecordingAbandoned();

			/**
			 * Resets query allocation and invalidates cached results. Matches @c ResetQueries. Submissions the GPU has finished
			 * no longer count as in flight, so pools can be reset without resolving their results first.
			 *
			 * @note	Called on the thread recording the command buffer.
			 */
			void ResetAllocation();

			/**
			 * Returns true when @p queryId belongs to the currently allocated range.
			 *
			 * @note	Thread safe.
			 */
			bool IsQueryAllocated(GpuQueryId queryId) const;

		private:
			/** Removes the in-flight submissions whose queue has signaled their event value. Caller must hold mStateMutex. */
			void RemoveCompletedSubmissions();

			MetalGpuDevice& mGpuDevice;
			u32 mFirstVisibilityResultSlot = RangeAllocator::kInvalidIndex; /**< First slot of an occlusion pool in the device's visibility result buffer, if it has any. */
			id<MTL4CounterHeap> mCounterHeap = nil; /**< Counter heap of a timestamp pool, shared with other pools. */
			u32 mFirstCounterHeapSlot = RangeAllocator::kInvalidIndex; /**< First slot of a timestamp pool in mCounterHeap, if it has any. */
			u32 mNextQueryId = 0;
			u32 mRecordedCommandBuffers = 0;
			u32 mPendingSubmissions = 0;

			mutable Mutex mStateMutex;
			ConditionVariable mSubmissionCondition;
			Vector<u64> mResolvedResults;
			bool mResultsCached = false;
			bool mSupported = false;
			bool mSubmissionFailed = false;
			bool mSubmissionFailureReported = false;

			// Tracks every (queue, eventValue) pair this pool was written from that has not yet been
			// resolved. Appended to by MarkSubmitted (or the existing entry's value is replaced with the
			// max, since event values within a queue are monotonic). Entries are removed by TryResolve and
			// ResetAllocation once the queue's signaled value catches up.
			Vector<std::pair<MetalGpuQueue*, u64>> mInFlightSubmissions;
		};

		/** @} */
	} // namespace render
} // namespace b3d
