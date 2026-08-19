//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12SwapChain.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuQueue.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12Framebuffer.h"
#include "Threading/B3DScheduler.h"

using namespace b3d;
using namespace b3d::render;

D3D12SwapChain::D3D12SwapChain(D3D12ResourceManager* owner, const D3D12SwapChainCreateInformation& createInformation, D3D12GpuDevice& device)
	:Super(owner, "SwapChain"), mDevice(device), mCreateInformation(createInformation), mWidth(createInformation.Width), mHeight(createInformation.Height), mVSync(createInformation.VSync), mVsyncInterval(createInformation.VsyncInterval)
{
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

	// Delete the framebuffers and queue the back-buffer image wrappers for destruction.
	for (u32 backBufferIndex = 0; backBufferIndex < mBackBufferCount; backBufferIndex++)
	{
		if (mFramebuffers[backBufferIndex] != nullptr)
		{
			B3DDelete(mFramebuffers[backBufferIndex]);
			mFramebuffers[backBufferIndex] = nullptr;
		}

		if (mBackBufferImages[backBufferIndex] != nullptr)
		{
			mBackBufferImages[backBufferIndex]->Destroy();
			mBackBufferImages[backBufferIndex] = nullptr;
		}

		mBackBuffers[backBufferIndex].Reset();
	}

	// The image wrapper owns the depth buffer's allocation.
	if (mDepthStencilImage != nullptr)
	{
		mDepthStencilImage->Destroy();
		mDepthStencilImage = nullptr;
	}

	mDepthStencilBuffer.Reset();
	mSwapChain.Reset();

	mBackBufferCount = 0;
	mIsInitialized = false;

	B3D_LOG(Verbose, LogRenderBackend, "Destroyed D3D12 swap chain");
}

