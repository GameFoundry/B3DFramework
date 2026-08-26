//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuParameterSet.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12GpuQueue.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12Framebuffer.h"
#include "B3DD3D12Queries.h"
#include "B3DD3D12SwapChain.h"
#include "B3DD3D12RenderTexture.h"
#include "B3DID3D12RenderWindowSurface.h"
#include "B3DD3D12GpuPipelineState.h"
#include "B3DD3D12BarrierUtility.h"
#include "Utility/B3DD3D12BarrierBatch.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Managers/B3DD3D12VertexInputManager.h"
#include "Profiling/B3DRenderStats.h"
#include "Image/B3DPixelUtility.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DRenderWindow.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Converts an engine queue type to the D3D12 command list type able to record for it. */
	D3D12_COMMAND_LIST_TYPE GetCommandListType(GpuQueueType queueType)
	{
		switch (queueType)
		{
		case GQT_COMPUTE:
			return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case GQT_TRANSFER:
			return D3D12_COMMAND_LIST_TYPE_COPY;
		default:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}

	/**
	 * Blocks the calling thread until @p fence reaches @p fenceValue, or until @p timeoutMilliseconds elapses.
	 * Returns false if the wait could not be started at all.
	 */
	bool WaitForFenceValue(ID3D12Fence* fence, u64 fenceValue, DWORD timeoutMilliseconds)
	{
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (fenceEvent == nullptr)
			return false;

		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, timeoutMilliseconds);
		CloseHandle(fenceEvent);

		return true;
	}
}

D3D12GpuCommandBufferPool::D3D12GpuCommandBufferPool(D3D12GpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation) : GpuCommandBufferPool(device, createInformation)
{
	const D3D12_COMMAND_LIST_TYPE commandListType = GetCommandListType(createInformation.Type);

	HRESULT hr = device.GetD3D12Device()->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&mCommandAllocator));
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to create command allocator");
}

D3D12GpuCommandBufferPool::~D3D12GpuCommandBufferPool()
{
	D3D12GpuCommandBufferPool::Destroy();
}

void D3D12GpuCommandBufferPool::Destroy()
{
	if (mIsDestroyed)
		return;

	EnsureValidThread();

	// Wait for all command buffers to finish executing
	bool areAnyCommandBuffersStillExecuting = false;
	for (const auto& commandBufferPair : mCommandBuffers)
	{
		if (commandBufferPair.second->GetState() != GpuCommandBufferState::Ready)
		{
			areAnyCommandBuffersStillExecuting = true;
			break;
		}
	}

	if (areAnyCommandBuffersStillExecuting)
		static_cast<D3D12GpuDevice&>(mGpuDevice).WaitUntilIdle();

	mMessageQueue.PostRequestShutdownCommand(true);

	mCommandBuffers.clear();
	mCommandAllocator.Reset();

	Base::Destroy();
}

TShared<GpuCommandBuffer> D3D12GpuCommandBufferPool::FindOrCreate(const GpuCommandBufferCreateInformation& createInformation)
{
	EnsureValidThread();

	// Try to find a ready command buffer
	for (const auto& commandBufferPair : mCommandBuffers)
	{
		if (commandBufferPair.second->GetState() != GpuCommandBufferState::Ready)
			continue;

		commandBufferPair.second->SetName(createInformation.Name);
		commandBufferPair.second->Begin();

		return commandBufferPair.second;
	}

	return Create(createInformation);
}

TShared<GpuCommandBuffer> D3D12GpuCommandBufferPool::Create(const GpuCommandBufferCreateInformation& createInformation)
{
	EnsureValidThread();

	D3D12GpuDevice& d3d12Device = static_cast<D3D12GpuDevice&>(mGpuDevice);

	const D3D12_COMMAND_LIST_TYPE commandListType = GetCommandListType(mInformation.Type);

	// Node mask 0 (single GPU), no initial pipeline state
	ComPtr<ID3D12GraphicsCommandList> baseCommandList;
	HRESULT hr = d3d12Device.GetD3D12Device()->CreateCommandList(0, commandListType, mCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&baseCommandList));
	if(!B3D_ENSURE_LOG(SUCCEEDED(hr), "D3D12: Failed to create a command list (hr={0}).", (u32)hr))
		return nullptr;

	ComPtr<ID3D12GraphicsCommandList7> commandList;
	hr = baseCommandList.As(&commandList);
	if(!B3D_ENSURE_LOG(SUCCEEDED(hr), "D3D12: ID3D12GraphicsCommandList7 is unavailable (hr={0}).", (u32)hr))
		return nullptr;

	// Command lists are created in recording state, close it for now
	hr = commandList->Close();
	if(!B3D_ENSURE_LOG(SUCCEEDED(hr), "D3D12: Failed to close a newly created command list (hr={0}).", (u32)hr))
		return nullptr;

	TShared<D3D12GpuCommandBuffer> commandBuffer = B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuCommandBuffer>())
		D3D12GpuCommandBuffer(d3d12Device, *this, mNextCommandBufferId++, commandList.Get(), mInformation.Thread, mInformation.Type, createInformation),
		[](D3D12GpuCommandBuffer* commandBuffer)
		{
			B3DDelete(commandBuffer);
		});

	mCommandBuffers[commandBuffer->GetId()] = commandBuffer;

	commandBuffer->SetShared(commandBuffer);
	commandBuffer->Begin();

	return commandBuffer;
}

void D3D12GpuCommandBufferPool::Reset()
{
	EnsureValidThread();

	for(const auto& commandBufferPair : mCommandBuffers)
	{
		const GpuCommandBufferState state = commandBufferPair.second->GetState();
		if(!B3D_ENSURE(state == GpuCommandBufferState::Ready || state == GpuCommandBufferState::Done || state == GpuCommandBufferState::RecordingDone))
			return;
	}

	for(const auto& commandBufferPair : mCommandBuffers)
	{
		if(commandBufferPair.second->GetState() != GpuCommandBufferState::Ready)
			commandBufferPair.second->Reset();
	}

	HRESULT hr = mCommandAllocator->Reset();
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to reset command allocator");
}

D3D12GpuCommandBuffer::D3D12GpuCommandBuffer(D3D12GpuDevice& device, D3D12GpuCommandBufferPool& pool, u32 id, ID3D12GraphicsCommandList7* commandList, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation) : GpuCommandBuffer(device, ownerThread, queueType, createInformation), mId(id), mCommandList(commandList), mPool(pool), mBarrierHelper(&mResourceTracker, queueType), mGraphicsPipelineRequiresBind(true), mGraphicsRootSignatureRequiresBind(true), mComputePipelineRequiresBind(true), mPrimitiveTopologyRequiresBind(true), mViewportRequiresBind(true), mStencilReferenceValueRequiresBind(true), mScissorRequiresBind(true), mGraphicsParametersRequireBind(false), mComputeParametersRequireBind(false), mVertexInputsDirty(false)
{
	HRESULT hr = GetD3D12GpuDevice().GetD3D12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to create fence");

	// Incremented by the queue just before each submit's fence signal, so it always identifies the latest submission
	mFenceValue = 0;

	SetName(createInformation.Name);
}

D3D12GpuCommandBuffer::~D3D12GpuCommandBuffer()
{
	if (IsRecording())
	{
		End();
		Reset();
	}
	else if (mState == GpuCommandBufferState::RecordingDone)
	{
		// Recorded but never submitted - release the tracked resources without a NotifyUsed/NotifyDone cycle.
		Reset();
	}

	if (mState == GpuCommandBufferState::Executing || mState == GpuCommandBufferState::Done)
	{
		// Wait for command buffer to finish
		if (mFence->GetCompletedValue() < mFenceValue)
			WaitForFenceValue(mFence.Get(), mFenceValue, 1000); // Give up after one second

		Reset();
	}

	mCommandList.Reset();
	mFence.Reset();
}

void D3D12GpuCommandBuffer::Begin()
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Ready);

	HRESULT hr = mCommandList->Reset(mPool.GetD3D12CommandAllocator(), nullptr);
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to reset command list");

	if (GetQueueType() != GQT_TRANSFER)
	{
		D3D12DescriptorManager& descriptorManager = GetD3D12GpuDevice().GetDescriptorManager();
		ID3D12DescriptorHeap* descriptorHeaps[] = {
			descriptorManager.GetDescriptorHeap(D3D12DescriptorHeapType::CBV_SRV_UAV),
			descriptorManager.GetDescriptorHeap(D3D12DescriptorHeapType::Sampler)
		};

		mCommandList->SetDescriptorHeaps((UINT)std::size(descriptorHeaps), descriptorHeaps);
	}

	mState = GpuCommandBufferState::Recording;

	mLastBoundGraphicsPipeline = nullptr;
	mRequiredVertexBufferBindingCount = 0;
	mGraphicsPipelineRequiresBind = true;
	mGraphicsRootSignatureRequiresBind = true;
	mComputePipelineRequiresBind = true;
	mPrimitiveTopologyRequiresBind = true;
	mViewportRequiresBind = true;
	mStencilReferenceValueRequiresBind = true;
	mScissorRequiresBind = true;
	mGraphicsParametersRequireBind = false;
	mComputeParametersRequireBind = false;
	mVertexInputsDirty = false;
}

