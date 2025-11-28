//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuParameters.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12GpuQueue.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12Framebuffer.h"
#include "B3DD3D12Queries.h"
#include "B3DD3D12SwapChain.h"
#include "B3DD3D12RenderTexture.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Profiling/B3DRenderStats.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"

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
	// Convert queue usage to D3D12 command list type
	D3D12_COMMAND_LIST_TYPE commandListType;
	switch (createInformation.Usage)
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
		if (commandBufferPair.second->GetState() != CommandBufferState::Ready)
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

SPtr<GpuCommandBuffer> D3D12GpuCommandBufferPool::FindOrCreate(const GpuCommandBufferCreateInformation& createInformation)
{
	EnsureValidThread();

	// Try to find a ready command buffer
	for (const auto& commandBufferPair : mCommandBuffers)
	{
		if (commandBufferPair.second->GetState() != CommandBufferState::Ready)
			continue;

		commandBufferPair.second->SetName(createInformation.Name);
		commandBufferPair.second->Begin();

		return commandBufferPair.second;
	}

	return Create(createInformation);
}

SPtr<GpuCommandBuffer> D3D12GpuCommandBufferPool::Create(const GpuCommandBufferCreateInformation& createInformation)
{
	EnsureValidThread();

	D3D12GpuDevice& d3d12Device = static_cast<D3D12GpuDevice&>(mGpuDevice);

	// Convert queue usage to command list type
	D3D12_COMMAND_LIST_TYPE commandListType;
	switch (mInformation.Usage)
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

	SPtr<D3D12GpuCommandBuffer> commandBuffer = B3DMakeSharedFromExisting(
		new (B3DAllocate<D3D12GpuCommandBuffer>()) D3D12GpuCommandBuffer(
			d3d12Device, *this, mNextCommandBufferId++, commandList.Get(),
			mInformation.Thread, mInformation.Usage, createInformation),
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
	ID3D12GraphicsCommandList* commandList, ThreadId ownerThread, GpuQueueUsage queueType,
	const GpuCommandBufferCreateInformation& createInformation)
	: GpuCommandBuffer(device, ownerThread, queueType, createInformation)
	, mId(id)
	, mCommandList(commandList)
	, mPool(pool)
	, mOwnerThread(ownerThread)
	, mSyncMask(0)
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

	mFenceValue = 1;

	SetName(createInformation.Name);
}

D3D12GpuCommandBuffer::~D3D12GpuCommandBuffer()
{
	if (IsRecording())
	{
		End();
		Reset();
	}

	if (mState == State::Submitted || mState == State::Done)
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
	B3D_ASSERT(mState == State::Ready);

	// Reset command list
	HRESULT hr = mCommandList->Reset(mPool.GetD3D12CommandAllocator(), nullptr);
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to reset command list");

	mState = State::Recording;

	// Reset state tracking
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
	B3D_ASSERT(mState == State::Recording || mState == State::RecordingRenderPass);

	// End render pass if active
	if (mState == State::RecordingRenderPass)
		EndRenderPass();

	// Close command list
	HRESULT hr = mCommandList->Close();
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to close command list");

	mRenderTarget = nullptr;
	mState = State::RecordingDone;
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

CommandBufferState D3D12GpuCommandBuffer::GetState() const
{
	switch (mState)
	{
	case State::Ready:
		return CommandBufferState::Ready;
	case State::Recording:
	case State::RecordingRenderPass:
	case State::RecordingDone:
		return CommandBufferState::Recording;
	case State::Submitted:
		return CommandBufferState::Executing;
	case State::Done:
		return CommandBufferState::Done;
	default:
		return CommandBufferState::Ready;
	}
}

void D3D12GpuCommandBuffer::SetGpuParameterSet(const SPtr<GpuParameterSet>& parameters)
{
	mBoundParams = std::static_pointer_cast<D3D12GpuParameters>(parameters);
	mBoundParamsDirty = true;
}

void D3D12GpuCommandBuffer::SetDynamicBufferOffset(u32 bufferIndex, u32 offset)
{
	// TODO: Implement dynamic buffer offsets using root descriptors
	B3D_LOG(Warning, RenderBackend, "Dynamic buffer offsets not yet implemented for D3D12");
}

void D3D12GpuCommandBuffer::SetGpuGraphicsPipelineState(const SPtr<GpuGraphicsPipelineState>& pipelineState)
{
	mGraphicsPipeline = std::static_pointer_cast<D3D12GpuGraphicsPipelineState>(pipelineState);
	mGfxPipelineRequiresBind = true;
}

void D3D12GpuCommandBuffer::SetGpuComputePipelineState(const SPtr<GpuComputePipelineState>& pipelineState)
{
	mComputePipeline = std::static_pointer_cast<D3D12GpuComputePipelineState>(pipelineState);
	mCmpPipelineRequiresBind = true;
}

void D3D12GpuCommandBuffer::SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 bufferCount)
{
	mVertexBuffers.clear();
	for (u32 i = 0; i < bufferCount; i++)
	{
		if (buffers[i])
			mVertexBuffers.push_back(std::static_pointer_cast<D3D12GpuBuffer>(buffers[i]));
	}

	mVertexInputsDirty = true;
}

