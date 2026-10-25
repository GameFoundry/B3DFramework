//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuParameterSet.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12GpuQueue.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12BufferPool.h"
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
	if(createInformation.UsePoolReset)
	{
		const D3D12_COMMAND_LIST_TYPE commandListType = GetCommandListType(createInformation.Type);

		HRESULT hr = device.GetD3D12Device()->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&mCommandAllocator));
		B3D_ASSERT(SUCCEEDED(hr) && "Failed to create command allocator");
	}
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

	// A command allocator only gives its memory back when it is reset, which requires the GPU to be done with every command list
	// recorded with it. Without pool reset command buffers complete one by one, so each gets an allocator of its own.
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	if(!GetUsePoolReset())
	{
		HRESULT hr = d3d12Device.GetD3D12Device()->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&commandAllocator));
		if(!B3D_ENSURE_LOG(SUCCEEDED(hr), "D3D12: Failed to create a command allocator (hr={0}).", (u32)hr))
			return nullptr;
	}

	// Node mask 0 (single GPU), no initial pipeline state
	ComPtr<ID3D12GraphicsCommandList> baseCommandList;
	HRESULT hr = d3d12Device.GetD3D12Device()->CreateCommandList(0, commandListType, commandAllocator != nullptr ? commandAllocator.Get() : mCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&baseCommandList));
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
		D3D12GpuCommandBuffer(d3d12Device, *this, mNextCommandBufferId++, commandList.Get(), commandAllocator.Get(), mInformation.Thread, mInformation.Type, createInformation),
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

	if(mCommandAllocator != nullptr)
	{
		HRESULT hr = mCommandAllocator->Reset();
		B3D_ASSERT(SUCCEEDED(hr) && "Failed to reset command allocator");
	}
}

D3D12GpuCommandBuffer::D3D12GpuCommandBuffer(D3D12GpuDevice& device, D3D12GpuCommandBufferPool& pool, u32 id, ID3D12GraphicsCommandList7* commandList, ID3D12CommandAllocator* commandAllocator, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation) : GpuCommandBuffer(device, ownerThread, queueType, createInformation), mId(id), mCommandList(commandList), mCommandAllocator(commandAllocator), mPool(pool), mGraphicsPipelineRequiresBind(true), mGraphicsRootSignatureRequiresBind(true), mComputePipelineRequiresBind(true), mPrimitiveTopologyRequiresBind(true), mViewportRequiresBind(true), mStencilReferenceValueRequiresBind(true), mScissorRequiresBind(true), mGraphicsParametersRequireBind(false), mComputeParametersRequireBind(false), mGraphicsPushConstantsRequireBind(false), mComputePushConstantsRequireBind(false), mVertexInputsDirty(false)
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
	mCommandAllocator.Reset();
	mFence.Reset();
}

void D3D12GpuCommandBuffer::Begin()
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Ready);

	// A ready command buffer is not executing, so the GPU is done with the commands recorded with its own allocator
	ID3D12CommandAllocator* commandAllocator = mPool.GetD3D12CommandAllocator();
	if(mCommandAllocator != nullptr)
	{
		HRESULT allocatorResult = mCommandAllocator->Reset();
		B3D_ASSERT(SUCCEEDED(allocatorResult) && "Failed to reset command allocator");

		commandAllocator = mCommandAllocator.Get();
	}

	HRESULT hr = mCommandList->Reset(commandAllocator, nullptr);
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
	mPushConstants.Clear();
	mGraphicsPushConstantsRequireBind = false;
	mComputePushConstantsRequireBind = false;
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
	GpuCommandBuffer::SetGpuParameterSet(parameters);
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

void D3D12GpuCommandBuffer::SetPushConstants(u32 offsetInBytes, u32 sizeInBytes, const void* data)
{
	EnsureValidThread();

	if(sizeInBytes == 0)
		return;

	if(!B3D_ENSURE_LOG(data != nullptr, "Push-constant data cannot be null for a non-empty update."))
		return;

	if(!B3D_ENSURE_LOG((offsetInBytes & 3u) == 0 && (sizeInBytes & 3u) == 0, "Push-constant offsets and sizes must be aligned to four bytes."))
		return;

	if(!B3D_ENSURE_LOG(offsetInBytes <= kMaxPushConstantSizeInBytes && sizeInBytes <= kMaxPushConstantSizeInBytes - offsetInBytes, "Push-constant update at offset {0} with size {1} exceeds the {2}-byte D3D12 block.", offsetInBytes, sizeInBytes, kMaxPushConstantSizeInBytes))
		return;

	mPushConstants.Write(offsetInBytes, sizeInBytes, data);
	mGraphicsPushConstantsRequireBind = true;
	mComputePushConstantsRequireBind = true;
}

void D3D12GpuCommandBuffer::SetGpuGraphicsPipelineState(const TShared<GpuGraphicsPipelineState>& pipelineState)
{
	GpuCommandBuffer::SetGpuGraphicsPipelineState(pipelineState);
	EnsureValidThread();

	if(mGraphicsPipeline == pipelineState)
		return;

	mGraphicsPipeline = std::static_pointer_cast<D3D12GpuGraphicsPipelineState>(pipelineState);
	mGraphicsPipelineRequiresBind = true;
	mGraphicsRootSignatureRequiresBind = true;
	mGraphicsPushConstantsRequireBind = true;
	mVertexInputsDirty = true;
}

