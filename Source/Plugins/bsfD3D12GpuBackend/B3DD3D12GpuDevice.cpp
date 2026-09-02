//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuQueue.h"
#include "String/B3DUnicode.h"
#include <chrono>
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12GpuBackend.h"
#include "Managers/B3DD3D12DescriptorManager.h"

#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12BufferPool.h"
#include "B3DD3D12GpuParameterSet.h"
#include "B3DD3D12GpuParameterSetPool.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12GpuPipelineState.h"
#include "B3DD3D12GpuProgram.h"
#include "B3DD3D12GpuTimelineFence.h"
#include "B3DD3D12Queries.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12SamplerState.h"
#include "B3DD3D12Texture.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "Utility/B3DBitwise.h"

#if B3D_PLATFORM_WIN32
#	include "Private/Win32/B3DWin32VideoModeInfo.h"
#else
	static_assert(false, "DirectX 12 is only supported on Windows.");
#endif

using namespace b3d;
using namespace b3d::render;

D3D12GpuDevice::D3D12GpuDevice(IDXGIAdapter4* adapter) : mAdapter(adapter)
{
	HRESULT hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
	if(FAILED(hr))
		B3D_LOG(Fatal, LogRenderBackend, "Failed to create the selected D3D12 device (hr={0}).", (u32)hr);

	D3D12_FEATURE_DATA_D3D12_OPTIONS12 options = {};
	hr = mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options, sizeof(options));
	if(FAILED(hr))
		B3D_LOG(Fatal, LogRenderBackend, "Failed to query D3D12_OPTIONS12 on the selected device (hr={0}).", (u32)hr);

	if(!options.EnhancedBarriersSupported)
		B3D_LOG(Fatal, LogRenderBackend, "The selected D3D12 device does not support enhanced barriers.");

	// TODO - Query D3D12_FEATURE_ARCHITECTURE and add architecture-aware custom L0 heap policies where CPU-visible default resources outperform abstract heaps, especially WRITE_BACK on CacheCoherentUMA.
	hr = mDevice.As(&mEnhancedDevice);
	if(FAILED(hr))
		B3D_LOG(Fatal, LogRenderBackend, "ID3D12Device10 is unavailable on the selected D3D12 device (hr={0}).", (u32)hr);

	constexpr D3D12_COMMAND_LIST_TYPE kCommandListTypes[GQT_COUNT] = { D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_LIST_TYPE_COPY };

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		queueDesc.Type = kCommandListTypes[queueTypeIndex];

		const bool isGraphicsQueue = queueTypeIndex == GQT_GRAPHICS;

		ID3D12CommandQueue* commandQueue;
		hr = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
		B3D_ASSERT((SUCCEEDED(hr) || !isGraphicsQueue) && "Failed to create the graphics command queue");

		if (FAILED(hr) && !isGraphicsQueue)
			continue;

		mQueueInfos[queueTypeIndex].Queues.push_back(B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuQueue>()) D3D12GpuQueue(*this, (GpuQueueType)queueTypeIndex, 0, commandQueue)));
	}

	mQueueInfos[GQT_GRAPHICS].Queues[0]->GetD3D12Handle()->GetTimestampFrequency(&mTimestampFrequency);

	InitializeCapabilities();

	mDescriptorManager = B3DNew<D3D12DescriptorManager>(*this);
	mResourceManager = B3DNew<D3D12ResourceManager>(*this);
	mHeapBackend = B3DMakeUnique<D3D12HeapBackend>(*this);
	mBufferPool = B3DMakeUnique<D3D12BufferPool>(*this);

#if B3D_PLATFORM_WIN32
	mVideoModeInfo = B3DMakeShared<Win32VideoModeInfo>();
#else
	static_assert(false, "mVideoModeInfo needs to be created.");
#endif

