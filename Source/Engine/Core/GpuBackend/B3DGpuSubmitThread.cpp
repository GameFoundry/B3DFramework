//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuSubmitThread.h"
#include "B3DApplication.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuResourceTracker.h"
#include "GpuBackend/B3DGpuTransferQueue.h"
#include "GpuBackend/B3DGpuSwapChain.h"
#include "Threading/B3DBlockingCall.h"
#include "Threading/B3DScheduler.h"
#include "Utility/B3DConfigVariable.h"

using namespace b3d;
using namespace b3d::render;

namespace b3d::render
{
	TConfigVariable<bool> gRenderEnableSubmitThread("render.EnableSubmitThread",
		"Runs GPU queue submit and present operations on a dedicated worker thread. If false all submit "
		"commands are executed inline on the calling thread - for debugging only.",
		true,
		ConfigVariableFlag::ReadOnly);
}

/** The active submit thread, tracked so AssertIfNotSubmitThread() can be a free function. At most one exists at a time. */
static GpuSubmitThread* gActiveSubmitThread = nullptr;

static void RunSubmitThreadCommand(SingleConsumerQueue& commandQueue, std::function<void()>&& function, const char* commandName, bool waitUntilComplete = false)
{
	if (gRenderEnableSubmitThread)
		commandQueue.PostCommand(std::move(function), commandName, waitUntilComplete);
	else
		function();
}

GpuSubmitThread::GpuSubmitThread(GpuDevice& gpuDevice, IGpuSubmitThreadBackend& backend)
	: mGpuDevice(gpuDevice), mBackend(backend)
{
	B3D_ASSERT(gActiveSubmitThread == nullptr);
	gActiveSubmitThread = this;

	if (gRenderEnableSubmitThread)
	{
		mCommandQueue.ScheduleRunUntilShutdown(GetApplication().GetTaskScheduler(), false);
	}

	auto fnInitialize = [this]()
	{
		for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
		{
			const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;
			if (mGpuDevice.GetQueueCount(queueType) == 0)
				continue;

			GpuCommandBufferPoolCreateInformation poolCreateInformation;
			poolCreateInformation.Thread = B3D_CURRENT_THREAD_ID;
			poolCreateInformation.Type = queueType;

			mCommandBufferPools[queueTypeIndex] = mGpuDevice.CreateGpuCommandBufferPool(poolCreateInformation);
		}
	};

	// Must wait until it starts so we have a fiber assigned for thread id checks
	RunSubmitThreadCommand(mCommandQueue, std::move(fnInitialize), "Initialize submit thread", true);
}

GpuSubmitThread::~GpuSubmitThread()
{
	auto fnDestroy = [this]()
	{
		for (auto& pool : mCommandBufferPools)
			pool = nullptr;

		for (Deque<FenceSignal>& fenceSignals : mFenceSignals)
			fenceSignals.clear();
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnDestroy), "Cleanup submit thread");
	mCommandQueue.PostRequestShutdownCommand(true);

	gActiveSubmitThread = nullptr;
}

void GpuSubmitThread::QueueSubmit(const TShared<GpuCommandBuffer>& commandBuffer, GpuQueue& queue, GpuQueueMask syncMask, TInlineArray<GpuTimelineFenceAndValue, 2> signalFences, bool blocking)
{
	// The transfers queued to the resources the command buffer uses, and to bindless resources it may use, are taken now, on the calling
	// thread, so a transfer queued after this call can't join them. They are submitted before the command buffer.
	GpuTransferQueue& transferQueue = mGpuDevice.GetTransferQueue();
	const TArrayView<IGpuResource* const> transferResources = commandBuffer->GetResourceTracker().GetResourcesWithQueuedTransfers();

	u64 transferBatch = 0;
	if(!transferResources.IsEmpty() || transferQueue.HasBindlessTransfers())
		transferBatch = transferQueue.TakeQueuedTransfers(transferResources);

	auto fnCommand = [this, commandBuffer, &queue, syncMask, transferBatch, signalFences = std::move(signalFences)]() mutable
	{
		syncMask |= commandBuffer->GetQueueSyncMask();

		// The batch is submitted now, or was by an earlier command, on the first graphics queue
		if(transferBatch != 0)
		{
			SubmitTakenTransfers(transferBatch);
			syncMask |= GpuQueueMask(mGpuDevice.GetQueue(GQT_GRAPHICS, 0)->GetId());
		}

		Submit(queue, commandBuffer, syncMask, signalFences);
	};

	commandBuffer->NotifyWillQueueForSubmit(queue.GetId(), syncMask | commandBuffer->GetQueueSyncMask());
	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Command buffer submit");

	if (blocking)
		WaitUntilIdle();
}

