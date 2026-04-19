//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12RenderWindowSurface.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12SwapChain.h"
#include "B3DD3D12Utility.h"

using namespace b3d::render;

D3D12RenderWindowSurface::D3D12RenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation)
	: mDevice(static_cast<D3D12GpuDevice&>(*GetD3D12GpuBackend().GetPrimaryDevice()))
	, mWindowHandle((HWND)createInformation.PlatformWindowHandle)
	, mCreateDepthBuffer(createInformation.CreateDepthBuffer)
	, mUseHardwareSRGB(createInformation.UseHardwareSRGB)
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

	// Create swap chain
	D3D12SwapChainCreateInformation swapChainCreateInfo;
	swapChainCreateInfo.WindowHandle = mWindowHandle;
	swapChainCreateInfo.Width = createInformation.Width;
	swapChainCreateInfo.Height = createInformation.Height;
	swapChainCreateInfo.VSync = createInformation.VSync;
	swapChainCreateInfo.ColorFormat = mColorFormat;
	swapChainCreateInfo.DepthStencilFormat = mDepthFormat;
	swapChainCreateInfo.CreateDepthBuffer = mCreateDepthBuffer;

	mSwapChain = B3DNew<D3D12SwapChain>(swapChainCreateInfo, mDevice);
	mSwapChain->Initialize();

	B3D_LOG(Info, LogRenderBackend, "Created D3D12 render window surface: width={0}, height={1}, vsync={2}, srgb={3}",
		createInformation.Width, createInformation.Height, createInformation.VSync, mUseHardwareSRGB);
}

D3D12RenderWindowSurface::~D3D12RenderWindowSurface()
{
	Destroy();
}

void D3D12RenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync)
{
	if (!mSwapChain || mIsDestroyed)
		return;

	B3D_LOG(Info, LogRenderBackend, "Rebuilding D3D12 swap chain: width={0}, height={1}, vsync={2}", width, height, vsync);

	// Wait for GPU to finish all work before destroying swap chain
	mDevice.WaitUntilIdle();

	// Destroy old swap chain
	mSwapChain->Destroy();
	mSwapChain = nullptr;

	// Create new swap chain with updated parameters
	D3D12SwapChainCreateInformation swapChainCreateInfo;
	swapChainCreateInfo.WindowHandle = mWindowHandle;
	swapChainCreateInfo.Width = width;
	swapChainCreateInfo.Height = height;
	swapChainCreateInfo.VSync = vsync;
	swapChainCreateInfo.ColorFormat = mColorFormat;
	swapChainCreateInfo.DepthStencilFormat = mDepthFormat;
	swapChainCreateInfo.CreateDepthBuffer = mCreateDepthBuffer;

	mSwapChain = B3DNew<D3D12SwapChain>(swapChainCreateInfo, mDevice);
	mSwapChain->Initialize();
}

void D3D12RenderWindowSurface::MarkSwapChainAsInvalid()
{
	if (mSwapChain != nullptr && !mIsDestroyed)
	{
		// Mark swap chain as needing rebuild
		// The actual rebuild will happen on the next frame when RebuildSwapChain is called
		B3D_LOG(Info, LogRenderBackend, "D3D12 swap chain marked as invalid");
	}
}

void D3D12RenderWindowSurface::Destroy()
{
	if (mIsDestroyed)
		return;

	B3D_LOG(Info, LogRenderBackend, "Destroying D3D12 render window surface");

	// Wait for GPU to finish all work before destroying swap chain
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
