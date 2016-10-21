//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsEventQuery.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Vulkan swap chain containing two or more buffers for rendering and presenting onto the screen. */
	class VulkanSwapChain
	{
	public:
		~VulkanSwapChain();

		/** Rebuilds the swap chain with the provided properties. Destroys any previously existing swap chain. */
		void rebuild(const SPtr<VulkanDevice>& device, VkSurfaceKHR surface, UINT32 width, UINT32 height, bool vsync, 
			VkFormat colorFormat, VkColorSpaceKHR colorSpace);

		// TODO - swapBuffers(), getBackBuffer()

		/**
		 * Returns the actual width of the swap chain, in pixels. This might differ from the requested size in case it
		 * wasn't supported.
		 */
		UINT32 getWidth() const { return mWidth; }

		/** 
		 * Returns the actual height of the swap chain, in pixels. This might differ from the requested size in case it
		 * wasn't supported.
		 */
		UINT32 getHeight() const { return mHeight; }
	private:
		/** Description of a single swap chain surface. */
		struct SwapChainSurface
		{
			VkImage image;
			VkImageView view;
		};

		SPtr<VulkanDevice> mDevice;
		VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;

		UINT32 mWidth = 0;
		UINT32 mHeight = 0;
		Vector<SwapChainSurface> mSurfaces;
	};

	/** @} */
}