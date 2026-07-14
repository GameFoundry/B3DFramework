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
		GpuQueue(GpuDevice& gpuDevice, GpuQueueType type, u32 index);

		GpuDevice& mGpuDevice;
		GpuQueueType mType;
		u32 mIndex;
	};

	/** @} */

} // namespace b3d