#if B3D_BUILD_TYPE_DEVELOPMENT
	// TODO: Replace polling with an event registered against UINT64_MAX on a monitored fence, which D3D12 signals on
	// device removal. Route that notification and failed D3D12 calls through a thread-safe log-once DRED handler.
	mWatchdogShouldExit.store(false, std::memory_order_relaxed);
	mDeviceRemovalWatchdog = std::thread([this]()
	{
		while (!mWatchdogShouldExit.load(std::memory_order_relaxed))
		{
			if (mDevice->GetDeviceRemovedReason() != S_OK)
			{
				if (!mLoggedDeviceRemoval)
				{
					mLoggedDeviceRemoval = true;
					LogDeviceRemovalBreadcrumbs();
				}

				return;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	});
#endif

	// All native queues and managers are ready; submissions remain owned by the device until destruction.
	IGpuSubmitThreadBackend& submitThreadBackend = *this;
	mSubmitThread = B3DMakeUnique<GpuSubmitThread>(*this, submitThreadBackend);
}

D3D12GpuDevice::~D3D12GpuDevice()
{
	if (mSubmitThread != nullptr)
	{
		WaitUntilIdle();
		mSubmitThread = nullptr;
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	mWatchdogShouldExit.store(true, std::memory_order_relaxed);
	if (mDeviceRemovalWatchdog.joinable())
		mDeviceRemovalWatchdog.join();
#endif

	mCachedSamplerStates.clear();

	for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		for (TShared<D3D12GpuQueue>& queue : mQueueInfos[queueTypeIndex].Queues)
			queue = nullptr;
	}

	// Buffer pages own allocations from the native heap allocators and manager-owned page resources.
	mBufferPool.reset();

	B3DDelete(mResourceManager);
	B3DDelete(mDescriptorManager);

	// Allocators own the native heaps, so destroy every allocator before its heap backend.
	for(TUnique<GpuMemoryAllocator>& allocator : mGpuMemoryAllocators)
	{
		if(allocator != nullptr)
		{
			allocator->ReclaimUnused(true);
			allocator.reset();
		}
	}
	mHeapBackend.reset();

	mEnhancedDevice.Reset();
	mDevice.Reset();
	mAdapter.Reset();
}

TShared<GpuQueue> D3D12GpuDevice::GetQueue(GpuQueueType type, u32 index) const
{
	if (index >= GetQueueCount(type))
		return nullptr;

	return mQueueInfos[(u32)type].Queues[index];
}

TShared<GpuCommandBufferPool> D3D12GpuDevice::CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation)
{
	return B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuCommandBufferPool>()) D3D12GpuCommandBufferPool(*this, createInformation));
}

TShared<render::Texture> D3D12GpuDevice::CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	D3D12Texture* rawTexture = new (B3DAllocate<D3D12Texture>()) D3D12Texture(createInformation, *this);

	TShared<Texture> output = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy) ? B3DMakeSharedFromExisting(rawTexture) : MakeSharedStandalone<D3D12Texture>(rawTexture);

	output->SetShared(output);

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

TShared<render::GpuBuffer> D3D12GpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	const D3D12BufferPool::MemoryType memoryType = (D3D12BufferPool::MemoryType)PickBufferMemoryType(createInformation);
	if(memoryType == D3D12BufferPool::MemoryType::Count)
	{
		// TODO - Fall back to device-local memory when CPU-visible storage is combined with unsupported resource flags.
		B3D_LOG(Error, LogRenderBackend, "D3D12: Unsupported buffer memory configuration (type={0}, flags={1}).", (u32)createInformation.Type, (u32)createInformation.Flags);
		return nullptr;
	}

	return CreateGpuBuffer(createInformation, mBufferPool->GetOrCreatePersistentAllocator(memoryType), flags);
}

TShared<render::GpuBuffer> D3D12GpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, IGpuAllocator& allocator, GpuObjectCreateFlags flags)
{
	D3D12GpuBuffer* rawBuffer = new (B3DAllocate<D3D12GpuBuffer>()) D3D12GpuBuffer(createInformation, *this, allocator);

	TShared<GpuBuffer> output = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy) ? B3DMakeSharedFromExisting(rawBuffer) : MakeSharedStandalone<D3D12GpuBuffer>(rawBuffer);

	output->SetShared(output);

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

u32 D3D12GpuDevice::PickBufferMemoryType(const GpuBufferCreateInformation& createInformation) const
{
	const D3D12_HEAP_TYPE heapType = D3D12Utility::GetHeapType(createInformation.Type, createInformation.Flags);
	const D3D12_RESOURCE_FLAGS resourceFlags = D3D12Utility::GetBufferResourceFlags(createInformation.Flags);
	return (u32)D3D12BufferPool::GetMemoryType(heapType, resourceFlags);
}

TUnique<IGpuAllocator> D3D12GpuDevice::CreateTransientAllocator(u32 memoryType, IGpuCompletionTracker& completionTracker)
{
	return mBufferPool->CreateTransientAllocator(memoryType, completionTracker);
}

