//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalRenderWindowSurface.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuQueue.h"
#include "B3DMetalGpuCommandBuffer.h"
#include "B3DMetalUtility.h"
#include "Image/B3DPixelData.h"
#include "Debug/B3DLog.h"

#if TARGET_OS_OSX
#import <AppKit/AppKit.h>
#elif TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif

namespace b3d::render
{
	MetalRenderWindowSurface::MetalRenderWindowSurface(MetalGpuDevice& device, const RenderWindowSurfaceCreateInformation& createInformation)
		: mGpuDevice(device)
		, mWidth(createInformation.Width)
		, mHeight(createInformation.Height)
		, mVSync(createInformation.VSync)
		, mVSyncInterval(createInformation.VsyncInterval == 0 ? 1 : createInformation.VsyncInterval)
		, mHwGamma(createInformation.UseHardwareSRGB)
		, mPendingWidth(createInformation.Width)
		, mPendingHeight(createInformation.Height)
		, mPendingVSync(createInformation.VSync)
	{
		if (createInformation.Headless)
			return;

		mLayer = [CAMetalLayer layer];
		mLayer.device = device.GetMetalDevice();
		mLayer.pixelFormat = mHwGamma ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;
		// framebufferOnly = NO so the drawable texture advertises blit-source usage, which
		// MetalGpuCommandBuffer::ReadAsync relies on to copy the presented frame into a PixelData
		// staging buffer. The YES path happens to work on Apple Silicon today but fails under Metal
		// API validation and is undefined per the CAMetalLayer contract. The small TBDR perf cost
		// is acceptable given that ReadAsync is a documented feature of the engine's RenderWindow.
		mLayer.framebufferOnly = NO;
		mLayer.drawableSize = CGSizeMake(mWidth, mHeight);

		// Pin the drawable pool at 3. CAMetalLayer's default is already 3 on current macOS but
		// was 2 in older releases, and the explicit value also documents intent: triple-buffering
		// gives the CPU one frame of slack over the GPU, which the engine's fiber scheduler assumes.
		mLayer.maximumDrawableCount = 3;

		// Match the layer's colorspace to the chosen pixel format. CAMetalLayer on wide-gamut
		// displays (Apple P3) would otherwise reinterpret our output as P3:
		//   - Hardware sRGB encoding (BGRA8Unorm_sRGB): the texture already encodes into sRGB,
		//     so the layer must treat the bytes as sRGB. kCGColorSpaceSRGB.
		//   - Linear pixel format (BGRA8Unorm): the engine wrote linear values (e.g. tonemapped
		//     HDR output that stayed in linear space). kCGColorSpaceLinearSRGB keeps them linear
		//     through the compositor instead of second-applying the sRGB transfer function.
		const CFStringRef colorspaceName = mHwGamma ? kCGColorSpaceSRGB : kCGColorSpaceLinearSRGB;
		CGColorSpaceRef layerColorspace = CGColorSpaceCreateWithName(colorspaceName);
		mLayer.colorspace = layerColorspace;
		CGColorSpaceRelease(layerColorspace);

#if TARGET_OS_OSX
		mLayer.displaySyncEnabled = mVSync ? YES : NO;
		// Serialize present with NSWindow transactions so AppKit's live-resize loop can't display
		// a frame before the window geometry update lands. Without this, dragging a window edge
		// produces a 1-frame tearing band between the old and new drawable sizes.
		mLayer.presentsWithTransaction = YES;
#endif

#if TARGET_OS_OSX
		NSWindow* window = (__bridge NSWindow*)(void*)createInformation.PlatformWindowHandle;
		if (window == nil)
		{
			B3D_LOG(Error, LogRenderBackend, "MetalRenderWindowSurface created with a null NSWindow handle.");
			mValid = false;
			return;
		}

		NSView* view = [window contentView];
		[view setWantsLayer:YES];
		[view setLayer:mLayer];

		CGFloat scale = [window backingScaleFactor];
		mLayer.contentsScale = scale;
#elif TARGET_OS_IPHONE
		UIView* view = (__bridge UIView*)(void*)createInformation.PlatformWindowHandle;
		if (view == nil)
		{
			B3D_LOG(Error, LogRenderBackend, "MetalRenderWindowSurface created with a null UIView handle.");
			mValid = false;
			return;
		}

		[[view layer] addSublayer:mLayer];
#else
		B3D_LOG(Error, LogRenderBackend, "Metal render windows are not supported on this Apple platform variant.");
		mValid = false;
#endif
	}

