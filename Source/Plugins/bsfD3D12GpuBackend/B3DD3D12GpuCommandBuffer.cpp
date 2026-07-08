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
#include "B3DD3D12RenderWindowSurface.h"
#include "B3DD3D12GpuPipelineState.h"
#include "B3DD3D12BarrierUtility.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Profiling/B3DRenderStats.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DRenderWindow.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Converts engine draw operation to D3D12 primitive topology. */
	D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology(DrawOperationType drawOp)
	{
		switch (drawOp)
		{
		case DOT_POINT_LIST:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case DOT_LINE_LIST:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case DOT_LINE_STRIP:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case DOT_TRIANGLE_LIST:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case DOT_TRIANGLE_STRIP:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case DOT_TRIANGLE_FAN:
			// D3D12 doesn't support triangle fans, fall back to triangle list
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		default:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}
}

D3D12GpuCommandBufferPool::D3D12GpuCommandBufferPool(D3D12GpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation)
	: GpuCommandBufferPool(device, createInformation)
{
	// Convert queue type to D3D12 command list type
	D3D12_COMMAND_LIST_TYPE commandListType;
	switch (createInformation.Type)
	{
	case GQT_GRAPHICS:
		commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	case GQT_COMPUTE:
		commandListType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		break;
	case GQT_TRANSFER:
		commandListType = D3D12_COMMAND_LIST_TYPE_COPY;
		break;
	default:
		commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	}

	// Create command allocator
	HRESULT hr = device.GetD3D12Device()->CreateCommandAllocator(
		commandListType,
		IID_PPV_ARGS(&mCommandAllocator)
	);

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
	{
		// Wait for GPU to finish
		static_cast<D3D12GpuDevice&>(mGpuDevice).WaitUntilIdle();
	}

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

	// Convert queue type to command list type
	D3D12_COMMAND_LIST_TYPE commandListType;
	switch (mInformation.Type)
	{
	case GQT_GRAPHICS:
		commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	case GQT_COMPUTE:
		commandListType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		break;
	case GQT_TRANSFER:
		commandListType = D3D12_COMMAND_LIST_TYPE_COPY;
		break;
	default:
		commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	}

	// Create command list
	ComPtr<ID3D12GraphicsCommandList> commandList;
	HRESULT hr = d3d12Device.GetD3D12Device()->CreateCommandList(
		0, // Node mask
		commandListType,
		mCommandAllocator.Get(),
		nullptr, // Initial pipeline state
		IID_PPV_ARGS(&commandList)
	);

	B3D_ASSERT(SUCCEEDED(hr) && "Failed to create command list");

	// Command lists are created in recording state, close it for now
	commandList->Close();

	TShared<D3D12GpuCommandBuffer> commandBuffer = B3DMakeSharedFromExisting(
		new (B3DAllocate<D3D12GpuCommandBuffer>()) D3D12GpuCommandBuffer(
			d3d12Device, *this, mNextCommandBufferId++, commandList.Get(),
			mInformation.Thread, mInformation.Type, createInformation),
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

	// Reset the command allocator
	HRESULT hr = mCommandAllocator->Reset();
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to reset command allocator");
}

D3D12GpuCommandBuffer::D3D12GpuCommandBuffer(D3D12GpuDevice& device, D3D12GpuCommandBufferPool& pool, u32 id,
	ID3D12GraphicsCommandList* commandList, ThreadId ownerThread, GpuQueueType queueType,
	const GpuCommandBufferCreateInformation& createInformation)
	: GpuCommandBuffer(device, ownerThread, queueType, createInformation)
	, mId(id)
	, mCommandList(commandList)
	, mPool(pool)
	, mBarrierHelper(&mResourceTracker)
	, mGfxPipelineRequiresBind(true)
	, mCmpPipelineRequiresBind(true)
	, mViewportRequiresBind(true)
	, mStencilRefRequiresBind(true)
	, mScissorRequiresBind(true)
	, mBoundParamsDirty(false)
	, mVertexInputsDirty(false)
{
	// Create fence for command buffer completion
	D3D12GpuDevice& d3d12Device = GetD3D12GpuDevice();
	HRESULT hr = d3d12Device.GetD3D12Device()->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFence)
	);

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
		const u64 completedValue = mFence->GetCompletedValue();
		if (completedValue < mFenceValue)
		{
			HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (fenceEvent)
			{
				mFence->SetEventOnCompletion(mFenceValue, fenceEvent);
				WaitForSingleObject(fenceEvent, 1000); // Wait 1 second
				CloseHandle(fenceEvent);
			}
		}

		Reset();
	}

	mCommandList.Reset();
	mFence.Reset();
}

void D3D12GpuCommandBuffer::Begin()
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Ready);

	// Reset command list
	HRESULT hr = mCommandList->Reset(mPool.GetD3D12CommandAllocator(), nullptr);
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to reset command list");

	mState = GpuCommandBufferState::Recording;

	// Reset state tracking
	mLastBoundGraphicsPipeline = nullptr;
	mGfxPipelineRequiresBind = true;
	mCmpPipelineRequiresBind = true;
	mViewportRequiresBind = true;
	mStencilRefRequiresBind = true;
	mScissorRequiresBind = true;
	mBoundParamsDirty = false;
	mVertexInputsDirty = false;
}

