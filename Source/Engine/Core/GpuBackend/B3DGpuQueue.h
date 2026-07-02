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
		class GpuSubmitThread;
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
			B3D_ASSERT(Id < (B3D_MAX_QUEUES_PER_TYPE * GQT_COUNT));
		}

		GpuQueueId(GpuQueueType type, u32 index)
		{
			switch(type)
			{
			case GQT_COMPUTE:
				Id = B3D_MAX_QUEUES_PER_TYPE + index;
				break;
			case GQT_TRANSFER:
				Id = B3D_MAX_QUEUES_PER_TYPE * 2 + index;
				break;
			default:
				Id = index;
			}

			B3D_ASSERT(Id < (B3D_MAX_QUEUES_PER_TYPE * GQT_COUNT));
		}

		GpuQueueType GetType() const
		{
			if(Id >= B3D_MAX_QUEUES_PER_TYPE * 2)
				return GQT_TRANSFER;

			if(Id >= B3D_MAX_QUEUES_PER_TYPE)
				return GQT_COMPUTE;

			return GQT_GRAPHICS;

		}

		u32 GetIndex() const
		{
			if(Id >= B3D_MAX_QUEUES_PER_TYPE * 2)
				return Id - B3D_MAX_QUEUES_PER_TYPE * 2;

			if(Id >= B3D_MAX_QUEUES_PER_TYPE)
				return Id - B3D_MAX_QUEUES_PER_TYPE;

			return Id;
		}

		u32 Id;
	};

	/** Mask that represents zero or multiple GPU queues. */
	struct B3D_EXPORT GpuQueueMask
	{
		GpuQueueMask(u32 mask = 0)
			: Mask(mask)
		{ }

		GpuQueueMask(GpuQueueId id)
		{
			u32 bitShift = 0;
			switch(id.GetType())
			{
			case GQT_GRAPHICS:
				break;
			case GQT_COMPUTE:
				bitShift = 8;
				break;
			case GQT_TRANSFER:
				bitShift = 16;
				break;
			default:
				break;
			}

			Mask = 1 << id.GetIndex() << bitShift;
		}

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

	/** Command buffer to submit and any timeline fences to signal. */
	struct GpuSubmissionInformation
	{
		/** Command buffer to submit  */
		TShared<render::GpuCommandBuffer> CommandBuffer;

		/**
		 * Optional synchronization mask that determines if the submitted command buffer
		 * depends on any other command buffers submitted on other queues.
		 *
		 * This mask is only relevant if your command buffers are executing on different
		 * queues, and are dependent. If they are executing on the same queue then they will
		 * execute sequentially in the order they are submitted. Otherwise, if there is a
		 * dependency you must make state it explicitly here.
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

		/**
		 * Submits a command buffer on this queue.
		 *
		 * @param	information					Command buffer + signal fences to submit.
		 */
		virtual void SubmitCommandBuffer(const GpuSubmissionInformation& information) = 0;

		/**
		 * Presents the back-buffer image from the provided window onto the window, using the appropriate queue that supports present operations.
		 *
		 * @param	renderWindow		Window whose back-buffer to present.
		 * @param	syncMask			Optional synchronization mask that determines if the present operation
		 *								depends on any command buffers submitted on other queues.
		 */
		virtual void PresentRenderWindow(const TShared<render::RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) = 0;

		/** Blocks the calling thread until all operations on the queue finish executing on the GPU. */
		virtual void WaitUntilIdle() = 0;

	protected:
		friend class render::GpuSubmitThread;

		/** @name Submit thread
		 *  Interface driven by GpuSubmitThread. Not part of the public API.
		 *  @{
		 */

		/**
		 * Submits a command buffer on this queue. The backend derives any per-command-buffer submit data it needs
		 * from @p commandBuffer internally.
		 *
		 * @param	commandBuffer	Command buffer to submit.
		 * @param	syncMask		Inter-queue synchronization mask.
		 * @param	signalFences	Timeline-fence + value pairs to signal when the submit completes.
		 *
		 * @note	Submit thread only.
		 */
		virtual void ExecuteSubmitOnSubmitThread(const TShared<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences);

		/**
		 * Checks if any of the active command buffers finished executing on the queue and updates their states
		 * accordingly.
		 *
		 * @param	forceWait		If true, waits until the relevant command buffers finish executing.
		 * @param	queueEmpty		If true, the caller guarantees the queue will be empty (e.g. on shutdown),
		 *							allowing all needed resources to be freed.
		 * @param	lastSubmitIndex	Index of the last submitted command buffer to check. If ~0u, all submitted
		 *							command buffers are checked.
		 *
		 * @note	Submit thread only.
		 */
		virtual void RefreshCompletionState(bool forceWait, bool queueEmpty = false, u32 lastSubmitIndex = ~0u);

		/**
		 * Returns the submit index of the most recently submitted work (command buffer or present) on this queue,
		 * or 0 if nothing has been submitted yet. Capture this at a frame boundary and pass it to
		 * RefreshCompletionState() to wait for all of that frame's work to complete.
		 *
		 * @note	Submit thread only.
		 */
		virtual u32 GetLastSubmitIndex() const;

		/**
		 * Blocks until all work submitted on this queue finishes executing on the GPU, using the backend's native
		 * wait. Unlike WaitUntilIdle() this must not route through the submit thread - it is what the submit thread
		 * itself calls to perform the wait.
		 *
		 * @note	Submit thread only.
		 */
		virtual void WaitUntilIdleOnSubmitThread();

		/** @} */

		GpuQueue(GpuDevice& gpuDevice, GpuQueueType type, u32 index);

		GpuDevice& mGpuDevice;
		GpuQueueType mType;
		u32 mIndex;
	};

	/** @} */

} // namespace b3d
