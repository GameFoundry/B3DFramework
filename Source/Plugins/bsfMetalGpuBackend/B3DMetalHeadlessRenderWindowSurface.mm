//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalHeadlessRenderWindowSurface.h"
#include "B3DMetalGpuDevice.h"
#include "Debug/B3DLog.h"

namespace b3d::render
{
	MetalHeadlessRenderWindowSurface::MetalHeadlessRenderWindowSurface(MetalGpuDevice& device, const RenderWindowSurfaceCreateInformation& createInformation)
		: mGpuDevice(device)
		, mWidth(createInformation.Width)
		, mHeight(createInformation.Height)
		, mVSync(createInformation.VSync)
		, mCreateDepthBuffer(createInformation.CreateDepthBuffer)
		, mUseHardwareSRGB(createInformation.UseHardwareSRGB)
	{
		CreateSwapChainImages();
	}

	MetalHeadlessRenderWindowSurface::~MetalHeadlessRenderWindowSurface()
	{
		Destroy();
	}

	void MetalHeadlessRenderWindowSurface::CreateSwapChainImages()
	{
		@autoreleasepool
		{
			if (mWidth == 0 || mHeight == 0)
			{
				B3D_LOG(Error, LogRenderBackend, "Headless render window surface created with zero size ({0}x{1}).", mWidth, mHeight);
				mIsValid = false;
				return;
			}

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();

			// BGRA8 matches the format the windowed surface's CAMetalLayer would use, so headless and windowed
			// rendering produce byte-identical readbacks (GetColorPixelFormat() returns PF_BGRA8 for both).
			const MTLPixelFormat colorFormat = mUseHardwareSRGB ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;

			MTLTextureDescriptor* colorDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:colorFormat
				width:mWidth height:mHeight mipmapped:NO];
			colorDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
			colorDescriptor.storageMode = MTLStorageModePrivate;

			for (u32 imageIndex = 0; imageIndex < kImageCount; imageIndex++)
			{
				// Store a strong texture reference until DestroySwapChainImages().
				id<MTLTexture> colorTexture = [device newTextureWithDescriptor:colorDescriptor];
				if (colorTexture == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "Failed to create headless swap chain color image {0} ({1}x{2}).", imageIndex, mWidth, mHeight);
					DestroySwapChainImages();
					mIsValid = false;
					return;
				}

				colorTexture.label = [NSString stringWithFormat:@"HeadlessSwapChainColor%u", imageIndex];
				mColorTextures[imageIndex] = colorTexture;
			}

			if (mCreateDepthBuffer)
			{
				// Depth32Float_Stencil8 is the universally supported depth/stencil format on Apple GPUs. Private
				// rather than memoryless storage because the engine's LoadMask can legally request depth contents
				// to be preserved across render passes.
				MTLTextureDescriptor* depthDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8
					width:mWidth height:mHeight mipmapped:NO];
				depthDescriptor.usage = MTLTextureUsageRenderTarget;
				depthDescriptor.storageMode = MTLStorageModePrivate;

				mDepthStencilTexture = [device newTextureWithDescriptor:depthDescriptor];
				if (mDepthStencilTexture == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "Failed to create headless swap chain depth image ({0}x{1}).", mWidth, mHeight);
					DestroySwapChainImages();
					mIsValid = false;
					return;
				}

				mDepthStencilTexture.label = @"HeadlessSwapChainDepthStencil";
			}

			mCurrentImageIndex = 0;
			mIsValid = true;
		}
	}

	void MetalHeadlessRenderWindowSurface::DestroySwapChainImages()
	{
		// In-flight MTLCommandBuffers retain the textures they reference (default retained-references mode), so
		// dropping our strong references here is safe even while a prior frame is still executing on the GPU.
		for (u32 imageIndex = 0; imageIndex < kImageCount; imageIndex++)
		{
			if (mColorTextures[imageIndex] != nil)
			{
#if !__has_feature(objc_arc)
				[mColorTextures[imageIndex] release];
#endif
				mColorTextures[imageIndex] = nil;
			}
		}

		if (mDepthStencilTexture != nil)
		{
#if !__has_feature(objc_arc)
			[mDepthStencilTexture release];
#endif
			mDepthStencilTexture = nil;
		}
	}

	MTLTextureRef MetalHeadlessRenderWindowSurface::AcquireColorTexture()
	{
		// Even when flagged invalid (pending rebuild) the current image remains usable — rendering at a stale size
		// is preferable to dropping the frame; the rebuild lands before the next acquire.
		if (mIsDestroyed)
			return nil;

		if (mIsSwapQueued)
		{
			mCurrentImageIndex = (mCurrentImageIndex + 1) % kImageCount;
			mIsSwapQueued = false;
		}

		return mColorTextures[mCurrentImageIndex];
	}

	MTLPixelFormatValue MetalHeadlessRenderWindowSurface::GetColorFormat() const
	{
		return mUseHardwareSRGB ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;
	}

	void MetalHeadlessRenderWindowSurface::SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask)
	{
		// Headless surfaces have nothing to present and nothing to synchronize against the compositor — a "swap"
		// just cycles to the next offscreen image, mirroring VulkanHeadlessRenderWindowSurface. The queue and sync
		// mask are intentionally unused: any producer/consumer hazards on the offscreen images are handled by the
		// regular per-resource tracking on the command buffers that render to / read from them.
		if (mIsDestroyed || !mIsValid)
			return;

		mIsSwapQueued = true;
	}

	void MetalHeadlessRenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync, u32 vsyncInterval)
	{
		if (mIsDestroyed)
			return;
		if (mIsValid && width == mWidth && height == mHeight && vsync == mVSync)
			return;

		mGpuDevice.WaitUntilIdle();

		mWidth = width;
		mHeight = height;
		mVSync = vsync;
		mIsSwapQueued = false;

		DestroySwapChainImages();
		CreateSwapChainImages();
	}

	void MetalHeadlessRenderWindowSurface::MarkSwapChainAsInvalid()
	{
		// The owning RenderWindow's properties (size/vsync) changed. Flag the surface so the command buffer's
		// invalid-swap-chain path triggers RenderWindow::RebuildSwapChain, which recreates the images at the new
		// size via RebuildSwapChain() above.
		mIsValid = false;
	}

	void MetalHeadlessRenderWindowSurface::Destroy()
	{
		if (mIsDestroyed)
			return;

		mIsDestroyed = true;
		mIsValid = false;
		DestroySwapChainImages();
	}
} // namespace b3d::render