void D3D12GpuCommandBuffer::End()
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass);

	// End render pass if active
	if (mState == GpuCommandBufferState::RecordingRenderPass)
		EndRenderPass();

	// Copy the results of every query written during this recording into its pool's readback buffer. The pools are
	// notified so their results become readable once this command buffer completes on the GPU.
	for (const auto& usedQueryPool : mUsedQueryPools)
	{
		D3D12GpuQueryPool* d3d12QueryPool = static_cast<D3D12GpuQueryPool*>(usedQueryPool.get());
		const u32 allocatedQueryCount = d3d12QueryPool->GetAllocatedQueryCount();
		if (allocatedQueryCount == 0)
			continue;

		mCommandList->ResolveQueryData(d3d12QueryPool->GetD3D12QueryHeap(), d3d12QueryPool->GetD3D12QueryType(), 0, allocatedQueryCount, d3d12QueryPool->GetReadbackBuffer(), 0);
		d3d12QueryPool->NotifyResolveScheduled(*this);
	}

	HRESULT hr = mCommandList->Close();
	if(FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to close command list '{0}' (hr={1}).", mName, (u32)hr);
		GetD3D12GpuDevice().LogDebugLayerMessages();
	}
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to close command list");

	mRenderTarget = nullptr;
	mState = GpuCommandBufferState::RecordingDone;
}

void D3D12GpuCommandBuffer::SetName(const StringView& name)
{
	EnsureValidThread();

	GpuCommandBuffer::SetName(name);

	if (mCommandList)
	{
		const WString wideName(name.begin(), name.end());
		mCommandList->SetName(wideName.c_str());
	}
}

void D3D12GpuCommandBuffer::SetGpuParameterSet(const TShared<GpuParameterSet>& parameters)
{
	EnsureValidThread();

	if (!B3D_ENSURE(parameters != nullptr))
		return;

	const u32 set = parameters->GetSet();
	if (set >= (u32)mBoundParameterSets.size())
		mBoundParameterSets.resize(set + 1);

	mBoundParameterSets[set] = std::static_pointer_cast<D3D12GpuParameters>(parameters);
	mGraphicsParametersRequireBind = true;
	mComputeParametersRequireBind = true;

	// Freshly bound parameters carry their own offsets; overrides only apply on top of an already-bound set
	if (set < (u32)mDynamicOffsetOverridesPerSet.size())
		mDynamicOffsetOverridesPerSet[set].clear();
}

void D3D12GpuCommandBuffer::SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset)
{
	EnsureValidThread();

	if (set >= (u32)mDynamicOffsetOverridesPerSet.size())
		mDynamicOffsetOverridesPerSet.resize(set + 1);

	UnorderedMap<u32, u32>& dynamicOffsetOverrides = mDynamicOffsetOverridesPerSet[set];
	const auto dynamicOffsetIterator = dynamicOffsetOverrides.find(bufferIndex);
	if(dynamicOffsetIterator != dynamicOffsetOverrides.end() && dynamicOffsetIterator->second == offset)
		return;

	dynamicOffsetOverrides[bufferIndex] = offset;
	mGraphicsParametersRequireBind = true;
	mComputeParametersRequireBind = true;
}

void D3D12GpuCommandBuffer::SetGpuGraphicsPipelineState(const TShared<GpuGraphicsPipelineState>& pipelineState)
{
	EnsureValidThread();

	if(mGraphicsPipeline == pipelineState)
		return;

	mGraphicsPipeline = std::static_pointer_cast<D3D12GpuGraphicsPipelineState>(pipelineState);
	mGraphicsPipelineRequiresBind = true;
	mGraphicsRootSignatureRequiresBind = true;
	mVertexInputsDirty = true;
}

void D3D12GpuCommandBuffer::SetGpuComputePipelineState(const TShared<GpuComputePipelineState>& pipelineState)
{
	EnsureValidThread();

	if(mComputePipeline == pipelineState)
		return;

	mComputePipeline = std::static_pointer_cast<D3D12GpuComputePipelineState>(pipelineState);
	mComputePipelineRequiresBind = true;
}

void D3D12GpuCommandBuffer::SetVertexBuffers(u32 index, TShared<GpuBuffer>* buffers, u32 bufferCount)
{
	EnsureValidThread();

	const u32 endIndex = index + bufferCount;
	bool hasChanged = mVertexBuffers.size() < endIndex;
	if (mVertexBuffers.size() < endIndex)
		mVertexBuffers.resize(endIndex);

	for (u32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
	{
		const TShared<D3D12GpuBuffer> buffer = std::static_pointer_cast<D3D12GpuBuffer>(buffers[bufferIndex]);
		if(mVertexBuffers[index + bufferIndex] == buffer)
			continue;

		mVertexBuffers[index + bufferIndex] = buffer;
		hasChanged = true;
	}

	if(!hasChanged)
		return;

	mVertexInputsDirty = true;
}

void D3D12GpuCommandBuffer::SetIndexBuffer(const TShared<GpuBuffer>& buffer)
{
	EnsureValidThread();

	const TShared<D3D12GpuBuffer> d3d12Buffer = std::static_pointer_cast<D3D12GpuBuffer>(buffer);
	if(mIndexBuffer == d3d12Buffer)
		return;

	mIndexBuffer = d3d12Buffer;
	mVertexInputsDirty = true;
}

void D3D12GpuCommandBuffer::SetVertexDescription(const TShared<VertexDescription>& vertexDescription)
{
	EnsureValidThread();

	if(mVertexDescription == vertexDescription)
		return;

	mVertexDescription = vertexDescription;
	mGraphicsPipelineRequiresBind = true;
	mVertexInputsDirty = true;
}

void D3D12GpuCommandBuffer::SetDrawOperation(DrawOperationType operation)
{
	EnsureValidThread();

	if(mDrawOperation == operation)
		return;

	mDrawOperation = operation;
	mGraphicsPipelineRequiresBind = true;
	mPrimitiveTopologyRequiresBind = true;
}

void D3D12GpuCommandBuffer::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	EnsureValidThread();

	if(!B3D_ENSURE(IsInRenderPass()))
		return;

	if (!IsReadyForRender())
		return;

	if(!BindGraphicsPipeline())
		return;

	BindDynamicStates(false);
	BindVertexInputs();
	BindGpuParameterSets(true);

	// Barriers accumulated by the bind-time tracking above. Parameter sets are normally pre-registered at BeginRenderPass so this is usually empty.
	mBarrierHelper.Execute(*this);

	if (instanceCount == 0)
		instanceCount = 1;

	mCommandList->DrawInstanced(vertexCount, instanceCount, vertexOffset, firstInstance);
}

void D3D12GpuCommandBuffer::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	EnsureValidThread();

	if(!B3D_ENSURE(IsInRenderPass()))
		return;

	if (!IsReadyForRender())
		return;

	if(!BindGraphicsPipeline())
		return;

	BindDynamicStates(false);
	BindVertexInputs();
	BindGpuParameterSets(true);

	// See Draw()
	mBarrierHelper.Execute(*this);

	if (instanceCount == 0)
		instanceCount = 1;

	mCommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, vertexOffset, firstInstance);
}

void D3D12GpuCommandBuffer::DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
{
	EnsureValidThread();

	if(!B3D_ENSURE(!IsInRenderPass()))
		return;

	if (!mComputePipeline)
		return;

	if (mComputePipelineRequiresBind)
	{
		D3D12Pipeline* pipeline = mComputePipeline->GetD3D12PipelineState();
		D3D12RootSignature* rootSignature = mComputePipeline->GetRootSignature();
		if(pipeline == nullptr || rootSignature == nullptr)
			return;

		mResourceTracker.TrackResourceUsage(pipeline, GpuAccessFlag::Read);
		mResourceTracker.TrackResourceUsage(rootSignature, GpuAccessFlag::Read);

		mCommandList->SetPipelineState(pipeline->Get());
		mCommandList->SetComputeRootSignature(rootSignature->Get());

		mComputePipelineRequiresBind = false;
		mComputeParametersRequireBind = true;
	}

	if (mComputeParametersRequireBind)
		BindGpuParameterSets(false);
	else
		TrackGpuParameterSets(false);

	mBarrierHelper.Execute(*this);

	mCommandList->Dispatch(groupCountX, groupCountY, groupCountZ);

	mResourceTracker.ClearShaderFlagsForAllRenderPassImageSubresources();
}

void D3D12GpuCommandBuffer::BeginRenderPass(const RenderPassCreateInformation& createInformation)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording);

	const TShared<RenderTarget>& target = createInformation.Target;
	if (!B3D_ENSURE(target != nullptr))
		return;

	mRenderTarget = target;
	mRenderTargetReadOnlyMask = createInformation.ReadOnlyMask;
	mFramebuffer = nullptr;
	mGraphicsPipelineRequiresBind = true;

	// Get framebuffer for the render target
	D3D12SwapChain* swapChain = nullptr;
	if (target->GetProperties().IsWindow)
	{
		RenderWindow* renderWindow = static_cast<RenderWindow*>(target.get());
		const TShared<IRenderWindowSurface>& surface = renderWindow->GetRenderWindowSurface();

		if (surface != nullptr)
		{
			ID3D12RenderWindowSurface* d3d12Surface = static_cast<ID3D12RenderWindowSurface*>(surface.get());
			if (!d3d12Surface->IsSwapChainValid())
				renderWindow->RebuildSwapChain();

			mFramebuffer = d3d12Surface->GetActiveFramebuffer();
			swapChain = d3d12Surface->GetSwapChain();
		}
	}
	else
	{
		// RenderTexture owns its framebuffer
		D3D12RenderTexture* renderTexture = static_cast<D3D12RenderTexture*>(target.get());
		mFramebuffer = renderTexture->GetFramebuffer();
	}

	mState = GpuCommandBufferState::RecordingRenderPass;

	// The normalized viewport must be resolved again for the new render target.
	mViewportRequiresBind = true;

	// Disabled scissoring is emulated with a full-viewport rectangle, which depends on the render-target size.
	if (!mIsScissorTestEnabled)
		mScissorRequiresBind = true;

	D3D12RenderPassResourceUsage renderPassResourceUsage;
	if(mFramebuffer != nullptr)
		renderPassResourceUsage = D3D12RenderPassResourceUsage(mFramebuffer->GetAttachments(), mFramebuffer->GetAttachmentCount(), mRenderTargetReadOnlyMask);

	// Register parameter resources once, collecting shader reads that overlap attachments before their transitions are resolved.
	for (const TShared<GpuParameterSet>& parameterSet : createInformation.Parameters)
	{
		if (parameterSet == nullptr)
			continue;

		const TShared<GpuPipelineParameterSetLayout>& setLayout = parameterSet->GetLayout();
		if (setLayout != nullptr)
			static_cast<D3D12GpuParameters*>(parameterSet.get())->TrackBoundResources(mResourceTracker, mBarrierHelper, *setLayout, &renderPassResourceUsage);
	}

	if(mFramebuffer != nullptr)
		mResourceTracker.TrackRenderTargetUsage(renderPassResourceUsage, mBarrierHelper);

	if (swapChain != nullptr)
		mResourceTracker.TrackSwapChainUsage(swapChain);

	mBarrierHelper.Execute(*this);

	// Set render targets if framebuffer exists
	if (mFramebuffer)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles = mFramebuffer->GetRenderTargetViews();
		const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = mFramebuffer->GetDepthStencilView(mRenderTargetReadOnlyMask);
		const u32 renderTargetCount = mFramebuffer->GetColorAttachmentCount();

		mCommandList->OMSetRenderTargets(renderTargetCount, rtvHandles, FALSE, dsvHandle);
	}

	// Apply clear operations requested for the render pass start
	if (createInformation.ClearMask != RT_NONE)
		ClearViewportArea(GetRenderPassArea(), createInformation.ClearMask);
}

