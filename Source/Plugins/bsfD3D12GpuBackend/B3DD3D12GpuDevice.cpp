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
#include "ThirdParty/D3D12MemAlloc.h"

#include "B3DD3D12GpuBuffer.h"
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
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "Utility/B3DBitwise.h"

#if B3D_PLATFORM_WIN32
#	include "Private/Win32/B3DWin32VideoModeInfo.h"
#else
	static_assert(false, "DirectX 12 is only supported on Windows.");
#endif

using namespace b3d;
using namespace b3d::render;

D3D12GpuDevice::D3D12GpuDevice(IDXGIAdapter4* adapter)
	: mAdapter(adapter)
{
	HRESULT hr;

	// Create D3D12 device
	hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
	B3D_ASSERT(SUCCEEDED(hr) && "Failed to create D3D12 device");

#if B3D_BUILD_TYPE_DEVELOPMENT
	{
		// Clear colors are runtime-driven (viewport background, render target clear parameters) so they can never
		// reliably match the fixed optimized clear value provided at resource creation; the mismatch message is a
		// pure performance note that would otherwise spam the log on every clear.
		ComPtr<ID3D12InfoQueue> infoQueue;
		if (SUCCEEDED(mDevice.As(&infoQueue)))
		{
			D3D12_MESSAGE_ID deniedMessages[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE
			};

			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = (UINT)std::size(deniedMessages);
			filter.DenyList.pIDList = deniedMessages;
			infoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif

	// Create command queues for each queue type
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	// Graphics queue (always present)
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ID3D12CommandQueue* graphicsQueue;
	hr = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&graphicsQueue));
	B3D_ASSERT(SUCCEEDED(hr));

	mQueueInfos[GQT_GRAPHICS].Queues.push_back(
		B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuQueue>()) D3D12GpuQueue(*this, GQT_GRAPHICS, 0, graphicsQueue))
	);

	// Compute queue (optional, use direct queue if not available)
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	ID3D12CommandQueue* computeQueue;
	hr = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&computeQueue));
	if (SUCCEEDED(hr))
	{
		mQueueInfos[GQT_COMPUTE].Queues.push_back(
			B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuQueue>()) D3D12GpuQueue(*this, GQT_COMPUTE, 0, computeQueue))
		);
	}

	// Copy/Transfer queue (optional, use direct queue if not available)
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	ID3D12CommandQueue* copyQueue;
	hr = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&copyQueue));
	if (SUCCEEDED(hr))
	{
		mQueueInfos[GQT_TRANSFER].Queues.push_back(
			B3DMakeSharedFromExisting(new (B3DAllocate<D3D12GpuQueue>()) D3D12GpuQueue(*this, GQT_TRANSFER, 0, copyQueue))
		);
	}

	// Query timestamp frequency
	ID3D12CommandQueue* timestampQueue = mQueueInfos[GQT_GRAPHICS].Queues[0]->GetD3D12Handle();
	timestampQueue->GetTimestampFrequency(&mTimestampFrequency);

	// Initialize capabilities
	InitializeCapabilities();

	// Create descriptor manager
	mDescriptorManager = B3DNew<D3D12DescriptorManager>(*this);

	// Create resource manager
	mResourceManager = B3DNew<D3D12ResourceManager>(*this);

	// Create memory allocator
	D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
	allocatorDesc.pDevice = mDevice.Get();
	allocatorDesc.pAdapter = mAdapter.Get();

	hr = D3D12MA::CreateAllocator(&allocatorDesc, &mAllocator);
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create D3D12 memory allocator");
	}

	// Initialize video mode information
#if B3D_PLATFORM_WIN32
	mVideoModeInfo = B3DMakeShared<Win32VideoModeInfo>();
#else
	static_assert(false, "mVideoModeInfo needs to be created.");
#endif

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Device-removal watchdog: on removal any subsequent D3D12 call can fail an assert and abort before the regular
	// LogDebugLayerMessages() hook runs, losing the DRED breadcrumbs. A polling thread dumps them the moment the
	// removal happens instead.
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
}

D3D12GpuDevice::~D3D12GpuDevice()
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	mWatchdogShouldExit.store(true, std::memory_order_relaxed);
	if (mDeviceRemovalWatchdog.joinable())
		mDeviceRemovalWatchdog.join();
