//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuSplitBarrier.h"

#if B3D_GPU_EXPLICIT_BARRIERS

namespace b3d::render
{
	GpuSplitBarrier::GpuSplitBarrier(TOptional<GpuQueueType> releaseQueue, TOptional<GpuQueueType> acquireQueue)
		: mReleaseQueueType(releaseQueue.has_value() ? releaseQueue : acquireQueue), mAcquireQueueType(acquireQueue.has_value() ? acquireQueue : releaseQueue)
	{ }

	GpuSplitBarrier::~GpuSplitBarrier()
	{
		// A split barrier that was never used is fine, such as when the work that needed it was culled
		const bool isReleased = mIsReleased;
		const bool isAcquired = mIsAcquired;
		if(isReleased == isAcquired)
			return;

		B3D_ENSURE_LOG(!isReleased, "A split barrier was released but never acquired.");
		B3D_ENSURE_LOG(!isAcquired, "A split barrier was acquired but never released. The GPU waits for the release indefinitely.");
	}

	bool GpuSplitBarrier::ValidateQueueType(bool isRelease, GpuQueueType queueType)
	{
		const TOptional<GpuQueueType>& createdQueue = isRelease ? mReleaseQueueType : mAcquireQueueType;
		if(createdQueue.has_value())
		{
			if(isRelease)
				return B3D_ENSURE_LOG(*createdQueue == queueType, "The release of a split barrier was recorded on a queue type other than the one the split barrier was created for.");

			return B3D_ENSURE_LOG(*createdQueue == queueType, "The acquire of a split barrier was recorded on a queue type other than the one the split barrier was created for.");
		}

		// Without queue types both halves are on the same queue, so the half recorded first determines the type
		u32 recordedQueueType = GQT_COUNT;
		if(mRecordedQueueType.compare_exchange_strong(recordedQueueType, (u32)queueType))
			return true;

		return B3D_ENSURE_LOG(recordedQueueType == (u32)queueType, "The halves of a split barrier were recorded on different queue types. Create the split barrier with the queue type of each half.");
	}
} // namespace b3d::render

#endif
