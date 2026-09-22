//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DIMetalRenderWindowSurface.h"
#include "B3DMetalResource.h"
#include "GpuBackend/B3DGpuSwapChain.h"
#include "Threading/B3DSingleConsumerQueue.h"
#include "Threading/B3DThreading.h"
#include <atomic>

namespace b3d::render
{
	class MetalRenderWindowSurface;

	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Metal swap chain bridging a CAMetalLayer drawable between the render and submit threads.
	 *
	 * Unlike the Vulkan and D3D12 swap chains, images are not acquired through GpuSubmitThread::QueueImageAcquire():
	 * a CAMetalLayer drawable is an opaque object rather than an index into a fixed image set, and its texture must
	 * be placed in the render pass descriptor when the encoder is created, which happens on the render thread. The
	 * drawable is therefore acquired on the render thread at BeginRenderPass, i.e. at command recording time, and
	 * is held from then until the submit thread presents it. AcquireImage() and NotifyWasImageAcquireQueued() are
	 * consequently never called on this backend and assert if they are.
	 *
	 * The drawable is acquired through AcquireDrawable(), and once a render pass has written to it
	 * SwapBuffers() hands it over for presentation on the submit thread. Only one drawable is held at a time.
	 */
	class MetalSwapChain final : public TMetalResource<GpuSwapChain>
	{
		using Super = TMetalResource<GpuSwapChain>;

		/** Drawable that has been queued for present, together with the presentation settings captured at that time. */
		struct PendingDrawable
		{
			u32 Index = 0;
			CAMetalDrawableRef Drawable = nullptr;
			bool VSync = false;
			u32 VSyncInterval = 1;
			float RefreshRate = 60.0f;
		};

	public:
		/**
		 * Creates a swap chain for the provided surface. The surface must outlive the swap chain, as its presentation
		 * settings are read whenever a drawable is queued for present.
		 */
		MetalSwapChain(MetalResourceManager* owner, MetalRenderWindowSurface& surface);
		~MetalSwapChain() override;

		SingleConsumerQueue& GetMessageQueue() override { return mMessageQueue; }
		void AcquireImage() override;
		void Present(u32 imageIndex, GpuQueue& queue, GpuQueueMask syncMask) override;
		bool TryGetFirstAcquiredImageIndex(u32& outImageIndex) const override;
		void NotifyWasImageAcquireQueued() override;
		void NotifyWasPresentQueued(u32 imageIndex) override;
		bool IsRetired() const override { return mIsRetired.load(std::memory_order_acquire); }

		/**
		 * Acquires the next drawable from @p layer and returns its color texture, or returns the texture of the
		 * already-held drawable if one was acquired earlier this frame. Returns null if the swap chain is retired or
		 * the layer has no drawable available.
		 *
		 * @note	Render thread only.
		 */
		MTLTextureRef AcquireDrawable(CAMetalLayerRef layer);

		/** Returns the color texture of the currently held drawable, or null if no drawable is held. */
		MTLTextureRef GetCurrentTexture() const;

		/**
		 * Releases the currently held drawable without presenting it, if any. Used when the render pass that acquired
		 * it failed to open its encoder, when the surface is invalidated, or when SwapBuffers() finds nothing was
		 * rendered into the drawable.
		 */
		void AbortCurrentDrawable();

		/**
		 * Records that a render encoder was successfully opened against the currently held drawable. Only such
		 * drawables are reported by TryGetFirstAcquiredImageIndex() and therefore eligible for present.
		 */
		void MarkDrawableAsRendered();

		/**
		 * Marks the swap chain as retired. Retired swap chain can still present drawables that were already queued,
		 * but no new drawables can be acquired. Safe to call from any thread.
		 */
		void Retire();

	private:
#ifdef __OBJC__
		/** Releases the provided drawable reference and resets it to nil. No-op if the reference is already nil. */
		// __strong: an unqualified id& parameter defaults to __autoreleasing under ARC and cannot bind to strong lvalues
		void ReleaseDrawable(CAMetalDrawableRef __strong& drawable);
#endif