void D3D12SwapChain::Initialize()
{
	if (mIsInitialized)
		return;

	// Create the native swap chain.
	IDXGIFactory6* factory = GetD3D12GpuBackend().GetDXGIFactory();
	if (!factory)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to get DXGI factory for swap chain creation");
		return;
	}

	TShared<GpuQueue> queue = mDevice.GetQueue(GQT_GRAPHICS, 0);
	if (!queue)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to get graphics queue for swap chain creation");
		return;
	}

	D3D12GpuQueue* d3d12Queue = static_cast<D3D12GpuQueue*>(queue.get());
	ID3D12CommandQueue* commandQueue = d3d12Queue->GetD3D12Handle();
	mBackBufferCount = GetD3D12BackBufferCount();

	DXGI_SWAP_CHAIN_DESC1 swapChainDescription = {};
	swapChainDescription.Width = mWidth;
	swapChainDescription.Height = mHeight;
	swapChainDescription.Format = mCreateInformation.ColorFormat;
	swapChainDescription.Stereo = FALSE;
	swapChainDescription.SampleDesc.Count = 1;
	swapChainDescription.SampleDesc.Quality = 0;
	swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescription.BufferCount = mBackBufferCount;
	swapChainDescription.Scaling = DXGI_SCALING_STRETCH;
	swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDescription.Flags = 0;

	ComPtr<IDXGISwapChain1> swapChain;
	HRESULT result = factory->CreateSwapChainForHwnd(commandQueue, mCreateInformation.WindowHandle, &swapChainDescription, nullptr, nullptr, &swapChain);
	if (FAILED(result))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create swap chain: HRESULT={0}", (u32)result);
		return;
	}

	result = swapChain.As(&mSwapChain);
	if (FAILED(result))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to upgrade swap chain interface: HRESULT={0}", (u32)result);
		return;
	}

	factory->MakeWindowAssociation(mCreateInformation.WindowHandle, DXGI_MWA_NO_ALT_ENTER);

	// Create the back-buffer image wrappers.
	for (u32 backBufferIndex = 0; backBufferIndex < mBackBufferCount; backBufferIndex++)
	{
		result = mSwapChain->GetBuffer(backBufferIndex, IID_PPV_ARGS(&mBackBuffers[backBufferIndex]));
		if (FAILED(result))
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to get swap chain back buffer {0}: HRESULT={1}", backBufferIndex, (u32)result);
			mBackBuffers[backBufferIndex].Reset();

			continue;
		}

		String name = "SwapChain BackBuffer " + ToString(backBufferIndex);
		mBackBuffers[backBufferIndex]->SetName(ToWideString(name).c_str());

		D3D12ImageCreateInformation imageCreateInformation;
		imageCreateInformation.Resource = mBackBuffers[backBufferIndex];
		imageCreateInformation.Format = mCreateInformation.ColorFormat;
		imageCreateInformation.InitialLayout = D3D12TextureLayout::Common();
		imageCreateInformation.FaceCount = 1;
		imageCreateInformation.MipLevelCount = 1;
		imageCreateInformation.Aspect = GpuTextureAspectFlag::Color;
		imageCreateInformation.IsPresentable = true;
		imageCreateInformation.Name = name;

		mBackBufferImages[backBufferIndex] = mDevice.GetResourceManager().Create<D3D12Image>(imageCreateInformation);
	}

	// Create the optional depth-stencil image.
	if (mCreateInformation.CreateDepthBuffer && mCreateInformation.DepthStencilFormat != DXGI_FORMAT_UNKNOWN)
	{
		D3D12_RESOURCE_DESC depthStencilDescription = {};
		depthStencilDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDescription.Width = mWidth;
		depthStencilDescription.Height = mHeight;
		depthStencilDescription.DepthOrArraySize = 1;
		depthStencilDescription.MipLevels = 1;
		depthStencilDescription.Format = mCreateInformation.DepthStencilFormat;
		depthStencilDescription.SampleDesc.Count = 1;
		depthStencilDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = mCreateInformation.DepthStencilFormat;
		clearValue.DepthStencil.Depth = 1.0f;

		GpuResourceLocation depthStencilAllocation;
		result = mDevice.CreateResource(depthStencilDescription, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_UNDEFINED, &clearValue, mDepthStencilBuffer, depthStencilAllocation);

		if (FAILED(result))
			B3D_LOG(Error, LogRenderBackend, "Failed to create depth stencil buffer: HRESULT={0}", (u32)result);
		else
		{
			mDepthStencilBuffer->SetName(L"SwapChain DepthStencil Buffer");
			const bool hasStencil = mCreateInformation.DepthStencilFormat == DXGI_FORMAT_D24_UNORM_S8_UINT || mCreateInformation.DepthStencilFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

			D3D12ImageCreateInformation imageCreateInformation;
			imageCreateInformation.Resource = mDepthStencilBuffer;
			imageCreateInformation.Allocation = depthStencilAllocation;
			imageCreateInformation.Format = mCreateInformation.DepthStencilFormat;
			imageCreateInformation.InitialLayout = D3D12TextureLayout::Undefined();
			imageCreateInformation.FaceCount = 1;
			imageCreateInformation.MipLevelCount = 1;
			imageCreateInformation.Aspect = hasStencil ? (GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil) : GpuTextureAspectFlags(GpuTextureAspectFlag::Depth);
			imageCreateInformation.Name = "SwapChain DepthStencil Buffer";

			mDepthStencilImage = mDevice.GetResourceManager().Create<D3D12Image>(imageCreateInformation);
		}
	}

	// Create the framebuffer wrappers.
	for(u32 backBufferIndex = 0; backBufferIndex < mBackBufferCount; ++backBufferIndex)
	{
		D3D12FramebufferCreateInformation framebufferCreateInformation;
		framebufferCreateInformation.Width = mWidth;
		framebufferCreateInformation.Height = mHeight;
		framebufferCreateInformation.ColorAttachments[0].Image = GetBackBufferImage(backBufferIndex);
		framebufferCreateInformation.DepthStencilAttachment.Image = mDepthStencilImage;

		mFramebuffers[backBufferIndex] = B3DNew<D3D12Framebuffer>(framebufferCreateInformation);
	}

	mIsInitialized = true;
}

