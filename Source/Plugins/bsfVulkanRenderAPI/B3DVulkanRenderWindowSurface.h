//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DIVulkanRenderWindowSurface.h"
#include "B3DVulkanGpuQueue.h"

namespace b3d::render
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	class VulkanSurface;

	/** Vulkan render window surface implementation that manages a VkSwapChain. */
	class VulkanRenderWindowSurface : public IVulkanRenderWindowSurface
	{
	public:
		VulkanRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation);
		~VulkanRenderWindowSurface();

		// IRenderWindowSurface
		void RebuildSwapChain(u32 width, u32 height, bool vsync) override;
		void MarkSwapChainAsInvalid() override;
		void SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask) override;
		void Destroy() override;

		// IVulkanRenderWindowSurface
		VulkanFramebuffer* GetActiveFramebuffer(bool acquireIfUnavailable = true) override;
		bool AppendWaitSemaphoresIfRequired(TInlineArray<VulkanSemaphore*, 8>& outSemaphores) override;
		bool IsSwapChainValid() const override;
		VulkanSwapChain* GetSwapChain() const override { return mSwapChain; }

	private:
		SPtr<VulkanSurface> mSurface;
		VkColorSpaceKHR mColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		VkFormat mColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;
		bool mCreateDepthBuffer = false;
		u32 mPresentQueueFamily = 0;
		VulkanSwapChain* mSwapChain = nullptr;
		u64 mPlatformWindowHandle = 0;
		bool mIsDestroyed = false;
		u32 mActiveImageIndex = 0;
	};

	/** @} */
} // namespace b3d::render