	MetalRenderWindowSurface::~MetalRenderWindowSurface()
	{
		// Dtor mirrors @c Destroy() exactly — the Obj-C strongs are released and @c mValid flips
		// false. Delegating keeps the teardown in one place; calling @c Destroy() on an already-
		// destroyed surface is idempotent (setting nil strongs to nil and false to false).
		Destroy();
	}

	id<CAMetalDrawable> MetalRenderWindowSurface::AcquireDrawable()
	{
		@autoreleasepool
		{
			if (!mValid || mLayer == nil)
				return nil;

			// Drain any pending resize staged by RebuildSwapChain / MarkSwapChainAsInvalid. Writes to
			// @c mLayer.drawableSize must stay on the render thread — the property is documented
			// as thread-safe but concurrent writes are non-deterministic, and AppKit-driven resizes on
			// the main thread would otherwise race the render thread's drawable acquisition.
			if (mNeedsDrawableSizeReapply.exchange(false, std::memory_order_acquire))
			{
				u32 width;
				u32 height;
				bool vsync;
				{
					Lock lock(mPendingStateMutex);
					width = mPendingWidth;
					height = mPendingHeight;
					vsync = mPendingVSync;
				}

				mWidth = width;
				mHeight = height;
				mVSync = vsync;
				mLayer.drawableSize = CGSizeMake(mWidth, mHeight);
#if TARGET_OS_OSX
				mLayer.displaySyncEnabled = mVSync ? YES : NO;
#endif
			}

			if (mCurrentDrawable == nil)
				mCurrentDrawable = [mLayer nextDrawable];

			return mCurrentDrawable;
		}
	}

	void MetalRenderWindowSurface::AbortCurrentDrawable()
	{
		// Drop the drawable without presenting. Letting it release naturally simply frees the slot
		// back into the CAMetalLayer drawable pool without incrementing the presentation counter,
		// which is exactly what we want after an encoder-creation failure.
		mCurrentDrawable = nil;
		mDrawableWasRenderedInto = false;
	}

	void MetalRenderWindowSurface::MarkDrawableAsRendered()
	{
		// The cmd buffer calls this right after @c renderCommandEncoderWithDescriptor: succeeds.
		// The render pass's color attachment is configured with a store action against the drawable
		// texture, so a successful encoder open is sufficient evidence that the drawable will carry
		// defined content at submission time. Guard against the theoretical no-drawable case so a
		// stray call after a teardown path is a no-op instead of spuriously arming the flag.
		if (mCurrentDrawable != nil)
			mDrawableWasRenderedInto = true;
	}

	MTLPixelFormat MetalRenderWindowSurface::GetColorFormat() const
	{
		return mLayer ? mLayer.pixelFormat : MTLPixelFormatBGRA8Unorm;
	}