void D3D12GpuCommandBuffer::End()
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass);

	// End render pass if active
	if (mState == GpuCommandBufferState::RecordingRenderPass)
		EndRenderPass();

	// Close command list
	HRESULT hr = mCommandList->Close();
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to close command list");

	mRenderTarget = nullptr;
	mState = GpuCommandBufferState::RecordingDone;
}

void D3D12GpuCommandBuffer::SetName(const StringView& name)
{
	GpuCommandBuffer::SetName(name);

	if (mCommandList)
	{
		std::wstring wideName(name.begin(), name.end());
		mCommandList->SetName(wideName.c_str());
	}
}

void D3D12GpuCommandBuffer::SetGpuParameterSet(const TShared<GpuParameterSet>& parameters)
{
	mBoundParams = std::static_pointer_cast<D3D12GpuParameters>(parameters);
	mBoundParamsDirty = true;
}

void D3D12GpuCommandBuffer::SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset)
{
	// TODO(d3d12-port): D3D12GpuParameters binds uniform buffers as CBVs baked into descriptor-heap tables
	// (SetGraphics/ComputeRootDescriptorTable), not as root CBVs. Applying a per-draw dynamic offset there requires
	// either promoting the affected CBV to a root descriptor (SetGraphicsRootConstantBufferView with base + offset)
	// or re-creating the CBV descriptor at the new offset, both of which need the root-signature/descriptor design
	// in B3DD3D12GpuParameterSet / B3DD3D12GpuPipelineParameterLayout (owned elsewhere) to expose the dynamic-offset
	// slot. Until then this is a no-op; the base class stores the offset but it is not yet honored at draw time.
	B3D_LOG(Warning, LogRenderBackend, "D3D12: SetDynamicBufferOffset not implemented (descriptor-table CBV binding; needs root-CBV or CBV rebuild support)");
}

void D3D12GpuCommandBuffer::SetGpuGraphicsPipelineState(const TShared<GpuGraphicsPipelineState>& pipelineState)
{
	mGraphicsPipeline = std::static_pointer_cast<D3D12GpuGraphicsPipelineState>(pipelineState);
	mGfxPipelineRequiresBind = true;
}

void D3D12GpuCommandBuffer::SetGpuComputePipelineState(const TShared<GpuComputePipelineState>& pipelineState)
{
	mComputePipeline = std::static_pointer_cast<D3D12GpuComputePipelineState>(pipelineState);
	mCmpPipelineRequiresBind = true;
}

void D3D12GpuCommandBuffer::SetVertexBuffers(u32 index, TShared<GpuBuffer>* buffers, u32 bufferCount)
{
	mVertexBuffers.clear();
	for (u32 i = 0; i < bufferCount; i++)
	{
		if (buffers[i])
			mVertexBuffers.push_back(std::static_pointer_cast<D3D12GpuBuffer>(buffers[i]));
	}

	mVertexInputsDirty = true;
}

void D3D12GpuCommandBuffer::SetIndexBuffer(const TShared<GpuBuffer>& buffer)
{
	mIndexBuffer = std::static_pointer_cast<D3D12GpuBuffer>(buffer);
	mVertexInputsDirty = true;
}

void D3D12GpuCommandBuffer::SetVertexDescription(const TShared<VertexDescription>& vertexDescription)
{
	mVertexDescription = vertexDescription;
}

void D3D12GpuCommandBuffer::SetDrawOperation(DrawOperationType operation)
{
	mDrawOp = operation;
}

void D3D12GpuCommandBuffer::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	if (!IsReadyForRender())
		return;

	BindGraphicsPipeline();
	BindDynamicStates(false);
	BindVertexInputs();
	BindGpuParams();

	// Barriers accumulated by the bind-time tracking above. Parameter sets are normally pre-registered at
	// BeginRenderPass so this is usually empty; when it is not, emitting mid-pass is legal in D3D12 (rendering uses
	// OMSetRenderTargets, not the native render-pass API).
	mBarrierHelper.Execute(*this);

	if (instanceCount == 0)
		instanceCount = 1;

	mCommandList->DrawInstanced(vertexCount, instanceCount, vertexOffset, firstInstance);
}

void D3D12GpuCommandBuffer::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	if (!IsReadyForRender())
		return;

	BindGraphicsPipeline();
	BindDynamicStates(false);
	BindVertexInputs();
	BindGpuParams();

	// See Draw() for why executing mid-pass is fine.
	mBarrierHelper.Execute(*this);

	if (instanceCount == 0)
		instanceCount = 1;

	mCommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, vertexOffset, firstInstance);
}

