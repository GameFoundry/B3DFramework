//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuQueue.h"
#include "B3DGpuDevice.h"
#include "B3DRenderWindow.h"
#include "Profiling/B3DRenderStats.h"

using namespace b3d;

const GpuQueueMask GpuQueueMask::kNone = GpuQueueMask(0);
const GpuQueueMask GpuQueueMask::kAll = GpuQueueMask(~0u);

GpuQueueCoverage::GpuQueueCoverage(const GpuQueueFenceValues& fenceValues)
{
	for(GpuQueueFenceValues& covered : mCovered)
		covered = fenceValues;
}

GpuQueueFenceValues GpuQueueCoverage::GetUncoveredWaits(GpuQueueId queue, const GpuQueueFenceValues& waits) const
{
	const GpuQueueFenceValues& covered = mCovered[queue.Id];

	GpuQueueFenceValues uncoveredWaits = {};
	for(u32 queueId = 0; queueId < kMaximumGpuQueueCount; queueId++)
	{
		if(queueId != queue.Id && waits[queueId] > covered[queueId])
			uncoveredWaits[queueId] = waits[queueId];
	}

	return uncoveredWaits;
}

void GpuQueueCoverage::AddSubmission(GpuQueueId queue, const GpuQueueFenceValues& waits, u64 fenceValue)
{
	GpuQueueFenceValues& covered = mCovered[queue.Id];

	for(u32 waitQueueId = 0; waitQueueId < kMaximumGpuQueueCount; waitQueueId++)
	{
		const u64 waitValue = waits[waitQueueId];
		if(waitQueueId == queue.Id || waitValue == 0)
			continue;

		covered[waitQueueId] = std::max(covered[waitQueueId], waitValue);

		// The waited-for submission is the queue's latest known one or a later one, so it is ordered after everything the queue's latest
		// known submission is. An older submission's orderings aren't known, so only the wait itself counts.
		const GpuQueueFenceValues& waitQueueCovered = mCovered[waitQueueId];
		if(waitValue < waitQueueCovered[waitQueueId])
			continue;

		for(u32 queueId = 0; queueId < kMaximumGpuQueueCount; queueId++)
			covered[queueId] = std::max(covered[queueId], waitQueueCovered[queueId]);
	}

	covered[queue.Id] = fenceValue;
}

GpuQueue::GpuQueue(GpuDevice& gpuDevice, GpuQueueType type, u32 index)
	:mGpuDevice(gpuDevice), mType(type), mIndex(index)
{
}

void GpuQueue::PresentRenderWindow(const TShared<render::RenderWindow>& renderWindow, GpuQueueMask syncMask)
{
	if(renderWindow == nullptr)
		return;

	render::IRenderWindowSurface* const surface = renderWindow->GetRenderWindowSurface().get();
	if(surface == nullptr)
		return;

	renderWindow->NotifySwapBuffersRequested();

	// Captures read the image before the present engine takes it over
	renderWindow->ResolveCaptures(*this, syncMask);

	surface->SwapBuffers(*this, syncMask);

	B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
}

void GpuQueue::WaitUntilFence(const TShared<GpuTimelineFence>& fence, u64 value)
{
	// Without the submit thread nothing tracks command buffer completion, and the fence wait is all there is to do
	if (!mGpuDevice.HasSubmitThread())
	{
		fence->Wait(value);
		return;
	}

	mGpuDevice.GetSubmitThread().WaitUntilFence(*this, fence, value);
}
