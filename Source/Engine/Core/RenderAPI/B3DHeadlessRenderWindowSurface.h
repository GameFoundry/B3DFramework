//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DRenderWindow.h"
#include "RenderAPI/B3DRenderTexture.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Backend-agnostic render window surface implementation for headless mode.
	 * Creates GPU textures that mimic a swap chain, cycling between them on present.
	 * Used for headless rendering in automated testing or offscreen rendering scenarios.
	 */
	class B3D_EXPORT HeadlessRenderWindowSurface : public IRenderWindowSurface
	{
	public:
		/** Number of images in the headless swap chain (triple buffering). */
		static constexpr u32 kImageCount = 3;

		HeadlessRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation);
		~HeadlessRenderWindowSurface();

		void RebuildSwapChain(u32 width, u32 height, bool vsync) override;
		void MarkSwapChainAsInvalid() override;
		void Destroy() override;

		/** Returns the index of the current image in the swap chain. */
		u32 GetCurrentImageIndex() const { return mCurrentImageIndex; }

		/** Returns the render texture for the specified image index. */
		const SPtr<RenderTexture>& GetImage(u32 index) const { return mImages[index]; }

		/** Returns the render texture for the current image. */
		const SPtr<RenderTexture>& GetCurrentImage() const { return mImages[mCurrentImageIndex]; }

		/** Cycles to the next image in the swap chain (called during present). */
		void Present();

		/** Acquires the next image for rendering (makes current image available). */
		void AcquireNextImage();

		/** Returns true if the swap chain is marked as invalid and needs rebuilding. */
		bool IsValid() const { return mIsValid; }

		/** Returns the width of the swap chain images. */
		u32 GetWidth() const { return mWidth; }

		/** Returns the height of the swap chain images. */
		u32 GetHeight() const { return mHeight; }

	private:
		void CreateImages();
		void DestroyImages();

	private:
		u32 mWidth = 0;
		u32 mHeight = 0;
		bool mVSync = false;
		bool mCreateDepthBuffer = false;
		bool mUseHardwareSRGB = false;
		bool mIsValid = true;
		bool mIsDestroyed = false;

		SPtr<RenderTexture> mImages[kImageCount];
		u32 mCurrentImageIndex = 0;
	};

	/** @} */
} // namespace b3d::render