void D3D12GpuCommandBuffer::SetViewport(const Area2& area)
{
	EnsureValidThread();

	if(mNormalizedViewportArea == area)
		return;

	mNormalizedViewportArea = area;
	mViewportRequiresBind = true;

	// D3D12 has no scissor-test disable: the disabled state is emulated with a full-viewport scissor rect,
	// so the stamped rect must follow every viewport change or it stays sized for the previous target.
	if (!mIsScissorTestEnabled)
		mScissorRequiresBind = true;
}

void D3D12GpuCommandBuffer::ClearRenderTarget(RenderSurfaceMask mask)
{
	EnsureValidThread();

	ClearViewportArea(GetRenderPassArea(), mask);
}

void D3D12GpuCommandBuffer::ClearViewport(RenderSurfaceMask mask)
{
	EnsureValidThread();

	ClearViewportArea(GetViewportArea(), mask);
}

void D3D12GpuCommandBuffer::ClearViewportArea(const Area2I& area, RenderSurfaceMask mask)
{
	if (!mFramebuffer || !mRenderTarget)
		return;

	const RenderTargetClearValues& clearValues = mRenderTarget->GetClearValues();

	D3D12_RECT rect;
	rect.left = area.X;
	rect.top = area.Y;
	rect.right = area.X + area.Width;
	rect.bottom = area.Y + area.Height;

	// Clear color attachments
	const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles = mFramebuffer->GetRenderTargetViews();
	const u32 renderTargetCount = mFramebuffer->GetColorAttachmentCount();

	for (u32 renderTargetIndex = 0; renderTargetIndex < renderTargetCount; renderTargetIndex++)
	{
		const u32 colorIndex = mFramebuffer->GetColorAttachment(renderTargetIndex).ColorIndex;
		if (!mask.IsSet((RenderSurfaceMaskBits)(RT_COLOR0 << colorIndex)))
			continue;

		const Color& color = clearValues.Colors[colorIndex];
		const float clearColor[4] = { color.R, color.G, color.B, color.A };

		mCommandList->ClearRenderTargetView(rtvHandles[renderTargetIndex], clearColor, 1, &rect);
	}

	// Clear depth/stencil attachment
	const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = mFramebuffer->GetDepthStencilView(mRenderTargetReadOnlyMask);
	if (dsvHandle && (mask.IsSet(RT_DEPTH) || mask.IsSet(RT_STENCIL)))
	{
		D3D12_CLEAR_FLAGS clearFlags = (D3D12_CLEAR_FLAGS)0;
		if (mask.IsSet(RT_DEPTH))
			clearFlags |= D3D12_CLEAR_FLAG_DEPTH;

		if (mask.IsSet(RT_STENCIL))
			clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

		mCommandList->ClearDepthStencilView(*dsvHandle, clearFlags, clearValues.Depth, clearValues.Stencil, 1, &rect);
	}
}

void D3D12GpuCommandBuffer::EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom)
{
	EnsureValidThread();

	const Area2I area(left, top, right - left, bottom - top);
	if(mIsScissorTestEnabled && mScissor == area)
		return;

	mScissor = area;
	mIsScissorTestEnabled = true;
	mScissorRequiresBind = true;
}

void D3D12GpuCommandBuffer::DisableScissorTest()
{
	EnsureValidThread();

	if(!mIsScissorTestEnabled)
		return;

	mIsScissorTestEnabled = false;
	mScissorRequiresBind = true;
}

void D3D12GpuCommandBuffer::SetStencilReferenceValue(u32 value)
{
	EnsureValidThread();

	if(mStencilReferenceValue == value)
		return;

	mStencilReferenceValue = value;
	mStencilReferenceValueRequiresBind = true;
}

void D3D12GpuCommandBuffer::WriteTimestamp(GpuQueryId query, const TShared<GpuQueryPool>& queryPool)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass);

	if (!query.IsValid() || !queryPool)
	{
		B3D_LOG(Error, LogRenderBackend, "Invalid query or query pool");
		return;
	}

	D3D12GpuQueryPool* d3d12QueryPool = static_cast<D3D12GpuQueryPool*>(queryPool.get());

	if (d3d12QueryPool->GetQueryType() != GpuQueryType::Timestamp)
	{
		B3D_LOG(Error, LogRenderBackend, "Query pool is not a timestamp query pool");
		return;
	}

	// EndQuery for timestamp queries records the current GPU timestamp
	mCommandList->EndQuery(d3d12QueryPool->GetD3D12QueryHeap(), d3d12QueryPool->GetD3D12QueryType(), query.Id);

	TrackQueryPool(queryPool);
}

void D3D12GpuCommandBuffer::BeginQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool, GpuQueryFlags flags)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass);

	if (!query.IsValid() || !queryPool)
	{
		B3D_LOG(Error, LogRenderBackend, "Invalid query or query pool");
		return;
	}

	D3D12GpuQueryPool* d3d12QueryPool = static_cast<D3D12GpuQueryPool*>(queryPool.get());

	if (d3d12QueryPool->GetQueryType() == GpuQueryType::Timestamp)
	{
		B3D_LOG(Error, LogRenderBackend, "Timestamp queries don't support BeginQuery, use WriteTimestamp instead");
		return;
	}

	d3d12QueryPool->SelectOcclusionQueryType(flags);

	mCommandList->BeginQuery(d3d12QueryPool->GetD3D12QueryHeap(), d3d12QueryPool->GetD3D12QueryType(), query.Id);

	TrackQueryPool(queryPool);
}

void D3D12GpuCommandBuffer::EndQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass);

	if (!query.IsValid() || !queryPool)
	{
		B3D_LOG(Error, LogRenderBackend, "Invalid query or query pool");
		return;
	}

	D3D12GpuQueryPool* d3d12QueryPool = static_cast<D3D12GpuQueryPool*>(queryPool.get());

	if (d3d12QueryPool->GetQueryType() == GpuQueryType::Timestamp)
	{
		B3D_LOG(Error, LogRenderBackend, "Timestamp queries don't support EndQuery, use WriteTimestamp instead");
		return;
	}

	mCommandList->EndQuery(d3d12QueryPool->GetD3D12QueryHeap(), d3d12QueryPool->GetD3D12QueryType(), query.Id);
}

void D3D12GpuCommandBuffer::ResetQueries(const TShared<GpuQueryPool>& queryPool)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass);

	if (!queryPool)
	{
		B3D_LOG(Error, LogRenderBackend, "Invalid query pool");
		return;
	}

	// D3D12 query heap entries don't require a GPU-side reset (writing a query simply overwrites the entry), so
	// resetting only restarts the pool's allocator per the GpuCommandBuffer contract.
	static_cast<D3D12GpuQueryPool*>(queryPool.get())->Reset();
}

void D3D12GpuCommandBuffer::TrackQueryPool(const TShared<GpuQueryPool>& queryPool)
{
	for (const auto& usedQueryPool : mUsedQueryPools)
	{
		if (usedQueryPool == queryPool)
			return;
	}

	mUsedQueryPools.push_back(queryPool);
}

namespace
{
	/** Value of PIX_EVENT_ANSI_VERSION, as expected by ID3D12GraphicsCommandList Begin/SetMarker metadata. */
	constexpr UINT kPixEventAnsiVersion = 1;
}

void D3D12GpuCommandBuffer::BeginLabel(const StringView& name)
{
	EnsureValidThread();

#if B3D_BUILD_TYPE_DEVELOPMENT
	// ANSI event marker, understood by PIX/RenderDoc without requiring the PIX event runtime
	const String eventName(name.data(), name.size());
	mCommandList->BeginEvent(kPixEventAnsiVersion, eventName.c_str(), (UINT)eventName.size() + 1);
#endif
}

void D3D12GpuCommandBuffer::EndLabel()
{
	EnsureValidThread();

#if B3D_BUILD_TYPE_DEVELOPMENT
	mCommandList->EndEvent();
#endif
}