void D3D12GpuCommandBuffer::SetGpuComputePipelineState(const TShared<GpuComputePipelineState>& pipelineState)
{
	GpuCommandBuffer::SetGpuComputePipelineState(pipelineState);
	EnsureValidThread();

	if(mComputePipeline == pipelineState)
		return;

	mComputePipeline = std::static_pointer_cast<D3D12GpuComputePipelineState>(pipelineState);
	mComputePipelineRequiresBind = true;
	mComputePushConstantsRequireBind = true;
}

void D3D12GpuCommandBuffer::SetVertexBuffers(u32 index, TShared<GpuBuffer>* buffers, u32 bufferCount)
{
	GpuCommandBuffer::SetVertexBuffers(index, buffers, bufferCount);
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
	GpuCommandBuffer::SetIndexBuffer(buffer);
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
	GpuCommandBuffer::Draw(vertexOffset, vertexCount, instanceCount, firstInstance);
	EnsureValidThread();

	if(vertexCount == 0)
		return;

	if(!B3D_ENSURE(IsInRenderPass()))
		return;

	if (!IsReadyForRender())
		return;

	if(!BindGraphicsPipeline())
		return;

	BindDynamicStates(false);
	BindVertexInputs();

	if(!BindGpuParameterSets(true))
		return;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(!ValidateDrawAccesses())
		return;
#endif

	BindPushConstants(true);

	if (instanceCount == 0)
		instanceCount = 1;

	mCommandList->DrawInstanced(vertexCount, instanceCount, vertexOffset, firstInstance);
}

void D3D12GpuCommandBuffer::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	GpuCommandBuffer::DrawIndexed(startIndex, indexCount, vertexOffset, vertexCount, instanceCount, firstInstance);
	EnsureValidThread();

	if(indexCount == 0)
		return;

	if(!B3D_ENSURE(IsInRenderPass()))
		return;

	if (!IsReadyForRender())
		return;

	if(!BindGraphicsPipeline())
		return;

	BindDynamicStates(false);
	BindVertexInputs();

	if(!BindGpuParameterSets(true))
		return;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(!ValidateDrawAccesses())
		return;
#endif

	BindPushConstants(true);

	if (instanceCount == 0)
		instanceCount = 1;

	mCommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, vertexOffset, firstInstance);
}

void D3D12GpuCommandBuffer::DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
{
	GpuCommandBuffer::DispatchCompute(groupCountX, groupCountY, groupCountZ);
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

		// Graphics and compute share the command list's pipeline-state binding.
		mLastBoundGraphicsPipeline = nullptr;
		mGraphicsPipelineRequiresBind = true;

		mComputePipelineRequiresBind = false;
		mComputeParametersRequireBind = true;
		mComputePushConstantsRequireBind = true;
	}

	if(!BindGpuParameterSets(false))
		return;

	BindPushConstants(false);

	mCommandList->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void D3D12GpuCommandBuffer::BeginRenderPass(const RenderPassCreateInformation& createInformation)
{
	GpuCommandBuffer::BeginRenderPass(createInformation);
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording);

	const TShared<RenderTarget>& target = createInformation.Target;
	if (!B3D_ENSURE(target != nullptr))
		return;

	mRenderTarget = target;
	mRenderTargetReadOnlyMask = createInformation.ReadOnlyMask;
	mFramebuffer = nullptr;
	mGraphicsPipelineRequiresBind = true;

	// Sets that stay bound are tracked again, against the attachments of the new render pass
	mGraphicsParametersRequireBind = true;

	// Get framebuffer for the render target
	if (target->GetProperties().IsWindow)
		mFramebuffer = AcquireWindowFramebuffer(static_cast<RenderWindow&>(*target));
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

	GpuRenderPassAttachmentUsageArray renderPassAttachmentUsages;
	if(mFramebuffer != nullptr)
	{
		renderPassAttachmentUsages = mFramebuffer->BuildRenderPassAttachmentUsages(mRenderTargetReadOnlyMask, createInformation.LoadMask);

		for(GpuRenderPassAttachmentUsage& attachmentUsage : renderPassAttachmentUsages)
			attachmentUsage.FinalLayout.reset();
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	mDrawAccessValidator.BeginRenderPass();
#endif
	mResourceTracker.BeginRenderPass(renderPassAttachmentUsages);

	// Set render targets if framebuffer exists
	if (mFramebuffer)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles = mFramebuffer->GetRenderTargetViews();
		const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = mFramebuffer->GetDepthStencilView(mRenderTargetReadOnlyMask);
		const u32 renderTargetCount = mFramebuffer->GetColorAttachmentCount();

		mCommandList->OMSetRenderTargets(renderTargetCount, rtvHandles, FALSE, dsvHandle);
	}

	// Apply clear operations requested for the render pass start
	RenderTargetClearValues clearValues;
	const RenderSurfaceMask clearMask = GpuBackendUtility::GetRenderPassClears(createInformation, clearValues);
	if (clearMask != RT_NONE)
		ClearViewportArea(GetRenderPassArea(), clearMask, clearValues);
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

	if (mRenderTarget)
		ClearViewportArea(GetRenderPassArea(), mask, mRenderTarget->GetClearValues());
}

void D3D12GpuCommandBuffer::ClearViewport(RenderSurfaceMask mask)
{
	EnsureValidThread();

	if (mRenderTarget)
		ClearViewportArea(GetViewportArea(), mask, mRenderTarget->GetClearValues());
}

