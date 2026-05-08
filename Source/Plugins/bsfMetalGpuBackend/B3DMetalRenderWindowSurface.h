//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "Threading/B3DThreading.h"
#include <atomic>

namespace b3d::render
{
	class MetalGpuDevice;

	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Metal implementation of IRenderWindowSurface.
	 *
	 * Attaches a @c CAMetalLayer to the native platform view (NSView on macOS, UIView on iOS). The
	 * layer owns the drawable queue; @c AcquireDrawable() pulls the next drawable to render into,
	 * and @c SwapBuffers() commits it for presentation.
	 */
	class MetalRenderWindowSurface final : public IRenderWindowSurface
	{
	public:
		MetalRenderWindowSurface(MetalGpuDevice& device, const RenderWindowSurfaceCreateInformation& createInformation);
		~MetalRenderWindowSurface() override;

		void SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask) override;
		void RebuildSwapChain(u32 width, u32 height, bool vsync) override;
		void MarkSwapChainAsInvalid() override;
		TAsyncOp<TShared<PixelData>> ReadAsync(GpuCommandBuffer& commandBuffer) override;
		void Destroy() override;

#ifdef __OBJC__
		/**
		 * Acquires the next drawable from the swap chain. Must be called once per frame before
		 * opening a render encoder that writes to this surface.
		 */
		id<CAMetalDrawable> AcquireDrawable();

		/** Returns the pixel format of the drawable texture (derived from the CAMetalLayer). */
		MTLPixelFormat GetColorFormat() const;

		/**
		 * Releases the currently-acquired drawable without presenting it. Called when the render pass
		 * that acquired it failed to open its encoder — without this, the drawable would remain held
		 * until the next successful @c SwapBuffers, stalling the drawable pool for one or more frames.
		 */
		void AbortCurrentDrawable();

		/**
		 * Records that the currently-acquired drawable has had a render encoder open successfully
		 * against it, so @c SwapBuffers is permitted to present it. Called by the command buffer
		 * after @c renderCommandEncoderWithDescriptor: returns non-nil — the render pass's color
		 * attachment is configured to store to the drawable texture, so a valid encoder guarantees
		 * the drawable will carry rendered content at submission time. If no encoder ever opens
		 * against a drawable (e.g. all render passes aborted mid-frame), @c SwapBuffers detects the
		 * unset flag and drops the drawable without presenting, avoiding a frame of undefined
		 * content on screen.
		 */
		void MarkDrawableAsRendered();
#endif

	private:
		MetalGpuDevice& mGpuDevice;

#ifdef __OBJC__
		// Obj-C strong members held directly on the class (ARC). Previously hidden behind a pimpl
		// to keep Obj-C types out of the header, but the header already gates its Obj-C surface on
		// @c __OBJC__ so the pimpl added an allocation + pointer-chase with no encapsulation win.
		CAMetalLayer* mLayer = nil;
		id<CAMetalDrawable> mCurrentDrawable = nil;
		// Tracks whether the currently-held drawable has been rendered into by at least one render
		// pass this frame. Flipped true by @c MarkDrawableAsRendered (called from the cmd buffer
		// after a successful encoder open) and reset whenever the drawable slot turns over
		// (@c AbortCurrentDrawable, @c SwapBuffers tail, @c Destroy, @c MarkSwapChainAsInvalid).
		// Prevents presenting a drawable whose contents are undefined because no render pass ever
		// wrote to it — the previous behavior silently pushed garbage to the compositor.
		bool mDrawableWasRenderedInto = false;
#endif

		// These four fields are render-thread-only after construction. @c RebuildSwapChain /
		// @c MarkSwapChainAsInvalid (which may fire from the main thread during live-resize) stage
		// their values into the @c mPending* fields below and set @c mNeedsDrawableSizeReapply; the
		// render thread drains them in @c AcquireDrawable.
		u32 mWidth = 0;
		u32 mHeight = 0;
		bool mVSync = false;
		// Present rate divisor. 1 means every refresh (native rate), 2 means half rate, etc. Used by
		// @c SwapBuffers to pick between plain @c presentDrawable: (interval <= 1) and the timed
		// @c presentDrawable:afterMinimumDuration: / @c presentAfterMinimumDuration: variant
		// (interval > 1) so the engine can run at a fractional display rate without burning CPU.
		u32 mVSyncInterval = 1;
		bool mHwGamma = false;
		bool mValid = true;

		// Set by MarkSwapChainAsInvalid / RebuildSwapChain; AcquireDrawable re-applies drawableSize
		// before requesting the next drawable so the layer resyncs with the engine's expected size
		// after a displayed-surface invalidation (window moved between displays, scale factor change,
		// etc.). Atomic because writes come from any thread, reads/clear happen on the render thread.
		std::atomic<bool> mNeedsDrawableSizeReapply{ false };

		// Guards the pending resize payload. Held only long enough to copy a small POD struct, so
		// contention between the (infrequent) resize writer and the render thread is negligible.
		mutable Mutex mPendingStateMutex;
		u32 mPendingWidth = 0;
		u32 mPendingHeight = 0;
		bool mPendingVSync = false;
	};

	/** @} */
} // namespace b3d::render