void D3D12SwapChain::Destroy()
{
	// Process pending queued-operation unbind notifications so the resource can be destroyed immediately when its
	// bound count reaches zero (important for shutdown / rebuild).
	mMessageQueue.RunUntilIdle();

	Super::Destroy();
}

u32 D3D12SwapChain::GetCurrentBackBufferIndex() const
{
	if (!mSwapChain)
		return 0;

	return mSwapChain->GetCurrentBackBufferIndex();
}

D3D12Image* D3D12SwapChain::GetBackBufferImage(u32 index) const
{
	if (index >= mBackBufferCount)
		return nullptr;

	return mBackBufferImages[index];
}

D3D12Framebuffer* D3D12SwapChain::GetFramebufferForImage(u32 index) const
{
	if (index >= mBackBufferCount)
		return nullptr;

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
		B3D_LOG(Error, LogRenderBackend, "Failed to present swap chain: HRESULT={0}", (u32)hr);

	return hr;
}

void D3D12SwapChain::AcquireImage()
{
	// Called from the submit thread, or one of its helper workers (the submit thread offloads acquires to worker
	// threads as they may block). DXGI has no async acquire; the currently addressable back buffer is available
	// immediately, so we just record its index for the render thread to consume.
	AssertIfRenderThread();

	bool isAcquireValid = true;

	if (mIsRetired)
	{
		B3D_LOG(Error, LogRenderBackend, "Attempting to acquire an image from a retired swap chain.");
		isAcquireValid = false;
	}
	else if (!mSwapChain)
		isAcquireValid = false;

	const u32 imageIndex = isAcquireValid ? GetCurrentBackBufferIndex() : 0;

	// The pending count must be decremented even when the acquire fails, otherwise WaitForPendingAcquires()
	// would never wake up.
	Lock lock(mAcquireMutex);

	if (isAcquireValid)
		mAcquiredImageIndices.Add(imageIndex);

	B3D_ASSERT(mPendingAcquireCount > 0);
	mPendingAcquireCount--;
	mAcquireSignal.NotifyAll();
}

void D3D12SwapChain::Present(u32 imageIndex, GpuQueue& queue, GpuQueueMask syncMask)
{
	AssertIfNotSubmitThread();

	if (!mSwapChain)
		return;

	D3D12GpuQueue& d3d12Queue = static_cast<D3D12GpuQueue&>(queue);

	// DXGI flip-model presents the swap chain's current back buffer; there is no per-image present target. No DXGI
	// present happens between acquire and present, so the current back buffer still matches the acquired image index.
	B3D_ASSERT(imageIndex == GetCurrentBackBufferIndex() && "Presenting an image other than the current DXGI back buffer.");
	(void)imageIndex;

	// Issue the DXGI present on the selected queue after its cross-queue dependencies.
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

void D3D12SwapChain::WaitForPendingAcquires()
{
	Lock lock(mAcquireMutex);
	mAcquireSignal.Wait(lock, [this] { return mPendingAcquireCount == 0; });
}

void D3D12SwapChain::NotifyWasImageAcquireQueued()
{
	Lock lock(mAcquireMutex);
	mPendingAcquireCount++;
}

void D3D12SwapChain::NotifyWasPresentQueued(u32 imageIndex)
{
	{
		Lock lock(mAcquireMutex);

		const auto acquiredImageIterator = std::find(mAcquiredImageIndices.begin(), mAcquiredImageIndices.end(), imageIndex);
		if (acquiredImageIterator != mAcquiredImageIndices.end())
			mAcquiredImageIndices.erase(acquiredImageIterator);
		else
			B3D_ASSERT(false && "Presenting a swap chain image that wasn't acquired.");
	}
}