void D3D12GpuCommandBuffer::DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
{
	if (!mComputePipeline)
		return;

	if (mCmpPipelineRequiresBind)
	{
		mCommandList->SetPipelineState(mComputePipeline->GetD3D12PipelineState());
		mCommandList->SetComputeRootSignature(mComputePipeline->GetRootSignature());
		mCmpPipelineRequiresBind = false;
	}

	BindGpuParams();

	// Registration must run on every dispatch (not just when the bound set changes), so back-to-back dispatches
	// using the same UAVs get the write-hazard (UAV) barriers between them.
	if (mBoundParams != nullptr)
		mBoundParams->TrackBoundResources(mResourceTracker, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	mCommandList->Dispatch(groupCountX, groupCountY, groupCountZ);

	// Reset the per-dispatch shader-use flags (mirrors the Vulkan backend).
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
	mRenderTargetLoadMask = createInformation.LoadMask;
	mFramebuffer = nullptr;

	// Get framebuffer for the render target
	if (target->GetProperties().IsWindow)
	{
		// For RenderWindow, get framebuffer from the render window surface.
		// The surface/swap chain owns framebuffers for each back buffer.
		const RenderWindow* renderWindow = static_cast<const RenderWindow*>(target.get());
		const TShared<IRenderWindowSurface>& surfacePtr = renderWindow->GetRenderWindowSurface();

		if (surfacePtr)
		{
			D3D12RenderWindowSurface* d3d12Surface = static_cast<D3D12RenderWindowSurface*>(surfacePtr.get());
			D3D12SwapChain* swapChain = d3d12Surface->GetSwapChain();

			if (swapChain)
			{
				// Set the render target on the swap chain for framebuffer creation (if not already set)
				swapChain->SetRenderTarget(renderWindow);

				// Get the framebuffer for the current back buffer
				u32 backBufferIndex = swapChain->GetCurrentBackBufferIndex();
				mFramebuffer = d3d12Surface->GetFramebuffer(backBufferIndex);
			}
		}
	}
	else
	{
		// RenderTexture owns its framebuffer
		D3D12RenderTexture* renderTexture = static_cast<D3D12RenderTexture*>(target.get());
		mFramebuffer = renderTexture->GetFramebuffer();
	}

	mState = GpuCommandBufferState::RecordingRenderPass;

	// Register the framebuffer attachments (and the swap chain, for window targets) with the tracker. This must
	// come before the parameter-set registration below so that when a pass samples one of its own attachments, the
	// shared tracker sees the framebuffer use first and keeps the attachment layout authoritative.
	if (mFramebuffer != nullptr)
	{
		D3D12RenderTargetAttachment attachments[B3D_MAXIMUM_RENDER_TARGET_COUNT + 1];
		const u32 attachmentCount = BuildRenderTargetAttachments(*mFramebuffer, attachments);

		mResourceTracker.TrackRenderTargetUsage(attachments, attachmentCount, mRenderTargetReadOnlyMask, mBarrierHelper);
	}

	// Pre-register all parameter sets that will be bound during the pass, so their barriers/transitions are issued
	// before the pass begins (mirrors the Vulkan backend).
	for (const TShared<GpuParameterSet>& parameterSet : createInformation.Parameters)
	{
		if (parameterSet == nullptr)
			continue;

		static_cast<D3D12GpuParameters*>(parameterSet.get())->TrackBoundResources(mResourceTracker, mBarrierHelper);
	}

	if (target->GetProperties().IsWindow)
	{
		const RenderWindow* renderWindow = static_cast<const RenderWindow*>(target.get());
		const TShared<IRenderWindowSurface>& surfacePtr = renderWindow->GetRenderWindowSurface();
		if (surfacePtr != nullptr)
		{
			D3D12SwapChain* swapChain = static_cast<D3D12RenderWindowSurface*>(surfacePtr.get())->GetSwapChain();
			if (swapChain != nullptr)
				mResourceTracker.TrackSwapChainUsage(swapChain);
		}
	}

	mBarrierHelper.Execute(*this);

	// Set render targets if framebuffer exists
	if (mFramebuffer)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles = mFramebuffer->GetRenderTargetViews();
		const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = mFramebuffer->GetDepthStencilView();
		u32 numRTVs = mFramebuffer->GetNumColorAttachments();

		mCommandList->OMSetRenderTargets(numRTVs, rtvHandles, FALSE, dsvHandle);
	}

	// Apply clear operations requested for the render pass start
	if (createInformation.ClearMask != RT_NONE)
	{
		ClearViewportArea(GetRenderPassArea(), createInformation.ClearMask, createInformation.ClearColor,
			createInformation.ClearDepth, (u16)createInformation.ClearStencil);
	}
}

void D3D12GpuCommandBuffer::SetViewport(const Area2& area)
{
	mNormalizedViewportArea = area;
	mViewportRequiresBind = true;
}

void D3D12GpuCommandBuffer::ClearRenderTarget(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil)
{
	EnsureValidThread();

	ClearViewportArea(GetRenderPassArea(), mask, color, depth, stencil);
}

void D3D12GpuCommandBuffer::ClearViewport(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil)
{
	EnsureValidThread();

	ClearViewportArea(GetViewportArea(), mask, color, depth, stencil);
}

