//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12SwapChain.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuQueue.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12Framebuffer.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "Threading/B3DScheduler.h"

using namespace b3d;
using namespace b3d::render;

D3D12SwapChain::D3D12SwapChain(D3D12ResourceManager* owner, const D3D12SwapChainCreateInformation& createInfo, D3D12GpuDevice& device)
	: Super(owner, "SwapChain")
	, mDevice(device)
	, mCreateInfo(createInfo)
	, mRenderTarget(nullptr)
	, mBackBufferCount(0)
	, mWidth(createInfo.Width)
	, mHeight(createInfo.Height)
	, mVSync(createInfo.VSync)
	, mVsyncInterval(createInfo.VsyncInterval)
	, mIsInitialized(false)
{
	// Initialize descriptor handles and framebuffers to null
	for (u32 i = 0; i < kMaxBackBuffers; i++)
	{
		mBackBufferRTVs[i].ptr = 0;
		mFramebuffers[i] = nullptr;
	}
	mDepthStencilView.ptr = 0;

	// Present-completion notifies (swap chain NotifyUnbound) are posted back to this queue and processed on the thread
	// responsible for the swap chain (the render thread).
	Scheduler* const scheduler = Scheduler::Get();
	if (B3D_ENSURE(scheduler))
		mMessageQueue.ScheduleRunUntilShutdown(*scheduler, true);
}

D3D12SwapChain::~D3D12SwapChain()
{
	mMessageQueue.PostRequestShutdownCommand(true);

	if (!mIsInitialized)
		return;

	D3D12DescriptorManager& descriptorManager = mDevice.GetDescriptorManager();

	// Delete framebuffers
	for (u32 i = 0; i < mBackBufferCount; i++)
	{
		if (mFramebuffers[i] != nullptr)
		{
			B3DDelete(mFramebuffers[i]);
			mFramebuffers[i] = nullptr;
		}
	}

	// Free render target views
	for (u32 i = 0; i < mBackBufferCount; i++)
	{
		if (mBackBufferRTVs[i].ptr != 0)
		{
			descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::RTV, mBackBufferRTVs[i]);
			mBackBufferRTVs[i].ptr = 0;
		}
		mBackBuffers[i].Reset();
	}

	// Free depth stencil view
	if (mDepthStencilView.ptr != 0)
	{
		descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::DSV, mDepthStencilView);
		mDepthStencilView.ptr = 0;
	}

	if (mDepthStencilAllocation)
	{
		mDepthStencilAllocation->Release();
		mDepthStencilAllocation = nullptr;
	}

	mDepthStencilBuffer.Reset();
	mSwapChain.Reset();

	mBackBufferCount = 0;
	mIsInitialized = false;

	B3D_LOG(Info, LogRenderBackend, "Destroyed D3D12 swap chain");
}

void D3D12SwapChain::Initialize()
{
	if (mIsInitialized)
		return;

	CreateSwapChain();
	GetBackBufferResources();
	CreateRenderTargetViews();

	if (mCreateInfo.CreateDepthBuffer)
	{
		CreateDepthStencilBuffer();
	}

	mIsInitialized = true;

	B3D_LOG(Info, LogRenderBackend, "Initialized D3D12 swap chain: {0}x{1}, format={2}, buffers={3}",
		mWidth, mHeight, (u32)mCreateInfo.ColorFormat, mBackBufferCount);
}

void D3D12SwapChain::Destroy()
{
	// Process any pending messages, most importantly present-completion NotifyUnbound notifies, so the resource can be
	// destroyed immediately when its bound count reaches zero (important for shutdown / rebuild).
	mMessageQueue.RunUntilIdle();

	Super::Destroy();
}