void D3D12GpuCommandBuffer::InsertLabel(const StringView& name)
{
	EnsureValidThread();

#if B3D_BUILD_TYPE_DEVELOPMENT
	const String eventName(name.data(), name.size());
	mCommandList->SetMarker(kPixEventAnsiVersion, eventName.c_str(), (UINT)eventName.size() + 1);
#endif
}

void D3D12GpuCommandBuffer::EndRenderPass()
{
	EnsureValidThread();

	if (mState != GpuCommandBufferState::RecordingRenderPass)
		return;

	if (mFramebuffer != nullptr)
	{
		// Swap-chain back buffers must be in PRESENT (COMMON) state by the time the queue presents; there is no
		// other point in the frame where a transition can be recorded, so it happens as the pass ends (Vulkan
		// analog: finalLayout = PRESENT_SRC on window render passes).
		if (mRenderTarget != nullptr && mRenderTarget->GetProperties().IsWindow)
		{
			const u32 colorAttachmentCount = mFramebuffer->GetColorAttachmentCount();
			for (u32 colorIndex = 0; colorIndex < colorAttachmentCount; colorIndex++)
			{
				const D3D12FramebufferAttachment& attachment = mFramebuffer->GetColorAttachment(colorIndex);
				if (attachment.Image == nullptr)
					continue;

				mResourceTracker.TrackImageUsage(attachment.Image, attachment.Image->GetRange(attachment.Surface), GpuImageLayout::Present, GpuImageLayout::Present, GpuResourceUseFlag::ColorAttachment, GpuAccessFlag::Read, mBarrierHelper);
			}

			mBarrierHelper.Execute(*this);
		}

		// Reset the per-pass shader/attachment usage flags
		// needs no final-layout move: attachments stay in their in-pass states until the next transition.
		mResourceTracker.ClearShaderFlagsForAllRenderPassImageSubresources();

		for (u32 attachmentIndex = 0; attachmentIndex < mFramebuffer->GetAttachmentCount(); attachmentIndex++)
			mResourceTracker.ClearRenderTargetFlagsForImage(mFramebuffer->GetAttachments()[attachmentIndex].Image);
	}

	mState = GpuCommandBufferState::Recording;
}

bool D3D12GpuCommandBuffer::IsReadyForRender() const
{
	if (!mGraphicsPipeline)
		return false;

	if (mGraphicsPipeline->GetInputDeclaration() == nullptr)
		return false;

	return mRenderTarget != nullptr && mVertexDescription != nullptr;
}

bool D3D12GpuCommandBuffer::BindGraphicsPipeline()
{
	if (!mGraphicsPipeline || !mFramebuffer)
		return false;

	if(mGraphicsPipelineRequiresBind)
	{
		// Map the bound vertex buffer layout to the vertex shader's inputs; the result is part of the pipeline
		// variant and tells us how many vertex buffer slots the pipeline fetches from
		const TShared<D3D12VertexInput> vertexInput = D3D12VertexInputManager::Instance().GetVertexInput(mVertexDescription, mGraphicsPipeline->GetInputDeclaration());
		if (vertexInput == nullptr)
			return false;

		if (mRequiredVertexBufferBindingCount != vertexInput->GetVertexBufferBindingCount())
		{
			mRequiredVertexBufferBindingCount = vertexInput->GetVertexBufferBindingCount();
			mVertexInputsDirty = true;
		}

		// Resolve the pipeline variant matching the current framebuffer formats, vertex input and draw operation.
		// Variants are cached by the pipeline state object, so this is a lookup on all but the first encounter.
		D3D12PipelineVariantKey variantKey;
		variantKey.RenderTargetCount = mFramebuffer->GetColorAttachmentCount();
		for (u32 renderTargetIndex = 0; renderTargetIndex < variantKey.RenderTargetCount; renderTargetIndex++)
			variantKey.RenderTargetFormats[renderTargetIndex] = mFramebuffer->GetColorFormat(renderTargetIndex);
		variantKey.DepthStencilFormat = mFramebuffer->GetDepthStencilFormat();
		variantKey.SampleCount = mFramebuffer->GetSampleCount();
		variantKey.TopologyType = D3D12Utility::GetPrimitiveTopologyType(mDrawOperation);
		variantKey.VertexInputId = vertexInput->GetId();

		D3D12Pipeline* pipeline = mGraphicsPipeline->FindOrCreatePipeline(variantKey, *vertexInput);
		if (!pipeline)
			return false;

		if (pipeline != mLastBoundGraphicsPipeline)
		{
			mResourceTracker.TrackResourceUsage(pipeline, GpuAccessFlag::Read);

			mCommandList->SetPipelineState(pipeline->Get());
			mLastBoundGraphicsPipeline = pipeline;
		}

		mGraphicsPipelineRequiresBind = false;
	}

	if (mGraphicsRootSignatureRequiresBind)
	{
		D3D12RootSignature* rootSignature = mGraphicsPipeline->GetRootSignature();
		if(rootSignature == nullptr)
			return false;

		mResourceTracker.TrackResourceUsage(rootSignature, GpuAccessFlag::Read);
		mCommandList->SetGraphicsRootSignature(rootSignature->Get());

		mGraphicsRootSignatureRequiresBind = false;

		// Setting a root signature wipes all of the command list's graphics root arguments; re-record them on the next parameter bind
		mGraphicsParametersRequireBind = true;
	}

	if(mPrimitiveTopologyRequiresBind)
	{
		mCommandList->IASetPrimitiveTopology(D3D12Utility::GetPrimitiveTopology(mDrawOperation));
		mPrimitiveTopologyRequiresBind = false;
	}

	return true;
}

void D3D12GpuCommandBuffer::BindDynamicStates(bool forceAll)
{
	if (mViewportRequiresBind || forceAll)
	{
		const Area2I viewportArea = GetViewportArea();

		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = (FLOAT)viewportArea.X;
		viewport.TopLeftY = (FLOAT)viewportArea.Y;
		viewport.Width = (FLOAT)viewportArea.Width;
		viewport.Height = (FLOAT)viewportArea.Height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		mCommandList->RSSetViewports(1, &viewport);
		mViewportRequiresBind = false;
	}

	// Bind scissor rect. D3D12 has no scissor-test disable, so the disabled state is emulated with a rect covering the entire viewport.
	if (mScissorRequiresBind || forceAll)
	{
		const Area2I scissorArea = mIsScissorTestEnabled ? mScissor : GetViewportArea();

		D3D12_RECT scissorRect;
		scissorRect.left = scissorArea.X;
		scissorRect.top = scissorArea.Y;
		scissorRect.right = scissorArea.X + scissorArea.Width;
		scissorRect.bottom = scissorArea.Y + scissorArea.Height;

		mCommandList->RSSetScissorRects(1, &scissorRect);
		mScissorRequiresBind = false;
	}

	if (mStencilReferenceValueRequiresBind || forceAll)
	{
		mCommandList->OMSetStencilRef(mStencilReferenceValue);
		mStencilReferenceValueRequiresBind = false;
	}
}

void D3D12GpuCommandBuffer::BindVertexInputs()
{
	if (!mVertexInputsDirty)
		return;

	if (mRequiredVertexBufferBindingCount > 0)
	{
		TInlineArray<D3D12_VERTEX_BUFFER_VIEW, B3D_MAX_BOUND_VERTEX_BUFFERS> vertexBufferViews;
		vertexBufferViews.Resize(mRequiredVertexBufferBindingCount);

		for (u32 slot = 0; slot < mRequiredVertexBufferBindingCount; slot++)
		{
			if (slot >= (u32)mVertexBuffers.size() || mVertexBuffers[slot] == nullptr)
				continue;

			const TShared<D3D12GpuBuffer>& buffer = mVertexBuffers[slot];
			mResourceTracker.TrackBufferUsage(buffer->GetD3D12Buffer(), GpuResourceUseFlag::VertexBuffer, GpuAccessFlag::Read, mBarrierHelper);
			vertexBufferViews[slot] = buffer->GetVertexBufferView();
		}

		// Empty views read zero in the shader
		mCommandList->IASetVertexBuffers(0, (UINT)vertexBufferViews.Size(), vertexBufferViews.Data());
	}

	if (mIndexBuffer)
	{
		mResourceTracker.TrackBufferUsage(mIndexBuffer->GetD3D12Buffer(), GpuResourceUseFlag::IndexBuffer, GpuAccessFlag::Read, mBarrierHelper);
		mCommandList->IASetIndexBuffer(&mIndexBuffer->GetIndexBufferView());
	}
	else
		mCommandList->IASetIndexBuffer(nullptr);

	mVertexInputsDirty = false;
}

void D3D12GpuCommandBuffer::BindGpuParameterSets(bool isGraphics)
{
	const bool requiresBind = isGraphics ? mGraphicsParametersRequireBind : mComputeParametersRequireBind;
	if (!requiresBind || mBoundParameterSets.empty())
		return;

	const D3D12GpuPipelineParameterLayout* parameterLayout = nullptr;
	if (isGraphics)
		parameterLayout = mGraphicsPipeline != nullptr ? mGraphicsPipeline->GetD3D12ParameterLayout() : nullptr;
	else
		parameterLayout = mComputePipeline != nullptr ? mComputePipeline->GetD3D12ParameterLayout() : nullptr;

	if (parameterLayout == nullptr)
		return;

	D3D12GpuDevice& device = GetD3D12GpuDevice();

	const u32 layoutSetCount = parameterLayout->GetSetCount();
	for (u32 setIndex = 0; setIndex < (u32)mBoundParameterSets.size(); setIndex++)
	{
		const TShared<D3D12GpuParameters>& parameters = mBoundParameterSets[setIndex];
		if (parameters == nullptr)
			continue;

		// Sets beyond the active pipeline's layout can linger from earlier binds under a different pipeline, the current root signature has no parameters for them
		if (setIndex >= layoutSetCount)
			continue;

		const bool hasDynamicOffsetOverrides = setIndex < (u32)mDynamicOffsetOverridesPerSet.size() && !mDynamicOffsetOverridesPerSet[setIndex].empty();
		const UnorderedMap<u32, u32>* dynamicOffsets = hasDynamicOffsetOverrides ? &mDynamicOffsetOverridesPerSet[setIndex] : nullptr;

		const TShared<GpuPipelineParameterSetLayout> pipelineSetLayout = parameterLayout->GetSet(setIndex);
		if (pipelineSetLayout == nullptr)
			continue;

		parameters->TrackBoundResources(mResourceTracker, mBarrierHelper, *pipelineSetLayout);
		parameters->BindDescriptors(device, mResourceTracker, mCommandList.Get(), isGraphics, parameterLayout->GetDescriptorSetLayout(setIndex), dynamicOffsets);
	}

	if (isGraphics)
		mGraphicsParametersRequireBind = false;
	else
		mComputeParametersRequireBind = false;
}