void D3D12GpuCommandBuffer::SetIndexBuffer(const SPtr<GpuBuffer>& buffer)
{
	mIndexBuffer = std::static_pointer_cast<D3D12GpuBuffer>(buffer);
	mVertexInputsDirty = true;
}

void D3D12GpuCommandBuffer::SetVertexDescription(const SPtr<VertexDescription>& vertexDescription)
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

	mCommandList->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void D3D12GpuCommandBuffer::SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask)
{
	mRenderTarget = target;
	mRenderTargetReadOnlyFlags = readOnlyFlags;
	mRenderTargetLoadMask = loadMask;

	// Get framebuffer for the render target
	if (target)
	{
		// Check if this is a RenderTexture
		D3D12RenderTexture* renderTexture = dynamic_cast<D3D12RenderTexture*>(target.get());
		if (renderTexture)
		{
			// RenderTexture owns its framebuffer
			mFramebuffer = renderTexture->GetFramebuffer();
		}
		else
		{
			// For RenderWindow, get framebuffer from the render window surface
			// The surface/swap chain owns framebuffers for each back buffer
			const RenderWindow* renderWindow = static_cast<const RenderWindow*>(target.get());
			const SPtr<IRenderWindowSurface>& surfacePtr = renderWindow->GetRenderWindowSurface();

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
				else
				{
					mFramebuffer = nullptr;
				}
			}
			else
			{
				mFramebuffer = nullptr;
			}
		}
	}
	else
	{
		mFramebuffer = nullptr;
	}

	// TODO: Transition resources to render target state

	BeginRenderPass();
}

void D3D12GpuCommandBuffer::SetViewport(const Area2& area)
{
	mNormalizedViewportArea = area;
	mViewportRequiresBind = true;
}

void D3D12GpuCommandBuffer::ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	// TODO: Implement render target clearing
	B3D_LOG(Warning, RenderBackend, "ClearRenderTarget not yet fully implemented for D3D12");
}

void D3D12GpuCommandBuffer::ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	// TODO: Implement viewport clearing
	B3D_LOG(Warning, RenderBackend, "ClearViewport not yet fully implemented for D3D12");
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

void D3D12GpuCommandBuffer::WriteTimestamp(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool)
{
	EnsureValidThread();
	B3D_ASSERT(mState == State::Recording || mState == State::RecordingRenderPass);

	if (!query.IsValid() || !queryPool)
	{
		B3D_LOG(Error, RenderBackend, "Invalid query or query pool");
		return;
	}

	D3D12GpuQueryPool* d3d12QueryPool = static_cast<D3D12GpuQueryPool*>(queryPool.get());

	if (d3d12QueryPool->GetQueryType() != GpuQueryType::Timestamp)
	{
		B3D_LOG(Error, RenderBackend, "Query pool is not a timestamp query pool");
		return;
	}

	// EndQuery for timestamp queries records the current GPU timestamp
	mCommandList->EndQuery(d3d12QueryPool->GetD3D12QueryHeap(), d3d12QueryPool->GetD3D12QueryType(), query.Id);
}