void D3D12SwapChain::CreateSwapChain()
{
	IDXGIFactory6* factory = GetD3D12GpuBackend().GetDXGIFactory();
	if (!factory)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to get DXGI factory for swap chain creation");
		return;
	}

	// Get the graphics queue for presenting
	TShared<GpuQueue> queue = mDevice.GetQueue(GQT_GRAPHICS, 0);
	if (!queue)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to get graphics queue for swap chain creation");
		return;
	}

	D3D12GpuQueue* d3d12Queue = static_cast<D3D12GpuQueue*>(queue.get());
	ID3D12CommandQueue* commandQueue = d3d12Queue->GetD3D12Handle();

	// Use triple buffering for smooth performance
	mBackBufferCount = 3;

	// Create swap chain descriptor
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = mWidth;
	swapChainDesc.Height = mHeight;
	swapChainDesc.Format = mCreateInfo.ColorFormat;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = mBackBufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	// Create swap chain
	ComPtr<IDXGISwapChain1> swapChain1;
	HRESULT hr = factory->CreateSwapChainForHwnd(
		commandQueue,
		mCreateInfo.WindowHandle,
		&swapChainDesc,
		nullptr, // Fullscreen descriptor
		nullptr, // Restrict output
		&swapChain1
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create swap chain: HRESULT={0}", (u32)hr);
		return;
	}

	// Upgrade to IDXGISwapChain4
	hr = swapChain1.As(&mSwapChain);
	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to upgrade swap chain interface: HRESULT={0}", (u32)hr);
		return;
	}

	// Disable Alt+Enter fullscreen toggle (we'll handle fullscreen ourselves)
	factory->MakeWindowAssociation(mCreateInfo.WindowHandle, DXGI_MWA_NO_ALT_ENTER);
}

void D3D12SwapChain::GetBackBufferResources()
{
	if (!mSwapChain)
		return;

	// Get back buffer resources from the swap chain
	for (u32 i = 0; i < mBackBufferCount; i++)
	{
		HRESULT hr = mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]));
		if (FAILED(hr))
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to get swap chain back buffer {0}: HRESULT={1}", i, (u32)hr);
			mBackBuffers[i].Reset();
		}
		else
		{
			// Set debug name for the back buffer
			String name = "SwapChain BackBuffer " + ToString(i);
			mBackBuffers[i]->SetName(ToWideString(name).c_str());
		}
	}
}

void D3D12SwapChain::CreateRenderTargetViews()
{
	if (!mSwapChain)
		return;

	D3D12DescriptorManager& descriptorManager = mDevice.GetDescriptorManager();
	ID3D12Device* d3d12Device = mDevice.GetD3D12Device();

	// Create render target views for each back buffer
	for (u32 i = 0; i < mBackBufferCount; i++)
	{
		if (!mBackBuffers[i])
			continue;

		// Allocate descriptor
		mBackBufferRTVs[i] = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::RTV);

		if (mBackBufferRTVs[i].ptr == 0)
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to allocate RTV descriptor for back buffer {0}", i);
			continue;
		}

		// Create render target view
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = mCreateInfo.ColorFormat;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		d3d12Device->CreateRenderTargetView(mBackBuffers[i].Get(), &rtvDesc, mBackBufferRTVs[i]);

		B3D_LOG(Info, LogRenderBackend, "Created RTV for back buffer {0}", i);
	}
}

void D3D12SwapChain::CreateDepthStencilBuffer()
{
	if (mCreateInfo.DepthStencilFormat == DXGI_FORMAT_UNKNOWN)
		return;

	D3D12DescriptorManager& descriptorManager = mDevice.GetDescriptorManager();
	ID3D12Device* d3d12Device = mDevice.GetD3D12Device();

	// Create depth stencil buffer
	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = mCreateInfo.DepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = mCreateInfo.DepthStencilFormat;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12MA::ALLOCATION_DESC allocDesc = {};
	allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
	allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

	HRESULT hr = mDevice.GetAllocator()->CreateResource(
		&allocDesc,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		&mDepthStencilAllocation,
		IID_PPV_ARGS(&mDepthStencilBuffer)
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create depth stencil buffer: HRESULT={0}", (u32)hr);
		return;
	}

	mDepthStencilBuffer->SetName(L"SwapChain DepthStencil Buffer");

	// Allocate descriptor
	mDepthStencilView = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::DSV);

	if (mDepthStencilView.ptr == 0)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to allocate DSV descriptor");
		return;
	}

	// Create depth stencil view
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = mCreateInfo.DepthStencilFormat;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2D.MipSlice = 0;

	d3d12Device->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, mDepthStencilView);

	B3D_LOG(Info, LogRenderBackend, "Created depth stencil buffer: {0}x{1}, format={2}", mWidth, mHeight, (u32)mCreateInfo.DepthStencilFormat);
}

u32 D3D12SwapChain::GetCurrentBackBufferIndex() const
{
	if (!mSwapChain)
		return 0;

	return mSwapChain->GetCurrentBackBufferIndex();
}

ID3D12Resource* D3D12SwapChain::GetBackBuffer(u32 index) const
{
	if (index >= mBackBufferCount)
		return nullptr;

	return mBackBuffers[index].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12SwapChain::GetBackBufferRTV(u32 index) const
{
	if (index >= mBackBufferCount)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE nullHandle = {};
		nullHandle.ptr = 0;
		return nullHandle;
	}

	return mBackBufferRTVs[index];
}

