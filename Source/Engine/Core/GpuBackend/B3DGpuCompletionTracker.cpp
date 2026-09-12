//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuCompletionTracker.h"
#include "B3DGpuDevice.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;

bool GpuFrameCompletionTracker::IsMarkerComplete(u64 marker) const
{
	const u64 currentFrame = mFrameIndex.load(std::memory_order_acquire);
	return marker + RenderThread::kMaximumFramesInFlight <= currentFrame;
}

GpuFenceCompletionTracker::GpuFenceCompletionTracker(GpuDevice& device)
	: mDevice(device)
{
}

const TShared<GpuTimelineFence>& GpuFenceCompletionTracker::GetOrCreateFence(GpuQueueId queue)
{
	TShared<GpuTimelineFence>& fence = mQueueFences[queue.Id];
	if (fence == nullptr)
		fence = mDevice.CreateTimelineFence();

	return fence;
}

bool GpuFenceCompletionTracker::IsMarkerComplete(u64 marker) const
{
	for (const PendingSubmission& pending : mPending)
	{
		if (pending.Value > marker)
			break;

		if (!mQueueFences[pending.Queue.Id]->IsSignaled(pending.Value))
			return false;
	}

	return true;
}

GpuTimelineFenceAndValue GpuFenceCompletionTracker::NotifyWillSubmit(GpuQueueId queue)
{
	// Drop submissions already observed complete, keeping the in-flight list bounded
	while (!mPending.Empty() && mQueueFences[mPending.Front().Queue.Id]->IsSignaled(mPending.Front().Value))
		mPending.Remove(0);

	const u64 value = mNextValue++;
	mLastSubmitted = value;
	mPending.Add(PendingSubmission(queue, value));

	GpuTimelineFenceAndValue fenceAndValue;
	fenceAndValue.Fence = GetOrCreateFence(queue);
	fenceAndValue.Value = value;

	return fenceAndValue;
}

void GpuFenceCompletionTracker::WaitUntilComplete()
{
	for (const PendingSubmission& pending : mPending)
		mQueueFences[pending.Queue.Id]->Wait(pending.Value);

	mPending.Clear();
}