void D3D12GpuCommandBuffer::ClearViewportArea(const Area2I& area, RenderSurfaceMask mask, const RenderTargetClearValues& clearValues)
{
	if (!mFramebuffer)
		return;

	D3D12_RECT rect;
	rect.left = area.X;
	rect.top = area.Y;
	rect.right = area.X + area.Width;
	rect.bottom = area.Y + area.Height;

	// Clear color attachments
	const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles = mFramebuffer->GetRenderTargetViews();
	const TArrayView<const GpuFramebufferAttachment> colorAttachments = mFramebuffer->GetColorAttachments();
	const u32 renderTargetCount = mFramebuffer->GetColorAttachmentCount();

	for (u32 renderTargetIndex = 0; renderTargetIndex < renderTargetCount; renderTargetIndex++)
	{
		const u32 colorIndex = colorAttachments[renderTargetIndex].GetIndex();
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

	mResourceTracker.EndRenderPass();

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
			mComputePipelineRequiresBind = true;
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
		mGraphicsPushConstantsRequireBind = true;
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
			mResourceTracker.TrackBufferUsage(buffer->GetD3D12Buffer(), GpuAccessFlag::Read);
			vertexBufferViews[slot] = buffer->GetVertexBufferView();
		}

		// Empty views read zero in the shader
		mCommandList->IASetVertexBuffers(0, (UINT)vertexBufferViews.Size(), vertexBufferViews.Data());
	}

	if (mIndexBuffer)
	{
		mResourceTracker.TrackBufferUsage(mIndexBuffer->GetD3D12Buffer(), GpuAccessFlag::Read);
		mCommandList->IASetIndexBuffer(&mIndexBuffer->GetIndexBufferView());
	}
	else
		mCommandList->IASetIndexBuffer(nullptr);

	mVertexInputsDirty = false;
}

bool D3D12GpuCommandBuffer::BindGpuParameterSets(bool isGraphics)
{
	const bool requiresBind = isGraphics ? mGraphicsParametersRequireBind : mComputeParametersRequireBind;
	if (!requiresBind && isGraphics)
		return true;

	const D3D12GpuPipelineParameterLayout* parameterLayout = nullptr;
	if (isGraphics)
		parameterLayout = mGraphicsPipeline != nullptr ? mGraphicsPipeline->GetD3D12ParameterLayout() : nullptr;
	else
		parameterLayout = mComputePipeline != nullptr ? mComputePipeline->GetD3D12ParameterLayout() : nullptr;

	if (parameterLayout == nullptr)
		return false;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(isGraphics)
		mDrawAccessValidator.ClearBindings();
#endif

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

#if B3D_BUILD_TYPE_DEVELOPMENT
		if(isGraphics)
			mDrawAccessValidator.AddParameterSet(*parameters, *pipelineSetLayout);
#endif

		// Compute sets are tracked on every dispatch, graphics sets whenever they are bound
		if(!parameters->TrackResources(*pipelineSetLayout, mResourceTracker))
			return false;

		if(requiresBind)
			parameters->BindDescriptors(device, mResourceTracker, mCommandList.Get(), isGraphics, parameterLayout->GetDescriptorSetLayout(setIndex), dynamicOffsets);
	}

	if (isGraphics)
		mGraphicsParametersRequireBind = false;
	else
		mComputeParametersRequireBind = false;

	return true;
}

void D3D12GpuCommandBuffer::BindPushConstants(bool isGraphics)
{
	if((isGraphics && !mGraphicsPushConstantsRequireBind) || (!isGraphics && !mComputePushConstantsRequireBind))
		return;

	const D3D12GpuPipelineParameterLayout* parameterLayout = nullptr;
	if(isGraphics)
		parameterLayout = mGraphicsPipeline != nullptr ? mGraphicsPipeline->GetD3D12ParameterLayout() : nullptr;
	else
		parameterLayout = mComputePipeline != nullptr ? mComputePipeline->GetD3D12ParameterLayout() : nullptr;

	if(parameterLayout == nullptr)
		return;

	const u32 pushConstantBufferSize = parameterLayout->GetPushConstantBufferSize();
	B3D_ASSERT((pushConstantBufferSize & 3u) == 0);
	B3D_ASSERT(pushConstantBufferSize <= kMaxPushConstantSizeInBytes);

	const u32 valueCount = pushConstantBufferSize / sizeof(u32);
	if(valueCount != 0)
	{
		if(isGraphics)
			mCommandList->SetGraphicsRoot32BitConstants(kD3D12PushConstantRootParameterIndex, valueCount, mPushConstants.Values.data(), 0);
		else
			mCommandList->SetComputeRoot32BitConstants(kD3D12PushConstantRootParameterIndex, valueCount, mPushConstants.Values.data(), 0);
	}

	if(isGraphics)
		mGraphicsPushConstantsRequireBind = false;
	else
		mComputePushConstantsRequireBind = false;
}

