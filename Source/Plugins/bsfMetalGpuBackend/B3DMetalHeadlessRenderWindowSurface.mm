//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalHeadlessRenderWindowSurface.h"
#include "B3DMetalGpuDevice.h"
#include "Debug/B3DLog.h"

namespace b3d::render
{
	MetalHeadlessRenderWindowSurface::MetalHeadlessRenderWindowSurface(MetalGpuDevice& device, const RenderWindowSurfaceCreateInformation& createInformation)
		: mGpuDevice(device), mWidth(createInformation.Width), mHeight(createInformation.Height), mVSync(createInformation.VSync), mCreateDepthBuffer(createInformation.CreateDepthBuffer), mUseHardwareSRGB(createInformation.UseHardwareSRGB)
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

			MTLTextureDescriptor* colorDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:colorFormat width:mWidth height:mHeight mipmapped:NO];
			colorDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
			colorDescriptor.storageMode = MTLStorageModePrivate;

			// TODO - Allocate the color and depth images through MetalHeapAllocator::AllocateTexture, as the Vulkan headless surface does through its allocator. Needs tracked MetalImages so their release is deferred until in-flight command buffers retire.
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
				MTLTextureDescriptor* depthDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8 width:mWidth height:mHeight mipmapped:NO];
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
