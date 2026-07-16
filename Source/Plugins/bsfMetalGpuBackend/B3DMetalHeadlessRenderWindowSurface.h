//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DIMetalRenderWindowSurface.h"

namespace b3d::render
{
	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Metal render window surface implementation for headless rendering.
	 * Creates GPU textures that mimic a swap chain, cycling between them on present.
	 * Used for headless rendering in automated testing or offscreen rendering scenarios.
	 *
	 * Unlike the windowed surface there is no CAMetalLayer, no drawable and no present operation — SwapBuffers only
	 * advances the current image index. AbortCurrentDrawable/MarkDrawableAsRendered inherit their no-op defaults.
	 */
	class MetalHeadlessRenderWindowSurface final : public IMetalRenderWindowSurface
	{
	public:
		/** Number of images in the headless swap chain (triple buffering). */
		static constexpr u32 kImageCount = 3;

		MetalHeadlessRenderWindowSurface(MetalGpuDevice& device, const RenderWindowSurfaceCreateInformation& createInformation);
		~MetalHeadlessRenderWindowSurface() override;

		// IRenderWindowSurface
		void RebuildSwapChain(u32 width, u32 height, bool vsync, u32 vsyncInterval) override;
		void MarkSwapChainAsInvalid() override;
		void SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask) override;
		void Destroy() override;

		// IMetalRenderWindowSurface
		MTLTextureRef AcquireColorTexture() override;
		MTLTextureRef GetCurrentColorTexture() const override { return mColorTextures[mCurrentImageIndex]; }
		MTLTextureRef GetDepthStencilTexture() const override { return mDepthStencilTexture; }
		MTLPixelFormatValue GetColorFormat() const override;
		PixelFormat GetColorPixelFormat() const override { return PF_BGRA8; }
		bool IsSwapChainValid() const override { return mIsValid && mColorTextures[mCurrentImageIndex] != nullptr; }

	private:
		/** Creates all the necessary swap chain images at the current size. Render thread only. */
		void CreateSwapChainImages();

		/** Releases all swap chain images. Render thread only. */
		void DestroySwapChainImages();

		MetalGpuDevice& mGpuDevice;

		u32 mWidth = 0;
		u32 mHeight = 0;
		bool mVSync = false;
		bool mCreateDepthBuffer = false;
		bool mUseHardwareSRGB = false;
		bool mIsValid = true;
		bool mIsDestroyed = false;
		bool mIsSwapQueued = false;
		u32 mCurrentImageIndex = 0;

		// Obj-C strong (manually retained) members, +1 owned from newTextureWithDescriptor:. Declared through the
		// unconditional handle aliases so the class layout is identical in .cpp and .mm translation units.
		MTLTextureRef mColorTextures[kImageCount] = {};
		MTLTextureRef mDepthStencilTexture = nullptr;
	};

	/** @} */
} // namespace b3d::render