void D3D12GpuCommandBuffer::ClearViewportArea(const Area2I& area, RenderSurfaceMask mask, const Color& color, float depth, u16 stencil)
{
	if (!mFramebuffer)
		return;

	D3D12_RECT rect;
	rect.left = area.X;
	rect.top = area.Y;
	rect.right = area.X + area.Width;
	rect.bottom = area.Y + area.Height;

	// Clear color attachments
	if (mask != RT_NONE)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles = mFramebuffer->GetRenderTargetViews();
		const u32 numRTVs = mFramebuffer->GetNumColorAttachments();
		const float clearColor[4] = { color.R, color.G, color.B, color.A };

		for (u32 i = 0; i < numRTVs; i++)
		{
			if (mask.IsSet((RenderSurfaceMaskBits)(RT_COLOR0 << i)))
				mCommandList->ClearRenderTargetView(rtvHandles[i], clearColor, 1, &rect);
		}
	}

	// Clear depth/stencil attachment
	const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = mFramebuffer->GetDepthStencilView();
	if (dsvHandle && (mask.IsSet(RT_DEPTH) || mask.IsSet(RT_STENCIL)))
	{
		D3D12_CLEAR_FLAGS clearFlags = (D3D12_CLEAR_FLAGS)0;
		if (mask.IsSet(RT_DEPTH))
			clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
		if (mask.IsSet(RT_STENCIL))
			clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

		mCommandList->ClearDepthStencilView(*dsvHandle, clearFlags, depth, (UINT8)stencil, 1, &rect);
	}
}

u32 D3D12GpuCommandBuffer::BuildRenderTargetAttachments(const D3D12Framebuffer& framebuffer, D3D12RenderTargetAttachment* outAttachments) const
{
	u32 attachmentCount = 0;

	const u32 numColor = framebuffer.GetNumColorAttachments();
	for(u32 i = 0; i < numColor; i++)
	{
		const D3D12Framebuffer::Attachment& attachment = framebuffer.GetColorAttachment(i);
		if(attachment.Image == nullptr)
			continue;

		outAttachments[attachmentCount].Image = attachment.Image;
		outAttachments[attachmentCount].Surface = attachment.Surface;
		outAttachments[attachmentCount].ColorIndex = i;
		outAttachments[attachmentCount].IsDepthStencil = false;
		attachmentCount++;
	}

	const D3D12Framebuffer::Attachment& depthAttachment = framebuffer.GetDepthStencilAttachment();
	if(depthAttachment.Image != nullptr)
	{
		outAttachments[attachmentCount].Image = depthAttachment.Image;
		outAttachments[attachmentCount].Surface = depthAttachment.Surface;
		outAttachments[attachmentCount].ColorIndex = 0;
		outAttachments[attachmentCount].IsDepthStencil = true;
		attachmentCount++;
	}

	return attachmentCount;
}

void D3D12GpuCommandBuffer::EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom)
{
	mScissor.X = left;
	mScissor.Y = top;
	mScissor.Width = right - left;
	mScissor.Height = bottom - top;
	mIsScissorTestEnabled = true;
	mScissorRequiresBind = true;
}

void D3D12GpuCommandBuffer::DisableScissorTest()
{
	mIsScissorTestEnabled = false;
	mScissorRequiresBind = true;
}

void D3D12GpuCommandBuffer::SetStencilReferenceValue(u32 value)
{
	mStencilRef = value;
	mStencilRefRequiresBind = true;
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

	// Timestamp queries don't support BeginQuery
	if (d3d12QueryPool->GetQueryType() == GpuQueryType::Timestamp)
	{
		B3D_LOG(Error, LogRenderBackend, "Timestamp queries don't support BeginQuery, use WriteTimestamp instead");
		return;
	}

	// Begin the query
	mCommandList->BeginQuery(d3d12QueryPool->GetD3D12QueryHeap(), d3d12QueryPool->GetD3D12QueryType(), query.Id);
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

	// Timestamp queries don't support EndQuery
	if (d3d12QueryPool->GetQueryType() == GpuQueryType::Timestamp)
	{
		B3D_LOG(Error, LogRenderBackend, "Timestamp queries don't support EndQuery, use WriteTimestamp instead");
		return;
	}

	// End the query
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

	D3D12GpuQueryPool* d3d12QueryPool = static_cast<D3D12GpuQueryPool*>(queryPool.get());

	// In D3D12, we resolve query data to a buffer
	// This copies query results from the query heap to the readback buffer
	u64 destOffset = 0;

	// Resolve all allocated queries to the readback buffer
	u32 allocatedQueryCount = d3d12QueryPool->GetAllocatedQueryCount();
	if (allocatedQueryCount == 0)
		return; // Nothing to resolve

	mCommandList->ResolveQueryData(
		d3d12QueryPool->GetD3D12QueryHeap(),
		d3d12QueryPool->GetD3D12QueryType(),
		0, // Start index
		allocatedQueryCount,
		d3d12QueryPool->GetReadbackBuffer(),
		destOffset
	);
}

namespace
{
	/** Value of PIX_EVENT_ANSI_VERSION, as expected by ID3D12GraphicsCommandList Begin/SetMarker metadata. */
	constexpr UINT kPixEventAnsiVersion = 1;
}

void D3D12GpuCommandBuffer::BeginLabel(const StringView& name)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	// ANSI event marker, understood by PIX/RenderDoc without requiring the PIX event runtime
	const String eventName(name.data(), name.size());
	mCommandList->BeginEvent(kPixEventAnsiVersion, eventName.c_str(), (UINT)eventName.size() + 1);
#endif
}

void D3D12GpuCommandBuffer::EndLabel()
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	mCommandList->EndEvent();
#endif
}

void D3D12GpuCommandBuffer::InsertLabel(const StringView& name)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	const String eventName(name.data(), name.size());
	mCommandList->SetMarker(kPixEventAnsiVersion, eventName.c_str(), (UINT)eventName.size() + 1);
#endif
}

