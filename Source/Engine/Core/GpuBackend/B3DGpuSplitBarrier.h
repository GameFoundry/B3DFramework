//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuQueue.h"

#if B3D_GPU_EXPLICIT_BARRIERS

namespace b3d::render
{
	class GpuCommandBuffer;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Connects the release of a split barrier with its acquire (see GpuCommandBuffer::ReleaseBarriers()). Create it with
	 * GpuDevice::CreateSplitBarrier() before recording either half, so the halves can be recorded in any order, on any threads.
	 * Backends derive from it to hold the native objects that connect the halves, such as a label the release writes and the acquire
	 * waits on.
	 *
	 * Every command buffer that records either half keeps the object alive until it is reset, so the native objects outlive all GPU
	 * work that uses them, even if one of the command buffers is discarded without being submitted. Destroying the object after only
	 * one half was recorded is an error.
	 */
	class B3D_EXPORT GpuSplitBarrier
	{
	public:
		GpuSplitBarrier() = default;
		virtual ~GpuSplitBarrier();

		GpuSplitBarrier(const GpuSplitBarrier& other) = delete;
		GpuSplitBarrier& operator=(const GpuSplitBarrier& other) = delete;

		/** Returns true if the release has been recorded. */
		bool IsReleased() const { return mIsReleased; }

		/** Returns true if the acquire has been recorded. */
		bool IsAcquired() const { return mIsAcquired; }

	private:
		friend class GpuCommandBuffer;

		/** Progress of the command buffer that records the release. */
		enum class ReleaseSubmission : u8
		{
			Pending, /**< The release command buffer was not submitted yet. */
			Submitted, /**< The release command buffer was queued for submission on mReleaseQueue. */
			Discarded /**< The release command buffer was reset without being submitted. */
		};

		std::atomic<bool> mIsReleased = false;
		std::atomic<bool> mIsAcquired = false;

#if B3D_BUILD_TYPE_DEVELOPMENT
		std::atomic<ReleaseSubmission> mReleaseSubmission = ReleaseSubmission::Pending;
		std::atomic<u32> mReleaseQueue = 0; /**< GpuQueueId::Id of the queue the release was submitted on. */
		std::atomic<u64> mBarrierHash = 0; /**< Hash of the barriers of the half recorded first, or 0 if neither was recorded. */
#endif
	};

	/** @} */
} // namespace b3d::render

#endif