void D3D12GpuCommandBuffer::TrackGpuParameterSets(bool isGraphics)
{
	const D3D12GpuPipelineParameterLayout* parameterLayout = nullptr;
	if (isGraphics)
		parameterLayout = mGraphicsPipeline != nullptr ? mGraphicsPipeline->GetD3D12ParameterLayout() : nullptr;
	else
		parameterLayout = mComputePipeline != nullptr ? mComputePipeline->GetD3D12ParameterLayout() : nullptr;

	if (parameterLayout == nullptr)
		return;

	const u32 setCount = parameterLayout->GetSetCount();
	for (u32 setIndex = 0; setIndex < (u32)mBoundParameterSets.size() && setIndex < setCount; setIndex++)
	{
		const TShared<D3D12GpuParameters>& parameters = mBoundParameterSets[setIndex];
		if (parameters == nullptr)
			continue;

		const TShared<GpuPipelineParameterSetLayout> pipelineSetLayout = parameterLayout->GetSet(setIndex);
		if (pipelineSetLayout != nullptr)
			parameters->TrackBoundResources(mResourceTracker, mBarrierHelper, *pipelineSetLayout);
	}
}

namespace
{
	/** Determines transitions required in-between ExecuteCommandList calls, as well as waits required between queues. */
	class D3D12SubmissionTransitionVisitor : public GpuSubmissionTransitionVisitor
	{
	public:
		D3D12SubmissionTransitionVisitor(D3D12GpuDevice& device, GpuQueueId destinationQueueId, D3D12GpuCommandBufferSubmitInformation& submitInformation) : mDevice(device), mDestinationQueueId(destinationQueueId), mSubmitInformation(submitInformation)
		{ }

		void VisitBuffer(const GpuSubmissionBufferTransition& transition) override
		{
			AddParallelAccessWait(transition);
		}

		void VisitImage(const GpuSubmissionImageTransition& transition) override
		{
			// Resolve the native layouts at the command buffer boundaries.
			D3D12Image* const image = static_cast<D3D12Image*>(transition.Image);
			D3D12ImageSubresource* const subresource = static_cast<D3D12ImageSubresource*>(transition.StateResource);

			const GpuQueueType destinationQueueType = mDestinationQueueId.GetType();
			const D3D12TextureLayout committedLayout = subresource->GetLayout();

			auto fnResolveLayout = [image, destinationQueueType, aspects = transition.ImageRange.AspectMask](GpuImageLayout logicalLayout, const D3D12TextureLayout& fallback)
			{
				if(logicalLayout == GpuImageLayout::Undefined)
					return fallback;

				const D3D12TextureLayout translatedLayout = image->GetTextureLayout(logicalLayout, destinationQueueType);
				return D3D12TextureLayout(translatedLayout.GetLayout(aspects));
			};

			const D3D12TextureLayout initialLayout = fnResolveLayout(transition.InitialLayout, committedLayout);
			const D3D12TextureLayout finalLayout = fnResolveLayout(transition.FinalLayout, initialLayout);

			if(!B3D_ENSURE_LOG(D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(initialLayout, transition.ImageRange.AspectMask, destinationQueueType), "D3D12 image layout is not supported on destination queue type {0}.", (u32)destinationQueueType))
			{
				mSubmitInformation.RequiredWaitMask |= transition.ParallelAccessWaitMask;
				return;
			}

			// Select a queue capable of establishing the initial layout.
			D3D12TextureLayout transitionSourceLayout = committedLayout;
			GpuQueueId layoutTransitionQueueId;
			const bool hasLayoutTransitionQueue = subresource->GetLayoutTransitionQueueId(layoutTransitionQueueId);
			bool layoutTransitionQueueChanged = false;

			const bool needsLayoutTransition = committedLayout != initialLayout;
			const bool destinationCanTransitionLayout =
				D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(committedLayout, transition.ImageRange.AspectMask, destinationQueueType) &&
				D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(initialLayout, transition.ImageRange.AspectMask, destinationQueueType);

			// Transition layout on the source queue if the destination queue cannot perform the transform itself
			const bool needsSourceRelease = hasLayoutTransitionQueue && layoutTransitionQueueId.Id != mDestinationQueueId.Id && needsLayoutTransition && !destinationCanTransitionLayout;
			if(needsSourceRelease)
			{
				const GpuQueueType sourceQueueType = layoutTransitionQueueId.GetType();
				const D3D12TextureLayout releaseLayout = D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(initialLayout, transition.ImageRange.AspectMask, sourceQueueType) ? initialLayout : D3D12TextureLayout::Common();

				SourceTransitionBuildInformation& sourceTransition = GetSourceTransition(layoutTransitionQueueId, transition.ExclusiveAccessWaitMask);
				sourceTransition.Barriers.AddTextureBarrier(D3D12BarrierUtility::GetTextureBarrier(image->GetD3D12Resource(), transition.ImageRange, GpuBarrierScope(), GpuImageLayout::Undefined, GpuImageLayout::Undefined, committedLayout, releaseLayout));

				mSubmitInformation.RequiredWaitMask |= layoutTransitionQueueId;
				transitionSourceLayout = releaseLayout;
			}
			// Destination queue cannot perform the layout transition, and no layout transition queue is associated with the resource, use the graphics queue for layout transition
			else if(!hasLayoutTransitionQueue && needsLayoutTransition && !destinationCanTransitionLayout)
			{
				if(!B3D_ENSURE_LOG(mDevice.GetQueueCount(GQT_GRAPHICS) > 0, "D3D12 texture activation requires a graphics queue."))
					return;

				const GpuQueueId activationQueueId(GQT_GRAPHICS, 0);
				if(!B3D_ENSURE_LOG(
					D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(committedLayout, transition.ImageRange.AspectMask, GQT_GRAPHICS) &&
					D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(initialLayout, transition.ImageRange.AspectMask, GQT_GRAPHICS),
					"D3D12 texture layouts cannot be activated on a graphics queue."))
				{
					return;
				}

				SourceTransitionBuildInformation& sourceTransition = GetSourceTransition(activationQueueId, GpuQueueMask::kNone);
				sourceTransition.Barriers.AddTextureBarrier(D3D12BarrierUtility::GetTextureBarrier(image->GetD3D12Resource(), transition.ImageRange, GpuBarrierScope(), GpuImageLayout::Undefined, GpuImageLayout::Undefined, committedLayout, initialLayout));

				mSubmitInformation.RequiredWaitMask |= activationQueueId;
				transitionSourceLayout = initialLayout;

				// Set new transition queue
				layoutTransitionQueueId = activationQueueId;
				layoutTransitionQueueChanged = true;
			}
			else if(committedLayout != initialLayout)
				mSubmitInformation.RequiredWaitMask |= transition.ExclusiveAccessWaitMask;
			else
				mSubmitInformation.RequiredWaitMask |= transition.ParallelAccessWaitMask;

			// Record any remaining transition in the destination submission prologue.
			if(transitionSourceLayout != initialLayout)
			{
				const GpuBarrierScope barrier(GpuStageFlag::None, GpuAccessFlag::None, transition.SubmissionBarrierAccessScope.GetStages(), transition.SubmissionBarrierAccessScope.GetAccess());
				mDestinationBarriers.AddTextureBarrier(D3D12BarrierUtility::GetTextureBarrier(image->GetD3D12Resource(), transition.ImageRange, barrier, GpuImageLayout::Undefined, transition.InitialLayout, transitionSourceLayout, initialLayout));

				layoutTransitionQueueId = mDestinationQueueId;
				layoutTransitionQueueChanged = true;
			}

			if(initialLayout != finalLayout)
			{
				layoutTransitionQueueId = mDestinationQueueId;
				layoutTransitionQueueChanged = true;
			}

			// Publish the native state used by later submissions.
			subresource->SetLayout(finalLayout);

			if(layoutTransitionQueueChanged)
				subresource->SetLayoutTransitionQueueId(layoutTransitionQueueId);
		}

