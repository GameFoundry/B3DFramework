//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuQueue.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12GpuTimelineFence.h"
#include "B3DD3D12SwapChain.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "Profiling/B3DRenderStats.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuQueue::D3D12GpuQueue(D3D12GpuDevice& device, GpuQueueType type, u32 index, ID3D12CommandQueue* d3d12Queue)
	: GpuQueue(device, type, index)
	, mQueue(d3d12Queue)
{
	// Create a fence for synchronization
	ID3D12Device* d3d12Device = device.GetD3D12Device();
	HRESULT hr = d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create fence for GPU queue");
		return;
	}

	// Create an event for CPU-GPU synchronization
	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (mFenceEvent == nullptr)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create fence event for GPU queue");
	}
}

D3D12GpuQueue::~D3D12GpuQueue()
{
	// Queues are destroyed after the submit thread has been stopped (and all GPU work waited on), so a native
	// wait covers any work that may still be pending.
	WaitUntilIdleNative();

	if (mFenceEvent)
	{
		CloseHandle(mFenceEvent);
		mFenceEvent = nullptr;
	}

	mFence.Reset();
	mQueue.Reset();
}

void D3D12GpuQueue::SubmitCommandBuffer(const GpuSubmissionInformation& information)
{
	if (!B3D_ENSURE(information.CommandBuffer))
		return;

	D3D12GpuCommandBuffer& commandBuffer = static_cast<D3D12GpuCommandBuffer&>(*information.CommandBuffer);
	if (!B3D_ENSURE(commandBuffer.GetQueueType() == mType))
		return;

	if (commandBuffer.GetState() == GpuCommandBufferState::Executing)
	{
		B3D_LOG(Error, LogRenderBackend, "Cannot submit a command buffer that's still executing.");
		return;
	}

	if (!B3D_ENSURE(!commandBuffer.IsInRenderPass()))
		commandBuffer.EndRenderPass();

	if (commandBuffer.IsRecording())
		commandBuffer.End();

	commandBuffer.SetIsSubmitted();
	mGpuDevice.GetSubmitThread().QueueSubmit(information.CommandBuffer, *this, information.SyncMask, information.SignalFences);
}

void D3D12GpuQueue::PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask)
{
	if (renderWindow == nullptr)
		return;

	IRenderWindowSurface* surface = renderWindow->GetRenderWindowSurface().get();
	if (surface == nullptr)
		return;

	renderWindow->NotifySwapBuffersRequested();
	surface->SwapBuffers(*this, syncMask);

	B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
}

bool D3D12GpuQueue::IsExecuting() const
{
	AssertIfNotSubmitThread();

	if (mLastSubmittedCommandBuffer == nullptr)
		return false;

	return mLastSubmittedCommandBuffer->IsSubmitted() || mLastSubmittedCommandBuffer->IsDone();
}

