//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuSubmitThread.h"
#include "B3DApplication.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
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
		{
			pool = nullptr;
		}
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnDestroy), "Cleanup submit thread");
	mCommandQueue.PostRequestShutdownCommand(true);

	gActiveSubmitThread = nullptr;
}

void GpuSubmitThread::QueueSubmit(const TShared<GpuCommandBuffer>& commandBuffer, GpuQueue& queue, GpuQueueMask syncMask, TInlineArray<GpuTimelineFenceAndValue, 2> signalFences, bool blocking)
{
	auto fnCommand = [this, commandBuffer, &queue, syncMask, signalFences = std::move(signalFences)]() mutable
	{
		syncMask |= commandBuffer->GetQueueSyncMask();
		mBackend.ExecuteSubmit(queue, commandBuffer, syncMask, signalFences);
	};

	mBackend.NotifyWillQueueForSubmit(*commandBuffer);
	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Command buffer submit");

	if (blocking)
		WaitUntilIdle();
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
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Swap chain present");
	swapChain.NotifyWasPresentQueued(acquiredImageIndex);
}

void GpuSubmitThread::QueueImageAcquire(GpuSwapChain& swapChain)
{
	auto fnCommand = [&swapChain]
	{
		gRenderEnableSubmitThread ? RunBlockingCallAsYieldable([&swapChain] { swapChain.AcquireImage(); }) : swapChain.AcquireImage();

		swapChain.GetMessageQueue().PostCommand([&swapChain] { swapChain.NotifyUnbound(); });
	};

	B3D_ASSERT(!swapChain.IsRetired());

	swapChain.NotifyWasImageAcquireQueued();
	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Acquire swap chain image");
}

void GpuSubmitThread::QueueEndFrameAndWaitForPreviousFrame()
{
	const u32 frameIndex = mCurrentFrameIndex;
	const u32 nextFrameIndex = (frameIndex + 1) % kFrameCount;

	mCurrentFrameIndex = nextFrameIndex;

	// Mark this frame's end processing as pending (will be signalled when submit thread finishes)
	mFrameMarkers[frameIndex].CompletionEvent.Reset();

	auto fnCommand = [this, frameIndex, nextFrameIndex]
	{
		// Snapshot the last submit index on every queue, marking the boundary of all work issued during this frame. By the
		// time this runs all of the frame's submissions have already executed (the command queue is processed in order), so
		// each queue's most recent submit index covers the whole frame.
		FrameCompletionMarker& currentMarker = mFrameMarkers[frameIndex];
		mGpuDevice.DoForEachQueue([this, &currentMarker](GpuQueue& queue)
		{
			currentMarker.LastSubmitIndices[queue.GetId().Id] = mBackend.GetLastSubmitIndex(queue);
		});

		// Wait for all of the previous frame's work to finish on every queue, up to the submit index captured at that frame's
		// boundary. We must wait on the whole frame rather than just its last command buffer: the intra-queue semaphore chain
		// is intentionally broken across present, so a trailing post-present command buffer can complete while earlier (heavier)
		// command buffers from the same frame are still executing. Reusing their command buffer pools or resources before they
		// finish would lead to GPU faults and device loss.
		const FrameCompletionMarker& previousMarker = mFrameMarkers[nextFrameIndex];
		mGpuDevice.DoForEachQueue([this, &previousMarker](GpuQueue& queue)
		{
			const u32 lastSubmitIndex = previousMarker.LastSubmitIndices[queue.GetId().Id];
			mBackend.RefreshCompletionState(queue, true, false, lastSubmitIndex);
		});

		// TODO: This could be signalled earlier. In case the frame's work finishes earlier the submit thread could set the signal
		// before this point. This would avoid the render thread blocking if the work is already finished.
		mFrameMarkers[nextFrameIndex].CompletionEvent.Signal();
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "End frame");

	// We're about to start rendering frame at 'nextFrameIndex', so we must make sure it has completed on the GPU, and we have sent the Reset() calls to their
	// message queues, as we're about to re-use those command buffers.
	mCurrentFrameIndex = nextFrameIndex;
	mFrameMarkers[nextFrameIndex].CompletionEvent.Wait();
}

void GpuSubmitThread::WaitUntilIdle(bool performCleanupForShutdown)
{
	auto fnCommand = [this, performCleanupForShutdown]()
	{
		auto fnWait = [this] { mBackend.WaitUntilIdle(); };
		gRenderEnableSubmitThread ? RunBlockingCallAsYieldable(fnWait) : fnWait();

		mGpuDevice.DoForEachQueue([this, performCleanupForShutdown](GpuQueue& queue)
		{
			mBackend.RefreshCompletionState(queue, true, performCleanupForShutdown);
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
		auto fnWait = [this, &queue] { mBackend.WaitUntilIdle(queue); };
		gRenderEnableSubmitThread ? RunBlockingCallAsYieldable(fnWait) : fnWait();

		mBackend.RefreshCompletionState(queue, true);
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Queue wait idle", true);
}

u32 GpuSubmitThread::GetThreadId() const
{
	return mCommandQueue.GetThreadId();
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
