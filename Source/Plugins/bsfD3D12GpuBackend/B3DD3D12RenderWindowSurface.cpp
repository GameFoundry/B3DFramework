//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12RenderWindowSurface.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12SwapChain.h"
#include "B3DD3D12Framebuffer.h"
#include "GpuBackend/B3DGpuSubmitThread.h"

using namespace b3d;
using namespace b3d::render;

D3D12RenderWindowSurface::D3D12RenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation) 
	: mDevice(static_cast<D3D12GpuDevice&>(*GetD3D12GpuBackend().GetPrimaryDevice())), mCreateDepthBuffer(createInformation.CreateDepthBuffer), mVsyncInterval(createInformation.VsyncInterval), mWindowHandle((HWND)createInformation.PlatformWindowHandle)
{
	mColorFormat = createInformation.UseHardwareSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

	if (mCreateDepthBuffer)
		mDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	CreateSwapChain(createInformation.Width, createInformation.Height, createInformation.VSync);
}

D3D12RenderWindowSurface::~D3D12RenderWindowSurface()
{
	Destroy();
}

void D3D12RenderWindowSurface::CreateSwapChain(u32 width, u32 height, bool vsync)
{
	D3D12SwapChainCreateInformation swapChainCreateInformation;
	swapChainCreateInformation.WindowHandle = mWindowHandle;
	swapChainCreateInformation.Width = width;
	swapChainCreateInformation.Height = height;
	swapChainCreateInformation.VSync = vsync;
	swapChainCreateInformation.VsyncInterval = mVsyncInterval;
	swapChainCreateInformation.ColorFormat = mColorFormat;
	swapChainCreateInformation.DepthStencilFormat = mDepthFormat;
	swapChainCreateInformation.CreateDepthBuffer = mCreateDepthBuffer;

	mSwapChain = mDevice.GetResourceManager().Create<D3D12SwapChain>(swapChainCreateInformation, mDevice);
	mSwapChain->Initialize();
	mIsValid = true;
	mHasAcquiredImage = false;

	// Kick the first image acquire so the render thread has an acquired image for the first frame
	if (mDevice.HasSubmitThread())
		mDevice.GetSubmitThread().QueueImageAcquire(*mSwapChain);
}

void D3D12RenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync, u32 vsyncInterval)
{
	if (!mSwapChain || mIsDestroyed)
		return;

	mDevice.WaitUntilIdle();

	D3D12SwapChain* oldSwapChain = mSwapChain;
	oldSwapChain->MarkAsRetired(); // TODO - Don't think we need to support retired functionality on D3D12, that's a Vulkan thing

	mVsyncInterval = vsyncInterval == 0 ? 1 : vsyncInterval;

	CreateSwapChain(width, height, vsync);
	oldSwapChain->Destroy();
}

void D3D12RenderWindowSurface::SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask)
{
	if (!mSwapChain || mIsDestroyed)
		return;

	GpuSubmitThread& submitThread = mDevice.GetSubmitThread();

	// Ensure any queued acquire has executed, so the present below consumes the up-to-date acquired-image list and
	// the acquire queued after it can't overlap a still-pending one.
	mSwapChain->WaitForPendingAcquires();

	// Present the image that was rendered this frame. First acquired-but-not-yet-presented image is presented.
	submitThread.QueuePresent(queue, *mSwapChain, syncMask);
	mHasAcquiredImage = false;

	// Queue acquire the image for the next frame
	if (!mSwapChain->IsRetired())
		submitThread.QueueImageAcquire(*mSwapChain);
}

void D3D12RenderWindowSurface::MarkSwapChainAsInvalid()
{
	if (mIsDestroyed)
		return;

	mIsValid = false;
}

void D3D12RenderWindowSurface::Destroy()
{
	if (mIsDestroyed)
		return;

	// Wait for GPU to finish all work (and drain any pending present entries) before releasing the swap chain.
	mDevice.WaitUntilIdle();

	if (mSwapChain)
	{
		mSwapChain->Destroy();
		mSwapChain = nullptr;
	}

	mIsValid = false;
	mIsDestroyed = true;
	mHasAcquiredImage = false;
}

D3D12Image* D3D12RenderWindowSurface::GetCurrentColorImage() const
{
	if (!mSwapChain || !mHasAcquiredImage)
		return nullptr;

	return mSwapChain->GetBackBufferImage(mActiveImageIndex);
}

u32 D3D12RenderWindowSurface::GetWidth() const
{
	return mSwapChain != nullptr ? mSwapChain->GetWidth() : 0;
}

u32 D3D12RenderWindowSurface::GetHeight() const
{
	return mSwapChain != nullptr ? mSwapChain->GetHeight() : 0;
}

D3D12Framebuffer* D3D12RenderWindowSurface::GetActiveFramebuffer()
{
	if (!mSwapChain)
		return nullptr;

	// If there is a swap chain acquire already queued, wait for it
	mSwapChain->WaitForPendingAcquires();

	u32 imageIndex;
	bool isImageAcquired = mSwapChain->TryGetFirstAcquiredImageIndex(imageIndex);

	// Fresh swap chain with no queued acquire yet - should never happen, but queue and wait just in case
	if (!isImageAcquired && !mSwapChain->IsRetired() && mDevice.HasSubmitThread())
	{
		mDevice.GetSubmitThread().QueueImageAcquire(*mSwapChain);
		mSwapChain->WaitForPendingAcquires();
		isImageAcquired = mSwapChain->TryGetFirstAcquiredImageIndex(imageIndex);
	}

	if (!isImageAcquired)
	{
		mHasAcquiredImage = false;
		return nullptr;
	}

	mActiveImageIndex = imageIndex;
	mHasAcquiredImage = true;
	return mSwapChain->GetFramebufferForImage(mActiveImageIndex);
}