TShared<GpuQueryPool> D3D12GpuDevice::CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation)
{
	return B3DMakeShared<D3D12GpuQueryPool>(*this, createInformation);
}

TShared<SamplerState> D3D12GpuDevice::CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	TShared<SamplerState> output = B3DMakeSharedFromExisting(new (B3DAllocate<D3D12SamplerState>()) D3D12SamplerState(createInformation, *this));

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

TShared<EventQuery> D3D12GpuDevice::CreateEventQuery()
{
	return B3DMakeSharedFromExisting(new (B3DAllocate<D3D12EventQuery>()) D3D12EventQuery());
}

TShared<GpuProgram> D3D12GpuDevice::CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	TShared<GpuProgram> output = B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuProgram>()) D3D12GpuProgram(createInformation, *this));

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

TShared<GpuGraphicsPipelineState> D3D12GpuDevice::CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	TShared<D3D12GpuGraphicsPipelineState> output = B3DMakeSharedFromExisting<D3D12GpuGraphicsPipelineState>(new (B3DAllocate<D3D12GpuGraphicsPipelineState>()) D3D12GpuGraphicsPipelineState(createInformation, *this));

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

TShared<GpuComputePipelineState> D3D12GpuDevice::CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	TShared<D3D12GpuComputePipelineState> output = B3DMakeSharedFromExisting<D3D12GpuComputePipelineState>(new (B3DAllocate<D3D12GpuComputePipelineState>()) D3D12GpuComputePipelineState(createInformation, *this));

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

TShared<GpuPipelineParameterLayout> D3D12GpuDevice::CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation)
{
	return B3DMakeSharedFromExisting<D3D12GpuPipelineParameterLayout>(new (B3DAllocate<D3D12GpuPipelineParameterLayout>()) D3D12GpuPipelineParameterLayout(createInformation, *this));
}

TShared<GpuPipelineParameterSetLayout> D3D12GpuDevice::CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& /*resourceTableLayout*/, u32 /*tableIndex*/)
{
	return B3DMakeShared<D3D12GpuPipelineParameterSetLayout>(parameterDescription);
}

u32 D3D12GpuDevice::GetUniformBufferParameterSlot(u32 registerIndex) const
{
	// Slots encode the HLSL register class alongside the register index (see MapRegisterToSlot()); engine-authored
	// parameter descriptions must use the same encoding to remain slot-compatible with shader-reflected layouts.
	return MapRegisterToSlot(registerIndex, HLSLRegisterClass::ConstantBuffer);
}

TUnique<GpuParameterSetPool> D3D12GpuDevice::CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation)
{
	return B3DMakeUnique<D3D12GpuParameterSetPool>(*this, createInformation);
}

TShared<GpuTimelineFence> D3D12GpuDevice::CreateTimelineFence()
{
	return B3DMakeShared<D3D12GpuTimelineFence>(*this);
}

void D3D12GpuDevice::WaitUntilIdle()
{
	// The submit thread lives from the end of construction to the start of destruction; outside that window the
	// native wait is sufficient.
	if (mSubmitThread == nullptr)
		ExecuteWaitUntilIdle();
	else
		GetSubmitThread().WaitUntilIdle();

}

void D3D12GpuDevice::NotifyWillQueueForSubmit(GpuCommandBuffer& commandBuffer)
{
	static_cast<D3D12GpuCommandBuffer&>(commandBuffer).NotifyWillQueueForSubmit();
}

void D3D12GpuDevice::ExecuteSubmit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences)
{
	const TShared<D3D12GpuCommandBuffer> d3d12CommandBuffer = std::static_pointer_cast<D3D12GpuCommandBuffer>(commandBuffer);
	const D3D12GpuCommandBufferSubmitInformation submitInformation = d3d12CommandBuffer->PrepareForSubmitOnSubmitThread(queue.GetType(), queue.GetIndex());
	static_cast<D3D12GpuQueue&>(queue).ExecuteSubmitOnSubmitThread(submitInformation, syncMask, signalFences);
}

void D3D12GpuDevice::RefreshCompletionState(GpuQueue& queue, bool forceWait, u32 lastSubmitIndex)
{
	static_cast<D3D12GpuQueue&>(queue).RefreshCompletionState(forceWait, lastSubmitIndex);
}