void D3D12GpuQueue::ExecuteSubmitOnSubmitThread(const TShared<D3D12GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences)
{
	AssertIfNotSubmitThread();

	if (!B3D_ENSURE(commandBuffer))
		return;

	D3D12GpuDevice& device = GetDevice();

	// No need to explicitly sync with any entries on the same queue - same-queue submissions execute in order
	syncMask &= ~GpuQueueMask(GetId());

	// Make the queue wait for outstanding work on every queue in the sync mask
	if (!syncMask.IsEmpty())
	{
		for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
		{
			const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;
			const u32 queueCount = device.GetQueueCount(queueType);

			for (u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
			{
				if (!syncMask.IsSet(GpuQueueId(queueType, queueIndex)))
					continue;

				const TShared<D3D12GpuQueue> otherQueue = std::static_pointer_cast<D3D12GpuQueue>(device.GetQueue(queueType, queueIndex));
				if (otherQueue == nullptr || otherQueue.get() == this || !otherQueue->IsExecuting())
					continue;

				Wait(otherQueue->GetSubmitFence(), otherQueue->GetLastSignaledFenceValue());
			}
		}
	}

	ID3D12CommandList* commandLists[] = { commandBuffer->GetD3D12Handle() };
	ExecuteCommandLists(commandLists, 1);

	// Mark every tracked resource as in-flight on this queue (matched by NotifyDone when the completion callback
	// runs the command buffer's Reset).
	commandBuffer->NotifyWasSubmittedToQueue(GetId());

	// Surface any validation errors the debug layer recorded for this submission
	device.LogDebugLayerMessages();

	// Signal the command buffer's own completion fence. The fence value identifies this submission and is what
	// UpdateExecutionStatus() polls for.
	commandBuffer->mFenceValue++;
	Signal(commandBuffer->GetFence(), commandBuffer->GetFenceValue());

	// Signal the queue submit fence, used by other queues for cross-queue synchronization
	mLastSignaledFenceValue = mNextFenceValue++;
	Signal(mFence.Get(), mLastSignaledFenceValue);

	// Signal any explicitly requested timeline fences
	for (const GpuTimelineFenceAndValue& entry : signalFences)
	{
		B3D_ASSERT(entry.Fence != nullptr);

		D3D12GpuTimelineFence* fence = static_cast<D3D12GpuTimelineFence*>(entry.Fence.get());
		if (fence->GetD3D12Fence() != nullptr)
			Signal(fence->GetD3D12Fence(), entry.Value);
	}

	{
		Lock lock(mMutex);
		mActiveSubmissions.push_back(QueueSubmissionInformation(commandBuffer, mNextSubmitIndex++));
	}

	mLastSubmittedCommandBuffer = commandBuffer;
}

void D3D12GpuQueue::RefreshCompletionState(bool forceWait, bool queueEmpty, u32 lastSubmitIndex)
{
	AssertIfNotSubmitThread();

	u32 lastFinishedSubmission = 0;

	auto it = mActiveSubmissions.begin();
	while (it != mActiveSubmissions.end())
	{
		const TShared<D3D12GpuCommandBuffer> commandBuffer = it->CommandBuffer;
		if (commandBuffer == nullptr)
		{
			++it;
			continue;
		}

		if (lastSubmitIndex != ~0u && it->SubmitIndex > lastSubmitIndex)
			break;

		if (!commandBuffer->UpdateExecutionStatus(forceWait))
		{
			B3D_ASSERT(!forceWait);
			break; // No chance of any later CBs being done either
		}

		lastFinishedSubmission = it->SubmitIndex;
		++it;
	}

	if (queueEmpty)
		lastFinishedSubmission = mNextSubmitIndex - 1;

	WaitGroup waitGroup;

	{
		Lock lock(mMutex);
		it = mActiveSubmissions.begin();
		while (it != mActiveSubmissions.end())
		{
			if (it->SubmitIndex > lastFinishedSubmission)
				break;

			const TShared<D3D12GpuCommandBuffer> commandBuffer = it->CommandBuffer;
			if (commandBuffer != nullptr)
			{
				SingleConsumerQueue& messageQueue = commandBuffer->GetPool().GetMessageQueue();

				waitGroup.Increment();
				messageQueue.PostCommand([commandBuffer, waitGroup = forceWait ? &waitGroup : nullptr]()
				{
					commandBuffer->mState = GpuCommandBufferState::Done;
					commandBuffer->OnDidComplete();
					commandBuffer->Reset();

					if (waitGroup != nullptr)
						waitGroup->NotifyDone();
				}, "CommandBufferCompleteCallback");

				if (mLastSubmittedCommandBuffer == commandBuffer)
					mLastSubmittedCommandBuffer = nullptr;
			}
			else if (it->PresentSwapChain != nullptr)
			{
				// Present entry: the swap chain image is no longer being used by the GPU. Post NotifyUnbound() back
				// to the swap chain's message queue (processed on the render thread), matching the NotifyBound() that
				// was issued when the present was queued.
				D3D12SwapChain* const swapChain = it->PresentSwapChain;
				SingleConsumerQueue& messageQueue = swapChain->GetMessageQueue();

				waitGroup.Increment();
				messageQueue.PostCommand([swapChain, waitGroup = forceWait ? &waitGroup : nullptr]()
				{
					swapChain->NotifyUnbound();

					if (waitGroup != nullptr)
						waitGroup->NotifyDone();
				}, "SwapChainPresentCompleteCallback");
			}

			it = mActiveSubmissions.erase(it);
		}
	}

	// Ensure the message back callbacks also trigger in the force wait case
	if (forceWait)
		waitGroup.Wait();
}

void D3D12GpuQueue::WaitUntilIdle()
{
	// During shutdown (or on non-primary devices) the submit thread doesn't exist; the native wait suffices then.
	if (!GetDevice().HasSubmitThread())
	{
		WaitUntilIdleNative();
		return;
	}

	mGpuDevice.GetSubmitThread().WaitUntilIdle(*this);
}

void D3D12GpuQueue::WaitUntilIdleNative()
{
	if (!mQueue || !mFence || !mFenceEvent)
		return;

	// Signal the fence with the next value
	u64 fenceValue = mNextFenceValue++;
	HRESULT hr = mQueue->Signal(mFence.Get(), fenceValue);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to signal fence in WaitUntilIdle");
		return;
	}

	// Wait until the fence reaches the signaled value
	if (mFence->GetCompletedValue() < fenceValue)
	{
		hr = mFence->SetEventOnCompletion(fenceValue, mFenceEvent);
		if (SUCCEEDED(hr))
		{
			WaitForSingleObject(mFenceEvent, INFINITE);
		}
		else
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to set fence event in WaitUntilIdle");
		}
	}

	GetDevice().LogDebugLayerMessages();
}

