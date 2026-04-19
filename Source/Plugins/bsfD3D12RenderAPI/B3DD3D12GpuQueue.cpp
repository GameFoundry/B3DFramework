//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuQueue.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12SwapChain.h"
#include "RenderAPI/B3DRenderWindow.h"
#include "Profiling/B3DRenderStats.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuQueue::D3D12GpuQueue(D3D12GpuDevice& device, GpuQueueUsage usage, u32 index, ID3D12CommandQueue* d3d12Queue)
	: GpuQueue(device, usage, index)
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

	B3D_LOG(Info, LogRenderBackend, "Created D3D12 GPU queue: usage={0}, index={1}", (u32)usage, index);
}

D3D12GpuQueue::~D3D12GpuQueue()
{
	// Wait for all work to complete before destroying the queue
	WaitUntilIdle();

	if (mFenceEvent)
	{
		CloseHandle(mFenceEvent);
		mFenceEvent = nullptr;
	}

	mFence.Reset();
	mQueue.Reset();

	B3D_LOG(Info, LogRenderBackend, "Destroyed D3D12 GPU queue");
}

void D3D12GpuQueue::SubmitCommandBuffer(const SPtr<GpuCommandBuffer>& commandBuffer, u32 syncMask, bool flushTransferCommandBuffer)
{
	if (!commandBuffer)
		return;

	// Optionally flush transfer command buffer first
	if (flushTransferCommandBuffer)
	{
		// TODO: Implement transfer command buffer flushing
		// This would submit any pending transfer operations before the main command buffer
	}

	D3D12GpuCommandBuffer* d3d12CommandBuffer = static_cast<D3D12GpuCommandBuffer*>(commandBuffer.get());

	// Make sure the command buffer is in the correct state
	if (!d3d12CommandBuffer->IsReadyForSubmit())
	{
		B3D_LOG(Warning, LogRenderBackend, "Attempting to submit command buffer that is not ready for submission");
		return;
	}

	// Get the D3D12 command list
	ID3D12GraphicsCommandList* commandList = d3d12CommandBuffer->GetD3D12Handle();
	if (!commandList)
	{
		B3D_LOG(Error, LogRenderBackend, "Command buffer has no D3D12 command list");
		return;
	}

	// Execute the command list
	ID3D12CommandList* commandLists[] = { commandList };
	ExecuteCommandLists(commandLists, 1);

	// Signal the command buffer's fence so it knows when execution is complete
	ID3D12Fence* commandBufferFence = d3d12CommandBuffer->GetFence();
	if (commandBufferFence)
	{
		u64 fenceValue = d3d12CommandBuffer->GetFenceValue();
		Signal(commandBufferFence, fenceValue);
	}

	// Mark the command buffer as submitted
	d3d12CommandBuffer->SetIsSubmitted();

	// TODO: Handle syncMask for cross-queue dependencies
	// This would require waiting on fences from other queues before submitting

	B3D_INCREMENT_RENDER_STATISTIC(NumCommandBuffersSubmitted);
}

void D3D12GpuQueue::WaitUntilIdle()
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
}

void D3D12GpuQueue::PresentRenderWindow(const SPtr<RenderWindow>& renderWindow, u32 syncMask)
{
	if (!renderWindow)
		return;

	// TODO: Get the D3D12SwapChain from the render window
	// For now, this is a stub since we need to integrate with the render window system

	// The typical flow would be:
	// 1. Get the swap chain from the render window
	// 2. Call Present() on the swap chain
	// 3. Handle vsync based on render window settings

	B3D_LOG(Warning, LogRenderBackend, "D3D12GpuQueue::PresentRenderWindow not fully implemented");

	// Placeholder for when swap chain integration is complete:
	// D3D12SwapChain* swapChain = GetSwapChainFromRenderWindow(renderWindow);
	// if (swapChain)
	// {
	//     Present(swapChain);
	// }

	B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
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

HRESULT D3D12GpuQueue::Present(D3D12SwapChain* swapChain)
{
	if (!swapChain)
		return E_INVALIDARG;

	// Present with vsync (1) or without vsync (0)
	// TODO: Get vsync setting from render window or configuration
	u32 syncInterval = 1; // 1 = vsync enabled

	HRESULT hr = swapChain->Present(syncInterval);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to present swap chain");
	}

	return hr;
}