u32 D3D12GpuDevice::GetLastSubmitIndex(const GpuQueue& queue) const
{
	return static_cast<const D3D12GpuQueue&>(queue).GetLastSubmitIndex();
}

void D3D12GpuDevice::ExecuteWaitUntilIdle()
{
	for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		const u32 queueCount = GetQueueCount((GpuQueueType)queueTypeIndex);
		for (u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
		{
			const TShared<D3D12GpuQueue> queue = std::static_pointer_cast<D3D12GpuQueue>(GetQueue((GpuQueueType)queueTypeIndex, queueIndex));
			if (queue)
				queue->WaitUntilIdleNative();
		}
	}
}

void D3D12GpuDevice::ExecuteWaitUntilIdle(GpuQueue& queue)
{
	static_cast<D3D12GpuQueue&>(queue).WaitUntilIdleNative();
}

void D3D12GpuDevice::BeginFrame()
{
	ASSERT_IF_NOT_RENDER_THREAD
}

void D3D12GpuDevice::EndFrame()
{
	ASSERT_IF_NOT_RENDER_THREAD

	// Signal end-of-frame to submit thread. This blocks until the previous frame's resources are safe to reuse.
	GetSubmitThread().QueueEndFrameAndWaitForPreviousFrame();
}

D3D12GpuDevice::MemoryPoolType D3D12GpuDevice::GetMemoryPoolType(const D3D12_RESOURCE_DESC& resourceDesc, D3D12_HEAP_TYPE heapType)
{
	if(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		switch(heapType)
		{
		case D3D12_HEAP_TYPE_DEFAULT: return MemoryPoolType::DefaultBuffer;
		case D3D12_HEAP_TYPE_UPLOAD: return MemoryPoolType::UploadBuffer;
		case D3D12_HEAP_TYPE_READBACK: return MemoryPoolType::ReadbackBuffer;
		default: return MemoryPoolType::Count;
		}
	}

	if(heapType != D3D12_HEAP_TYPE_DEFAULT)
		return MemoryPoolType::Count;

	const bool isRenderTarget = (resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) != 0;
	if(isRenderTarget)
		return resourceDesc.SampleDesc.Count > 1 ? MemoryPoolType::DefaultMsaaRenderTargetTexture : MemoryPoolType::DefaultRenderTargetTexture;

	return resourceDesc.SampleDesc.Count > 1 ? MemoryPoolType::DefaultMsaaTexture : MemoryPoolType::DefaultTexture;
}

D3D12GpuDevice::GpuMemoryAllocator& D3D12GpuDevice::GetOrCreateGpuMemoryAllocator(MemoryPoolType poolType)
{
	B3D_ASSERT(poolType < MemoryPoolType::Count);

	Lock lock(mGpuMemoryAllocatorMutex);
	TUnique<GpuMemoryAllocator>& slot = mGpuMemoryAllocators[(u32)poolType];
	if(slot != nullptr)
		return *slot;

	D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;
	u64 heapAlignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	u64 initialHeapSize = 64ull * 1024 * 1024;

	switch(poolType)
	{
	case MemoryPoolType::DefaultBuffer:
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		break;
	case MemoryPoolType::DefaultTexture:
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
		break;
	case MemoryPoolType::DefaultMsaaTexture:
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
		heapAlignment = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
		break;
	case MemoryPoolType::DefaultRenderTargetTexture:
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		break;
	case MemoryPoolType::DefaultMsaaRenderTargetTexture:
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		heapAlignment = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
		break;
	case MemoryPoolType::UploadBuffer:
		heapType = D3D12_HEAP_TYPE_UPLOAD;
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		initialHeapSize = 16ull * 1024 * 1024;
		break;
	case MemoryPoolType::ReadbackBuffer:
		heapType = D3D12_HEAP_TYPE_READBACK;
		heapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		initialHeapSize = 16ull * 1024 * 1024;
		break;
	default:
		B3D_ASSERT(false && "Invalid D3D12 memory pool type.");
		break;
	}

	GpuMemoryAllocator::Configuration configuration;
	configuration.InitialHeapSize = initialHeapSize;
	configuration.MaxHeapSize = 256ull * 1024 * 1024;
	configuration.GrowthFactor = 2;
	configuration.MaxEmptyHeapCount = 1;
	configuration.MinAllocationSize = 16;
	configuration.Granularity = 1;
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
	configuration.HeapCreateInfo.Type = heapType;
	configuration.HeapCreateInfo.Flags = heapFlags;
	configuration.HeapCreateInfo.Alignment = heapAlignment;

	slot = B3DMakeUnique<GpuMemoryAllocator>(mHeapBackend.get(), nullptr, configuration);
	return *slot;
}

