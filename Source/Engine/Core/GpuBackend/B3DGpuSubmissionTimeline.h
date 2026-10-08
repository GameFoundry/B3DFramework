//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuQueue.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Planned GPU submissions, in submit order, together with the queue waits each one performs. A submission is one command
	 * buffer submitted to a queue, and is identified by its index in the timeline. Determines whether the work of one
	 * submission is ordered before another, either on the same queue or through a chain of queue waits. Work submitted before
	 * the timeline is not described by it.
	 *
	 * Assumes submissions are submitted in timeline order, and that a queue wait waits on the latest work submitted to the
	 * queue at the time of the submission.
	 */
	class B3D_EXPORT GpuSubmissionTimeline
	{
	public:
		GpuSubmissionTimeline();

		/**
		 * Appends a submission on @p queue that waits on the latest earlier submission of every queue in @p waitQueues, as
		 * GpuSubmissionInformation::SyncMask does. A wait on a queue without an earlier submission in the timeline orders
		 * nothing in the timeline. Returns the index of the submission.
		 */
		u32 AddSubmission(GpuQueueId queue, GpuQueueMask waitQueues);

		/** Returns the queue @p submission runs on. */
		GpuQueueId GetQueue(u32 submission) const { return mSubmissions[submission].Queue; }

		/** Returns the number of submissions in the timeline. */
		u32 GetSubmissionCount() const { return (u32)mSubmissions.Size(); }

		/**
		 * Returns true if all work of submission @p earlier is ordered before submission @p later: both run on the same queue
		 * and @p earlier is not after @p later, or @p later transitively waits on a submission of earlier's queue that is not
		 * before @p earlier.
		 */
		bool IsOrderedBefore(u32 earlier, u32 later) const;

		/** Removes all submissions. */
		void Clear();

	private:
		static constexpr i32 kNotCovered = -1; /**< Not ordered after any submission of the queue in the timeline. */

		/** Planned submission, together with the latest work on every queue it is ordered after. */
		struct Submission
		{
			Submission(GpuQueueId queue);

			GpuQueueId Queue;

			/**
			 * Index of the latest submission of each queue that the submission is ordered after, or kNotCovered.
			 * Indexed by GpuQueueId::Id. The entry of the submission's own queue is the submission's index.
			 */
			i32 Covered[B3D_MAX_UNIQUE_QUEUES];
		};

		TInlineArray<Submission, 16> mSubmissions;
		i32 mLatestSubmissions[B3D_MAX_UNIQUE_QUEUES]; /**< Latest submission of each queue, or -1 if none. Indexed by GpuQueueId::Id. */
	};

	/** @} */
} // namespace b3d