		/**
		 * Records all barriers accumulated by the visits into freshly created command buffers and stores them on the
		 * submit information. Must be called once every transition has been visited.
		 */
		void Finalize()
		{
			auto fnRecordBarriers = [this](GpuQueueId queueId, const D3D12BarrierBatch& barriers, const StringView& name)
			{
				GpuCommandBufferPool& commandBufferPool = mDevice.GetSubmitThread().GetCommandBufferPool(queueId.GetType());
				const TShared<D3D12GpuCommandBuffer> commandBuffer = std::static_pointer_cast<D3D12GpuCommandBuffer>(commandBufferPool.Create(GpuCommandBufferCreateInformation::Create(name)));

				barriers.Record(*commandBuffer->GetD3D12Handle());
				commandBuffer->End();

				return commandBuffer;
			};

			for(SourceTransitionBuildInformation& transition : mSourceTransitions)
			{
				if(transition.Barriers.IsEmpty())
					continue;

				D3D12SourceQueueTransition sourceTransition;
				sourceTransition.QueueId = transition.QueueId;
				sourceTransition.WaitMask = transition.WaitMask;
				sourceTransition.CommandBuffer = fnRecordBarriers(transition.QueueId, transition.Barriers, "Source queue resource transitions");

				mSubmitInformation.SourceQueueTransitions.Add(std::move(sourceTransition));
			}

			if(!mDestinationBarriers.IsEmpty())
				mSubmitInformation.TransitionCommandBuffer = fnRecordBarriers(mDestinationQueueId, mDestinationBarriers, "Submission resource transitions");
		}

	private:
		/** Adds waits for earlier submissions on other queues. Same-queue ECL boundaries require no access barrier. */
		void AddParallelAccessWait(const GpuSubmissionTransition& transition)
		{
			mSubmitInformation.RequiredWaitMask |= transition.ParallelAccessWaitMask;
		}

		struct SourceTransitionBuildInformation
		{
			GpuQueueId QueueId;                            /**< Queue recording the release barriers. */
			GpuQueueMask WaitMask = GpuQueueMask::kNone;   /**< Queues the release must wait for. */
			D3D12BarrierBatch Barriers;                    /**< Native release barriers to record. */
		};

		/**
		 * Returns the entry accumulating the barriers to record on the provided queue, creating it if this is the
		 * first transition released from that queue. @p waitMask is merged into the entry's own wait mask.
		 */
		SourceTransitionBuildInformation& GetSourceTransition(GpuQueueId queueId, GpuQueueMask waitMask)
		{
			auto sourceTransitionIterator = std::find_if(mSourceTransitions.begin(), mSourceTransitions.end(), [queueId](const SourceTransitionBuildInformation& entry) { return entry.QueueId.Id == queueId.Id; });
			if(sourceTransitionIterator == mSourceTransitions.end())
			{
				SourceTransitionBuildInformation transition;
				transition.QueueId = queueId;
				transition.WaitMask = waitMask & ~GpuQueueMask(queueId);

				mSourceTransitions.Add(std::move(transition));
				return mSourceTransitions.back();
			}

			sourceTransitionIterator->WaitMask |= waitMask & ~GpuQueueMask(queueId);
			return *sourceTransitionIterator;
		}

		D3D12GpuDevice& mDevice;
		GpuQueueId mDestinationQueueId;
		D3D12GpuCommandBufferSubmitInformation& mSubmitInformation;
		D3D12BarrierBatch mDestinationBarriers;
		TInlineArray<SourceTransitionBuildInformation, 4> mSourceTransitions;
	};
}

D3D12GpuCommandBufferSubmitInformation D3D12GpuCommandBuffer::PrepareForSubmitOnSubmitThread(GpuQueueType queueType, u32 queueIndex)
{
	AssertIfNotSubmitThread();
	B3D_ASSERT(IsSubmitted());

	D3D12GpuCommandBufferSubmitInformation submitInformation;
	D3D12GpuDevice& device = GetD3D12GpuDevice();
	const GpuQueueId destinationQueueId(queueType, queueIndex);

	D3D12SubmissionTransitionVisitor visitor(device, destinationQueueId, submitInformation);
	mResourceTracker.ResolveSubmissionTransitions(destinationQueueId, visitor);
	visitor.Finalize();

	submitInformation.PrimaryCommandBuffer = std::static_pointer_cast<D3D12GpuCommandBuffer>(GetShared());
	return submitInformation;
}

void D3D12GpuCommandBuffer::NotifyWillQueueForSubmit()
{
	EnsureValidThread();

	// Clear everything not allowed on the submit thread.
	mGraphicsPipeline = nullptr;
	mComputePipeline = nullptr;
	mBoundParameterSets.clear();
	mIndexBuffer = nullptr;
	mVertexBuffers.clear();
}

void D3D12GpuCommandBuffer::NotifyWasSubmittedToQueue(GpuQueueId queueId)
{
	AssertIfNotSubmitThread();

	mSubmittedQueueId = queueId;
	mResourceTracker.NotifyUsed(queueId);
}

bool D3D12GpuCommandBuffer::UpdateExecutionStatus(bool block)
{
	if (mState != GpuCommandBufferState::Executing && mState != GpuCommandBufferState::Done)
		return true;

	// Note: only checks the fence. The Done state transition is posted back to the command buffer's owning thread by the queue's RefreshCompletionState
	if (mFence->GetCompletedValue() >= mFenceValue)
		return true;

	if (block && WaitForFenceValue(mFence.Get(), mFenceValue, INFINITE))
		return true;

	return false;
}

void D3D12GpuCommandBuffer::Reset()
{
	EnsureValidThread();

	const bool wasSubmitted = mState == GpuCommandBufferState::Executing || mState == GpuCommandBufferState::Done;
	if(wasSubmitted)
		mResourceTracker.NotifyDone(mSubmittedQueueId);
	else
		mResourceTracker.NotifyUnbound();

	mBarrierHelper.Clear();
	mResourceTracker.Clear();
	mQueueSyncMask = GpuQueueMask();

	OnDidComplete.Clear();
	OnDestroyed.Clear();

	mState = GpuCommandBufferState::Ready;

	mGraphicsPipeline = nullptr;
	mComputePipeline = nullptr;
	mVertexBuffers.clear();
	mIndexBuffer = nullptr;
	mBoundParameterSets.clear();
	mRenderTarget = nullptr;
	mFramebuffer = nullptr;
	mUsedQueryPools.clear();
	mDynamicOffsetOverridesPerSet.clear();
}

Area2I D3D12GpuCommandBuffer::GetViewportArea() const
{
	if (!mRenderTarget)
		return Area2I(0, 0, 0, 0);

	const u32 width = mRenderTarget->GetProperties().Width;
	const u32 height = mRenderTarget->GetProperties().Height;

	return Area2I((i32)(mNormalizedViewportArea.X * width), (i32)(mNormalizedViewportArea.Y * height), (i32)(mNormalizedViewportArea.Width * width), (i32)(mNormalizedViewportArea.Height * height));
}

Area2I D3D12GpuCommandBuffer::GetRenderPassArea() const
{
	if (!mRenderTarget)
		return Area2I(0, 0, 0, 0);

	return Area2I(0, 0, (i32)mRenderTarget->GetProperties().Width, (i32)mRenderTarget->GetProperties().Height);
}

void D3D12GpuCommandBuffer::IssueBarriers(const GpuBarriers& barriers)
{
	EnsureValidThread();

	if(!B3D_ENSURE(!IsInRenderPass()))
		return;

	for(const auto& barrier : barriers.BufferBarriers)
	{
		D3D12GpuBuffer* const gpuBuffer = static_cast<D3D12GpuBuffer*>(barrier.Object.get());
		if(gpuBuffer == nullptr || gpuBuffer->GetD3D12Buffer() == nullptr)
			continue;

		D3D12Buffer* const buffer = gpuBuffer->GetD3D12Buffer();

		mResourceTracker.TrackExplicitBufferBarrier(buffer, barrier.DestinationUsage, barrier.DestinationAccess, mBarrierHelper);

	}

	for(const auto& barrier : barriers.TextureBarriers)
	{
		D3D12Texture* const d3d12Texture = static_cast<D3D12Texture*>(barrier.Object.get());
		if(d3d12Texture == nullptr || d3d12Texture->GetD3D12Image() == nullptr)
			continue;

		D3D12Image* const image = d3d12Texture->GetD3D12Image();

		GpuTextureSubresourceRange maskedRange = barrier.SubresourceRange;
		maskedRange.AspectMask &= image->GetRange().AspectMask;

		mResourceTracker.TrackExplicitImageBarrier(image, maskedRange, barrier.DestinationUsage, barrier.DestinationAccess, barrier.DestinationLayout, mBarrierHelper);
	}

	for(const auto& barrier : barriers.RenderTargetBarriers)
	{
		if(barrier.Object == nullptr)
			continue;

		// Resolve the render target's framebuffer, which carries the per-attachment image references. This is the
		// only way to reach swap-chain back buffers, which have no standalone D3D12Texture wrapper.
		D3D12Framebuffer* framebuffer = nullptr;
		if(barrier.Object->GetProperties().IsWindow)
		{
			RenderWindow* const renderWindow = static_cast<RenderWindow*>(barrier.Object.get());
			const TShared<IRenderWindowSurface>& surface = renderWindow->GetRenderWindowSurface();
			if(surface != nullptr)
			{
				ID3D12RenderWindowSurface* d3d12Surface = static_cast<ID3D12RenderWindowSurface*>(surface.get());
				if(!d3d12Surface->IsSwapChainValid())
					renderWindow->RebuildSwapChain();

				framebuffer = d3d12Surface->GetActiveFramebuffer();
			}
		}
		else
		{
			D3D12RenderTexture* const renderTexture = static_cast<D3D12RenderTexture*>(barrier.Object.get());
			framebuffer = renderTexture->GetFramebuffer();
		}

		if(framebuffer == nullptr)
			continue;

		auto fnAddAttachmentBarrier = [this, &barrier](const D3D12FramebufferAttachment& attachment)
		{
			if(attachment.Image == nullptr)
				return;

			const GpuTextureSubresourceRange range = attachment.Image->GetRange(attachment.Surface);

			mResourceTracker.TrackExplicitImageBarrier(attachment.Image, range, barrier.DestinationUsage, barrier.DestinationAccess, barrier.DestinationLayout, mBarrierHelper);
		};

		for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
		{
			const RenderSurfaceMaskBits colorMask = static_cast<RenderSurfaceMaskBits>(RT_COLOR0 << colorIndex);
			if(barrier.SurfaceMask == colorMask)
			{
				fnAddAttachmentBarrier(framebuffer->GetColorAttachment(colorIndex));
				break;
			}
		}

		if(barrier.SurfaceMask == RT_DEPTH || barrier.SurfaceMask == RT_STENCIL)
			fnAddAttachmentBarrier(framebuffer->GetDepthStencilAttachment());
	}

	mBarrierHelper.Execute(*this);
}