HRESULT D3D12GpuDevice::CreateResource(const D3D12_RESOURCE_DESC& resourceDesc, D3D12_HEAP_TYPE heapType, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ComPtr<ID3D12Resource>& outResource, GpuResourceLocation& outAllocation)
{
	B3D_ASSERT(!outAllocation.IsValid());
	outResource.Reset();

	D3D12_RESOURCE_DESC1 enhancedResourceDescription = {};
	enhancedResourceDescription.Dimension = resourceDesc.Dimension;
	enhancedResourceDescription.Alignment = resourceDesc.Alignment;
	enhancedResourceDescription.Width = resourceDesc.Width;
	enhancedResourceDescription.Height = resourceDesc.Height;
	enhancedResourceDescription.DepthOrArraySize = resourceDesc.DepthOrArraySize;
	enhancedResourceDescription.MipLevels = resourceDesc.MipLevels;
	enhancedResourceDescription.Format = resourceDesc.Format;
	enhancedResourceDescription.SampleDesc = resourceDesc.SampleDesc;
	enhancedResourceDescription.Layout = resourceDesc.Layout;
	enhancedResourceDescription.Flags = resourceDesc.Flags;

	const MemoryPoolType poolType = GetMemoryPoolType(resourceDesc, heapType);
	if(poolType == MemoryPoolType::Count)
		return E_INVALIDARG;

	// TODO - Query D3D12_FEATURE_D3D12_TIGHT_ALIGNMENT and use D3D12_RESOURCE_FLAG_USE_TIGHT_ALIGNMENT to avoid 64 KiB placement granularity where supported.
	const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = mDevice->GetResourceAllocationInfo(0, 1, &resourceDesc);
	if(allocationInfo.SizeInBytes == UINT64_MAX || allocationInfo.Alignment == 0 || allocationInfo.Alignment > UINT32_MAX)
		return E_INVALIDARG;

	GpuMemoryAllocator& allocator = GetOrCreateGpuMemoryAllocator(poolType);
	const GpuResourceKind resourceKind = resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER || resourceDesc.Layout == D3D12_TEXTURE_LAYOUT_ROW_MAJOR ? GpuResourceKind::Linear : GpuResourceKind::NonLinear;

	if(!allocator.TryAllocate(allocationInfo.SizeInBytes, (u32)allocationInfo.Alignment, resourceKind, outAllocation))
		return E_OUTOFMEMORY;

	D3D12GpuHeap& heap = ToD3D12GpuHeap(outAllocation.Heap);
	const HRESULT hr = mEnhancedDevice->CreatePlacedResource2(heap.Heap.Get(), outAllocation.Offset,
		&enhancedResourceDescription, initialLayout, optimizedClearValue, 0, nullptr, IID_PPV_ARGS(&outResource));
	if(FAILED(hr))
		allocator.Free(outAllocation);

	return hr;
}

void D3D12GpuDevice::FreeMemory(GpuResourceLocation& allocation)
{
	if(!allocation.IsValid())
		return;

	allocation.Allocator->Free(allocation);
}

