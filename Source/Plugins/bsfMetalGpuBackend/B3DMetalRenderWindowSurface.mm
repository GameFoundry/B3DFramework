//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalRenderWindowSurface.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuQueue.h"
#include "B3DMetalResourceManager.h"
#include "Debug/B3DLog.h"
#define BS_COCOA_INTERNALS 1
#include "Private/MacOS/B3DMacOSPlatform.h"
#include "Private/MacOS/B3DMacOSWindow.h"

namespace b3d::render
{
	namespace
	{
		void LogPresentCommandBufferError(id<MTLCommandBuffer> commandBuffer)
		{
			if ([commandBuffer status] != MTLCommandBufferStatusError)
				return;

			NSError* error = [commandBuffer error];
			B3D_LOG(Fatal, LogRenderBackend, "Metal presentation command buffer failed ({0}, code {1}): {2}",
				error ? String([[error domain] UTF8String]) : String("<unknown domain>"),
				error ? (i64)[error code] : 0,
				error ? String([[error localizedDescription] UTF8String]) : String("No error details were provided."));
		}
	}

	MetalSwapChain::MetalSwapChain(MetalResourceManager* owner, MetalRenderWindowSurface& surface)
		: Super(owner, "Metal render-window swap chain"), mSurface(surface)
	{ }

	MetalSwapChain::~MetalSwapChain()
	{
		mMessageQueue.RunUntilIdle();
		AbortCurrentDrawable();

		Lock lock(mMutex);
		for (PendingDrawable& pendingDrawable : mPendingDrawables)
			ReleaseDrawable(pendingDrawable.Drawable);
		mPendingDrawables.clear();
	}

	void MetalSwapChain::ReleaseDrawable(CAMetalDrawableRef __strong& drawable)
	{
		if (drawable == nil)
			return;

#if !__has_feature(objc_arc)
		[drawable release];
#endif
		drawable = nil;
	}

	MTLTextureRef MetalSwapChain::AcquireDrawable(CAMetalLayerRef layer)
	{
		@autoreleasepool
		{
			Lock lock(mMutex);
			if (mIsRetired || layer == nil)
				return nil;

			if (mCurrentDrawable == nil)
			{
				mCurrentDrawable = [layer nextDrawable];
#if !__has_feature(objc_arc)
				[mCurrentDrawable retain];
#endif
				mCurrentDrawableIndex = mNextDrawableIndex++;
				mCurrentDrawableWasRenderedInto = false;
			}

			return mCurrentDrawable != nil ? mCurrentDrawable.texture : nil;
		}
	}

	MTLTextureRef MetalSwapChain::GetCurrentTexture() const
	{
		Lock lock(mMutex);
		return mCurrentDrawable != nil ? mCurrentDrawable.texture : nil;
	}

	void MetalSwapChain::AbortCurrentDrawable()
	{
		Lock lock(mMutex);
		ReleaseDrawable(mCurrentDrawable);
		mCurrentDrawableWasRenderedInto = false;
	}

	void MetalSwapChain::MarkDrawableAsRendered()
	{
		Lock lock(mMutex);
		if (mCurrentDrawable != nil)
			mCurrentDrawableWasRenderedInto = true;
	}

	bool MetalSwapChain::TryGetFirstAcquiredImageIndex(u32& outImageIndex) const
	{
		Lock lock(mMutex);
		if (mCurrentDrawable == nil || !mCurrentDrawableWasRenderedInto || mIsRetired)
			return false;

		outImageIndex = mCurrentDrawableIndex;
		return true;
	}

	void MetalSwapChain::NotifyWasPresentQueued(u32 imageIndex)
	{
		Lock lock(mMutex);
		B3D_ASSERT(mCurrentDrawable != nil && mCurrentDrawableIndex == imageIndex);
		if (mCurrentDrawable == nil || mCurrentDrawableIndex != imageIndex)
			return;

		mPendingDrawables.push_back({ imageIndex, mCurrentDrawable, mSurface.mVSync,
			mSurface.mVSyncInterval, mSurface.mRefreshRate });
		mCurrentDrawable = nil;
		mCurrentDrawableWasRenderedInto = false;
		NotifyBound();
	}