void GpuSubmitThread::QueueTransferSubmit(u64 lastBatch)
{
	auto fnCommand = [this, lastBatch]()
	{
		SubmitTakenTransfers(lastBatch);
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Transfer submit");
}

void GpuSubmitThread::Submit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences)
{
	mBackend.ExecuteSubmit(queue, commandBuffer, syncMask, signalFences);

	if (signalFences.IsEmpty())
		return;

	const u64 queueFenceValue = mBackend.GetLastSubmittedFenceValue(queue);
	for (const GpuTimelineFenceAndValue& signalFence : signalFences)
		mFenceSignals[queue.GetId().Id].push_back(FenceSignal{ signalFence.Fence, signalFence.Value, queueFenceValue });
}

void GpuSubmitThread::SubmitTakenTransfers(u64 lastBatch)
{
	// Batches that other threads took, and whose commands run later, are recorded here too, so batches execute in the order they were taken
	const GpuTransferQueue::RecordedTransfers recorded = mGpuDevice.GetTransferQueue().RecordTakenTransfers(lastBatch);
	if(recorded.CommandBuffer == nullptr)
		return;

	Submit(*mGpuDevice.GetQueue(GQT_GRAPHICS, 0), recorded.CommandBuffer, GpuQueueMask::kAll, TArrayView<const GpuTimelineFenceAndValue>(&recorded.SignalFence, 1));
}

TArrayView<const u64> GpuSubmitThread::ConsumeFrameFence(const GpuQueue& queue)
{
	AssertIfNotSubmitThread();

	if(!mFrameFencePendingQueues.IsSet(queue.GetId()))
		return {};

	mFrameFencePendingQueues &= ~GpuQueueMask(queue.GetId());
	return mFrameMarkers[mFrameFenceMarkerIndex].LastFenceValues;
}

void GpuSubmitThread::QueuePresent(GpuQueue& queue, GpuSwapChain& swapChain, GpuQueueMask syncMask)
{
	u32 acquiredImageIndex;
	const bool acquireSuccess = swapChain.TryGetFirstAcquiredImageIndex(acquiredImageIndex);
	if(!acquireSuccess)
	{
		B3D_LOG(Error, LogRenderBackend, "Unable to present image. No image has been acquired on the swap chain.");
		return;
	}

	auto fnCommand = [acquiredImageIndex, &queue, &swapChain, syncMask]
	{
		swapChain.Present(acquiredImageIndex, queue, syncMask);
		swapChain.GetMessageQueue().PostCommand([&swapChain] { swapChain.NotifyUnbound(); });
	};

	swapChain.NotifyBound();
	swapChain.NotifyWasPresentQueued(acquiredImageIndex);
	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Swap chain present");
}

void GpuSubmitThread::QueueImageAcquire(GpuSwapChain& swapChain)
{
	auto fnCommand = [&swapChain]
	{
		gRenderEnableSubmitThread ? RunBlockingCallAsYieldable([&swapChain] { swapChain.AcquireImage(); }) : swapChain.AcquireImage();

		swapChain.GetMessageQueue().PostCommand([&swapChain] { swapChain.NotifyUnbound(); });
	};

	B3D_ASSERT(!swapChain.IsRetired());

	swapChain.NotifyBound();
	swapChain.NotifyWasImageAcquireQueued();
	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Acquire swap chain image");
}

void GpuSubmitThread::QueueEndFrameAndWaitForPreviousFrame()
{
	const u32 markerIndex = mCurrentFrameMarkerIndex;
	const u32 nextMarkerIndex = (markerIndex + 1) % kFrameCount;

	mCurrentFrameMarkerIndex = nextMarkerIndex;

	// Mark this frame's end processing as pending (will be signalled when submit thread finishes)
	mFrameMarkers[markerIndex].CompletionEvent.Reset();

	auto fnCommand = [this, markerIndex, nextMarkerIndex]
	{
		// Snapshot the last fence value on every queue, marking the boundary of work issued during this frame. By the
		// time this runs all of the frame's submit commands have already executed because the command queue is
		// processed in order.
		FrameCompletionMarker& currentMarker = mFrameMarkers[markerIndex];
		mGpuDevice.DoForEachQueue([this, &currentMarker](GpuQueue& queue)
		{
			currentMarker.LastFenceValues[queue.GetId().Id] = mBackend.GetLastSubmittedFenceValue(queue);
		});

		// Order the next frame's first submission on every queue after all of this frame's work
		mFrameFencePendingQueues = GpuQueueMask::kAll;
		mFrameFenceMarkerIndex = markerIndex;

		// Wait for all command buffers from the previous frame, up to the fence value captured at that frame's boundary.
		// Checking the full range ensures every command buffer pool and its resources are safe to reuse.
		const FrameCompletionMarker& previousMarker = mFrameMarkers[nextMarkerIndex];
		mGpuDevice.DoForEachQueue([this, &previousMarker](GpuQueue& queue)
		{
			const u64 lastFenceValue = previousMarker.LastFenceValues[queue.GetId().Id];
			mBackend.RefreshCompletionState(queue, true, lastFenceValue);
			RemoveFenceSignals(queue, lastFenceValue);
		});

		// TODO: This could be signalled earlier. In case the frame's work finishes earlier the submit thread could set the signal
		// before this point. This would avoid the render thread blocking if the work is already finished.
		mFrameMarkers[nextMarkerIndex].CompletionEvent.Signal();
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "End frame");

	// We're about to start rendering the frame that uses marker 'nextMarkerIndex', so we must make sure it has completed on the GPU, and we have sent the Reset() calls to their
	// message queues, as we're about to re-use those command buffers.
	mCurrentFrameMarkerIndex = nextMarkerIndex;
	mFrameMarkers[nextMarkerIndex].CompletionEvent.Wait();
}