D3D12GpuCommandBufferSubmitInformation D3D12GpuCommandBuffer::PrepareForSubmitOnSubmitThread(GpuQueueType queueType)
{
	AssertIfNotSubmitThread();
	B3D_ASSERT(IsSubmitted());

	D3D12GpuCommandBufferSubmitInformation submitInformation;

	// Transition the images that requested it to rest (see IGpuImageResource::RequestRestingTransition()). The command buffer's first barrier
	// from the resting state can be in any stage of the queue, so the transitions make the images available to all of them. Layouts the queue
	// cannot transition, such as most layouts on copy queues, are transitioned on the graphics queue instead.
	const GpuBarrierScope restingBarrier(GpuStageFlag::None, GpuAccessFlag::None, GpuStageFlags(GpuStageFlag::All) & GpuBackendUtility::GetQueueStageFlags(queueType),
		GpuAccessFlag::Read | GpuAccessFlag::Write);

	D3D12BarrierBatch restingTransitions;
	D3D12BarrierBatch graphicsRestingTransitions;
	for(const auto& entry : mResourceTracker.GetImages())
	{
		D3D12Image& image = static_cast<D3D12Image&>(*entry.first);
		const GpuImageLayout restingLayout = image.ConsumeRestingTransition();
		if(restingLayout == GpuImageLayout::Undefined)
			continue;

		const D3D12TextureLayout restingTextureLayout = image.GetTextureLayout(restingLayout, queueType);
		for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
		{
			if(!image.GetRange().AspectMask.IsSet(aspect))
				continue;

			GpuTextureSubresourceRange range = image.GetRange();
			range.AspectMask = aspect;

			const D3D12TextureLayout committedLayout((D3D12_BARRIER_LAYOUT)image.GetNativeLayout(0, 0, aspect));
			const D3D12TextureLayout newLayout(restingTextureLayout.GetLayout(aspect));
			if(committedLayout.GetLayout(aspect) == newLayout.GetLayout(aspect))
				continue;

			// New images have no contents yet
			const D3D12_TEXTURE_BARRIER_FLAGS flags = committedLayout.IsUndefined(aspect) ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD : D3D12_TEXTURE_BARRIER_FLAG_NONE;

			if(D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(committedLayout, newLayout, aspect, queueType))
			{
				restingTransitions.AddTextureBarrier(D3D12BarrierUtility::GetTextureBarrier(image.GetD3D12Resource(), range, restingBarrier, GpuImageLayout::Undefined, restingLayout,
					committedLayout, newLayout, flags));
			}
			else
			{
				graphicsRestingTransitions.AddTextureBarrier(D3D12BarrierUtility::GetTextureBarrier(image.GetD3D12Resource(), range, GpuBarrierScope(), GpuImageLayout::Undefined,
					GpuImageLayout::Undefined, committedLayout, newLayout, flags));
			}

			image.SetNativeLayout(range, (u32)newLayout.GetLayout(aspect));
		}
	}

	auto fnRecordBarriers = [this](GpuQueueType barrierQueueType, const D3D12BarrierBatch& barriers)
	{
		GpuCommandBufferPool& commandBufferPool = GetD3D12GpuDevice().GetSubmitThread().GetCommandBufferPool(barrierQueueType);
		const TShared<D3D12GpuCommandBuffer> commandBuffer = std::static_pointer_cast<D3D12GpuCommandBuffer>(commandBufferPool.FindOrCreate(GpuCommandBufferCreateInformation::Create("Resting transitions")));

		barriers.Record(*commandBuffer->GetD3D12Handle());
		commandBuffer->End();

		return commandBuffer;
	};

	if(!restingTransitions.IsEmpty())
		submitInformation.TransitionCommandBuffer = fnRecordBarriers(queueType, restingTransitions);

	if(!graphicsRestingTransitions.IsEmpty())
		submitInformation.GraphicsTransitionCommandBuffer = fnRecordBarriers(GQT_GRAPHICS, graphicsRestingTransitions);

	PublishImageLayouts();

	submitInformation.PrimaryCommandBuffer = std::static_pointer_cast<D3D12GpuCommandBuffer>(GetShared());
	return submitInformation;
}

void D3D12GpuCommandBuffer::NotifyWillQueueForSubmit(GpuQueueId queueId, GpuQueueMask syncMask)
{
	GpuCommandBuffer::NotifyWillQueueForSubmit(queueId, syncMask);

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

	mResourceTracker.Clear();
	mQueueSyncMask = GpuQueueMask();

	ClearBarrierState(wasSubmitted);

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
	mPushConstants.Clear();
	mGraphicsPushConstantsRequireBind = false;
	mComputePushConstantsRequireBind = false;
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

/************************************************************************/
/* 								COPY COMMANDS                     		*/
/************************************************************************/

void D3D12GpuCommandBuffer::CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length)
{
	GpuCommandBuffer::CopyBufferToBuffer(source, destination, sourceOffset, destinationOffset, length);
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording && "Command buffer must be in recording state");

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	D3D12GpuBuffer* d3d12Source = static_cast<D3D12GpuBuffer*>(source.get());
	D3D12GpuBuffer* d3d12Destination = static_cast<D3D12GpuBuffer*>(destination.get());

	// UPLOAD-heap memory is CPU-write-only in D3D12; it cannot be a GPU copy destination. Engine paths must write such buffers through their persistent mapping instead (see GpuBufferUtility::Write).
	B3D_ENSURE_ONCE_LOG(d3d12Destination->GetD3D12Buffer() == nullptr || d3d12Destination->GetD3D12Buffer()->GetHeapType() != D3D12_HEAP_TYPE_UPLOAD, "D3D12: CopyBufferToBuffer destination '{0}' lives on the UPLOAD heap; the copy is invalid.", destination->GetName());

	mResourceTracker.TrackBufferUsage(d3d12Source->GetD3D12Buffer(), GpuAccessFlag::Read, sourceOffset);
	mResourceTracker.TrackBufferUsage(d3d12Destination->GetD3D12Buffer(), GpuAccessFlag::Write, destinationOffset);

	ID3D12Resource* sourceResource = d3d12Source->GetD3D12Resource();
	ID3D12Resource* destinationResource = d3d12Destination->GetD3D12Resource();
	B3D_ASSERT(sourceResource != nullptr && destinationResource != nullptr && "Source and destination buffers must be valid");

	mCommandList->CopyBufferRegion(destinationResource, d3d12Destination->GetD3D12Buffer()->GetOffset() + destinationOffset,
		sourceResource, d3d12Source->GetD3D12Buffer()->GetOffset() + sourceOffset, length);
}