void D3D12GpuQueue::ExecuteCommandLists(ID3D12CommandList* const* commandLists, u32 numCommandLists)
{
	if (!mQueue || numCommandLists == 0)
		return;

	mQueue->ExecuteCommandLists(numCommandLists, commandLists);
}

void D3D12GpuQueue::Signal(ID3D12Fence* fence, u64 value)
{
	if (!mQueue || !fence)
		return;

	HRESULT hr = mQueue->Signal(fence, value);
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to signal fence on GPU queue");
	}
}

void D3D12GpuQueue::Wait(ID3D12Fence* fence, u64 value)
{
	if (!mQueue || !fence)
		return;

	HRESULT hr = mQueue->Wait(fence, value);
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to wait on fence in GPU queue");
	}
}

HRESULT D3D12GpuQueue::Present(D3D12SwapChain* swapChain, GpuQueueMask syncMask)
{
	AssertIfNotSubmitThread();

	if (!swapChain)
		return E_INVALIDARG;

	D3D12GpuDevice& device = GetDevice();

	// No need to explicitly sync with entries on the same queue - same-queue submissions execute in order.
	syncMask &= ~GpuQueueMask(GetId());

	// Make the queue wait for outstanding work on every queue in the sync mask before presenting. DXGI presents on
	// this queue's command queue, so this ordering ensures the rendered image is finished on other queues first.
	if (!syncMask.IsEmpty())
	{
		for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
		{
			const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;
			const u32 queueCount = device.GetQueueCount(queueType);

			for (u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
			{
				if (!syncMask.IsSet(GpuQueueId(queueType, queueIndex)))
					continue;

				const TShared<D3D12GpuQueue> otherQueue = std::static_pointer_cast<D3D12GpuQueue>(device.GetQueue(queueType, queueIndex));
				if (otherQueue == nullptr || otherQueue.get() == this || !otherQueue->IsExecuting())
					continue;

				Wait(otherQueue->GetSubmitFence(), otherQueue->GetLastSignaledFenceValue());
			}
		}
	}

	// The DXGI present uses the swap chain's own vsync interval.
	HRESULT hr = swapChain->PresentDXGI();

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to present swap chain");
	}

	// Signal the queue submit fence so a later submission can detect the present has been ordered on the GPU, and
	// register a present entry. The entry has a null command buffer; it stays in mActiveSubmissions until a
	// following submission on this queue completes (or the queue is drained), at which point the swap chain's
	// NotifyUnbound() is posted back on its message queue.
	mLastSignaledFenceValue = mNextFenceValue++;
	Signal(mFence.Get(), mLastSignaledFenceValue);

	{
		Lock lock(mMutex);
		mActiveSubmissions.push_back(QueueSubmissionInformation(swapChain, mNextSubmitIndex++));
	}

	return hr;
}