/************************************************************************/
/* 								COPY COMMANDS                     		*/
/************************************************************************/

void D3D12GpuCommandBuffer::CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording && "Command buffer must be in recording state");

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	D3D12GpuBuffer* d3d12Source = static_cast<D3D12GpuBuffer*>(source.get());
	D3D12GpuBuffer* d3d12Destination = static_cast<D3D12GpuBuffer*>(destination.get());

	// UPLOAD-heap memory is CPU-write-only in D3D12; it cannot be a GPU copy destination. Engine paths must write such buffers through their persistent mapping instead (see GpuBufferUtility::Write).
	B3D_ENSURE_ONCE_LOG(d3d12Destination->GetD3D12Buffer() == nullptr || d3d12Destination->GetD3D12Buffer()->GetHeapType() != D3D12_HEAP_TYPE_UPLOAD, "D3D12: CopyBufferToBuffer destination '{0}' lives on the UPLOAD heap; the copy is invalid.", destination->GetName());

	mResourceTracker.TrackBufferUsage(d3d12Source->GetD3D12Buffer(), GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper, sourceOffset);
	mResourceTracker.TrackBufferUsage(d3d12Destination->GetD3D12Buffer(), GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper, destinationOffset);

	mBarrierHelper.Execute(*this);

	ID3D12Resource* sourceResource = d3d12Source->GetD3D12Resource();
	ID3D12Resource* destinationResource = d3d12Destination->GetD3D12Resource();
	B3D_ASSERT(sourceResource != nullptr && destinationResource != nullptr && "Source and destination buffers must be valid");

	mCommandList->CopyBufferRegion(destinationResource, d3d12Destination->GetD3D12Buffer()->GetOffset() + destinationOffset,
		sourceResource, d3d12Source->GetD3D12Buffer()->GetOffset() + sourceOffset, length);
}

void D3D12GpuCommandBuffer::CopyBufferToTexture(const TShared<GpuBuffer>& source, const TShared<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording && "Command buffer must be in recording state");

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	D3D12GpuBuffer* d3d12Source = static_cast<D3D12GpuBuffer*>(source.get());
	D3D12Texture* d3d12Destination = static_cast<D3D12Texture*>(destination.get());
	D3D12Image* destinationImage = d3d12Destination->GetD3D12Image();

	// Track the transfer and execute the copy-state transitions it requires.
	const GpuTextureSubresourceRange subresourceRange(mipLevel, 1, arrayLayer, 1, destinationImage->GetRange().AspectMask);
	mResourceTracker.TrackBufferUsage(d3d12Source->GetD3D12Buffer(), GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destinationImage, subresourceRange, GpuImageLayout::TransferDestination, GpuImageLayout::TransferDestination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);
	mBarrierHelper.Execute(*this);

	ID3D12Resource* textureResource = destinationImage->GetD3D12Resource();
	const D3D12_RESOURCE_DESC textureDesc = textureResource->GetDesc();

	// Compute the placed footprint for the requested subresource from the destination texture description.
	const u32 subresourceIndex = mipLevel + arrayLayer * textureDesc.MipLevels;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	GetD3D12GpuDevice().GetD3D12Device()->GetCopyableFootprints(&textureDesc, subresourceIndex, 1, d3d12Source->GetD3D12Buffer()->GetOffset() + bufferOffset, &footprint, nullptr, nullptr, nullptr);

	// The staging buffer was laid out using the texture's staging pitch, which is what the copy must read with
	footprint.Footprint.RowPitch = d3d12Destination->GetStagingRowPitchInBytes(mipLevel);

	ID3D12Resource* sourceResource = d3d12Source->GetD3D12Resource();
	B3D_ASSERT(sourceResource != nullptr && textureResource != nullptr && "Source buffer and destination texture must be valid");

	D3D12_TEXTURE_COPY_LOCATION sourceLocation = {};
	sourceLocation.pResource = sourceResource;
	sourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	sourceLocation.PlacedFootprint = footprint;

	D3D12_TEXTURE_COPY_LOCATION destinationLocation = {};
	destinationLocation.pResource = textureResource;
	destinationLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	destinationLocation.SubresourceIndex = subresourceIndex;

	mCommandList->CopyTextureRegion(&destinationLocation, 0, 0, 0, &sourceLocation, nullptr);
}

bool D3D12GpuCommandBuffer::CopyTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureCopyInformation& copyInformation)
{
	EnsureValidThread();

	if(!GpuCommandBuffer::CopyTexture(source, destination, copyInformation))
		return false;

	D3D12Texture* d3d12Source = static_cast<D3D12Texture*>(source.get());
	D3D12Texture* d3d12Destination = static_cast<D3D12Texture*>(destination.get());

	const TextureProperties& sourceProperties = d3d12Source->GetProperties();
	const TextureProperties& destinationProperties = d3d12Destination->GetProperties();

	D3D12Image* sourceImage = d3d12Source->GetD3D12Image();
	D3D12Image* destinationImage = d3d12Destination->GetD3D12Image();

	if(sourceImage == nullptr || destinationImage == nullptr)
		return false;

	// An empty source volume is the convention for "the entire subresource".
	const bool copyEntireSurface = copyInformation.SourceVolume.GetWidth() == 0 || copyInformation.SourceVolume.GetHeight() == 0 || copyInformation.SourceVolume.GetDepth() == 0;

	u32 copyWidth, copyHeight, copyDepth;
	if(copyEntireSurface)
	{
		PixelUtility::GetSizeForMipLevel(sourceProperties.Width, sourceProperties.Height, sourceProperties.Depth, copyInformation.SourceMip, copyWidth, copyHeight, copyDepth);
	}
	else
	{
		copyWidth = copyInformation.SourceVolume.GetWidth();
		copyHeight = copyInformation.SourceVolume.GetHeight();
		copyDepth = copyInformation.SourceVolume.GetDepth();
	}

	if(copyWidth == 0 || copyHeight == 0 || copyDepth == 0)
		return false;

	const bool needsResolve = sourceProperties.SampleCount > 1 && destinationProperties.SampleCount <= 1;
	if(needsResolve)
	{
		if(GetQueueType() != GQT_GRAPHICS)
		{
			B3D_LOG(Error, LogRenderBackend, "D3D12 texture resolves require a graphics command buffer.");
			return false;
		}

		const bool destinationAtOrigin = copyInformation.DestinationPosition.X == 0 && copyInformation.DestinationPosition.Y == 0 && copyInformation.DestinationPosition.Z == 0;
		if(!copyEntireSurface || !destinationAtOrigin)
		{
			B3D_LOG(Error, LogRenderBackend, "D3D12 texture resolves must cover the entire source subresource and begin at the destination origin.");
			return false;
		}

		u32 destinationWidth, destinationHeight, destinationDepth;
		PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth, copyInformation.DestinationMip, destinationWidth, destinationHeight, destinationDepth);
		if(copyWidth != destinationWidth || copyHeight != destinationHeight || copyDepth != destinationDepth)
		{
			B3D_LOG(Error, LogRenderBackend, "D3D12 texture resolve source and destination subresources must have matching dimensions.");
			return false;
		}
	}

	const GpuImageLayout sourceLayout = needsResolve ? GpuImageLayout::ResolveSource : GpuImageLayout::TransferSource;
	const GpuImageLayout destinationLayout = needsResolve ? GpuImageLayout::ResolveDestination : GpuImageLayout::TransferDestination;
	const GpuResourceUseFlags resourceUse = needsResolve ? GpuResourceUseFlag::Resolve : GpuResourceUseFlag::Transfer;
	const GpuTextureSubresourceRange sourceRange(copyInformation.SourceMip, 1, copyInformation.SourceFace, copyInformation.FaceCount, sourceImage->GetRange().AspectMask);
	const GpuTextureSubresourceRange destinationRange(copyInformation.DestinationMip, 1, copyInformation.DestinationFace, copyInformation.FaceCount, destinationImage->GetRange().AspectMask);

	mResourceTracker.TrackImageUsage(sourceImage, sourceRange, sourceLayout, sourceLayout, resourceUse, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destinationImage, destinationRange, destinationLayout, destinationLayout, resourceUse, GpuAccessFlag::Write, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	ID3D12Resource* sourceResource = sourceImage->GetD3D12Resource();
	ID3D12Resource* destinationResource = destinationImage->GetD3D12Resource();

	const u32 sourceMipCount = sourceResource->GetDesc().MipLevels;
	const u32 destinationMipCount = destinationResource->GetDesc().MipLevels;

	for(u32 face = 0; face < copyInformation.FaceCount; ++face)
	{
		const u32 sourceSubresource = copyInformation.SourceMip + (copyInformation.SourceFace + face) * sourceMipCount;
		const u32 destinationSubresource = copyInformation.DestinationMip + (copyInformation.DestinationFace + face) * destinationMipCount;

		if(needsResolve)
		{
			// ResolveSubresource always covers the entire subresource, so a sub-region resolve isn't expressible.
			mCommandList->ResolveSubresource(destinationResource, destinationSubresource, sourceResource, sourceSubresource, d3d12Destination->GetDXGIFormat());

			continue;
		}

		D3D12_TEXTURE_COPY_LOCATION sourceLocation = {};
		sourceLocation.pResource = sourceResource;
		sourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		sourceLocation.SubresourceIndex = sourceSubresource;

		D3D12_TEXTURE_COPY_LOCATION destinationLocation = {};
		destinationLocation.pResource = destinationResource;
		destinationLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		destinationLocation.SubresourceIndex = destinationSubresource;

		D3D12_BOX sourceBox;
		sourceBox.left = copyInformation.SourceVolume.Left;
		sourceBox.top = copyInformation.SourceVolume.Top;
		sourceBox.front = copyInformation.SourceVolume.Front;
		sourceBox.right = copyInformation.SourceVolume.Left + copyWidth;
		sourceBox.bottom = copyInformation.SourceVolume.Top + copyHeight;
		sourceBox.back = copyInformation.SourceVolume.Front + copyDepth;

		mCommandList->CopyTextureRegion(&destinationLocation, (u32)copyInformation.DestinationPosition.X, (u32)copyInformation.DestinationPosition.Y, (u32)copyInformation.DestinationPosition.Z, &sourceLocation, &sourceBox);
	}

	return true;
}