namespace
{
	/** Returns a human-readable name for a DRED auto-breadcrumb operation. */
	const char* GetBreadcrumbOperationName(D3D12_AUTO_BREADCRUMB_OP operation)
	{
		switch (operation)
		{
		case D3D12_AUTO_BREADCRUMB_OP_SETMARKER: return "SetMarker";
		case D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT: return "BeginEvent";
		case D3D12_AUTO_BREADCRUMB_OP_ENDEVENT: return "EndEvent";
		case D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED: return "DrawInstanced";
		case D3D12_AUTO_BREADCRUMB_OP_DRAWINDEXEDINSTANCED: return "DrawIndexedInstanced";
		case D3D12_AUTO_BREADCRUMB_OP_EXECUTEINDIRECT: return "ExecuteIndirect";
		case D3D12_AUTO_BREADCRUMB_OP_DISPATCH: return "Dispatch";
		case D3D12_AUTO_BREADCRUMB_OP_COPYBUFFERREGION: return "CopyBufferRegion";
		case D3D12_AUTO_BREADCRUMB_OP_COPYTEXTUREREGION: return "CopyTextureRegion";
		case D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE: return "CopyResource";
		case D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCE: return "ResolveSubresource";
		case D3D12_AUTO_BREADCRUMB_OP_CLEARRENDERTARGETVIEW: return "ClearRenderTargetView";
		case D3D12_AUTO_BREADCRUMB_OP_CLEARUNORDEREDACCESSVIEW: return "ClearUnorderedAccessView";
		case D3D12_AUTO_BREADCRUMB_OP_CLEARDEPTHSTENCILVIEW: return "ClearDepthStencilView";
		case D3D12_AUTO_BREADCRUMB_OP_RESOURCEBARRIER: return "ResourceBarrier";
		case D3D12_AUTO_BREADCRUMB_OP_EXECUTEBUNDLE: return "ExecuteBundle";
		case D3D12_AUTO_BREADCRUMB_OP_PRESENT: return "Present";
		case D3D12_AUTO_BREADCRUMB_OP_RESOLVEQUERYDATA: return "ResolveQueryData";
		case D3D12_AUTO_BREADCRUMB_OP_BEGINSUBMISSION: return "BeginSubmission";
		case D3D12_AUTO_BREADCRUMB_OP_ENDSUBMISSION: return "EndSubmission";
		case D3D12_AUTO_BREADCRUMB_OP_WRITEBUFFERIMMEDIATE: return "WriteBufferImmediate";
		default: return "<other>";
		}
	}
} // namespace

void D3D12GpuDevice::LogDeviceRemovalBreadcrumbs()
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	// DRED 1.2 (GetAutoBreadcrumbsOutput1) additionally carries per-breadcrumb context strings - the BeginEvent
	// marker names - which identify the hanging render pass by name.
	ComPtr<ID3D12DeviceRemovedExtendedData1> dred;
	if (FAILED(mDevice.As(&dred)))
		return;

	D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbs = {};
	if (SUCCEEDED(dred->GetAutoBreadcrumbsOutput1(&breadcrumbs)))
	{
		for (const D3D12_AUTO_BREADCRUMB_NODE1* node = breadcrumbs.pHeadAutoBreadcrumbNode; node != nullptr; node = node->pNext)
		{
			const u32 completedOperationCount = node->pLastBreadcrumbValue != nullptr ? *node->pLastBreadcrumbValue : 0;
			const u32 totalOperationCount = node->BreadcrumbCount;

			// Fully completed command lists are not where the hang is
			if (completedOperationCount == totalOperationCount)
				continue;

			String commandListName = "<unnamed>";
			if (node->pCommandListDebugNameA != nullptr)
				commandListName = node->pCommandListDebugNameA;
			else if (node->pCommandListDebugNameW != nullptr)
				commandListName = UTF8::FromWide(WString(node->pCommandListDebugNameW));

			B3D_LOG(Error, LogRenderBackend, "DRED: Command list '{0}' hung at op {1}/{2}.", commandListName, completedOperationCount, totalOperationCount);

			// Gather the marker strings so operations can be annotated with the pass they belong to
			UnorderedMap<u32, String> contextsByOperationIndex;
			for (u32 contextIndex = 0; contextIndex < node->BreadcrumbContextsCount; contextIndex++)
			{
				const D3D12_DRED_BREADCRUMB_CONTEXT& context = node->pBreadcrumbContexts[contextIndex];
				contextsByOperationIndex[context.BreadcrumbIndex] = UTF8::FromWide(WString(context.pContextString));
			}

			// Log the event markers preceding the hang (the innermost ones identify the active pass)
			for (i32 operationIndex = (i32)completedOperationCount; operationIndex >= 0; operationIndex--)
			{
				if (const auto contextIterator = contextsByOperationIndex.find((u32)operationIndex); contextIterator != contextsByOperationIndex.end())
				{
					B3D_LOG(Error, LogRenderBackend, "DRED:   last marker before hang: op[{0}] '{1}'", operationIndex, contextIterator->second);
					break;
				}
			}

			// Log the operations surrounding the hang point for context
			const u32 contextStart = completedOperationCount >= 10 ? completedOperationCount - 10 : 0;
			const u32 contextEnd = Math::Min(completedOperationCount + 4, totalOperationCount);
			for (u32 operationIndex = contextStart; operationIndex < contextEnd; operationIndex++)
			{
				String annotation;
				if (const auto contextIterator = contextsByOperationIndex.find(operationIndex); contextIterator != contextsByOperationIndex.end())
					annotation = " '" + contextIterator->second + "'";

				B3D_LOG(Error, LogRenderBackend, "DRED:   op[{0}]{1} {2}{3}", operationIndex, operationIndex == completedOperationCount ? " <-- HUNG" : "", GetBreadcrumbOperationName(node->pCommandHistory[operationIndex]), annotation);
			}
		}
	}

	D3D12_DRED_PAGE_FAULT_OUTPUT pageFault = {};
	if (SUCCEEDED(dred->GetPageFaultAllocationOutput(&pageFault)) && pageFault.PageFaultVA != 0)
		B3D_LOG(Error, LogRenderBackend, "DRED: Page fault at GPU VA {0:x}.", pageFault.PageFaultVA);