void D3D12GpuCommandBuffer::BeginQuery(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool, GpuQueryFlags flags)
{
	EnsureValidThread();
	B3D_ASSERT(mState == State::Recording || mState == State::RecordingRenderPass);

	if (!query.IsValid() || !queryPool)
	{
		B3D_LOG(Error, RenderBackend, "Invalid query or query pool");
		return;
	}

	D3D12GpuQueryPool* d3d12QueryPool = static_cast<D3D12GpuQueryPool*>(queryPool.get());

	// Timestamp queries don't support BeginQuery
	if (d3d12QueryPool->GetQueryType() == GpuQueryType::Timestamp)
	{
		B3D_LOG(Error, RenderBackend, "Timestamp queries don't support BeginQuery, use WriteTimestamp instead");
		return;
	}

	// Begin the query
	mCommandList->BeginQuery(d3d12QueryPool->GetD3D12QueryHeap(), d3d12QueryPool->GetD3D12QueryType(), query.Id);
}

void D3D12GpuCommandBuffer::EndQuery(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool)
{
	EnsureValidThread();
	B3D_ASSERT(mState == State::Recording || mState == State::RecordingRenderPass);

	if (!query.IsValid() || !queryPool)
	{
		B3D_LOG(Error, RenderBackend, "Invalid query or query pool");
		return;
	}

	D3D12GpuQueryPool* d3d12QueryPool = static_cast<D3D12GpuQueryPool*>(queryPool.get());

	// Timestamp queries don't support EndQuery
	if (d3d12QueryPool->GetQueryType() == GpuQueryType::Timestamp)
	{
		B3D_LOG(Error, RenderBackend, "Timestamp queries don't support EndQuery, use WriteTimestamp instead");
		return;
	}

	// End the query
	mCommandList->EndQuery(d3d12QueryPool->GetD3D12QueryHeap(), d3d12QueryPool->GetD3D12QueryType(), query.Id);
}

void D3D12GpuCommandBuffer::ResetQueries(const SPtr<GpuQueryPool>& queryPool)
{
	EnsureValidThread();
	B3D_ASSERT(mState == State::Recording || mState == State::RecordingRenderPass);

	if (!queryPool)
	{
		B3D_LOG(Error, RenderBackend, "Invalid query pool");
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

void D3D12GpuCommandBuffer::BeginLabel(const StringView& name)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	// Use PIX markers for debugging
	std::wstring wideName(name.begin(), name.end());
	PIXBeginEvent(mCommandList.Get(), 0, wideName.c_str());
#endif
}

void D3D12GpuCommandBuffer::EndLabel()
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	PIXEndEvent(mCommandList.Get());
#endif
}

void D3D12GpuCommandBuffer::InsertLabel(const StringView& name)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	std::wstring wideName(name.begin(), name.end());
	PIXSetMarker(mCommandList.Get(), 0, wideName.c_str());
#endif
}

void D3D12GpuCommandBuffer::BeginRenderPass()
{
	if (mState == State::RecordingRenderPass)
		return;

	mState = State::RecordingRenderPass;

	// Set render targets if framebuffer exists
	if (mFramebuffer)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles = mFramebuffer->GetRenderTargetViews();
		const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = mFramebuffer->GetDepthStencilView();
		u32 numRTVs = mFramebuffer->GetNumColorAttachments();

		mCommandList->OMSetRenderTargets(numRTVs, rtvHandles, FALSE, dsvHandle);
	}

	// TODO: Apply load operations (clear if needed based on mRenderTargetLoadMask)
}