void D3D12SwapChain::SetRenderTarget(const RenderTarget* renderTarget)
{
	mRenderTarget = renderTarget;

	// Create framebuffers now that we have the render target
	if (mIsInitialized && mRenderTarget)
	{
		CreateFramebuffers();
	}
}

D3D12Framebuffer* D3D12SwapChain::GetFramebuffer(u32 index) const
{
	if (index >= mBackBufferCount)
		return nullptr;

	// Lazy creation if framebuffers haven't been created yet
	if (!mFramebuffers[index] && mRenderTarget)
	{
		// Cast away const for lazy initialization
		D3D12SwapChain* mutableThis = const_cast<D3D12SwapChain*>(this);
		mutableThis->mFramebuffers[index] = B3DNew<D3D12Framebuffer>(mRenderTarget, index);
	}

	return mFramebuffers[index];
}

HRESULT D3D12SwapChain::PresentDXGI()
{
	if (!mSwapChain)
		return E_FAIL;

	// syncInterval: 0 = no vsync, 1 = vsync, 2 = every other refresh, etc.
	const u32 syncInterval = GetSyncInterval();
	HRESULT hr = mSwapChain->Present(syncInterval, 0);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to present swap chain: HRESULT={0}", (u32)hr);
	}

	return hr;
}

void D3D12SwapChain::AcquireImage()
{
	// Called from the submit thread, or one of its helper workers (the submit thread offloads acquires to worker
	// threads as they may block). DXGI has no async acquire; the currently addressable back buffer is available
	// immediately, so we just record its index for the render thread to consume.
	AssertIfRenderThread();

	if (mIsRetired)
	{
		B3D_LOG(Error, LogRenderBackend, "Attempting to acquire an image from a retired swap chain.");
		return;
	}

	if (!mSwapChain)
		return;

	const u32 imageIndex = mSwapChain->GetCurrentBackBufferIndex();

	Lock lock(mAcquireMutex);
	mAcquiredImageIndices.Add(imageIndex);
}

void D3D12SwapChain::Present(u32 imageIndex, GpuQueue& queue, GpuQueueMask syncMask)
{
	AssertIfNotSubmitThread();

	if (!mSwapChain)
		return;

	D3D12GpuQueue& d3d12Queue = static_cast<D3D12GpuQueue&>(queue);

	// DXGI flip-model presents the swap chain's current back buffer; there is no per-image present target. No DXGI
	// present happens between acquire and present, so the current back buffer still matches the acquired image index.
	B3D_ASSERT(imageIndex == mSwapChain->GetCurrentBackBufferIndex() && "Presenting an image other than the current DXGI back buffer.");
	(void)imageIndex;

	// Register the present entry on the queue; this issues the DXGI present and records a present entry so the
	// swap chain's NotifyUnbound() is posted back once a later submission on this queue completes.
	d3d12Queue.Present(this, syncMask);
}

bool D3D12SwapChain::TryGetFirstAcquiredImageIndex(u32& outImageIndex) const
{
	Lock lock(mAcquireMutex);

	if (mAcquiredImageIndices.Empty())
		return false;

	outImageIndex = mAcquiredImageIndices.Front();
	return true;
}

void D3D12SwapChain::NotifyWasImageAcquireQueued()
{
	NotifyBound();
}

void D3D12SwapChain::NotifyWasPresentQueued(u32 imageIndex)
{
	{
		Lock lock(mAcquireMutex);

		const auto found = std::find(mAcquiredImageIndices.begin(), mAcquiredImageIndices.end(), imageIndex);
		if (found != mAcquiredImageIndices.end())
			mAcquiredImageIndices.erase(found);
		else
			B3D_ASSERT(false && "Presenting a swap chain image that wasn't acquired.");
	}

	NotifyBound();
}

void D3D12SwapChain::CreateFramebuffers()
{
	if (!mRenderTarget)
	{
		B3D_LOG(Warning, LogRenderBackend, "Cannot create framebuffers without render target reference");
		return;
	}

	// Create one framebuffer per back buffer
	for (u32 i = 0; i < mBackBufferCount; i++)
	{
		if (mFramebuffers[i] != nullptr)
			B3DDelete(mFramebuffers[i]);

		mFramebuffers[i] = B3DNew<D3D12Framebuffer>(mRenderTarget, i);
		B3D_LOG(Info, LogRenderBackend, "Created framebuffer for back buffer {0}", i);
	}
}