#endif

	// Tear down the internal work context's transfer resources before other cleanup, while its queues are alive.
	mInternalWorkContext = nullptr;

	// Clear cached sampler states
	mCachedSamplerStates.clear();

	// Release queues
	for (u32 queueUsageIndex = 0; queueUsageIndex < GQT_COUNT; queueUsageIndex++)
	{
		for (auto& queue : mQueueInfos[queueUsageIndex].Queues)
			queue = nullptr;
	}

	// Release any pending deferred releases (the queues are idle by now) before tearing down the allocator
	DrainDeferredReleases(true);

	// Delete resource manager
	B3DDelete(mResourceManager);

	// Delete descriptor manager
	B3DDelete(mDescriptorManager);

	// Release memory allocator
	if (mAllocator)
	{
		mAllocator->Release();
		mAllocator = nullptr;
	}

	// Release device
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

	// Default: standalone (calling-thread deletion)
	// With RenderThreadDestroy flag: forward destruction to render thread
	TShared<Texture> output = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
		? B3DMakeSharedFromExisting(rawTexture)
		: MakeSharedStandalone<Texture>(rawTexture);

	output->SetShared(output);

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

TShared<render::GpuBuffer> D3D12GpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	D3D12GpuBuffer* rawBuffer = new (B3DAllocate<D3D12GpuBuffer>()) D3D12GpuBuffer(createInformation, *this);

	// Default: standalone (calling-thread deletion)
	// With RenderThreadDestroy flag: forward destruction to render thread
	TShared<GpuBuffer> output = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
		? B3DMakeSharedFromExisting(rawBuffer)
		: MakeSharedStandalone<GpuBuffer>(rawBuffer);

	output->SetShared(output);

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
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
	return B3DMakeSharedFromExisting(new (B3DAllocate<D3D12EventQuery>()) D3D12EventQuery(*this));
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
	TShared<D3D12GpuGraphicsPipelineState> output = B3DMakeSharedFromExisting<D3D12GpuGraphicsPipelineState>(
		new (B3DAllocate<D3D12GpuGraphicsPipelineState>()) D3D12GpuGraphicsPipelineState(createInformation, *this));

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

TShared<GpuComputePipelineState> D3D12GpuDevice::CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	TShared<D3D12GpuComputePipelineState> output = B3DMakeSharedFromExisting<D3D12GpuComputePipelineState>(
		new (B3DAllocate<D3D12GpuComputePipelineState>()) D3D12GpuComputePipelineState(createInformation, *this));

	if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

TShared<GpuPipelineParameterLayout> D3D12GpuDevice::CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation)
{
	return B3DMakeSharedFromExisting<D3D12GpuPipelineParameterLayout>(
		new (B3DAllocate<D3D12GpuPipelineParameterLayout>()) D3D12GpuPipelineParameterLayout(createInformation, *this));
}

namespace
{
	/** The D3D12 backend has no backend-specific set layout data; expose the protected base constructor. */
	class D3D12GpuPipelineParameterSetLayout : public GpuPipelineParameterSetLayout
	{
	public:
		D3D12GpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription)
			: GpuPipelineParameterSetLayout(parameterDescription)
		{}
	};
}

TShared<GpuPipelineParameterSetLayout> D3D12GpuDevice::CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& /*resourceTableLayout*/, u32 /*tableIndex*/)
{
	// TODO(d3d12-port): root-signature construction currently derives everything from the parameter description;
	// consuming the reflected bindings for exact root-signature population is the open follow-up. The per-stage
	// reflected tables are merged per set by D3D12GpuPipelineParameterLayout (GetReflectedSetEntries()), which is
	// where the follow-up should consume them - the per-set layout intentionally ignores the arguments here.
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

void D3D12GpuDevice::StartSubmitThread()
{
	// The conversion to the privately-inherited backend interface is only accessible from member context,
	// so it can't happen inside B3DMakeUnique's template.
	IGpuSubmitThreadBackend& backend = *this;
	mSubmitThread = B3DMakeUnique<GpuSubmitThread>(*this, backend);
}

void D3D12GpuDevice::StopSubmitThread()
{
	mSubmitThread = nullptr;
}

void D3D12GpuDevice::WaitUntilIdle()
{
	// Non-primary devices don't run a submit thread; their queues are never used so the native wait suffices.
	if (mSubmitThread == nullptr)
		ExecuteWaitUntilIdle();
	else
		GetSubmitThread().WaitUntilIdle();

	// The device is idle, so nothing deferred can still be referenced by the GPU
	DrainDeferredReleases(true);
}

void D3D12GpuDevice::NotifyWillQueueForSubmit(GpuCommandBuffer& commandBuffer)
{
	static_cast<D3D12GpuCommandBuffer&>(commandBuffer).NotifyWillQueueForSubmit();
}

void D3D12GpuDevice::ExecuteSubmit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences)
{
	static_cast<D3D12GpuQueue&>(queue).ExecuteSubmitOnSubmitThread(std::static_pointer_cast<D3D12GpuCommandBuffer>(commandBuffer), syncMask, signalFences);
}