void D3D12GpuCommandBuffer::EndRenderPass()
{
	if (mState != GpuCommandBufferState::RecordingRenderPass)
		return;

	// TODO: Apply store operations
	// TODO: Resolve MSAA if needed

	if (mFramebuffer != nullptr)
	{
		// Swap-chain back buffers must be in PRESENT (COMMON) state by the time the queue presents; there is no
		// other point in the frame where a transition can be recorded, so it happens as the pass ends (Vulkan
		// analog: finalLayout = PRESENT_SRC on window render passes).
		if (mRenderTarget != nullptr && mRenderTarget->GetProperties().IsWindow)
		{
			const u32 numColor = mFramebuffer->GetNumColorAttachments();
			for (u32 i = 0; i < numColor; i++)
			{
				const D3D12Framebuffer::Attachment& attachment = mFramebuffer->GetColorAttachment(i);
				if (attachment.Image == nullptr)
					continue;

				mResourceTracker.TrackImageUsage(attachment.Image, attachment.Image->GetRange(attachment.Surface),
					GpuImageLayout::Present, GpuImageLayout::Present, GpuResourceUseFlag::ColorAttachment,
					GpuAccessFlag::Read, mBarrierHelper);
			}

			mBarrierHelper.Execute(*this);
		}

		// Reset the per-pass shader/attachment usage flags (mirrors VulkanGpuCommandBuffer::EndRenderPass). D3D12
		// needs no final-layout move: attachments stay in their in-pass states until the next transition.
		mResourceTracker.ClearShaderFlagsForAllRenderPassImageSubresources();

		D3D12RenderTargetAttachment attachments[B3D_MAXIMUM_RENDER_TARGET_COUNT + 1];
		const u32 attachmentCount = BuildRenderTargetAttachments(*mFramebuffer, attachments);
		for (u32 i = 0; i < attachmentCount; i++)
			mResourceTracker.ClearRenderTargetFlagsForImage(attachments[i].Image);
	}

	mState = GpuCommandBufferState::Recording;
}

bool D3D12GpuCommandBuffer::IsReadyForRender()
{
	if (!mGraphicsPipeline)
		return false;

	if (!mRenderTarget)
		return false;

	return true;
}

bool D3D12GpuCommandBuffer::BindGraphicsPipeline()
{
	if (!mGraphicsPipeline || !mFramebuffer)
		return false;

	// Resolve the pipeline variant matching the current framebuffer formats and draw operation. Variants are
	// cached by the pipeline state object, so this is a lookup on all but the first encounter.
	D3D12PipelineVariantKey variantKey;
	variantKey.RenderTargetCount = mFramebuffer->GetNumColorAttachments();
	for (u32 i = 0; i < variantKey.RenderTargetCount; i++)
		variantKey.RenderTargetFormats[i] = mFramebuffer->GetColorFormat(i);
	variantKey.DepthStencilFormat = mFramebuffer->GetDepthStencilFormat();
	variantKey.SampleCount = mFramebuffer->GetSampleCount();
	variantKey.TopologyType = D3D12Utility::GetPrimitiveTopologyType(mDrawOp);

	ID3D12PipelineState* pipeline = mGraphicsPipeline->FindOrCreatePipeline(variantKey);
	if (!pipeline)
		return false;

	if (pipeline != mLastBoundGraphicsPipeline)
	{
		mCommandList->SetPipelineState(pipeline);
		mLastBoundGraphicsPipeline = pipeline;
	}

	if (mGfxPipelineRequiresBind)
	{
		mCommandList->SetGraphicsRootSignature(mGraphicsPipeline->GetRootSignature());
		mGfxPipelineRequiresBind = false;
	}

	// Set primitive topology (cheap dynamic state, set to match the current draw operation)
	mCommandList->IASetPrimitiveTopology(D3D12Utility::GetPrimitiveTopology(mDrawOp));

	return true;
}

