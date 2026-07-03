//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12RenderWindowSurface.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12SwapChain.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12Framebuffer.h"
#include "GpuBackend/B3DGpuSubmitThread.h"

using namespace b3d::render;

D3D12RenderWindowSurface::D3D12RenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation)
	: mDevice(static_cast<D3D12GpuDevice&>(*GetD3D12GpuBackend().GetPrimaryDevice()))
	, mWindowHandle((HWND)createInformation.PlatformWindowHandle)
	, mCreateDepthBuffer(createInformation.CreateDepthBuffer)
	, mUseHardwareSRGB(createInformation.UseHardwareSRGB)
	, mVsync(createInformation.VSync)
	, mVsyncInterval(createInformation.VsyncInterval)
{
	// Determine color format
	if (mUseHardwareSRGB)
	{
		// Use SRGB format for gamma-correct rendering
		mColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	}
	else
	{
		// Use standard format
		mColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	// Determine depth format
	if (mCreateDepthBuffer)
	{
		// Use standard 24-bit depth with 8-bit stencil
		mDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	}

	CreateSwapChainInternal(createInformation.Width, createInformation.Height, createInformation.VSync, nullptr);

	B3D_LOG(Info, LogRenderBackend, "Created D3D12 render window surface: width={0}, height={1}, vsync={2}, srgb={3}",
		createInformation.Width, createInformation.Height, createInformation.VSync, mUseHardwareSRGB);
}

D3D12RenderWindowSurface::~D3D12RenderWindowSurface()
{
	Destroy();
}

void D3D12RenderWindowSurface::CreateSwapChainInternal(u32 width, u32 height, bool vsync, D3D12SwapChain* oldSwapChain)
{
	(void)oldSwapChain;

	// Create swap chain
	D3D12SwapChainCreateInformation swapChainCreateInfo;
	swapChainCreateInfo.WindowHandle = mWindowHandle;
	swapChainCreateInfo.Width = width;
	swapChainCreateInfo.Height = height;
	swapChainCreateInfo.VSync = vsync;
	swapChainCreateInfo.VsyncInterval = mVsyncInterval;
	swapChainCreateInfo.ColorFormat = mColorFormat;
	swapChainCreateInfo.DepthStencilFormat = mDepthFormat;
	swapChainCreateInfo.CreateDepthBuffer = mCreateDepthBuffer;

	// The swap chain is an IGpuResource - its lifetime is owned by the device resource manager and it is released via
	// IGpuResource::Destroy() (deferred until it's no longer bound), not a direct delete.
	mSwapChain = mDevice.GetResourceManager().Create<D3D12SwapChain>(swapChainCreateInfo, mDevice);
	mSwapChain->Initialize();

	// Kick the first image acquire so the render thread has an acquired image for the first frame, and the
	// acquire/present bookkeeping stays balanced. The submit thread exists on the primary device by the time the
	// surface is created.
	if (mDevice.HasSubmitThread())
	{
		GpuSubmitThread& submitThread = mDevice.GetSubmitThread();
		submitThread.QueueImageAcquire(*mSwapChain);
	}
}

void D3D12RenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync)
{
	if (!mSwapChain || mIsDestroyed)
		return;

	B3D_LOG(Info, LogRenderBackend, "Rebuilding D3D12 swap chain: width={0}, height={1}, vsync={2}", width, height, vsync);

	// TODO(d3d12-port): This is a bring-up rebuild path. DXGI ResizeBuffers requires every outstanding back-buffer
	// reference to be released, which the graceful retire flow (present the old chain's pending images, then rebuild)
	// hasn't been wired up for yet. For now we fully drain the GPU, retire the old chain so any pending present entries
	// on the submit thread drain safely, then create a fresh chain. Replace with a ResizeBuffers-based graceful path
	// once the submit-thread present drain is verified end to end.
	mDevice.WaitUntilIdle();

	D3D12SwapChain* oldSwapChain = mSwapChain;

	// Prevent any further acquires on the old chain while its pending presents drain.
	oldSwapChain->MarkAsRetired();

	mVsync = vsync;

	CreateSwapChainInternal(width, height, vsync, oldSwapChain);

	// Release the old chain (deferred until it's no longer bound). The WaitUntilIdle above guarantees it isn't in use.
	oldSwapChain->Destroy();
}

void D3D12RenderWindowSurface::SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask)
{
	if (!mSwapChain || mIsDestroyed)
		return;

	GpuSubmitThread& submitThread = mDevice.GetSubmitThread();

	// Present the image that was rendered this frame. First acquired-but-not-yet-presented image is presented.
	submitThread.QueuePresent(queue, *mSwapChain, syncMask);

	// Acquire the image for the next frame. DXGI acquire is synchronous on the submit thread; queuing it here keeps
	// the acquire/present bookkeeping balanced (one acquire per present), mirroring the Vulkan surface.
	if (!mSwapChain->IsRetired())
		submitThread.QueueImageAcquire(*mSwapChain);
}

void D3D12RenderWindowSurface::MarkSwapChainAsInvalid()
{
	if (mSwapChain != nullptr && !mIsDestroyed)
	{
		// Mark swap chain as needing rebuild. The actual rebuild happens on the next frame when RebuildSwapChain is
		// called by the owning render window.
		B3D_LOG(Info, LogRenderBackend, "D3D12 swap chain marked as invalid");
	}
}

void D3D12RenderWindowSurface::Destroy()
{
	if (mIsDestroyed)
		return;

	B3D_LOG(Info, LogRenderBackend, "Destroying D3D12 render window surface");

	// Wait for GPU to finish all work (and drain any pending present entries) before releasing the swap chain.
	mDevice.WaitUntilIdle();

	if (mSwapChain)
	{
		mSwapChain->Destroy();
		mSwapChain = nullptr;
	}

	mIsDestroyed = true;
}

D3D12Framebuffer* D3D12RenderWindowSurface::GetFramebuffer(u32 backBufferIndex) const
{
	if (!mSwapChain)
		return nullptr;

	return mSwapChain->GetFramebuffer(backBufferIndex);
}

D3D12Framebuffer* D3D12RenderWindowSurface::GetActiveFramebuffer()
{
	if (!mSwapChain)
		return nullptr;

	u32 imageIndex;
	bool isImageAcquired = mSwapChain->TryGetFirstAcquiredImageIndex(imageIndex);

	// Fresh swap chain with no queued acquire yet - queue one and consume it.
	if (!isImageAcquired && !mSwapChain->IsRetired() && mDevice.HasSubmitThread())
	{
		mDevice.GetSubmitThread().QueueImageAcquire(*mSwapChain);
		isImageAcquired = mSwapChain->TryGetFirstAcquiredImageIndex(imageIndex);
	}

	if (!isImageAcquired)
		return nullptr;

	return mSwapChain->GetFramebuffer(imageIndex);
}