	void MetalRenderWindowSurface::SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask)
	{
		@autoreleasepool
		{
			if (!mValid || mCurrentDrawable == nil)
				return;

			if (!mDrawableWasRenderedInto)
			{
				// An acquired drawable that no render pass ever wrote to: presenting it would push
				// undefined content to the compositor. Drop it back to the pool instead. Logging is
				// unlatched on purpose — every occurrence indicates a bug in the frame's render-pass
				// composition (usually an early-return after @c AcquireDrawable without a matching
				// @c AbortCurrentDrawable).
				B3D_LOG(Warning, LogRenderBackend, "SwapBuffers skipped: drawable was acquired but no render pass wrote to it. Dropping the frame without present.");
				mCurrentDrawable = nil;
				return;
			}

			// We schedule the present through a short-lived command buffer acquired directly from the
			// queue; the engine's normal command-buffer submission flushes any rendering work that wrote to
			// the drawable before this command runs.
			auto& metalQueue = static_cast<MetalGpuQueue&>(queue);
			id<MTLCommandQueue> mtlQueue = metalQueue.GetMetalQueue();
			if (mtlQueue == nil)
			{
				mCurrentDrawable = nil;
				return;
			}

			id<MTLCommandBuffer> presentCB = [mtlQueue commandBuffer];

			// Honor cross-queue sync: the present must wait for any producer queues in the mask to reach
			// their latest signaled value, otherwise the drawable could be sampled or presented before the
			// render work targeting it has completed.
			const GpuQueueMask selfMask = queue.GetId();
			const GpuQueueMask waitMask = syncMask & ~selfMask;
			if (!waitMask.IsEmpty())
			{
				for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
				{
					const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;
					const u32 queueCount = mGpuDevice.GetQueueCount(queueType);
					for (u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
					{
						const GpuQueueId waitQueueId(queueType, queueIndex);
						if (!waitMask.IsSet(waitQueueId))
							continue;

						TShared<GpuQueue> queuePtr = mGpuDevice.GetQueue(queueType, queueIndex);
						auto waitQueue = std::static_pointer_cast<MetalGpuQueue>(queuePtr);
						if (!waitQueue)
							continue;

						id<MTLSharedEvent> waitEvent = waitQueue->GetSharedEvent();
						// Present waits on the producer queue's *scheduled* event value so the drawable
						// swap doesn't overtake any submission the graphics queue has already reserved.
						const u64 waitValue = waitQueue->GetLastScheduledEventValue();
						if (waitEvent == nil || waitValue == 0)
							continue;

						[presentCB encodeWaitForEvent:waitEvent value:waitValue];
					}
				}
			}

			// Compute the minimum display duration for a timed present. Only used when the engine has
			// requested an interval > 1 (half-rate, third-rate, etc.). We query the owning screen for
			// its refresh rate — NSScreen.maximumFramesPerSecond on macOS 12+, UIScreen on iOS. When
			// the query is unavailable or returns a non-positive value we fall back to 60Hz, which
			// matches the most common desktop/laptop display and keeps the divisor math sensible.
			const bool usePresentAfterMinimumDuration = mVSyncInterval > 1;
			CFTimeInterval minimumPresentDuration = 0.0;
			if (usePresentAfterMinimumDuration)
			{
				double refreshHz = 60.0;
#if TARGET_OS_OSX
				if (@available(macOS 12.0, *))
				{
					NSScreen* screen = [NSScreen mainScreen];
					if (screen != nil)
					{
						const NSInteger hz = [screen maximumFramesPerSecond];
						if (hz > 0)
							refreshHz = (double)hz;
					}
				}
#elif TARGET_OS_IPHONE
				UIScreen* screen = [UIScreen mainScreen];
				if (screen != nil)
				{
					const NSInteger hz = [screen maximumFramesPerSecond];
					if (hz > 0)
						refreshHz = (double)hz;
				}
#endif
				minimumPresentDuration = (CFTimeInterval)mVSyncInterval / refreshHz;
			}

#if TARGET_OS_OSX
			// Apple's documented contract for CAMetalLayer.presentsWithTransaction == YES (set in the
			// ctor) is: commit the cmd buffer, wait for it to reach scheduled, then call -present on the
			// drawable directly. Using -[MTLCommandBuffer presentDrawable:] under this flag is a no-op
			// for transaction serialization and trips Metal API validation. waitUntilScheduled blocks
			// only until the kernel driver accepts the submission (not GPU completion), which is
			// typically sub-ms and is what AppKit needs to pair our frame with the window-geometry
			// update in live resize.
			//
			// B18 (minimal): when @c waitMask was empty there is no cross-queue consumer waiting on
			// this present's signal, so skip the @c encodeSignalEvent + @c ReserveNextEventValue
			// pair entirely. The empty cmd buffer itself cannot be removed on this path —
			// @c waitUntilScheduled under @c presentsWithTransaction=YES requires *some* cmd buffer
			// to anchor the AppKit transaction handoff — but the signal plumbing is pure overhead
			// in that case. The full piggyback-present refactor (threading the drawable through
			// the render cmd buffer's own commit) is out of scope for this pass.
			if (!waitMask.IsEmpty())
			{
				const u64 signalValue = metalQueue.ReserveNextEventValue();
				id<MTLSharedEvent> signalEvent = metalQueue.GetSharedEvent();
				if (signalEvent != nil)
					[presentCB encodeSignalEvent:signalEvent value:signalValue];
			}

			[presentCB commit];
			[presentCB waitUntilScheduled];

			// Under presentsWithTransaction the drawable is presented directly, not via the cmd buffer.
			// For interval > 1 we ask Core Animation to hold the drawable on-screen for at least the
			// computed duration; this is the documented path for half-rate / third-rate output on
			// macOS fixed-refresh displays. Interval <= 1 keeps the zero-overhead plain -present.
			if (usePresentAfterMinimumDuration)
				[mCurrentDrawable presentAfterMinimumDuration:minimumPresentDuration];
			else
				[mCurrentDrawable present];
#else
			// iOS / tvOS async path: present rides on the command buffer. The timed variant lets us
			// keep the existing command-buffer-anchored signal path while honoring the interval.
			if (usePresentAfterMinimumDuration)
				[presentCB presentDrawable:mCurrentDrawable afterMinimumDuration:minimumPresentDuration];
			else
				[presentCB presentDrawable:mCurrentDrawable];

			// B18 (minimal): skip the signal-event plumbing when there are no cross-queue consumers
			// waiting on this submission. @c presentDrawable: already carries the Core Animation
			// completion guarantee the present itself needs; the shared event was only serving
			// cross-queue waiters that aren't in flight on this path.
			if (!waitMask.IsEmpty())
			{
				const u64 signalValue = metalQueue.ReserveNextEventValue();
				id<MTLSharedEvent> signalEvent = metalQueue.GetSharedEvent();
				if (signalEvent != nil)
					[presentCB encodeSignalEvent:signalEvent value:signalValue];
			}

			[presentCB commit];
#endif

			mCurrentDrawable = nil;
			mDrawableWasRenderedInto = false;
		}
	}

	void MetalRenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync)
	{
		// May be invoked from the main thread during AppKit live-resize or from the render thread
		// via the command buffer's swap-chain-invalid path. Either way, we only stage here — the
		// actual CAMetalLayer mutation happens in AcquireDrawable on the render thread so there is
		// exactly one writer to the layer's mutable state.
		{
			Lock lock(mPendingStateMutex);
			mPendingWidth = width;
			mPendingHeight = height;
			mPendingVSync = vsync;
		}
		mNeedsDrawableSizeReapply.store(true, std::memory_order_release);
	}

	void MetalRenderWindowSurface::MarkSwapChainAsInvalid()
	{
		// Dropping the drawable here is safe: the caller has guaranteed no render pass is currently
		// writing to it (the engine calls this on resize *after* the frame boundary). The pending-
		// resize flag is toggled so the next AcquireDrawable re-applies the staged width/height to
		// the layer — after a move between displays the layer's drawableSize may have drifted.
		// Pending values are not touched: they are either from a prior @c RebuildSwapChain (the
		// intended new size) or seeded from the ctor (the still-current size).
		mCurrentDrawable = nil;
		mDrawableWasRenderedInto = false;
		mNeedsDrawableSizeReapply.store(true, std::memory_order_release);
	}

	TAsyncOp<TShared<PixelData>> MetalRenderWindowSurface::ReadAsync(GpuCommandBuffer& commandBuffer)
	{
		@autoreleasepool
		{
			TAsyncOp<TShared<PixelData>> op;

			if (!mValid || mLayer == nil || mCurrentDrawable == nil)
			{
				// No drawable to read; signal "no frame produced" by completing with nullptr. Callers
				// (snapshot tests, tonemap capture path) already treat nullptr as the no-frame code
				// path, so this is a cleaner contract than inventing a zero-filled buffer at stale dims.
				op.CompleteOperation(nullptr);
				return op;
			}

			if (mLayer.framebufferOnly == YES)
			{
				// Defense in depth: the ctor sets framebufferOnly=NO so the drawable texture can be
				// used as a blit source, but if a future change ever flips this to YES the blit
				// below silently produces undefined bytes on Apple Silicon and fails Metal API
				// validation. Surface the error unambiguously instead.
				B3D_LOG(Error, LogRenderBackend, "ReadAsync requires framebufferOnly=NO on the CAMetalLayer; drawable is not blit-sampleable.");
				op.CompleteOperation(nullptr);
				return op;
			}

			id<MTLTexture> drawableTexture = mCurrentDrawable.texture;
			if (drawableTexture == nil)
			{
				// No drawable texture available; signal "no frame produced" with nullptr.
				op.CompleteOperation(nullptr);
				return op;
			}

			auto& metalCB = static_cast<MetalGpuCommandBuffer&>(commandBuffer);
			id<MTLBlitCommandEncoder> blit = metalCB.GetOrOpenBlitEncoder();
			if (blit == nil)
			{
				// Blit encoder could not be opened; signal "no frame produced" with nullptr.
				op.CompleteOperation(nullptr);
				return op;
			}

			// Derive width/height from the drawable's texture rather than the engine-visible mWidth/
			// mHeight fields. mWidth/mHeight are render-thread-owned and written in AcquireDrawable;
			// reading them from ReadAsync (which the engine permits on non-render fiber contexts) would
			// race those writes. The drawable's texture dimensions are the authoritative size the blit
			// must use regardless, so this is race-free and avoids the need for an atomic latch.
			const u32 width = (u32)drawableTexture.width;
			const u32 height = (u32)drawableTexture.height;

			// Pixel format used for the result. The drawable is always BGRA8 (sRGB variant is the same
			// byte layout), so we read back into PF_BGRA8 and let the caller reinterpret the colorspace
			// if needed — matching the Vulkan backend's behavior when reading an sRGB swapchain image.
			TShared<PixelData> pixelData = PixelData::Create(width, height, 1, PF_BGRA8);

			const u32 bytesPerRow = MetalUtility::GetTextureRowPitch(PF_BGRA8, width);
			const NSUInteger bufferSize = (NSUInteger)bytesPerRow * height;

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			id<MTLBuffer> stagingBuffer = [device newBufferWithLength:bufferSize options:MTLResourceStorageModeShared];
			if (stagingBuffer == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "ReadAsync: failed to allocate {0}-byte staging buffer.", (u64)bufferSize);
				op.CompleteOperation(nullptr);
				return op;
			}

			[blit copyFromTexture:drawableTexture
				sourceSlice:0
				sourceLevel:0
				sourceOrigin:MTLOriginMake(0, 0, 0)
				sourceSize:MTLSizeMake(width, height, 1)
				toBuffer:stagingBuffer
				destinationOffset:0
				destinationBytesPerRow:bytesPerRow
				destinationBytesPerImage:bufferSize];

			// Hook completion through the engine-level cmd buffer signal rather than Metal's raw
			// @c addCompletedHandler:. @c OnDidComplete is posted from Metal's internal completion
			// queue back to the pool-owner engine thread (see MetalGpuCommandBuffer::CommitInternal),
			// so the memcpy runs on the same thread the ReadAsync caller expects — identical to how
			// the Vulkan backend consumes this signal in B3DIVulkanRenderWindowSurface.cpp. The
			// shared-storage staging buffer is CPU-coherent the moment the GPU blit retires, which
			// is strictly before OnDidComplete fires, so no explicit synchronize step is required.
			const u32 resultRowPitch = pixelData->GetRowPitch();
			const u32 resultHeight = height;

			auto fnOnCommandBufferCompleted = [stagingBuffer, op, pixelData, bytesPerRow, resultRowPitch, resultHeight]() mutable
			{
				const u8* src = (const u8*)[stagingBuffer contents];
				u8* dst = pixelData->GetData();
				if (src != nullptr && dst != nullptr)
				{
					if (resultRowPitch == bytesPerRow)
						memcpy(dst, src, (size_t)bytesPerRow * resultHeight);
					else
					{
						for (u32 row = 0; row < resultHeight; row++)
							memcpy(dst + row * resultRowPitch, src + row * bytesPerRow, bytesPerRow);
					}
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
	}

	void MetalRenderWindowSurface::Destroy()
	{
		mCurrentDrawable = nil;
		mDrawableWasRenderedInto = false;
		mLayer = nil;
		mValid = false;
	}
} // namespace b3d::render