#endif
}

void D3D12GpuDevice::LogDebugLayerMessages()
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	ComPtr<ID3D12InfoQueue> infoQueue;
	if (FAILED(mDevice.As(&infoQueue)))
		return;

	// On device removal, log the DRED breadcrumbs (which command hung) once, before the queued messages
	if (mDevice->GetDeviceRemovedReason() != S_OK && !mLoggedDeviceRemoval)
	{
		mLoggedDeviceRemoval = true;
		LogDeviceRemovalBreadcrumbs();
	}

	const u64 messageCount = infoQueue->GetNumStoredMessages();
	for (u64 messageIndex = 0; messageIndex < messageCount; messageIndex++)
	{
		SIZE_T messageLength = 0;
		if (FAILED(infoQueue->GetMessage(messageIndex, nullptr, &messageLength)) || messageLength == 0)
			continue;

		Vector<u8> storage(messageLength);
		D3D12_MESSAGE* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
		if (FAILED(infoQueue->GetMessage(messageIndex, message, &messageLength)))
			continue;

		const StringView text(message->pDescription, (u32)message->DescriptionByteLength > 0 ? (u32)message->DescriptionByteLength - 1 : 0);
		if (message->Severity <= D3D12_MESSAGE_SEVERITY_ERROR)
			B3D_LOG(Error, LogRenderBackend, "D3D12 validation: {0}", text);
		else if (message->Severity == D3D12_MESSAGE_SEVERITY_WARNING)
			B3D_LOG(Warning, LogRenderBackend, "D3D12 validation: {0}", text);
	}

	infoQueue->ClearStoredMessages();
#endif
}

void D3D12GpuDevice::PresentRenderWindow(const TShared<render::RenderWindow>& renderWindow, GpuQueueMask syncMask)
{
	TShared<GpuQueue> queue = GetQueue(GQT_GRAPHICS, 0);
	if (!B3D_ENSURE(queue))
		return;

	queue->PresentRenderWindow(renderWindow, syncMask);
}

void D3D12GpuDevice::ConvertProjectionMatrix(const Matrix4& input, Matrix4& outMatrix)
{
	outMatrix = input;

	// Convert the depth range from [-1,1] to [0,1]. Unlike Vulkan, no Y-axis flip is required.
	outMatrix[2][0] = (outMatrix[2][0] + outMatrix[3][0]) / 2;
	outMatrix[2][1] = (outMatrix[2][1] + outMatrix[3][1]) / 2;
	outMatrix[2][2] = (outMatrix[2][2] + outMatrix[3][2]) / 2;
	outMatrix[2][3] = (outMatrix[2][3] + outMatrix[3][3]) / 2;
}