	void MetalSwapChain::Present(u32 imageIndex, GpuQueue& queue, GpuQueueMask syncMask)
	{
		AssertIfNotSubmitThread();

		PendingDrawable claimedDrawable;
		{
			Lock lock(mMutex);
			auto iterFind = std::find_if(mPendingDrawables.begin(), mPendingDrawables.end(),
				[imageIndex](const PendingDrawable& entry) { return entry.Index == imageIndex; });
			if (iterFind != mPendingDrawables.end())
			{
				claimedDrawable = *iterFind;
#if __has_feature(objc_arc)
				// The local strong keeps the drawable alive after erasing its pending entry.
#else
				[claimedDrawable.Drawable retain];
#endif
				ReleaseDrawable(iterFind->Drawable);
				mPendingDrawables.erase(iterFind);
			}
		}

		if (claimedDrawable.Drawable == nil)
		{
			NotifyUnbound();
			return;
		}

		auto& metalQueue = static_cast<MetalGpuQueue&>(queue);
		id<MTLCommandQueue> commandQueue = metalQueue.GetMetalQueue();
		id<MTLCommandBuffer> commandBuffer = commandQueue != nil ? [commandQueue commandBuffer] : nil;
		if (commandBuffer == nil)
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to allocate the Metal presentation command buffer.");
			ReleaseDrawable(claimedDrawable.Drawable);
			NotifyUnbound();
			return;
		}

		GpuQueueMask waitMask = syncMask & ~GpuQueueMask(queue.GetId());
		for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
		{
			const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;
			for (u32 queueIndex = 0; queueIndex < mSurface.mGpuDevice.GetQueueCount(queueType); queueIndex++)
			{
				const GpuQueueId waitQueueId(queueType, queueIndex);
				if (!waitMask.IsSet(waitQueueId))
					continue;

				auto waitQueue = std::static_pointer_cast<MetalGpuQueue>(mSurface.mGpuDevice.GetQueue(queueType, queueIndex));
				const u64 waitValue = waitQueue->GetLastCommittedEventValue();
				if (waitValue != 0)
					[commandBuffer encodeWaitForEvent:waitQueue->GetSharedEvent() value:waitValue];
			}
		}

		const bool useTimedPresent = claimedDrawable.VSync && claimedDrawable.VSyncInterval > 1;
		if (useTimedPresent)
		{
			const double refreshRate = claimedDrawable.RefreshRate > 0.0f ? claimedDrawable.RefreshRate : 60.0;
			[commandBuffer presentDrawable:claimedDrawable.Drawable
				afterMinimumDuration:(double)claimedDrawable.VSyncInterval / refreshRate];
		}
		else
			[commandBuffer presentDrawable:claimedDrawable.Drawable];

		const u64 signalValue = metalQueue.ReserveNextEventValue();
		[commandBuffer encodeSignalEvent:metalQueue.GetSharedEvent() value:signalValue];