void GpuSubmitThread::WaitUntilIdle(bool performCleanupForShutdown)
{
	auto fnCommand = [this, performCleanupForShutdown]()
	{
		auto fnWait = [this] { mBackend.ExecuteWaitUntilIdle(); };
		gRenderEnableSubmitThread ? RunBlockingCallAsYieldable(fnWait) : fnWait();

		mGpuDevice.DoForEachQueue([this](GpuQueue& queue)
		{
			mBackend.RefreshCompletionState(queue, true);
			RemoveFenceSignals(queue, ~0ull);
		});

		if (performCleanupForShutdown)
		{
			for (auto& pool : mCommandBufferPools)
			{
				pool = nullptr;
			}
		}
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Device wait idle", true);
}

void GpuSubmitThread::WaitUntilIdle(GpuQueue& queue)
{
	auto fnCommand = [this, &queue]()
	{
		auto fnWait = [this, &queue] { mBackend.ExecuteWaitUntilIdle(queue); };
		gRenderEnableSubmitThread ? RunBlockingCallAsYieldable(fnWait) : fnWait();

		mBackend.RefreshCompletionState(queue, true);
		RemoveFenceSignals(queue, ~0ull);
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Queue wait idle", true);
}

void GpuSubmitThread::WaitUntilFence(GpuQueue& queue, const TShared<GpuTimelineFence>& fence, u64 value)
{
	auto fnCommand = [this, &queue, fence, value]()
	{
		auto fnWait = [&fence, value] { fence->Wait(value); };
		gRenderEnableSubmitThread ? RunBlockingCallAsYieldable(fnWait) : fnWait();

		// Commands execute in order, so the submission that signals the value has been recorded by now, unless it was
		// already known to be complete and removed. Backends may signal the fence before the queue's own fence that
		// completion is tracked with, so the submission can still be finishing, and is waited for explicitly.
		const Deque<FenceSignal>& fenceSignals = mFenceSignals[queue.GetId().Id];
		const auto iterFenceSignal = std::find_if(fenceSignals.begin(), fenceSignals.end(), [&fence, value](const FenceSignal& fenceSignal)
		{
			return fenceSignal.Fence == fence && fenceSignal.Value >= value;
		});

		if (iterFenceSignal == fenceSignals.end())
		{
			mBackend.RefreshCompletionState(queue, false);
			return;
		}

		const u64 queueFenceValue = iterFenceSignal->QueueFenceValue;
		mBackend.RefreshCompletionState(queue, true, queueFenceValue);
		RemoveFenceSignals(queue, queueFenceValue);
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Wait for fence", true);
}

void GpuSubmitThread::RemoveFenceSignals(const GpuQueue& queue, u64 lastFenceValue)
{
	Deque<FenceSignal>& fenceSignals = mFenceSignals[queue.GetId().Id];
	while (!fenceSignals.empty() && fenceSignals.front().QueueFenceValue <= lastFenceValue)
		fenceSignals.pop_front();
}

u32 GpuSubmitThread::GetThreadId() const
{
	return mCommandQueue.GetThreadId();
}

void GpuSubmitThread::QueueWaitForEvent(SignalEvent& event)
{
	// Run inline, the wait would block the thread that is meant to signal the event
	if (!gRenderEnableSubmitThread)
		return;

	auto fnCommand = [&event]()
	{
		RunBlockingCallAsYieldable([&event] { event.Wait(); });
	};

	mCommandQueue.PostCommand(std::move(fnCommand), "Wait for event");
}

namespace b3d::render
{
	void AssertIfNotSubmitThread()
	{
		if(!gRenderEnableSubmitThread)
			return;

		B3D_ASSERT(gActiveSubmitThread != nullptr);

		const u32 currentThreadId = Thread::GetCurrentThreadId();
		B3D_ASSERT((currentThreadId == gActiveSubmitThread->GetThreadId()) && "This method can only be accessed from the submit thread.");
	}
} // namespace b3d::render