bool D3D12GpuCommandBuffer::BlitTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureBlitInformation& blitInformation)
{
	EnsureValidThread();

	if(!GpuCommandBuffer::BlitTexture(source, destination, blitInformation))
		return false;

	D3D12Texture* d3d12Source = static_cast<D3D12Texture*>(source.get());
	D3D12Texture* d3d12Destination = static_cast<D3D12Texture*>(destination.get());

	const TextureProperties& sourceProperties = d3d12Source->GetProperties();
	const TextureProperties& destinationProperties = d3d12Destination->GetProperties();

	D3D12Image* sourceImage = d3d12Source->GetD3D12Image();
	D3D12Image* destinationImage = d3d12Destination->GetD3D12Image();

	if(sourceImage == nullptr || destinationImage == nullptr)
		return false;

	// An empty volume is the convention for "the entire subresource".
	PixelVolume sourceVolume = blitInformation.SourceVolume;
	if(sourceVolume.GetWidth() == 0 || sourceVolume.GetHeight() == 0 || sourceVolume.GetDepth() == 0)
	{
		u32 mipWidth, mipHeight, mipDepth;
		PixelUtility::GetSizeForMipLevel(sourceProperties.Width, sourceProperties.Height, sourceProperties.Depth, blitInformation.SourceMip, mipWidth, mipHeight, mipDepth);

		sourceVolume.Right = sourceVolume.Left + mipWidth;
		sourceVolume.Bottom = sourceVolume.Top + mipHeight;
		sourceVolume.Back = sourceVolume.Front + mipDepth;
	}

	PixelVolume destinationVolume = blitInformation.DestinationVolume;
	if(destinationVolume.GetWidth() == 0 || destinationVolume.GetHeight() == 0 || destinationVolume.GetDepth() == 0)
	{
		u32 mipWidth, mipHeight, mipDepth;
		PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth, blitInformation.DestinationMip, mipWidth, mipHeight, mipDepth);

		destinationVolume.Right = destinationVolume.Left + mipWidth;
		destinationVolume.Bottom = destinationVolume.Top + mipHeight;
		destinationVolume.Back = destinationVolume.Front + mipDepth;
	}

	// D3D12 has no scaling/filtering blit equivalent to vkCmdBlitImage; only 1:1 region copies are supported.
	// TODO(d3d12-port): Shader-based blit for scaled or format-converting blits.
	if(sourceVolume.GetWidth() != destinationVolume.GetWidth() || sourceVolume.GetHeight() != destinationVolume.GetHeight() || sourceVolume.GetDepth() != destinationVolume.GetDepth())
	{
		B3D_ENSURE_ONCE_LOG(false, "D3D12: BlitTexture from '{0}' to '{1}' requires scaling, which is not supported; the blit is skipped.", source->GetName(), destination->GetName());
		return false;
	}

	// Track the transfer and execute the copy-state transitions it requires.
	const GpuTextureSubresourceRange sourceRange(blitInformation.SourceMip, 1, blitInformation.SourceFace, blitInformation.FaceCount, sourceImage->GetRange().AspectMask);
	const GpuTextureSubresourceRange destinationRange(blitInformation.DestinationMip, 1, blitInformation.DestinationFace, blitInformation.FaceCount, destinationImage->GetRange().AspectMask);

	mResourceTracker.TrackImageUsage(sourceImage, sourceRange, GpuImageLayout::TransferSource, GpuImageLayout::TransferSource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destinationImage, destinationRange, GpuImageLayout::TransferDestination, GpuImageLayout::TransferDestination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	ID3D12Resource* sourceResource = sourceImage->GetD3D12Resource();
	ID3D12Resource* destinationResource = destinationImage->GetD3D12Resource();

	const u32 sourceMipCount = sourceResource->GetDesc().MipLevels;
	const u32 destinationMipCount = destinationResource->GetDesc().MipLevels;

	D3D12_BOX sourceBox;
	sourceBox.left = sourceVolume.Left;
	sourceBox.top = sourceVolume.Top;
	sourceBox.front = sourceVolume.Front;
	sourceBox.right = sourceVolume.Right;
	sourceBox.bottom = sourceVolume.Bottom;
	sourceBox.back = sourceVolume.Back;

	for(u32 face = 0; face < blitInformation.FaceCount; ++face)
	{
		D3D12_TEXTURE_COPY_LOCATION sourceLocation = {};
		sourceLocation.pResource = sourceResource;
		sourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		sourceLocation.SubresourceIndex = blitInformation.SourceMip + (blitInformation.SourceFace + face) * sourceMipCount;

		D3D12_TEXTURE_COPY_LOCATION destinationLocation = {};
		destinationLocation.pResource = destinationResource;
		destinationLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		destinationLocation.SubresourceIndex = blitInformation.DestinationMip + (blitInformation.DestinationFace + face) * destinationMipCount;

		mCommandList->CopyTextureRegion(&destinationLocation, destinationVolume.Left, destinationVolume.Top, destinationVolume.Front, &sourceLocation, &sourceBox);
	}

	return true;
}

void D3D12GpuCommandBuffer::CopyTextureToBuffer(const TShared<Texture>& source, const TShared<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset)
{
	EnsureValidThread();

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	D3D12Texture* d3d12Source = static_cast<D3D12Texture*>(source.get());
	D3D12GpuBuffer* d3d12Destination = static_cast<D3D12GpuBuffer*>(destination.get());
	D3D12Image* sourceImage = d3d12Source->GetD3D12Image();

	const GpuTextureSubresourceRange subresourceRange(mipLevel, 1, arrayLayer, 1, sourceImage->GetRange().AspectMask);

	mResourceTracker.TrackImageUsage(sourceImage, subresourceRange, GpuImageLayout::TransferSource, GpuImageLayout::TransferSource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackBufferUsage(d3d12Destination->GetD3D12Buffer(), GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	ID3D12Resource* textureResource = sourceImage->GetD3D12Resource();
	const D3D12_RESOURCE_DESC textureDesc = textureResource->GetDesc();

	// Compute the placed footprint for the requested subresource from the source texture description.
	const u32 subresourceIndex = mipLevel + arrayLayer * textureDesc.MipLevels;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	GetD3D12GpuDevice().GetD3D12Device()->GetCopyableFootprints(&textureDesc, subresourceIndex, 1, d3d12Destination->GetD3D12Buffer()->GetOffset() + bufferOffset, &footprint, nullptr, nullptr, nullptr);

	// The staging buffer is laid out using the texture's staging pitch, which is what the copy must write with
	footprint.Footprint.RowPitch = d3d12Source->GetStagingRowPitchInBytes(mipLevel);

	CopyTextureToBuffer(textureResource, d3d12Destination->GetD3D12Resource(), footprint, subresourceIndex);
}

void D3D12GpuCommandBuffer::CopyImageToBuffer(D3D12Image* source, D3D12Buffer* destination, u32 width, u32 height, u32 rowPitchBytes)
{
	EnsureValidThread();

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	// Track the transfer and execute the copy-state transitions it requires.
	const GpuTextureSubresourceRange subresourceRange(0, 1, 0, 1, source->GetRange().AspectMask);

	mResourceTracker.TrackImageUsage(source, subresourceRange, GpuImageLayout::TransferSource, GpuImageLayout::TransferSource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackBufferUsage(destination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	footprint.Offset = destination->GetOffset();
	footprint.Footprint.Format = source->GetDXGIFormat();
	footprint.Footprint.Width = width;
	footprint.Footprint.Height = height;
	footprint.Footprint.Depth = 1;
	footprint.Footprint.RowPitch = rowPitchBytes;

	CopyTextureToBuffer(source->GetD3D12Resource(), destination->GetD3D12Resource(), footprint, 0);
}

void D3D12GpuCommandBuffer::CopyTextureToBuffer(ID3D12Resource* source, ID3D12Resource* destination, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout, u32 subresourceIndex)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording && "Command buffer must be in recording state");
	B3D_ASSERT(source && destination && "Source texture and destination buffer must be valid");

	D3D12_TEXTURE_COPY_LOCATION sourceLocation = {};
	sourceLocation.pResource = source;
	sourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	sourceLocation.SubresourceIndex = subresourceIndex;

	D3D12_TEXTURE_COPY_LOCATION destinationLocation = {};
	destinationLocation.pResource = destination;
	destinationLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	destinationLocation.PlacedFootprint = layout;

	mCommandList->CopyTextureRegion(&destinationLocation, 0, 0, 0, &sourceLocation, nullptr);
}
