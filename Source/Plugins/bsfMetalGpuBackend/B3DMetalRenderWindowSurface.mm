//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalRenderWindowSurface.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuQueue.h"
#include "B3DMetalResourceManager.h"
#include "B3DMetalTexture.h"
#include "Debug/B3DLog.h"
#include "Threading/B3DScheduler.h"
#define BS_COCOA_INTERNALS 1
#include "Private/MacOS/B3DMacOSPlatform.h"
#include "Private/MacOS/B3DMacOSWindow.h"

namespace b3d::render
{
	MetalSwapChain::MetalSwapChain(MetalResourceManager* owner, MetalRenderWindowSurface& surface)
		: Super(owner, "Metal render-window swap chain"), mSurface(surface)
	{
		// Present-completion notifies (swap chain NotifyUnbound) are posted back to this queue and processed on the thread
		// responsible for the swap chain (the render thread).
		Scheduler* const scheduler = Scheduler::Get();
		if (B3D_ENSURE(scheduler))
			mMessageQueue.ScheduleRunUntilShutdown(*scheduler, true);
	}

	MetalSwapChain::~MetalSwapChain()
	{
		mMessageQueue.PostRequestShutdownCommand(true);
		AbortCurrentDrawable();

		Lock lock(mMutex);
		mPendingDrawables.clear();
	}

	id<MTLTexture> MetalSwapChain::AcquireDrawable(CAMetalLayer* layer)
	{
		Lock lock(mMutex);
		if (mIsRetired || layer == nil)
			return nil;

		if (mCurrentDrawable == nil)
		{
			mCurrentDrawable = [layer nextDrawable];
			mCurrentDrawableIndex = mNextDrawableIndex++;
			mCurrentDrawableWasRenderedInto = false;
		}

		return mCurrentDrawable != nil ? mCurrentDrawable.texture : nil;
	}

	id<MTLTexture> MetalSwapChain::GetCurrentTexture() const
	{
		Lock lock(mMutex);
		return mCurrentDrawable != nil ? mCurrentDrawable.texture : nil;
	}

	id<CAMetalDrawable> MetalSwapChain::GetCurrentDrawable() const
	{
		Lock lock(mMutex);
		return mCurrentDrawable;
	}

	void MetalSwapChain::AbortCurrentDrawable()
	{
		Lock lock(mMutex);
		mCurrentDrawable = nil;
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

		mPendingDrawables.push_back({ imageIndex, mCurrentDrawable, mSurface.mVSync, mSurface.mVSyncInterval, mSurface.mRefreshRate });
		mCurrentDrawable = nil;
		mCurrentDrawableWasRenderedInto = false;
	}

	void MetalSwapChain::Present(u32 imageIndex, GpuQueue& queue, GpuQueueMask syncMask)
	{
		AssertIfNotSubmitThread();

		PendingDrawable claimedDrawable;
		{
			Lock lock(mMutex);
			auto iterFind = std::find_if(mPendingDrawables.begin(), mPendingDrawables.end(), [imageIndex](const PendingDrawable& entry) { return entry.Index == imageIndex; });
			if (iterFind != mPendingDrawables.end())
			{
				claimedDrawable = *iterFind;
				// The local strong keeps the drawable alive after erasing its pending entry.
				mPendingDrawables.erase(iterFind);
			}
		}

		if (claimedDrawable.Drawable == nil)
			return;

		// The queue already waits for the drawable since the submission that rendered to it
		auto& metalQueue = static_cast<MetalGpuQueue&>(queue);

		// Waits on the latest committed work of every other queue in the sync mask
		GpuQueueFenceValues waits = {};
		queue.GetGpuDevice().DoForEachQueue([syncMask, &waits](GpuQueue& otherQueue)
		{
			if (syncMask.IsSet(otherQueue.GetId()))
				waits[otherQueue.GetId().Id] = static_cast<MetalGpuQueue&>(otherQueue).GetLastCommittedEventValue();
		});

		metalQueue.WaitForQueues(waits);
		metalQueue.SignalDrawable(claimedDrawable.Drawable);

		const bool useTimedPresent = claimedDrawable.VSync && claimedDrawable.VSyncInterval > 1;
		if (useTimedPresent)
		{
			const double refreshRate = claimedDrawable.RefreshRate > 0.0f ? claimedDrawable.RefreshRate : 60.0;
			[claimedDrawable.Drawable presentAfterMinimumDuration:(double)claimedDrawable.VSyncInterval / refreshRate];
		}
		else
			[claimedDrawable.Drawable present];

		TShared<WaitGroup> ownerCompletion = B3DMakeShared<WaitGroup>();
		ownerCompletion->Increment();
		metalQueue.SignalSubmission([ownerCompletion]() { ownerCompletion->NotifyDone(); }, ownerCompletion);
	}

	void MetalSwapChain::AcquireImage()
	{
		B3D_ASSERT(false && "Metal drawables are acquired on the render thread at BeginRenderPass, not through the submit thread. See MetalSwapChain.");
	}