void D3D12GpuCommandBuffer::CopyBufferToTexture(const TShared<GpuBuffer>& source, const TShared<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer)
{
	GpuCommandBuffer::CopyBufferToTexture(source, destination, bufferOffset, mipLevel, arrayLayer);
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording && "Command buffer must be in recording state");

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	D3D12GpuBuffer* d3d12Source = static_cast<D3D12GpuBuffer*>(source.get());
	D3D12Texture* d3d12Destination = static_cast<D3D12Texture*>(destination.get());
	D3D12Image* destinationImage = d3d12Destination->GetD3D12Image();

	// Track the transfer
	const GpuTextureSubresourceRange subresourceRange(mipLevel, 1, arrayLayer, 1, destinationImage->GetRange().AspectMask);
	mResourceTracker.TrackBufferUsage(d3d12Source->GetD3D12Buffer(), GpuAccessFlag::Read);
	mResourceTracker.TrackImageUsage(destinationImage, subresourceRange, GpuAccessFlag::Write);

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
	const GpuTextureSubresourceRange sourceRange(copyInformation.SourceMip, 1, copyInformation.SourceFace, copyInformation.FaceCount, sourceImage->GetRange().AspectMask);
	const GpuTextureSubresourceRange destinationRange(copyInformation.DestinationMip, 1, copyInformation.DestinationFace, copyInformation.FaceCount, destinationImage->GetRange().AspectMask);

	mResourceTracker.TrackImageUsage(sourceImage, sourceRange, GpuAccessFlag::Read);
	mResourceTracker.TrackImageUsage(destinationImage, destinationRange, GpuAccessFlag::Write);

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
			mCommandList->ResolveSubresource(destinationResource, destinationSubresource, sourceResource, sourceSubresource, d3d12Destination->GetViewFormat());

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

	// Track the transfer
	const GpuTextureSubresourceRange sourceRange(blitInformation.SourceMip, 1, blitInformation.SourceFace, blitInformation.FaceCount, sourceImage->GetRange().AspectMask);
	const GpuTextureSubresourceRange destinationRange(blitInformation.DestinationMip, 1, blitInformation.DestinationFace, blitInformation.FaceCount, destinationImage->GetRange().AspectMask);

	mResourceTracker.TrackImageUsage(sourceImage, sourceRange, GpuAccessFlag::Read);
	mResourceTracker.TrackImageUsage(destinationImage, destinationRange, GpuAccessFlag::Write);

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
	GpuCommandBuffer::CopyTextureToBuffer(source, destination, mipLevel, arrayLayer, bufferOffset);
	EnsureValidThread();

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	D3D12Texture* d3d12Source = static_cast<D3D12Texture*>(source.get());
	D3D12GpuBuffer* d3d12Destination = static_cast<D3D12GpuBuffer*>(destination.get());
	D3D12Image* sourceImage = d3d12Source->GetD3D12Image();

	const GpuTextureSubresourceRange subresourceRange(mipLevel, 1, arrayLayer, 1, sourceImage->GetRange().AspectMask);

	mResourceTracker.TrackImageUsage(sourceImage, subresourceRange, GpuAccessFlag::Read);
	mResourceTracker.TrackBufferUsage(d3d12Destination->GetD3D12Buffer(), GpuAccessFlag::Write);

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

void D3D12GpuCommandBuffer::CopyWindowToBuffer(const TShared<RenderWindow>& source, const TShared<GpuBuffer>& destination)
{
	GpuCommandBuffer::CopyWindowToBuffer(source, destination);
	EnsureValidThread();

	if (source == nullptr || destination == nullptr)
		return;

	auto* const surface = static_cast<ID3D12RenderWindowSurface*>(source->GetRenderWindowSurface().get());
	if (surface == nullptr)
		return;

	const TOptional<RenderWindowImageLayout> layout = surface->GetImageLayout();
	D3D12Image* const sourceImage = surface->GetCurrentColorImage();
	D3D12Buffer* const destinationBuffer = static_cast<D3D12GpuBuffer*>(destination.get())->GetD3D12Buffer();
	if (!layout || sourceImage == nullptr || destinationBuffer == nullptr)
		return;

	const u32 rowPitchBytes = layout->Pitch.RowPitch * PixelUtility::GetBlockSize(layout->Format);
	CopyImageToBuffer(sourceImage, destinationBuffer, layout->Width, layout->Height, rowPitchBytes);
}