void D3D12GpuCommandBuffer::EndRenderPass()
{
	if (mState != State::RecordingRenderPass)
		return;

	// TODO: Apply store operations
	// TODO: Resolve MSAA if needed

	mState = State::Recording;
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
	if (!mGraphicsPipeline)
		return false;

	if (mGfxPipelineRequiresBind)
	{
		mCommandList->SetPipelineState(mGraphicsPipeline->GetD3D12PipelineState());
		mCommandList->SetGraphicsRootSignature(mGraphicsPipeline->GetRootSignature());

		// Set primitive topology
		D3D_PRIMITIVE_TOPOLOGY topology = mGraphicsPipeline->GetPrimitiveTopology();
		mCommandList->IASetPrimitiveTopology(topology);

		mGfxPipelineRequiresBind = false;
	}

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
				views.push_back(buffer->GetVertexBufferView());
		}

		if (!views.empty())
			mCommandList->IASetVertexBuffers(0, (UINT)views.size(), views.data());
	}

	// Bind index buffer
	if (mIndexBuffer)
	{
		mCommandList->IASetIndexBuffer(&mIndexBuffer->GetIndexBufferView());
	}

	mVertexInputsDirty = false;
}

void D3D12GpuCommandBuffer::BindGpuParams()
{
	if (!mBoundParamsDirty || !mBoundParams)
		return;

	// Determine if we're binding for graphics or compute pipeline
	bool isGraphics = (mGfxPipeline != nullptr);

	// Get the device and descriptor manager
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(GetD3D12GpuBackend().GetPrimaryDevice());
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

bool D3D12GpuCommandBuffer::UpdateExecutionStatus(bool block)
{
	if (mState != State::Submitted && mState != State::Done)
		return true;

	const u64 completedValue = mFence->GetCompletedValue();

	if (completedValue >= mFenceValue)
	{
		mState = State::Done;
		return true;
	}

	if (block)
	{
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (fenceEvent)
		{
			mFence->SetEventOnCompletion(mFenceValue, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
			CloseHandle(fenceEvent);

			mState = State::Done;
			return true;
		}
	}

	return false;
}

void D3D12GpuCommandBuffer::Reset()
{
	// Mark as ready for reuse
	mState = State::Ready;

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

	u32 width = mRenderTarget->GetWidth();
	u32 height = mRenderTarget->GetHeight();

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

	return Area2I(0, 0, mRenderTarget->GetWidth(), mRenderTarget->GetHeight());
}

/************************************************************************/
/* 								COPY COMMANDS                     		*/
/************************************************************************/

void D3D12GpuCommandBuffer::CopyBufferToBuffer(ID3D12Resource* source, ID3D12Resource* destination, u64 sourceOffset, u64 destinationOffset, u64 length)
{
	EnsureValidThread();
	B3D_ASSERT(mState == State::Recording && "Command buffer must be in recording state");
	B3D_ASSERT(source && destination && "Source and destination buffers must be valid");

	mCommandList->CopyBufferRegion(destination, destinationOffset, source, sourceOffset, length);
}

void D3D12GpuCommandBuffer::CopyBufferToTexture(ID3D12Resource* source, ID3D12Resource* destination, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout, u32 subresourceIndex)
{
	EnsureValidThread();
	B3D_ASSERT(mState == State::Recording && "Command buffer must be in recording state");
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

void D3D12GpuCommandBuffer::CopyTextureToBuffer(ID3D12Resource* source, ID3D12Resource* destination, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout, u32 subresourceIndex)
{
	EnsureValidThread();
	B3D_ASSERT(mState == State::Recording && "Command buffer must be in recording state");
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

void D3D12GpuCommandBuffer::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, u32 subresource)
{
	EnsureValidThread();
	B3D_ASSERT(mState == State::Recording && "Command buffer must be in recording state");
	B3D_ASSERT(resource && "Resource must be valid");

	// Skip transition if states are the same
	if (stateBefore == stateAfter)
		return;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource;
	barrier.Transition.Subresource = subresource;
	barrier.Transition.StateBefore = stateBefore;
	barrier.Transition.StateAfter = stateAfter;

	mCommandList->ResourceBarrier(1, &barrier);
}
