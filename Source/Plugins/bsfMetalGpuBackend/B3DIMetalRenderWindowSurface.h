//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DRenderWindow.h"

namespace b3d::render
{
	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Metal-specific interface for render window surfaces. Used as a common interface for regular Metal surfaces backed
	 * by a CAMetalLayer attached to an OS window, and faux surfaces for headless (offscreen) rendering scenarios.
	 *
	 * All methods are declared unconditionally (using the Obj-C handle aliases from B3DMetalPrerequisites.h) so the
	 * vtable layout is identical in Objective-C++ and plain C++ translation units.
	 */
	class IMetalRenderWindowSurface : public IRenderWindowSurface
	{
	public:
		/**
		 * Acquires (or returns the already-acquired) color texture for the current back buffer. For windowed surfaces
		 * this pulls the next CAMetalDrawable from the layer on first call each frame; for headless surfaces it returns
		 * the current offscreen image. Returns null if no back buffer is available (invalid or destroyed surface, or
		 * drawable pool exhaustion). Must be called on the render thread, once per render pass that targets the window.
		 */
		virtual MTLTextureRef AcquireColorTexture() = 0;

		/**
		 * Returns the color texture of the current back buffer without acquiring a new one, or null if none is
		 * currently held. Used for reading back the rendered frame.
		 */
		virtual MTLTextureRef GetCurrentColorTexture() const = 0;

		/**
		 * Returns the depth/stencil texture associated with the surface, or null if the surface was created without
		 * a depth buffer.
		 */
		virtual MTLTextureRef GetDepthStencilTexture() const = 0;

		/** Returns the native pixel format (MTLPixelFormat) of the color surface. Used for pipeline state keying. */
		virtual MTLPixelFormatValue GetColorFormat() const = 0;

		/** Returns the engine pixel format of the color surface. Used for reading back the rendered frame. */
		virtual PixelFormat GetColorPixelFormat() const = 0;

		/**
		 * Returns true if the underlying swap chain (drawable source or offscreen image set) is valid and can be used
		 * for acquiring back buffers. If it's not valid you should rebuild the swap chain from the owning RenderWindow,
		 * as most likely the window size changed.
		 */
		virtual bool IsSwapChainValid() const = 0;

		/**
		 * Releases the currently-acquired back buffer without presenting it. Called when the render pass that acquired
		 * it failed to open its encoder. No-op for headless surfaces.
		 */
		virtual void AbortCurrentDrawable() {}

		/**
		 * Records that the currently-acquired back buffer has had a render encoder open successfully against it, so
		 * SwapBuffers is permitted to present it. No-op for headless surfaces.
		 */
		virtual void MarkDrawableAsRendered() {}

		/**
		 * Shared readback implementation: blits the current color texture into a CPU-visible staging buffer via the
		 * provided command buffer, and completes when the command buffer finishes executing.
		 */
		TAsyncOp<TShared<PixelData>> ReadAsync(GpuCommandBuffer& commandBuffer) override;
	};

	/** @} */
} // namespace b3d::render
