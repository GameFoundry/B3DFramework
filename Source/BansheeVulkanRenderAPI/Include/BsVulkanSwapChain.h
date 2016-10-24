//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

#define BS_NUM_BACK_BUFFERS 1

	/** Description of a single swap chain surface. */
	struct SwapChainSurface
	{
		VkImage image;
		VkImageView view;
		VkSemaphore sync;
		bool acquired;
	};

	/** Vulkan swap chain containing two or more buffers for rendering and presenting onto the screen. */
	class VulkanSwapChain : INonCopyable
	{
	public:
		~VulkanSwapChain();

		/** Rebuilds the swap chain with the provided properties. Destroys any previously existing swap chain. */
		void rebuild(const SPtr<VulkanDevice>& device, VkSurfaceKHR surface, UINT32 width, UINT32 height, bool vsync, 
			VkFormat colorFormat, VkColorSpaceKHR colorSpace);

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

		/** 
		 * Presents the back buffer to the output device, swapping the buffers. 
		 *
		 * @param[in]	queue		Queue on which to queue the present operation. Must support present operations.
		 * @param[in]	semaphore	Optional semaphore to wait on before presenting the queue.
		 */
		void present(VkQueue queue, VkSemaphore semaphore);

		/**
		 * Returns the current back buffer image. 
		 * 
		 * @note Must only be called once in-between present() calls, or before the first present() call.
		 */
		SwapChainSurface acquireBackBuffer();
	private:
		SPtr<VulkanDevice> mDevice;
		VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;

		UINT32 mWidth = 0;
		UINT32 mHeight = 0;
		Vector<SwapChainSurface> mSurfaces;
		UINT32 mCurrentSemaphoreIdx = 0;
		UINT32 mCurrentBackBufferIdx = 0;
	};

	/** @} */
}