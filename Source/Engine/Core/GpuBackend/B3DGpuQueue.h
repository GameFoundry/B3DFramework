//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuTimelineFence.h"

namespace b3d
{
	class GpuDevice;

	namespace render
	{
		class GpuCommandBuffer;
		class RenderWindow;
	}

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Uniquely represents a GPU queue. */
	struct GpuQueueId
	{
		GpuQueueId(u32 id = 0)
			: Id(id)
		{
			B3D_ASSERT(Id < kMaximumGpuQueueCount);
		}

		GpuQueueId(GpuQueueType type, u32 index)
			: Id((u32)type * kMaximumGpuQueuesPerType + index)
		{
			B3D_ASSERT((u32)type < GQT_COUNT && index < kMaximumGpuQueuesPerType);
		}

		GpuQueueType GetType() const { return (GpuQueueType)(Id / kMaximumGpuQueuesPerType); }

		u32 GetIndex() const { return Id % kMaximumGpuQueuesPerType; }

		u32 Id;
	};

	/** Mask that represents zero or multiple GPU queues. */
	struct B3D_EXPORT GpuQueueMask
	{
		GpuQueueMask(u32 mask = 0)
			: Mask(mask)
		{ }

		GpuQueueMask(GpuQueueId id)
			: Mask(1u << id.Id)
		{ }

		bool operator==(GpuQueueMask rhs) const { return Mask == rhs.Mask; }
		bool operator!=(GpuQueueMask rhs) const { return Mask != rhs.Mask; }

		GpuQueueMask& operator=(GpuQueueId id)
		{
			Mask = GpuQueueMask(id).Mask;
			return *this;
		}

		GpuQueueMask& operator|=(GpuQueueMask rhs)
		{
			Mask |= rhs.Mask;
			return *this;
		}

		GpuQueueMask operator|(GpuQueueMask rhs) const
		{
			GpuQueueMask out(*this);
			out |= rhs;

			return out;
		}

		GpuQueueMask& operator&=(GpuQueueMask rhs)
		{
			Mask &= rhs.Mask;
			return *this;
		}

		GpuQueueMask operator&(GpuQueueMask rhs) const
		{
			GpuQueueMask out(*this);
			out &= rhs;

			return out;
		}

		GpuQueueMask& operator^=(GpuQueueMask rhs)
		{
			Mask ^= rhs.Mask;
			return *this;
		}

		GpuQueueMask operator^(GpuQueueMask rhs) const
		{
			GpuQueueMask out(*this);
			out ^= rhs;

			return out;
		}

		GpuQueueMask operator~() const
		{
			GpuQueueMask out;
			out.Mask = ~Mask;

			return out;
		}

		/** Returns true if no queues are part of the mask. */
		bool IsEmpty() const { return Mask == 0; }

		/** Returns true if the queue ID is part of the mask. */
		bool IsSet(GpuQueueId queueId) const { return (Mask & GpuQueueMask(queueId).Mask) != 0; }

		u32 Mask = 0;

		static const GpuQueueMask kNone;
		static const GpuQueueMask kAll;
	};

	/**
	 * Value of each queue's own fence, indexed by GpuQueueId::Id. A value names a submission on its queue together with all work submitted to
	 * the queue before it, and 0 names no work. See render::IGpuSubmitThreadBackend::GetLastSubmittedFenceValue().
	 */
	using GpuQueueFenceValues = Array<u64, kMaximumGpuQueueCount>;

	/**
	 * Work of every queue that the next submission on each queue is already ordered after, as queue fence values (see GpuQueueFenceValues).
	 * Lets a submitter leave out waits that earlier waits already satisfy. GpuSubmissionTimeline answers the same question for planned
	 * submissions.
	 *
	 * Assumes fence values increase in submission order on each queue, and that a wait on a queue's fence value orders the waiting
	 * submission, and every later submission on its queue, after the waited-for work and everything that work is ordered after. Submissions
	 * the coverage isn't told about only make it miss orderings, so they never cause a missing wait.
	 */
	class B3D_EXPORT GpuQueueCoverage
	{
	public:
		/** Creates coverage in which no submission is ordered after any work. */
		GpuQueueCoverage() = default;

