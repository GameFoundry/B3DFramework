//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "RenderAPI/B3DRenderWindow.h"

namespace b3d::render
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	class VulkanSurface;

	/** Interface that acts a bridge between Win32RenderWindow and VulkanSurface/VulkanSwapChain. */
	class VulkanRenderWindowSurface : public IRenderWindowSurface
	{
	public:
		VulkanRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation);
		~VulkanRenderWindowSurface();

		void RebuildSwapChain(u32 width, u32 height, bool vsync) override;
		void MarkSwapChainAsInvalid() override;
		void Destroy() override;

		/** Returns the swap chain owned by the surface. */
		VulkanSwapChain* GetSwapChain() const { return mSwapChain; }

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
	};

	/** @} */
} // namespace b3d::render
