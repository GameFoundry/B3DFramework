//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DIVulkanRenderWindowSurface.h"

namespace b3d::render
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**
	 * Vulkan render window surface implementation for headless rendering.
	 * Creates GPU textures and framebuffers that mimic a swap chain, cycling between them on present.
	 * Used for headless rendering in automated testing or offscreen rendering scenarios.
	 */
	class VulkanHeadlessRenderWindowSurface : public IVulkanRenderWindowSurface
	{
	public:
		/** Number of images in the headless swap chain (triple buffering). */
		static constexpr u32 kImageCount = 3;

		VulkanHeadlessRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation);
		~VulkanHeadlessRenderWindowSurface() override;

		// IRenderWindowSurface
		void RebuildSwapChain(u32 width, u32 height, bool vsync) override;
		void MarkSwapChainAsInvalid() override;
		void Destroy() override;
		void SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask) override;

		// IVulkanRenderWindowSurface
		VulkanFramebuffer* GetActiveFramebuffer(bool acquireIfUnavailable = true) override;
		bool AppendWaitSemaphoresIfRequired(TInlineArray<VulkanSemaphore*, 8>& outSemaphores) override { return false; }
		bool IsSwapChainValid() const override { return mIsValid && mFramebuffers[mCurrentImageIndex] != nullptr; }
		VulkanImage* GetCurrentColorImage() const override { return mColorImages[mCurrentImageIndex]; }
		PixelFormat GetColorPixelFormat() const override { return PF_RGBA8; }

	private:
		/** Creates all the necessary swap chain images and framebuffers. */
		void CreateSwapChainImages();
		void DestroySwapChainImages();

		u32 mWidth = 0;
		u32 mHeight = 0;
		bool mVSync = false;
		bool mCreateDepthBuffer = false;
		bool mUseHardwareSRGB = false;
		bool mIsSwapQueued = false;
		bool mIsValid = true;
		bool mIsDestroyed = false;
		u32 mCurrentImageIndex = 0;

		VulkanImage* mColorImages[kImageCount] = {};
		VulkanImage* mDepthImage = nullptr;
		VulkanFramebuffer* mFramebuffers[kImageCount] = {};
	};

	/** @} */
} // namespace b3d::render