void D3D12GpuDevice::RefreshCompletionState(GpuQueue& queue, bool forceWait, bool queueEmpty, u32 lastSubmitIndex)
{
	static_cast<D3D12GpuQueue&>(queue).RefreshCompletionState(forceWait, queueEmpty, lastSubmitIndex);
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

	// Advance the deferred-release ring; entries two frame boundaries old are safe to free now
	DrainDeferredReleases(false);
}

void D3D12GpuDevice::DeferNativeRelease(ComPtr<IUnknown> object, D3D12MA::Allocation* allocation)
{
	if (object == nullptr && allocation == nullptr)
		return;

	Lock lock(mDeferredReleaseMutex);
	mDeferredReleases[mCurrentDeferredReleaseFrame].push_back({ std::move(object), allocation });
}

void D3D12GpuDevice::DrainDeferredReleases(bool releaseAll)
{
	Lock lock(mDeferredReleaseMutex);

	auto fnReleaseList = [](Vector<DeferredRelease>& list)
	{
		for (DeferredRelease& entry : list)
		{
			// The object (e.g. an ID3D12Resource) must go before the memory backing it
			entry.Object.Reset();

			if (entry.Allocation != nullptr)
				entry.Allocation->Release();
		}

		list.clear();
	};

	if (releaseAll)
	{
		for (Vector<DeferredRelease>& list : mDeferredReleases)
			fnReleaseList(list);

		return;
	}

	// Advance the ring; with two frames of GPU work in flight at most, the list that is now current was filled
	// two frame boundaries ago and can no longer be referenced.
	mCurrentDeferredReleaseFrame = (mCurrentDeferredReleaseFrame + 1) % kDeferredReleaseFrameCount;
	fnReleaseList(mDeferredReleases[mCurrentDeferredReleaseFrame]);
}