GpuUniformBufferInformation D3D12GpuDevice::GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms)
{
	GpuUniformBufferInformation uniformBufferInformation;
	uniformBufferInformation.Size = 0; // In multiples of 4 bytes
	uniformBufferInformation.IsShareable = true;
	uniformBufferInformation.Name = name;
	uniformBufferInformation.Slot = 0;
	uniformBufferInformation.Set = 0;

	// The engine's uniform block definitions are authored to lay out identically under std140 and HLSL constant
	// buffer packing, so the shared std140 helper computes the sizes and offsets (same as the other backends)
	for (GpuUniformBufferMemberInformation& member : inOutUniforms)
	{
		u32 size; // In multiples of 4 bytes
		if (member.Type == GPDT_STRUCT)
		{
			// Structs are always aligned and rounded up to vec4 (16 bytes)
			size = Math::DivideAndRoundUp(member.ElementSize, 16U) * 4;
			uniformBufferInformation.Size = Math::DivideAndRoundUp(uniformBufferInformation.Size, 4U) * 4;
		}
		else
		{
			size = GpuBackendUtility::CalcStd140MemberSizeAndOffset(member.Type, member.ArraySize, uniformBufferInformation.Size);
		}

		member.ElementSize = size;
		member.ArrayElementStride = size;
		member.CpuOffset = uniformBufferInformation.Size;
		member.GpuOffset = 0;
		uniformBufferInformation.Size += size * member.ArraySize;
		member.ParentUniformBufferSlot = 0;
		member.ParentUniformBufferSet = 0;
	}

	// Constant buffer size must always be a multiple of 16 bytes (4 words)
	if (uniformBufferInformation.Size % 4 != 0)
		uniformBufferInformation.Size += (4 - (uniformBufferInformation.Size % 4));

	return uniformBufferInformation;
}

float D3D12GpuDevice::ConvertTimestampToMilliseconds(u64 timestamp)
{
	if (mTimestampFrequency == 0)
		return 0.0f;

	const double millisecondsPerTick = 1000.0 / (double)mTimestampFrequency;
	return (float)((double)timestamp * millisecondsPerTick);
}

void D3D12GpuDevice::InitializeCapabilities()
{
	DXGI_ADAPTER_DESC3 adapterDesc;
	mAdapter->GetDesc3(&adapterDesc);

	char deviceName[128];
	wcstombs(deviceName, adapterDesc.Description, sizeof(deviceName));

	mCapabilities.DeviceName = deviceName;
	mCapabilities.BackendName = "DirectX12";

	mCapabilities.VertexBufferCount = D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
	mCapabilities.RenderTargetCount = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;

	mCapabilities.SampledTexturesPerStage[GPT_VERTEX_PROGRAM] = D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
	mCapabilities.SampledTexturesPerStage[GPT_FRAGMENT_PROGRAM] = D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
	mCapabilities.SampledTexturesPerStage[GPT_COMPUTE_PROGRAM] = D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;

	mCapabilities.UniformBufferCountPerStage[GPT_VERTEX_PROGRAM] = D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
	mCapabilities.UniformBufferCountPerStage[GPT_FRAGMENT_PROGRAM] = D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
	mCapabilities.UniformBufferCountPerStage[GPT_COMPUTE_PROGRAM] = D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;

	mCapabilities.StorageTexturesPerStage[GPT_FRAGMENT_PROGRAM] = D3D12_UAV_SLOT_COUNT;
	mCapabilities.StorageTexturesPerStage[GPT_COMPUTE_PROGRAM] = D3D12_UAV_SLOT_COUNT;

	// Constant buffer addresses (root CBVs and suballocation offsets alike) must be 256-byte aligned in D3D12
	mCapabilities.MinimumUniformBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
	mCapabilities.MaximumPushConstantSize = kMaxPushConstantSizeInBytes;

	mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_BC);
	mCapabilities.SetCapability(RSC_GEOMETRY_PROGRAM);
	mCapabilities.SetCapability(RSC_COMPUTE_PROGRAM);
	mCapabilities.SetCapability(RSC_LOAD_STORE);
	mCapabilities.SetCapability(RSC_LOAD_STORE_MSAA);
	mCapabilities.SetCapability(RSC_BYTECODE_CACHING);
	mCapabilities.SetCapability(RSC_TEXTURE_VIEWS);
	mCapabilities.SetCapability(RSC_RENDER_TARGET_LAYERS);

	mCapabilities.Conventions.NdcYAxis = GpuBackendConventions::Axis::Up;
	mCapabilities.Conventions.MatrixOrder = GpuBackendConventions::MatrixOrder::ColumnMajor;

	switch (adapterDesc.VendorId)
	{
	case 0x10DE:
		mCapabilities.DeviceVendor = GPU_NVIDIA;
		break;
	case 0x1002:
	case 0x1022:
		mCapabilities.DeviceVendor = GPU_AMD;
		break;
	case 0x163C:
	case 0x8086:
	case 0x8087:
		mCapabilities.DeviceVendor = GPU_INTEL;
		break;
	default:
		mCapabilities.DeviceVendor = GPU_UNKNOWN;
		break;
	}
}