void D3D12GpuCommandBuffer::CopyImageToBuffer(D3D12Image* source, D3D12Buffer* destination, u32 width, u32 height, u32 rowPitchBytes)
{
	EnsureValidThread();

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	// Track the transfer
	const GpuTextureSubresourceRange subresourceRange(0, 1, 0, 1, source->GetRange().AspectMask);

	mResourceTracker.TrackImageUsage(source, subresourceRange, GpuAccessFlag::Read);
	mResourceTracker.TrackBufferUsage(destination, GpuAccessFlag::Write);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	footprint.Offset = destination->GetOffset();
	footprint.Footprint.Format = source->GetViewFormat();
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

D3D12Framebuffer* D3D12GpuCommandBuffer::AcquireWindowFramebuffer(RenderWindow& renderWindow)
{
	ID3D12RenderWindowSurface* const surface = static_cast<ID3D12RenderWindowSurface*>(renderWindow.GetRenderWindowSurface().get());
	if(surface == nullptr)
		return nullptr;

	if(!surface->IsSwapChainValid())
		renderWindow.RebuildSwapChain();

	if(D3D12SwapChain* const swapChain = surface->GetSwapChain())
		mResourceTracker.TrackSwapChainUsage(swapChain);

	return surface->GetActiveFramebuffer();
}

bool D3D12GpuCommandBuffer::RecordBarriers(const GpuBarriers& barriers, GpuBarrierPhase phase, GpuSplitBarrier* split)
{
	// What the barriers of each case record:
	//
	// Case            | Page and readback buffers | Placed buffers      | Alias global barrier | Texture barrier per aspect                | Layout write-back
	// --------------- | ------------------------- | ------------------- | -------------------- | ----------------------------------------- | -----------------
	// Full            | merged global barrier     | barrier             | yes                  | source -> destination                     | yes
	// SplitRelease    | -                         | SyncAfter = SPLIT   | yes                  | source -> destination, SyncAfter = SPLIT  | no
	// SplitAcquire    | merged global barrier     | SyncBefore = SPLIT  | -                    | source -> destination, SyncBefore = SPLIT | yes
	// TransferRelease | -                         | -                   | -                    | source -> handoff, SyncAfter = NONE       | no
	// TransferAcquire | -                         | -                   | -                    | handoff -> destination, SyncBefore = NONE | yes
	//
	// Buffers on pages are slices of a shared resource, so a barrier on that resource would also order the unrelated buffers on the page, and a
	// split pair would forbid accessing them between its halves. Those buffers share one global barrier instead, and so do readback buffers,
	// whose resource barriers the debug layer rejects (Agility SDK 1.619 reports BARRIER_INTEROP_INVALID_STATE, although the enhanced barrier
	// specification permits them). Placed buffers own their resource,
	// so they get barriers of their own. Global barriers cannot be split, so split barriers record them in one half: the alias global barrier
	// in the release, where the discards start, and the buffer global barrier in the acquire.
	//
	// The halves of a split barrier on queues of one type are identical, except that the release ends in the split and the acquire starts
	// from it. The pair may cross ExecuteCommandLists() boundaries, where it only transitions layouts.
	//
	// Halves on queues of different types are ordered by the queue wait of the acquire's submission, and the ExecuteCommandLists() boundary
	// between them flushes every cache. They record no global barriers, only layout transitions through a handoff layout that both queue
	// types can transition. Source layouts are native to the release queue type, destination layouts to the acquire queue type.
	enum class BarrierCase { Full, SplitRelease, SplitAcquire, TransferRelease, TransferAcquire };

	const bool transfersQueue = split != nullptr && split->IsQueueTransfer();
	BarrierCase barrierCase = BarrierCase::Full;
	if(phase == GpuBarrierPhase::Release)
		barrierCase = transfersQueue ? BarrierCase::TransferRelease : BarrierCase::SplitRelease;
	else if(phase == GpuBarrierPhase::Acquire)
		barrierCase = transfersQueue ? BarrierCase::TransferAcquire : BarrierCase::SplitAcquire;

	const bool recordsAliasGlobalBarrier = barrierCase == BarrierCase::Full || barrierCase == BarrierCase::SplitRelease;
	const bool recordsBufferGlobalBarrier = barrierCase == BarrierCase::Full || barrierCase == BarrierCase::SplitAcquire;
	const bool writesBackLayouts = barrierCase == BarrierCase::Full || barrierCase == BarrierCase::SplitAcquire || barrierCase == BarrierCase::TransferAcquire;

	const GpuQueueType releaseQueueType = transfersQueue ? *split->GetReleaseQueueType() : GetQueueType();
	const GpuQueueType acquireQueueType = transfersQueue ? *split->GetAcquireQueueType() : GetQueueType();

	// Global barriers
	GpuBarrierScope aliasScope;
	const auto fnAddAliasScope = [&aliasScope](const GpuBarrier& barrier)
	{
		aliasScope.SourceStages |= barrier.Source.Stages;
		aliasScope.SourceAccess |= barrier.Source.Access;
		aliasScope.DestinationStages |= barrier.Destination.Stages;
		aliasScope.DestinationAccess |= barrier.Destination.Access;
	};

	D3D12_GLOBAL_BARRIER bufferGlobalBarrier{ D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_ACCESS_NO_ACCESS, D3D12_BARRIER_ACCESS_NO_ACCESS };

	// Buffer barriers
	TInlineArray<D3D12_BUFFER_BARRIER, 8> bufferBarriers;
	const auto fnAddBufferBarrier = [&](D3D12Buffer& buffer, const GpuBarrier& barrier)
	{
		mResourceTracker.TrackBufferUsage(&buffer, barrier.Destination.Access | GpuAccessFlag::Read);

		if(barrier.Flags.IsSet(GpuBarrierFlag::AliasAcquire))
		{
			if(recordsAliasGlobalBarrier)
				fnAddAliasScope(barrier);

			return;
		}

		const GpuBarrierScope scope(barrier.Source.Stages, barrier.Source.Access, barrier.Destination.Stages, barrier.Destination.Access);
		D3D12BufferPage* const page = buffer.GetPage();
		if(page != nullptr || buffer.GetHeapType() == D3D12_HEAP_TYPE_READBACK)
		{
			if(recordsBufferGlobalBarrier)
			{
				const D3D12_RESOURCE_FLAGS flags = page != nullptr ? page->GetFlags() : D3D12_RESOURCE_FLAG_NONE;
				D3D12BarrierUtility::MergeGlobalBarrier(bufferGlobalBarrier, D3D12BarrierUtility::GetGlobalBufferBarrier(flags, scope));
			}

			return;
		}

		// A buffer barrier without work on either side orders nothing. Both halves of a split barrier make the same choice.
		D3D12_BUFFER_BARRIER nativeBarrier = D3D12BarrierUtility::GetBufferBarrier(buffer.GetD3D12Resource(), scope);
		if(nativeBarrier.SyncBefore == D3D12_BARRIER_SYNC_NONE || nativeBarrier.SyncAfter == D3D12_BARRIER_SYNC_NONE)
			return;

		switch(barrierCase)
		{
		case BarrierCase::Full:
			break;
		case BarrierCase::SplitRelease:
			nativeBarrier.SyncAfter = D3D12_BARRIER_SYNC_SPLIT;
			break;
		case BarrierCase::SplitAcquire:
			nativeBarrier.SyncBefore = D3D12_BARRIER_SYNC_SPLIT;
			break;
		case BarrierCase::TransferRelease:
		case BarrierCase::TransferAcquire:
			return;
		}

		bufferBarriers.Add(nativeBarrier);
	};

	// Texture barriers. Alias releases go to their own list, recorded before the activations of the textures that take over their memory.
	TInlineArray<D3D12_TEXTURE_BARRIER, 8> textureBarriers;
	TInlineArray<D3D12_TEXTURE_BARRIER, 8> aliasReleaseBarriers;
	const auto fnAddImageBarrier = [&](D3D12Image& image, const GpuTextureSubresourceRange& range, const GpuBarrier& barrier)
	{
		const bool isAliasAcquire = barrier.Flags.IsSet(GpuBarrierFlag::AliasAcquire);
		if(isAliasAcquire && !B3D_ENSURE_LOG(barrier.Source.Layout == GpuImageLayout::Undefined && image.IsFullRange(range), "An alias acquire of an image must start from GpuImageLayout::Undefined and cover the whole image."))
			return;

		// Copy queues cannot transition layouts, so their textures stay in the common layout even before their first use
		D3D12TextureLayout sourceLayout = image.GetTextureLayout(barrier.Source.Layout, releaseQueueType);
		if(releaseQueueType == GQT_TRANSFER && barrier.Source.Layout == GpuImageLayout::Undefined)
			sourceLayout = D3D12TextureLayout::Common();

		const D3D12TextureLayout destinationLayout = image.GetTextureLayout(barrier.Destination.Layout, acquireQueueType);

		// Layout transitions write the image
		GpuAccessFlags trackedAccess = barrier.Destination.Access | GpuAccessFlag::Read;
		if(sourceLayout != destinationLayout)
			trackedAccess |= GpuAccessFlag::Write;

		mResourceTracker.TrackImageUsage(&image, range, trackedAccess);

		// The transition this case records, and the queue type that performs it
		GpuBarrierScope scope(barrier.Source.Stages, barrier.Source.Access, barrier.Destination.Stages, barrier.Destination.Access);
		GpuImageLayout logicalBeforeLayout = barrier.Source.Layout;
		GpuImageLayout logicalAfterLayout = barrier.Destination.Layout;
		D3D12TextureLayout beforeLayout = sourceLayout;
		D3D12TextureLayout afterLayout = destinationLayout;
		GpuQueueType transitionQueueType = releaseQueueType;

		switch(barrierCase)
		{
		case BarrierCase::Full:
		case BarrierCase::SplitRelease:
		case BarrierCase::SplitAcquire:
			break;
		case BarrierCase::TransferRelease:
			// Nothing accesses the texture until the acquire, so the release synchronizes nothing after it
			scope.DestinationStages = GpuStageFlag::None;
			scope.DestinationAccess = GpuAccessFlag::None;
			logicalAfterLayout = GpuImageLayout::Undefined;
			afterLayout = D3D12BarrierUtility::GetTransferHandoffLayout(sourceLayout, destinationLayout, releaseQueueType);
			break;
		case BarrierCase::TransferAcquire:
			// Nothing accessed the texture since the release, so the acquire synchronizes nothing before it
			scope.SourceStages = GpuStageFlag::None;
			scope.SourceAccess = GpuAccessFlag::None;
			logicalBeforeLayout = GpuImageLayout::Undefined;
			beforeLayout = D3D12BarrierUtility::GetTransferHandoffLayout(sourceLayout, destinationLayout, releaseQueueType);
			transitionQueueType = acquireQueueType;
			break;
		}

		if(!B3D_ENSURE_LOG(D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(beforeLayout, afterLayout, range.AspectMask, transitionQueueType),
			"The barrier transitions the layout of image '{0}', which its queue type cannot do.", image.GetDebugName()))
		{
			return;
		}

		if(isAliasAcquire && recordsAliasGlobalBarrier)
			fnAddAliasScope(barrier);

		// Native texture barriers cover a single aspect
		for(GpuTextureAspectFlag aspect : { GpuTextureAspectFlag::Color, GpuTextureAspectFlag::Depth, GpuTextureAspectFlag::Stencil })
		{
			if(!range.AspectMask.IsSet(aspect))
				continue;

			GpuTextureSubresourceRange aspectRange = range;
			aspectRange.AspectMask = aspect;

			if(writesBackLayouts)
				AddImageLayout(image, aspectRange, (u32)destinationLayout.GetLayout(aspect));

			// Transfer halves synchronize nothing on one side, so they are only needed for layout transitions
			if(transfersQueue && beforeLayout.GetLayout(aspect) == afterLayout.GetLayout(aspect))
				continue;

			const D3D12_TEXTURE_BARRIER_FLAGS flags = beforeLayout.IsUndefined(aspect) ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD : D3D12_TEXTURE_BARRIER_FLAG_NONE;
			D3D12_TEXTURE_BARRIER nativeBarrier = D3D12BarrierUtility::GetTextureBarrier(image.GetD3D12Resource(), aspectRange, scope, logicalBeforeLayout, logicalAfterLayout,
				beforeLayout, afterLayout, flags);

			if(barrierCase == BarrierCase::SplitRelease)
				nativeBarrier.SyncAfter = D3D12_BARRIER_SYNC_SPLIT;
			else if(barrierCase == BarrierCase::SplitAcquire)
				nativeBarrier.SyncBefore = D3D12_BARRIER_SYNC_SPLIT;

			if(barrier.Flags.IsSet(GpuBarrierFlag::AliasRelease))
				aliasReleaseBarriers.Add(nativeBarrier);
			else
				textureBarriers.Add(nativeBarrier);
		}
	};

	// Resource resolution
	for(const GpuBufferBarrier& barrier : barriers.BufferBarriers)
	{
		auto* const gpuBuffer = static_cast<D3D12GpuBuffer*>(barrier.Object.get());
		D3D12Buffer* const buffer = gpuBuffer != nullptr ? gpuBuffer->GetD3D12Buffer() : nullptr;
		if(buffer == nullptr)
			continue;

		fnAddBufferBarrier(*buffer, barrier);
	}

	for(const GpuTextureBarrier& barrier : barriers.TextureBarriers)
	{
		auto* const texture = static_cast<D3D12Texture*>(barrier.Object.get());
		D3D12Image* const image = texture != nullptr ? texture->GetD3D12Image() : nullptr;
		if(image == nullptr)
			continue;

		fnAddImageBarrier(*image, GpuBackendUtility::ClampRange(barrier.SubresourceRange, image->GetRange()), barrier);
	}

	for(const GpuRenderTargetBarrier& barrier : barriers.RenderTargetBarriers)
	{
		RenderTarget* const renderTarget = barrier.Object.get();
		if(renderTarget == nullptr)
			continue;

		// Barriers can precede the render pass that would otherwise register the swap chain image
		D3D12Framebuffer* framebuffer;
		if(renderTarget->GetProperties().IsWindow)
			framebuffer = AcquireWindowFramebuffer(static_cast<RenderWindow&>(*renderTarget));
		else
			framebuffer = static_cast<D3D12RenderTexture*>(renderTarget)->GetFramebuffer();

		const GpuFramebufferAttachment* const attachment = framebuffer != nullptr ? framebuffer->FindAttachment(barrier.SurfaceMask) : nullptr;
		if(attachment == nullptr)
			continue;

		// The barrier covers the surface's faces and mip levels, and the aspects it selects
		GpuTextureSubresourceRange range = attachment->Range;
		range.AspectMask &= barrier.SubresourceRange.AspectMask;

		fnAddImageBarrier(static_cast<D3D12Image&>(*attachment->Image), range, barrier);
	}

	// Recording. Alias releases and global barriers go first, each in their own Barrier() call: a discard and an access of overlapping memory
	// in one call break the enhanced-barrier rules. A buffer barrier without work on either side orders nothing.
	D3D12BarrierBatch batch;
	for(const D3D12_TEXTURE_BARRIER& textureBarrier : aliasReleaseBarriers)
		batch.AddTextureBarrier(textureBarrier);

	batch.EndGroup();
	if(aliasScope.SourceStages != GpuStageFlag::None)
		batch.AddGlobalBarrier(D3D12BarrierUtility::GetAliasGlobalBarrier(aliasScope));

	if(bufferGlobalBarrier.SyncBefore != D3D12_BARRIER_SYNC_NONE && bufferGlobalBarrier.SyncAfter != D3D12_BARRIER_SYNC_NONE)
		batch.AddGlobalBarrier(bufferGlobalBarrier);

	for(const D3D12_BUFFER_BARRIER& bufferBarrier : bufferBarriers)
		batch.AddBufferBarrier(bufferBarrier);

	for(const D3D12_TEXTURE_BARRIER& textureBarrier : textureBarriers)
		batch.AddTextureBarrier(textureBarrier);

	batch.Record(*mCommandList.Get());
	return true;
}