		/** Creates coverage in which every later submission, on every queue, is ordered after the work of every queue up to its value in @p fenceValues. */
		explicit GpuQueueCoverage(const GpuQueueFenceValues& fenceValues);

		/**
		 * Returns the waits of @p waits that the next submission on @p queue is not already ordered after. Waits on @p queue itself are always
		 * left out, since its submissions execute in order.
		 */
		GpuQueueFenceValues GetUncoveredWaits(GpuQueueId queue, const GpuQueueFenceValues& waits) const;

		/**
		 * Records a submission on @p queue that is ordered after the work of every other queue up to its value in @p waits, whether the submission
		 * waits for it or is already ordered after it, and that signals the queue's fence with @p fenceValue.
		 */
		void AddSubmission(GpuQueueId queue, const GpuQueueFenceValues& waits, u64 fenceValue);

	private:
		/** Work of each queue (inner index) that the latest submission on each queue (outer index) is ordered after. Indexed by GpuQueueId::Id. */
		Array<GpuQueueFenceValues, kMaximumGpuQueueCount> mCovered = {};
	};

	/** Command buffer to submit and any timeline fences to signal. */
	struct GpuSubmissionInformation
	{
		/** Command buffer to submit  */
		TShared<render::GpuCommandBuffer> CommandBuffer;

		/**
		 * Optional synchronization mask that determines if the submitted command buffer
		 * depends on any other command buffers submitted on other queues.
		 *
		 * Use this for dependencies that are not represented by tracked resource accesses, such as ordering around
		 * queries or externally managed synchronization. Backends may add resource-derived queue dependencies to this
		 * mask automatically. Command buffers submitted to the same queue execute in submission order, but resource
		 * memory and execution dependencies still require the appropriate barriers.
		 */
		GpuQueueMask SyncMask = GpuQueueMask::kAll;

		/** Fence(s) to signal when execution completes. */
		TInlineArray<GpuTimelineFenceAndValue, 2> SignalFences;
	};

	/**
	 * Specifies a queue on which command buffers can be submitted on.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuQueue
	{
	public:
		virtual ~GpuQueue() = default;

		/** Determines which type of command buffer commands can be used on the command buffers submitted on the queue. */
		GpuQueueType GetType() const { return mType; }

		/** Returns the unique index of the queue, for its type. */
		u32 GetIndex() const { return mIndex; }

		/** Returns a unique identifier for this queue. */
		GpuQueueId GetId() const { return GpuQueueId(mType, mIndex); }

		/** Returns the device the queue belongs to. */
		GpuDevice& GetGpuDevice() const { return mGpuDevice; }

		/**
		 * Submits a command buffer on this queue.
		 *
		 * @param	information					Command buffer + signal fences to submit.
		 */
		virtual void SubmitCommandBuffer(const GpuSubmissionInformation& information) = 0;

		/**
		 * Presents the back-buffer image from the provided window onto the window. Captures requested on the window read the image before
		 * it is presented (see render::RenderWindow::RequestCapture()).
		 *
		 * @param	renderWindow		Window whose back-buffer to present.
		 * @param	syncMask			Optional synchronization mask that determines if the present operation
		 *								depends on any command buffers submitted on other queues.
		 *
		 * @note	Render thread only.
		 */
		void PresentRenderWindow(const TShared<render::RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll);

		/** Blocks the calling thread until all operations on the queue finish executing on the GPU. */
		virtual void WaitUntilIdle() = 0;

		/**
		 * Blocks the calling thread until @p fence reaches @p value, signalled by a command buffer submitted on this queue
		 * (see GpuSubmissionInformation::SignalFences). Unlike GpuTimelineFence::Wait(), this also completes the command
		 * buffers of every submission on the queue up to the one that signals the value, running their completion
		 * callbacks and making them reusable. Later submissions keep executing and are not waited on.
		 */
		void WaitUntilFence(const TShared<GpuTimelineFence>& fence, u64 value);

	protected:
		GpuQueue(GpuDevice& gpuDevice, GpuQueueType type, u32 index);

		GpuDevice& mGpuDevice;
		GpuQueueType mType;
		u32 mIndex;
	};

	/** @} */

} // namespace b3d