		TShared<WaitGroup> ownerCompletion = B3DMakeShared<WaitGroup>();
		ownerCompletion->Increment();
		[commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedBuffer)
		{
			LogPresentCommandBufferError(completedBuffer);
			NotifyUnbound();
			ownerCompletion->NotifyDone();
		}];

		[commandBuffer commit];
		metalQueue.NotifySubmissionCommitted(signalValue, commandBuffer, ownerCompletion);
		ReleaseDrawable(claimedDrawable.Drawable);
	}

	void MetalSwapChain::AcquireImage()
	{
		B3D_ASSERT(false && "CAMetalLayer drawables are acquired late by AcquireColorTexture().");
	}

	void MetalSwapChain::NotifyWasImageAcquireQueued()
	{
		B3D_ASSERT(false && "CAMetalLayer drawables do not use queued image acquisition.");
	}

	void MetalSwapChain::Retire()
	{
		mIsRetired.store(true, std::memory_order_release);
	}

	MetalRenderWindowSurface::MetalRenderWindowSurface(MetalGpuDevice& device, const RenderWindowSurfaceCreateInformation& createInformation)
		: mGpuDevice(device)
		, mWidth(createInformation.Width)
		, mHeight(createInformation.Height)
		, mVSync(createInformation.VSync)
		, mVSyncInterval(createInformation.VsyncInterval == 0 ? 1 : createInformation.VsyncInterval)
		, mRefreshRate(createInformation.RefreshRate)
		, mHwGamma(createInformation.UseHardwareSRGB)
		, mCreateDepthBuffer(createInformation.CreateDepthBuffer)
		, mPendingWidth(createInformation.Width)
		, mPendingHeight(createInformation.Height)
		, mPendingVSync(createInformation.VSync)
		, mPendingVSyncInterval(createInformation.VsyncInterval == 0 ? 1 : createInformation.VsyncInterval)
	{
		@autoreleasepool
		{
		if (createInformation.Headless)
		{
			// Headless surfaces are handled by MetalHeadlessRenderWindowSurface; the manager should never route
			// them here. Fail loudly instead of producing a silently non-functional surface.
			B3D_LOG(Error, LogRenderBackend, "MetalRenderWindowSurface created with Headless=true. Use MetalHeadlessRenderWindowSurface instead.");
			mValid = false;
			return;
		}

		MacOSPlatform::LockWindows();
		CocoaWindow* window = MacOSPlatform::GetWindow((u32)createInformation.PlatformWindowHandle);
		if (window != nullptr)
			mLayer = (__bridge CAMetalLayer*)window->GetLayerInternal();
#if !__has_feature(objc_arc)
		[mLayer retain];
#endif
		MacOSPlatform::UnlockWindows();

		if (mLayer == nil)
		{
			B3D_LOG(Error, LogRenderBackend, "MetalRenderWindowSurface could not resolve CAMetalLayer for Cocoa window ID {0}.",
				createInformation.PlatformWindowHandle);
			mValid = false;
			return;
		}

		mLayer.device = device.GetMetalDevice();
		mLayer.pixelFormat = mHwGamma ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;
		// framebufferOnly = NO so the drawable texture advertises blit-source usage, which the shared
		// IMetalRenderWindowSurface::ReadAsync relies on to copy the presented frame into a PixelData staging
		// buffer. Keeping this disabled has a persistent optimization cost versus framebufferOnly=YES, but the
		// current RenderWindow API cannot declare capture intent before drawable acquisition. Re-enable the fast path
		// when that backend-neutral capability is added; silently breaking window screenshots is not acceptable.
		mLayer.framebufferOnly = NO;
		mLayer.drawableSize = CGSizeMake(mWidth, mHeight);

		// Pin the drawable pool at 3. CAMetalLayer's default is already 3 on current macOS but was 2 in older
		// releases, and the explicit value also documents intent: triple-buffering gives the CPU one frame of
		// slack over the GPU, which the engine's fiber scheduler assumes.
		mLayer.maximumDrawableCount = 3;
		mLayer.allowsNextDrawableTimeout = YES;

		// SDR windows use the sRGB output color space in both modes. Hardware gamma controls whether the attachment
		// format performs the transfer encoding; it does not change how the compositor interprets the output gamut.
		CGColorSpaceRef layerColorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
		mLayer.colorspace = layerColorspace;
		CGColorSpaceRelease(layerColorspace);

#if TARGET_OS_OSX
		mLayer.displaySyncEnabled = mVSync ? YES : NO;
		mLayer.presentsWithTransaction = NO;
#endif

		mSwapChain = device.GetResourceManager().Create<MetalSwapChain>(*this);
		RecreateDepthStencilTextureIfNeeded();
		}
	}

	MetalRenderWindowSurface::~MetalRenderWindowSurface()
	{
		// Dtor mirrors @c Destroy() exactly — the Obj-C strongs are released and @c mValid flips false. Delegating
		// keeps the teardown in one place; calling @c Destroy() on an already-destroyed surface is idempotent.
		Destroy();
	}

	void MetalRenderWindowSurface::ReleaseCurrentDrawable()
	{
		if (mSwapChain != nullptr)
			mSwapChain->AbortCurrentDrawable();
	}

	void MetalRenderWindowSurface::ApplyPendingStateIfNeeded()
	{
		// Drain any pending resize staged by RebuildSwapChain / MarkSwapChainAsInvalid. Writes to
		// @c mLayer.drawableSize must stay on the render thread — the property is documented as thread-safe but
		// concurrent writes are non-deterministic, and AppKit-driven resizes on the main thread would otherwise
		// race the render thread's drawable acquisition.
		if (!mNeedsDrawableSizeReapply.exchange(false, std::memory_order_acquire))
			return;

		u32 width;
		u32 height;
		bool vsync;
		u32 vsyncInterval;
		{
			Lock lock(mPendingStateMutex);
			width = mPendingWidth;
			height = mPendingHeight;
			vsync = mPendingVSync;
			vsyncInterval = mPendingVSyncInterval;
		}

		mWidth = width;
		mHeight = height;
		mVSync = vsync;
		mVSyncInterval = vsyncInterval;
		mLayer.drawableSize = CGSizeMake(mWidth, mHeight);
#if TARGET_OS_OSX
		mLayer.displaySyncEnabled = mVSync ? YES : NO;
#endif

		RecreateDepthStencilTextureIfNeeded();
	}

	void MetalRenderWindowSurface::RecreateDepthStencilTextureIfNeeded()
	{
		if (!mCreateDepthBuffer || mWidth == 0 || mHeight == 0)
			return;

		if (mDepthStencilTexture != nil && (u32)mDepthStencilTexture.width == mWidth && (u32)mDepthStencilTexture.height == mHeight)
			return;

		// Depth32Float_Stencil8 is the universally supported depth/stencil format on Apple GPUs
		// (Depth24Unorm_Stencil8 is unavailable on Apple silicon). Private storage rather than memoryless because
		// the engine's LoadMask can legally request depth contents to be preserved across render passes.
		MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8
			width:mWidth height:mHeight mipmapped:NO];
		descriptor.usage = MTLTextureUsageRenderTarget;
		descriptor.storageMode = MTLStorageModePrivate;

		id<MTLTexture> newDepthStencil = [mGpuDevice.GetMetalDevice() newTextureWithDescriptor:descriptor];
		if (newDepthStencil == nil)
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to create the render window depth/stencil texture ({0}x{1}).", mWidth, mHeight);
			return;
		}

		newDepthStencil.label = @"RenderWindowDepthStencil";

		// Any in-flight command buffer that references the old texture retains it (default retained-references
		// mode), so replacing our strong reference here is safe even while a prior frame is still executing.
		if (mDepthStencilTexture != nil)
		{
#if !__has_feature(objc_arc)
			[mDepthStencilTexture release];
#endif
		}

		mDepthStencilTexture = newDepthStencil; // Owns the +1 from newTextureWithDescriptor:.
	}

	MTLTextureRef MetalRenderWindowSurface::AcquireColorTexture()
	{
		@autoreleasepool
		{
			if (!mValid || mLayer == nil)
				return nil;

			ApplyPendingStateIfNeeded();

			return mSwapChain != nullptr ? mSwapChain->AcquireDrawable(mLayer) : nil;
		}
	}

	MTLTextureRef MetalRenderWindowSurface::GetCurrentColorTexture() const
	{
		return mSwapChain != nullptr ? mSwapChain->GetCurrentTexture() : nil;
	}

	void MetalRenderWindowSurface::AbortCurrentDrawable()
	{
		ReleaseCurrentDrawable();
	}

	void MetalRenderWindowSurface::MarkDrawableAsRendered()
	{
		// The command buffer calls this right after @c renderCommandEncoderWithDescriptor: succeeds. The render
		// pass's color attachment is configured with a store action against the drawable texture, so a successful
		// encoder open is sufficient evidence that the drawable will carry defined content at submission time.
		// Guard against the theoretical no-drawable case so a stray call after a teardown path is a no-op.
		if (mSwapChain != nullptr)
			mSwapChain->MarkDrawableAsRendered();
	}

	MTLPixelFormatValue MetalRenderWindowSurface::GetColorFormat() const
	{
		return mLayer != nil ? mLayer.pixelFormat : MTLPixelFormatBGRA8Unorm;
	}

	void MetalRenderWindowSurface::SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask)
	{
		if (!mValid || mSwapChain == nullptr)
			return;
		u32 imageIndex;
		if (!mSwapChain->TryGetFirstAcquiredImageIndex(imageIndex))
		{
			mSwapChain->AbortCurrentDrawable();
			return;
		}

		mGpuDevice.GetSubmitThread().QueuePresent(queue, *mSwapChain, syncMask);
	}

	void MetalRenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync, u32 vsyncInterval)
	{
		// May be invoked from the main thread during AppKit live-resize or from the render thread via the command
		// buffer's swap-chain-invalid path. Either way, we only stage here — the actual CAMetalLayer mutation
		// happens in AcquireColorTexture on the render thread so there is exactly one writer to the layer's
		// mutable state.
		{
			Lock lock(mPendingStateMutex);
			mPendingWidth = width;
			mPendingHeight = height;
			mPendingVSync = vsync;
			mPendingVSyncInterval = vsyncInterval == 0 ? 1 : vsyncInterval;
		}
		mNeedsDrawableSizeReapply.store(true, std::memory_order_release);
	}

	void MetalRenderWindowSurface::MarkSwapChainAsInvalid()
	{
		// Dropping the drawable here is safe: the caller has guaranteed no render pass is currently writing to it
		// (the engine calls this on resize *after* the frame boundary). The pending-resize flag is toggled so the
		// next AcquireColorTexture re-applies the staged width/height to the layer — after a move between displays
		// the layer's drawableSize may have drifted. Pending values are not touched: they are either from a prior
		// @c RebuildSwapChain (the intended new size) or seeded from the ctor (the still-current size).
		ReleaseCurrentDrawable();
		mNeedsDrawableSizeReapply.store(true, std::memory_order_release);
	}

	void MetalRenderWindowSurface::Destroy()
	{
		if (!mValid && mSwapChain == nullptr && mLayer == nil)
			return;

		ReleaseCurrentDrawable();
		if (mSwapChain != nullptr)
		{
			mSwapChain->Retire();
			if (mGpuDevice.HasSubmitThread())
				mGpuDevice.GetSubmitThread().WaitUntilIdle();

			mSwapChain->GetMessageQueue().RunUntilIdle();
			mSwapChain->Destroy();
			mSwapChain = nullptr;
		}

		if (mDepthStencilTexture != nil)
		{
#if !__has_feature(objc_arc)
			[mDepthStencilTexture release];
#endif
			mDepthStencilTexture = nil;
		}

		if (mLayer != nil)
		{
#if !__has_feature(objc_arc)
			[mLayer release];
#endif
			mLayer = nil;
		}

		mValid = false;
	}
} // namespace b3d::render
