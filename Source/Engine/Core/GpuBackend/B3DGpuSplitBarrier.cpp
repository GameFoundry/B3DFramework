//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuSplitBarrier.h"

#if B3D_GPU_EXPLICIT_BARRIERS

namespace b3d::render
{
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
} // namespace b3d::render

#endif