		MetalRenderWindowSurface& mSurface;
		mutable Mutex mMutex;
		SingleConsumerQueue mMessageQueue;
		Vector<PendingDrawable> mPendingDrawables;
		CAMetalDrawableRef mCurrentDrawable = nullptr;
		u32 mCurrentDrawableIndex = 0;
		u32 mNextDrawableIndex = 1;
		bool mCurrentDrawableWasRenderedInto = false;
		std::atomic<bool> mIsRetired{ false };
	};

	/**
	 * Metal implementation of IMetalRenderWindowSurface for surfaces backed by an OS window.
	 * Resolves the @c CAMetalLayer attached to the engine's Cocoa window.
	 */
	class MetalRenderWindowSurface final : public IMetalRenderWindowSurface
	{
	public:
		MetalRenderWindowSurface(MetalGpuDevice& device, const RenderWindowSurfaceCreateInformation& createInformation);
		~MetalRenderWindowSurface() override;

		// IRenderWindowSurface
		void SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask) override;
		void RebuildSwapChain(u32 width, u32 height, bool vsync, u32 vsyncInterval) override;
		void MarkSwapChainAsInvalid() override;
		void Destroy() override;

		// IMetalRenderWindowSurface
		MTLTextureRef AcquireColorTexture() override;
		MTLTextureRef GetCurrentColorTexture() const override;
		MTLTextureRef GetDepthStencilTexture() const override { return mDepthStencilTexture; }
		MTLPixelFormatValue GetColorFormat() const override;
		PixelFormat GetColorPixelFormat() const override { return PF_BGRA8; }
		bool IsSwapChainValid() const override
		{
			return mValid && mLayer != nullptr && !mNeedsDrawableSizeReapply.load(std::memory_order_acquire);
		}
		void AbortCurrentDrawable() override;
		void MarkDrawableAsRendered() override;

	private:
		friend class MetalSwapChain;

		/**
		 * Drains any pending resize/vsync change staged by RebuildSwapChain / MarkSwapChainAsInvalid, re-applying
		 * the layer's drawableSize and recreating the depth buffer if the size changed. Render thread only.
		 */
		void ApplyPendingStateIfNeeded();

		/** (Re)creates the depth/stencil texture at the current surface size, if one was requested. Render thread only. */
		void RecreateDepthStencilTextureIfNeeded();

		/** Releases the currently-held drawable (if any) and resets the rendered-into flag. */
		void ReleaseCurrentDrawable();

		MetalGpuDevice& mGpuDevice;

		CAMetalLayerRef mLayer = nullptr;
		MTLTextureRef mDepthStencilTexture = nullptr;
		MetalSwapChain* mSwapChain = nullptr;

		// These fields are render-thread-only after construction. @c RebuildSwapChain / @c MarkSwapChainAsInvalid
		// (which may fire from the main thread during live-resize) stage their values into the @c mPending* fields
		// below and set @c mNeedsDrawableSizeReapply; the render thread drains them in @c AcquireColorTexture.
		u32 mWidth = 0;
		u32 mHeight = 0;
		bool mVSync = false;
		// Present rate divisor. 1 means every refresh, 2 means half rate, etc.
		u32 mVSyncInterval = 1;
		float mRefreshRate = 60.0f;
		bool mHwGamma = false;
		bool mCreateDepthBuffer = false;
		bool mValid = true;

		// Set by MarkSwapChainAsInvalid / RebuildSwapChain; AcquireColorTexture re-applies drawableSize before
		// requesting the next drawable so the layer resyncs with the engine's expected size after a
		// displayed-surface invalidation. Atomic because writes come from any thread, reads/clear happen on the
		// render thread.
		std::atomic<bool> mNeedsDrawableSizeReapply{ false };

		// Guards the pending resize payload. Held only long enough to copy a small POD struct, so contention
		// between the (infrequent) resize writer and the render thread is negligible.
		mutable Mutex mPendingStateMutex;
		u32 mPendingWidth = 0;
		u32 mPendingHeight = 0;
		bool mPendingVSync = false;
		u32 mPendingVSyncInterval = 1;
	};

	/** @} */
} // namespace b3d::render
