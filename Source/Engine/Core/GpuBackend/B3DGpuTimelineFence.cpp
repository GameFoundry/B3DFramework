//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuTimelineFence.h"
#include "Threading/B3DBlockingCall.h"
#include "Threading/B3DThreading.h"

using namespace b3d;

void GpuTimelineFence::Wait(u64 value)
{
	if (IsSignaled(value))
		return;

	// Offload the blocking wait to a pooled thread so the calling fiber yields rather than parking a
	// scheduler worker (which would starve the fiber pool). Returns once WaitBlocking observes the signal.
	RunBlockingCallAsYieldable([this, value]() { WaitInternal(value); });
}

void GpuTimelineFence::WaitInternal(u64 value)
{
	// Generic fallback: poll the completed value with a short sleep. Sleeping is not as bad as it sounds as this runs 
	// on worker thread whose main purpose is just to block (See Wait()). Backends with a native blocking wait override this. 
	while (!IsSignaled(value))
		B3D_THREAD_SLEEP(1)
}