namespace
{
	/** Returns a human-readable name for a DRED auto-breadcrumb op. */
	const char* GetBreadcrumbOpName(D3D12_AUTO_BREADCRUMB_OP op)
	{
		switch (op)
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
			const u32 completed = node->pLastBreadcrumbValue != nullptr ? *node->pLastBreadcrumbValue : 0;
			const u32 total = node->BreadcrumbCount;

			// Fully completed command lists are not where the hang is
			if (completed == total)
				continue;

			String listName = "<unnamed>";
			if (node->pCommandListDebugNameA != nullptr)
				listName = node->pCommandListDebugNameA;
			else if (node->pCommandListDebugNameW != nullptr)
				listName = UTF8::FromWide(WString(node->pCommandListDebugNameW));

			B3D_LOG(Error, LogRenderBackend, "DRED: Command list '{0}' hung at op {1}/{2}.", listName, completed, total);

			// Gather the marker strings so ops can be annotated with the pass they belong to
			UnorderedMap<u32, String> contextsByOp;
			for (u32 i = 0; i < node->BreadcrumbContextsCount; i++)
			{
				const D3D12_DRED_BREADCRUMB_CONTEXT& context = node->pBreadcrumbContexts[i];
				contextsByOp[context.BreadcrumbIndex] = UTF8::FromWide(WString(context.pContextString));
			}

			// Log the event markers preceding the hang (the innermost ones identify the active pass)
			for (i32 i = (i32)completed; i >= 0; i--)
			{
				if (const auto it = contextsByOp.find((u32)i); it != contextsByOp.end())
				{
					B3D_LOG(Error, LogRenderBackend, "DRED:   last marker before hang: op[{0}] '{1}'", i, it->second);
					break;
				}
			}

			// Log the ops surrounding the hang point for context
			const u32 contextStart = completed >= 10 ? completed - 10 : 0;
			const u32 contextEnd = Math::Min(completed + 4, total);
			for (u32 i = contextStart; i < contextEnd; i++)
			{
				String annotation;
				if (const auto it = contextsByOp.find(i); it != contextsByOp.end())
					annotation = " '" + it->second + "'";

				B3D_LOG(Error, LogRenderBackend, "DRED:   op[{0}]{1} {2}{3}", i, i == completed ? " <-- HUNG" : "",
					GetBreadcrumbOpName(node->pCommandHistory[i]), annotation);
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
	for (u64 i = 0; i < messageCount; i++)
	{
		SIZE_T messageLength = 0;
		if (FAILED(infoQueue->GetMessage(i, nullptr, &messageLength)) || messageLength == 0)
			continue;

		Vector<u8> storage(messageLength);
		D3D12_MESSAGE* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
		if (FAILED(infoQueue->GetMessage(i, message, &messageLength)))
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

GpuWorkContext& D3D12GpuDevice::GetInternalWorkContext()
{
	// The context owns its own fence completion tracker - the frame tracker is renderer-owned and a
	// backend cannot reach the renderer (layering).
	if (mInternalWorkContext == nullptr)
		mInternalWorkContext = GpuWorkContext::Create(*this);

	return *mInternalWorkContext;
}

void D3D12GpuDevice::PresentRenderWindow(const TShared<render::RenderWindow>& renderWindow, GpuQueueMask syncMask)
{
	TShared<GpuQueue> queue = GetQueue(GQT_GRAPHICS, 0);
	if (!B3D_ENSURE(queue))
		return;

	queue->PresentRenderWindow(renderWindow, syncMask);
}

void D3D12GpuDevice::ConvertProjectionMatrix(const Matrix4& input, Matrix4& output)
{
	output = input;

	// D3D12 uses depth range [0,1] (same as Vulkan)
	// No Y-axis flip needed for D3D12 (unlike Vulkan)
	// Convert depth range from [-1,1] to [0,1]
	output[2][0] = (output[2][0] + output[3][0]) / 2;
	output[2][1] = (output[2][1] + output[3][1]) / 2;
	output[2][2] = (output[2][2] + output[3][2]) / 2;
	output[2][3] = (output[2][3] + output[3][3]) / 2;
}

GpuUniformBufferInformation D3D12GpuDevice::GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms)
{
	GpuUniformBufferInformation buffer;
	buffer.Size = 0;
	buffer.IsShareable = true;
	buffer.Name = name;
	buffer.Slot = 0;
	buffer.Set = 0;

	// The engine's uniform block definitions are authored to lay out identically under std140 and HLSL constant
	// buffer packing, so the shared std140 helper computes the sizes and offsets (same as the other backends)
	for (auto& param : inOutUniforms)
	{
		u32 size;
		if (param.Type == GPDT_STRUCT)
		{
			// Structs are always aligned and rounded up to vec4 (16 bytes)
			size = Math::DivideAndRoundUp(param.ElementSize, 16U) * 4;
			buffer.Size = Math::DivideAndRoundUp(buffer.Size, 4U) * 4;
		}
		else
		{
			size = GpuBackendUtility::CalcStd140MemberSizeAndOffset(param.Type, param.ArraySize, buffer.Size);
		}

		param.ElementSize = size;
		param.ArrayElementStride = size;
		param.CpuOffset = buffer.Size;
		param.GpuOffset = 0;
		buffer.Size += size * param.ArraySize;
		param.ParentUniformBufferSlot = 0;
		param.ParentUniformBufferSet = 0;
	}

	// Constant buffer size must always be a multiple of 16 bytes (256 bits)
	if (buffer.Size % 4 != 0)
		buffer.Size += (4 - (buffer.Size % 4));

	return buffer;
}

float D3D12GpuDevice::ConvertTimestampToMilliseconds(u64 timestamp)
{
	if (mTimestampFrequency == 0)
		return 0.0f;

	const double timestampToMs = 1000.0 / (double)mTimestampFrequency;
	return (float)((double)timestamp * timestampToMs);
}

void D3D12GpuDevice::InitializeCapabilities()
{
	DXGI_ADAPTER_DESC3 adapterDesc;
	mAdapter->GetDesc3(&adapterDesc);

	// Convert adapter description to char string
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

	mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_BC);
	mCapabilities.SetCapability(RSC_COMPUTE_PROGRAM);
	mCapabilities.SetCapability(RSC_LOAD_STORE);
	mCapabilities.SetCapability(RSC_LOAD_STORE_MSAA);
	mCapabilities.SetCapability(RSC_BYTECODE_CACHING);
	mCapabilities.SetCapability(RSC_TEXTURE_VIEWS);
	mCapabilities.SetCapability(RSC_RENDER_TARGET_LAYERS);

	mCapabilities.Conventions.NdcYAxis = GpuBackendConventions::Axis::Up;
	mCapabilities.Conventions.MatrixOrder = GpuBackendConventions::MatrixOrder::ColumnMajor;

	// Set vendor
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

	B3D_LOG(Info, LogRenderBackend, "D3D12 Device: {0}", mCapabilities.DeviceName);
}