void D3D12GpuCommandBuffer::BindDynamicStates(bool forceAll)
{
	// Bind viewport
	if (mViewportRequiresBind || forceAll)
	{
		Area2I viewportArea = GetViewportArea();

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

	// Bind scissor rect
	if (mScissorRequiresBind || forceAll)
	{
		if (mIsScissorTestEnabled)
		{
			D3D12_RECT scissorRect;
			scissorRect.left = mScissor.X;
			scissorRect.top = mScissor.Y;
			scissorRect.right = mScissor.X + mScissor.Width;
			scissorRect.bottom = mScissor.Y + mScissor.Height;

			mCommandList->RSSetScissorRects(1, &scissorRect);
		}
		else
		{
			// Set scissor to full viewport
			Area2I viewportArea = GetViewportArea();
			D3D12_RECT scissorRect;
			scissorRect.left = viewportArea.X;
			scissorRect.top = viewportArea.Y;
			scissorRect.right = viewportArea.X + viewportArea.Width;
			scissorRect.bottom = viewportArea.Y + viewportArea.Height;

			mCommandList->RSSetScissorRects(1, &scissorRect);
		}

		mScissorRequiresBind = false;
	}

	// Bind stencil reference
	if (mStencilRefRequiresBind || forceAll)
	{
		mCommandList->OMSetStencilRef(mStencilRef);
		mStencilRefRequiresBind = false;
	}
}

void D3D12GpuCommandBuffer::BindVertexInputs()
{
	if (!mVertexInputsDirty)
		return;

	// Bind vertex buffers
	if (!mVertexBuffers.empty())
	{
		Vector<D3D12_VERTEX_BUFFER_VIEW> views;
		views.reserve(mVertexBuffers.size());

		for (const auto& buffer : mVertexBuffers)
		{
			if (buffer)
			{
				mResourceTracker.TrackBufferUsage(buffer->GetD3D12Buffer(), GpuResourceUseFlag::VertexBuffer,
					GpuAccessFlag::Read, mBarrierHelper);
				views.push_back(buffer->GetVertexBufferView());
			}
		}

		if (!views.empty())
			mCommandList->IASetVertexBuffers(0, (UINT)views.size(), views.data());
	}

	// Bind index buffer
	if (mIndexBuffer)
	{
		mResourceTracker.TrackBufferUsage(mIndexBuffer->GetD3D12Buffer(), GpuResourceUseFlag::IndexBuffer,
			GpuAccessFlag::Read, mBarrierHelper);
		mCommandList->IASetIndexBuffer(&mIndexBuffer->GetIndexBufferView());
	}

	mVertexInputsDirty = false;
}

void D3D12GpuCommandBuffer::BindGpuParams()
{
	if (!mBoundParamsDirty || !mBoundParams)
		return;

	// Register the set's resources with the tracker. Render-pass sets are normally pre-registered at
	// BeginRenderPass (via RenderPassCreateInformation::Parameters), in which case this only refreshes the
	// bookkeeping; for sets bound mid-pass or for compute it queues any required barriers, executed by the caller.
	mBoundParams->TrackBoundResources(mResourceTracker, mBarrierHelper);

	// Determine if we're binding for graphics or compute pipeline
	bool isGraphics = (mGraphicsPipeline != nullptr);

	// Get the device and descriptor manager
	D3D12GpuDevice& device = GetD3D12GpuDevice();
	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();

	// Set descriptor heaps (CBV/SRV/UAV and Sampler heaps must be bound before setting root parameters)
	ID3D12DescriptorHeap* descriptorHeaps[] = {
		descriptorManager.GetDescriptorHeap(D3D12DescriptorHeapType::CBV_SRV_UAV),
		descriptorManager.GetDescriptorHeap(D3D12DescriptorHeapType::Sampler)
	};
	mCommandList->SetDescriptorHeaps(2, descriptorHeaps);

	// Use the GpuParameters BindDescriptors method to handle all descriptor binding
	mBoundParams->BindDescriptors(device, mCommandList.Get(), isGraphics);

	mBoundParamsDirty = false;
}

void D3D12GpuCommandBuffer::NotifyWillQueueForSubmit()
{
	// Clear everything not allowed on the submit thread. The resources these referenced stay alive through the
	// resource tracker's bound counts until the GPU is done with them.
	mGraphicsPipeline = nullptr;
	mComputePipeline = nullptr;
	mBoundParams = nullptr;
	mIndexBuffer = nullptr;
	mVertexBuffers.clear();
}

void D3D12GpuCommandBuffer::NotifyWasSubmittedToQueue(GpuQueueId queueId)
{
	AssertIfNotSubmitThread();

	mSubmittedQueueId = queueId;
	mResourceTracker.NotifyUsed(queueId);
}

void D3D12GpuCommandBuffer::Cleanup()
{
	const bool wasSubmitted = mState == GpuCommandBufferState::Executing || mState == GpuCommandBufferState::Done;

	if (wasSubmitted)
		mResourceTracker.NotifyDone(mSubmittedQueueId);
	else
		mResourceTracker.NotifyUnbound();

	mResourceTracker.Clear();
}

bool D3D12GpuCommandBuffer::UpdateExecutionStatus(bool block)
{
	if (mState != GpuCommandBufferState::Executing && mState != GpuCommandBufferState::Done)
		return true;

	// Note: only checks the fence. The Done state transition is posted back to the command buffer's owning
	// thread by the queue's RefreshCompletionState, mirroring the other backends.
	const u64 completedValue = mFence->GetCompletedValue();

	if (completedValue >= mFenceValue)
		return true;

	if (block)
	{
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (fenceEvent)
		{
			mFence->SetEventOnCompletion(mFenceValue, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
			CloseHandle(fenceEvent);
			return true;
		}
	}

	return false;
}

void D3D12GpuCommandBuffer::Reset()
{
	// Release every tracked resource (NotifyDone/NotifyUnbound) before the state flips back to Ready
	Cleanup();

	// Mark as ready for reuse
	mState = GpuCommandBufferState::Ready;

	// Reset tracked state
	mGraphicsPipeline = nullptr;
	mComputePipeline = nullptr;
	mVertexBuffers.clear();
	mIndexBuffer = nullptr;
	mBoundParams = nullptr;
	mRenderTarget = nullptr;
	mFramebuffer = nullptr;
}

Area2I D3D12GpuCommandBuffer::GetViewportArea() const
{
	if (!mRenderTarget)
		return Area2I(0, 0, 0, 0);

	u32 width = mRenderTarget->GetProperties().Width;
	u32 height = mRenderTarget->GetProperties().Height;

	return Area2I(
		(i32)(mNormalizedViewportArea.X * width),
		(i32)(mNormalizedViewportArea.Y * height),
		(i32)(mNormalizedViewportArea.Width * width),
		(i32)(mNormalizedViewportArea.Height * height)
	);
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

		if(barrier.SourceUsage == GpuResourceUseFlag::Undefined)
			mBarrierHelper.AddBufferBarrier(buffer, barrier.DestinationUsage, barrier.DestinationAccess);
		else
			mBarrierHelper.AddBufferBarrier(buffer, barrier.SourceUsage, barrier.SourceAccess, barrier.DestinationUsage, barrier.DestinationAccess);

		// Keep the native state in sync with the destination usage (see D3D12ResourceTracker::TrackBufferUsage).
		mBarrierHelper.RequireBufferState(buffer, D3D12BarrierUtility::GetResourceState(barrier.DestinationUsage, barrier.DestinationAccess, false));
	}

	for(const auto& barrier : barriers.TextureBarriers)
	{
		D3D12Texture* const d3d12Texture = static_cast<D3D12Texture*>(barrier.Object.get());
		if(d3d12Texture == nullptr || d3d12Texture->GetD3D12Image() == nullptr)
			continue;

		D3D12Image* const image = d3d12Texture->GetD3D12Image();

		GpuTextureSubresourceRange maskedRange = barrier.SubresourceRange;
		maskedRange.AspectMask &= image->GetRange().AspectMask;

		if(barrier.SourceUsage == GpuResourceUseFlag::Undefined)
		{
			mBarrierHelper.AddImageBarrier(image, maskedRange, barrier.DestinationUsage, barrier.DestinationAccess,
				barrier.DestinationLayout);
		}
		else
		{
			mBarrierHelper.AddImageBarrier(image, maskedRange, barrier.SourceUsage, barrier.SourceAccess,
				barrier.DestinationUsage, barrier.DestinationAccess, barrier.SourceLayout, barrier.DestinationLayout);
		}
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
			const RenderWindow* const renderWindow = static_cast<const RenderWindow*>(barrier.Object.get());
			const TShared<IRenderWindowSurface>& surfacePtr = renderWindow->GetRenderWindowSurface();
			if(surfacePtr != nullptr)
			{
				D3D12RenderWindowSurface* d3d12Surface = static_cast<D3D12RenderWindowSurface*>(surfacePtr.get());
				D3D12SwapChain* swapChain = d3d12Surface->GetSwapChain();
				if(swapChain != nullptr)
					framebuffer = d3d12Surface->GetFramebuffer(swapChain->GetCurrentBackBufferIndex());
			}
		}
		else
		{
			D3D12RenderTexture* const renderTexture = static_cast<D3D12RenderTexture*>(barrier.Object.get());
			framebuffer = renderTexture->GetFramebuffer();
		}

		if(framebuffer == nullptr)
			continue;

		auto fnAddAttachmentBarrier = [this, &barrier](const D3D12Framebuffer::Attachment& attachment)
		{
			if(attachment.Image == nullptr)
				return;

			const GpuTextureSubresourceRange range = attachment.Image->GetRange(attachment.Surface);

			if(barrier.SourceUsage == GpuResourceUseFlag::Undefined)
			{
				mBarrierHelper.AddImageBarrier(attachment.Image, range, barrier.DestinationUsage,
					barrier.DestinationAccess, barrier.DestinationLayout);
			}
			else
			{
				mBarrierHelper.AddImageBarrier(attachment.Image, range, barrier.SourceUsage, barrier.SourceAccess,
					barrier.DestinationUsage, barrier.DestinationAccess, barrier.SourceLayout, barrier.DestinationLayout);
			}
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

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	D3D12GpuBuffer* d3d12Source = static_cast<D3D12GpuBuffer*>(source.get());
	D3D12GpuBuffer* d3d12Destination = static_cast<D3D12GpuBuffer*>(destination.get());

	// Track the transfer and execute the copy-state transitions it requires (UPLOAD/READBACK buffers are skipped
	// by the barrier helper). State is left in the copy state after the call (leave-and-track).
	mResourceTracker.TrackBufferUsage(d3d12Source->GetD3D12Buffer(), GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackBufferUsage(d3d12Destination->GetD3D12Buffer(), GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);
	mBarrierHelper.Execute(*this);

	CopyBufferToBufferRaw(d3d12Source->GetD3D12Resource(), d3d12Destination->GetD3D12Resource(), sourceOffset, destinationOffset, length);
}

void D3D12GpuCommandBuffer::CopyBufferToTexture(const TShared<GpuBuffer>& source, const TShared<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer)
{
	EnsureValidThread();

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	D3D12GpuBuffer* d3d12Source = static_cast<D3D12GpuBuffer*>(source.get());
	D3D12Texture* d3d12Destination = static_cast<D3D12Texture*>(destination.get());
	D3D12Image* destinationImage = d3d12Destination->GetD3D12Image();

	// Track the transfer and execute the copy-state transitions it requires.
	const GpuTextureSubresourceRange subresourceRange(mipLevel, 1, arrayLayer, 1, destinationImage->GetRange().AspectMask);
	mResourceTracker.TrackBufferUsage(d3d12Source->GetD3D12Buffer(), GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destinationImage, subresourceRange, GpuImageLayout::TransferDestination,
		GpuImageLayout::TransferDestination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);
	mBarrierHelper.Execute(*this);

	ID3D12Resource* textureResource = destinationImage->GetD3D12Resource();
	const D3D12_RESOURCE_DESC textureDesc = textureResource->GetDesc();

	// Compute the placed footprint for the requested subresource from the destination texture description.
	const u32 subresourceIndex = mipLevel + arrayLayer * textureDesc.MipLevels;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	GetD3D12GpuDevice().GetD3D12Device()->GetCopyableFootprints(&textureDesc, subresourceIndex, 1, bufferOffset, &footprint, nullptr, nullptr, nullptr);

	// The staging buffer was laid out using the texture's staging pitch, which is what the copy must read with
	footprint.Footprint.RowPitch = d3d12Destination->GetStagingRowPitchInBytes(mipLevel);

	CopyBufferToTextureRaw(d3d12Source->GetD3D12Resource(), textureResource, footprint, subresourceIndex);
}

void D3D12GpuCommandBuffer::CopyTextureToBuffer(const TShared<Texture>& source, const TShared<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset)
{
	EnsureValidThread();

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	D3D12Texture* d3d12Source = static_cast<D3D12Texture*>(source.get());
	D3D12GpuBuffer* d3d12Destination = static_cast<D3D12GpuBuffer*>(destination.get());
	D3D12Image* sourceImage = d3d12Source->GetD3D12Image();

	// Track the transfer and execute the copy-state transitions it requires.
	const GpuTextureSubresourceRange subresourceRange(mipLevel, 1, arrayLayer, 1, sourceImage->GetRange().AspectMask);
	mResourceTracker.TrackImageUsage(sourceImage, subresourceRange, GpuImageLayout::TransferSource,
		GpuImageLayout::TransferSource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackBufferUsage(d3d12Destination->GetD3D12Buffer(), GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);
	mBarrierHelper.Execute(*this);

	ID3D12Resource* textureResource = sourceImage->GetD3D12Resource();
	const D3D12_RESOURCE_DESC textureDesc = textureResource->GetDesc();

	// Compute the placed footprint for the requested subresource from the source texture description.
	const u32 subresourceIndex = mipLevel + arrayLayer * textureDesc.MipLevels;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	GetD3D12GpuDevice().GetD3D12Device()->GetCopyableFootprints(&textureDesc, subresourceIndex, 1, bufferOffset, &footprint, nullptr, nullptr, nullptr);

	// The staging buffer is laid out using the texture's staging pitch, which is what the copy must write with
	footprint.Footprint.RowPitch = d3d12Source->GetStagingRowPitchInBytes(mipLevel);

	CopyTextureToBufferRaw(textureResource, d3d12Destination->GetD3D12Resource(), footprint, subresourceIndex);
}

void D3D12GpuCommandBuffer::CopyImageToBuffer(D3D12Image* source, D3D12Buffer* destination, u32 width, u32 height, u32 rowPitchBytes)
{
	EnsureValidThread();

	if (!B3D_ENSURE(source != nullptr && destination != nullptr))
		return;

	// Track the transfer and execute the copy-state transitions it requires.
	const GpuTextureSubresourceRange subresourceRange(0, 1, 0, 1, source->GetRange().AspectMask);
	mResourceTracker.TrackImageUsage(source, subresourceRange, GpuImageLayout::TransferSource,
		GpuImageLayout::TransferSource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackBufferUsage(destination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);
	mBarrierHelper.Execute(*this);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	footprint.Offset = 0;
	footprint.Footprint.Format = source->GetDXGIFormat();
	footprint.Footprint.Width = width;
	footprint.Footprint.Height = height;
	footprint.Footprint.Depth = 1;
	footprint.Footprint.RowPitch = rowPitchBytes;

	CopyTextureToBufferRaw(source->GetD3D12Resource(), destination->GetD3D12Resource(), footprint, 0);
}

void D3D12GpuCommandBuffer::CopyBufferToBufferRaw(ID3D12Resource* source, ID3D12Resource* destination, u64 sourceOffset, u64 destinationOffset, u64 length)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording && "Command buffer must be in recording state");
	B3D_ASSERT(source && destination && "Source and destination buffers must be valid");

	mCommandList->CopyBufferRegion(destination, destinationOffset, source, sourceOffset, length);
}

void D3D12GpuCommandBuffer::CopyBufferToTextureRaw(ID3D12Resource* source, ID3D12Resource* destination, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout, u32 subresourceIndex)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording && "Command buffer must be in recording state");
	B3D_ASSERT(source && destination && "Source buffer and destination texture must be valid");

	D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
	srcLocation.pResource = source;
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	srcLocation.PlacedFootprint = layout;

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
	dstLocation.pResource = destination;
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstLocation.SubresourceIndex = subresourceIndex;

	mCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
}

void D3D12GpuCommandBuffer::CopyTextureToBufferRaw(ID3D12Resource* source, ID3D12Resource* destination, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout, u32 subresourceIndex)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording && "Command buffer must be in recording state");
	B3D_ASSERT(source && destination && "Source texture and destination buffer must be valid");

	D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
	srcLocation.pResource = source;
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	srcLocation.SubresourceIndex = subresourceIndex;

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
	dstLocation.pResource = destination;
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dstLocation.PlacedFootprint = layout;

	mCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
}

