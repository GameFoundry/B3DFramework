//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12RenderWindowSurface.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12SwapChain.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12Framebuffer.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "Image/B3DPixelUtility.h"

using namespace b3d;
using namespace b3d::render;

D3D12RenderWindowSurface::D3D12RenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation)
	: mDevice(static_cast<D3D12GpuDevice&>(*GetD3D12GpuBackend().GetPrimaryDevice()))
	, mWindowHandle((HWND)createInformation.PlatformWindowHandle)
	, mCreateDepthBuffer(createInformation.CreateDepthBuffer)
	, mUseHardwareSRGB(createInformation.UseHardwareSRGB)
	, mIsHeadless(createInformation.Headless)
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
	swapChainCreateInfo.Headless = mIsHeadless;

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

TAsyncOp<TShared<PixelData>> D3D12RenderWindowSurface::ReadAsync(GpuCommandBuffer& commandBuffer)
{
	if (mSwapChain == nullptr)
		return {};

	// Read the image holding the last fully rendered frame - captures typically run after the frame was presented.
	// Note this is only reliable for headless surfaces, whose back buffers are plain committed textures. DXGI
	// flip-discard back buffer contents are formally undefined after present, so windowed captures are best-effort.
	const u32 imageIndex = mSwapChain->GetLastPresentedImageIndex();
	D3D12Image* colorImage = mSwapChain->GetBackBufferImage(imageIndex);
	if (colorImage == nullptr)
		return {};

	const u32 width = mSwapChain->GetWidth();
	const u32 height = mSwapChain->GetHeight();
	const PixelFormat pixelFormat = PF_RGBA8;

	const TShared<PixelData> pixelData = B3DMakeShared<PixelData>(width, height, 1, pixelFormat);

	// Buffer rows must be padded to the placed-footprint pitch alignment.
	const u32 tightRowPitch = width * PixelUtility::GetBlockSize(pixelFormat);
	const u32 paddedRowPitch = Math::CeilToMultiple(tightRowPitch, (u32)D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	const u32 bufferSize = paddedRowPitch * height;

	GpuBufferCreateInformation bufferCreateInfo;
	bufferCreateInfo.Type = GpuBufferType::StagingRead;
	bufferCreateInfo.Staging.Size = bufferSize;

	TShared<GpuBuffer> stagingBuffer = mDevice.CreateGpuBuffer(bufferCreateInfo, GpuObjectCreateFlag::None);
	if (stagingBuffer == nullptr)
		return {};

	D3D12Buffer* d3d12StagingBuffer = static_cast<D3D12GpuBuffer*>(stagingBuffer.get())->GetD3D12Buffer();

	// Issue the copy command
	D3D12GpuCommandBuffer& d3d12CommandBuffer = static_cast<D3D12GpuCommandBuffer&>(commandBuffer);
	d3d12CommandBuffer.CopyImageToBuffer(colorImage, d3d12StagingBuffer, width, height, paddedRowPitch);

	// Set up async completion
	TAsyncOp<TShared<PixelData>> op;

	auto fnOnCommandBufferCompleted = [stagingBuffer, op, pixelData, tightRowPitch, paddedRowPitch, height]() mutable
	{
		GpuBufferMappedScope mapping = stagingBuffer->Map(GpuMapOption::Read);

		pixelData->AllocateInternalBuffer();

		const u8* source = (const u8*)mapping.GetMappedMemory();
		u8* destination = pixelData->GetData();

		if (paddedRowPitch == tightRowPitch)
			memcpy(destination, source, pixelData->GetSize());
		else
		{
			// De-pad the rows into the tightly packed pixel data
			for (u32 row = 0; row < height; row++)
				memcpy(destination + row * (u64)tightRowPitch, source + row * (u64)paddedRowPitch, tightRowPitch);
		}

		op.CompleteOperation(pixelData);
	};

	auto fnOnCommandBufferDestroyed = [op](bool isSubmitted) mutable
	{
		if (isSubmitted)
			return;

		op.CompleteOperation(nullptr);
	};

	commandBuffer.OnDidComplete.Connect(fnOnCommandBufferCompleted);
	commandBuffer.OnDestroyed.Connect(fnOnCommandBufferDestroyed);

	return op;
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
