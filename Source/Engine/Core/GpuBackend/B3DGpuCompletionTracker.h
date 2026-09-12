//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuTimelineFence.h"
#include "B3DGpuQueue.h"

namespace b3d
{
	class GpuDevice;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Abstraction over a monotonic GPU-completion marker, used by GPU allocators (and other
	 * deferred-cleanup consumers) to schedule reclamation against GPU progress. A "marker" is an
	 * opaque, monotonically increasing value handed out at record/submit time; the GPU is said to
	 * have "completed" a marker once every submission tagged with a value <= that marker has drained.
	 */
	class B3D_EXPORT IGpuCompletionTracker
	{
	public:
		virtual ~IGpuCompletionTracker() = default;

		/** Value the next piece of work recorded/submitted will be tagged with. Monotonic; starts at 0. */
		virtual u64 GetCurrentMarker() const = 0;

		/** Returns true once the GPU has drained every submission tagged with a value <= @p marker. */
		virtual bool IsMarkerComplete(u64 marker) const = 0;
	};

	/**
	 * Frame-count based completion tracker for the render thread's primary context. The marker is the
	 * frame index currently being recorded; a marker is "complete" only after the conservative
	 * RenderThread::kMaximumFramesInFlight lag, since at end-of-frame the device blocks until the
	 * previous frame's resources are safe to reuse.
	 */
	class B3D_EXPORT GpuFrameCompletionTracker : public IGpuCompletionTracker
	{
	public:
		/** Index of the frame currently being recorded. Monotonic; starts at 0. */
		u64 GetCurrentMarker() const override { return mFrameIndex.load(std::memory_order_acquire); }

		/**
		 * Returns true once the GPU has caught up such that frame @p marker is no longer in flight on
		 * any queue — i.e. the current frame index has advanced by at least
		 * RenderThread::kMaximumFramesInFlight beyond it.
		 */
		bool IsMarkerComplete(u64 marker) const override;

		/** Advances to the next frame. */
		void AdvanceFrame() { mFrameIndex.fetch_add(1, std::memory_order_acq_rel); }

	private:
		std::atomic<u64> mFrameIndex{0};
	};

	/**
	 * Timeline-fence based completion tracker for work spanning multiple queues. Composes one GpuTimelineFence
	 * per queue, created on first submission to that queue; the marker is the exact value the GPU signals on
	 * the submitting queue's fence (no conservative lag like GpuFrameCompletionTracker). Queues may complete
	 * out of order relative to each other - a marker is complete only once every submission tagged at or
	 * below it has drained, on every queue.
	 *
	 * Marker contract: GetCurrentMarker() returns the value the NEXT submit will signal. Submission
	 * records NotifyWillSubmit() (which yields that value and advances the marker), so a page retired
	 * right before the submit is stamped with exactly the value its submit signals.
	 */
	class B3D_EXPORT GpuFenceCompletionTracker : public IGpuCompletionTracker
	{
	public:
		/**
		 * @param	device	Device whose queues the tracked work runs on, and which creates the per-queue fences.
		 *					Must outlive the tracker.
		 */
		explicit GpuFenceCompletionTracker(GpuDevice& device);

		/** Value the next submit will signal. Monotonic; starts at 1 (0 is the timeline's unsignaled state). */
		u64 GetCurrentMarker() const override { return mNextValue; }

		/** Exact: true once every submission tagged with a value <= @p marker has signaled, on every queue. */
		bool IsMarkerComplete(u64 marker) const override;

		/**
		 * Records an impending submission on @p queue: returns the queue's fence + value to signal, and advances
		 * the marker. Must be added to the submission's signal fences.
		 */
		GpuTimelineFenceAndValue NotifyWillSubmit(GpuQueueId queue);

		/** Blocks until every submission recorded so far completes. No-op if nothing was submitted. */
		void WaitUntilComplete();

		/** Marker value of the most recent submission, or 0 if nothing has been submitted yet. */
		u64 GetLastSubmittedMarker() const { return mLastSubmitted; }

	private:
		/** Submission whose signal has not yet been observed. */
		struct PendingSubmission
		{
			PendingSubmission(GpuQueueId queue, u64 value)
				: Queue(queue), Value(value)
			{ }

			GpuQueueId Queue;
			u64 Value;
		};

		/** Returns the fence for @p queue, creating it on first use. */
		const TShared<GpuTimelineFence>& GetOrCreateFence(GpuQueueId queue);

		GpuDevice& mDevice;
		TShared<GpuTimelineFence> mQueueFences[B3D_MAX_UNIQUE_QUEUES]; /**< Indexed by GpuQueueId::Id; null until first use. */
		TInlineArray<PendingSubmission, 16> mPending; /**< Ascending by Value; pruned of signaled entries on submit and wait. */
		u64 mNextValue = 1;
		u64 mLastSubmitted = 0;
	};

	/** @} */

} // namespace b3d