	void MetalSwapChain::NotifyWasImageAcquireQueued()
	{
		B3D_ASSERT(false && "Metal drawables are acquired on the render thread at BeginRenderPass, not through the submit thread. See MetalSwapChain.");
	}

	void MetalSwapChain::Destroy()
	{
		// Process pending queued-operation unbind notifications so the resource can be destroyed immediately when its
		// bound count reaches zero (important for shutdown).
		mMessageQueue.RunUntilIdle();

		Super::Destroy();
	}

	void MetalSwapChain::Retire()
	{
		mIsRetired.store(true, std::memory_order_release);
	}

	MetalRenderWindowSurface::MetalRenderWindowSurface(MetalGpuDevice& device, const RenderWindowSurfaceCreateInformation& createInformation)
		: mGpuDevice(device), mWidth(createInformation.Width), mHeight(createInformation.Height), mVSync(createInformation.VSync), mVSyncInterval(createInformation.VsyncInterval == 0 ? 1 : createInformation.VsyncInterval), mRefreshRate(createInformation.RefreshRate), mHwGamma(createInformation.UseHardwareSRGB), mCreateDepthBuffer(createInformation.CreateDepthBuffer), mPendingWidth(createInformation.Width), mPendingHeight(createInformation.Height), mPendingVSync(createInformation.VSync), mPendingVSyncInterval(createInformation.VsyncInterval == 0 ? 1 : createInformation.VsyncInterval)
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

		// Development builds need screen capture capability. Ideally we can always keep this to YES and do screen capture without blitting directly from framebuffer.
#if B3D_BUILD_TYPE_DEVELOPMENT
		mLayer.framebufferOnly = NO;
#else
		mLayer.framebufferOnly = YES;
#endif
		mLayer.drawableSize = CGSizeMake(mWidth, mHeight);

		// Pin the drawable pool at 3 (CAMetalLayer's default on current macOS, but 2 in older releases). The
		// Vulkan and D3D12 backends run with 2 images because their acquire returns before the displayed
		// image is actually free and the wait for it is deferred to the GPU (acquire semaphore / DXGI flip
		// model), so the render thread is never blocked by the display. nextDrawable has no such deferral:
		// it blocks the calling render thread until a drawable is fully released. Since drawables are
		// acquired at command recording time (see MetalSwapChain), the render thread holds one drawable per
		// frame it is ahead of the display, and with only 2 drawables BeginRenderPass would stall until the
		// on-screen one retires at vblank. The third drawable lets the render thread run one frame ahead,
		// the same slack the other backends get from the deferred wait; nextDrawable blocking beyond that
		// is what paces the render thread against the display on Metal.
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

	MetalRenderWindowSurface::~MetalRenderWindowSurface()
	{
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

		if (mDepthStencilImage != nullptr)
		{
			id<MTLTexture> depthStencilTexture = mDepthStencilImage->GetMetalHandle();
			if ((u32)depthStencilTexture.width == mWidth && (u32)depthStencilTexture.height == mHeight)
				return;

			// Released once the command buffers that render to it finish executing
			mDepthStencilImage->Destroy();
			mDepthStencilImage = nullptr;
		}

		// Depth32Float_Stencil8 is the universally supported depth/stencil format on Apple GPUs (Depth24Unorm_Stencil8 is unavailable on Apple silicon).
		mDepthStencilImage = CreateSurfaceImage(mGpuDevice, MTLPixelFormatDepth32Float_Stencil8, PF_D32_S8X24, TextureUsageFlag::DepthStencil, mWidth, mHeight, "RenderWindowDepthStencil");
	}

	id<MTLTexture> MetalRenderWindowSurface::GetDepthStencilTexture() const
	{
		return mDepthStencilImage != nullptr ? mDepthStencilImage->GetMetalHandle() : nil;
	}

	id<MTLTexture> MetalRenderWindowSurface::AcquireColorTexture()
	{
		if (!mValid || mLayer == nil)
			return nil;

		ApplyPendingStateIfNeeded();

		return mSwapChain != nullptr ? mSwapChain->AcquireDrawable(mLayer) : nil;
	}

	id<MTLTexture> MetalRenderWindowSurface::GetCurrentColorTexture() const
	{
		return mSwapChain != nullptr ? mSwapChain->GetCurrentTexture() : nil;
	}

	id<CAMetalDrawable> MetalRenderWindowSurface::GetCurrentDrawable() const
	{
		return mSwapChain != nullptr ? mSwapChain->GetCurrentDrawable() : nil;
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

	MTLPixelFormat MetalRenderWindowSurface::GetColorFormat() const
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

	bool MetalRenderWindowSurface::HasAcquiredImage() const
	{
		u32 imageIndex;
		return mValid && mSwapChain != nullptr && mSwapChain->TryGetFirstAcquiredImageIndex(imageIndex);
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

			mSwapChain->Destroy();
			mSwapChain = nullptr;
		}

		if (mDepthStencilImage != nullptr)
		{
			mDepthStencilImage->Destroy();
			mDepthStencilImage = nullptr;
		}

		mLayer = nil;

		mValid = false;
	}
} // namespace b3d::render
