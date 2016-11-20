//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanFramebuffer.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Description of a single swap chain surface. */
	struct SwapChainSurface
	{
		VkImage image;
		VkImageView view;
		VkSemaphore sync;
		bool acquired;

		VulkanFramebuffer* framebuffer;
		VULKAN_FRAMEBUFFER_DESC framebufferDesc;
	};

	/** Vulkan swap chain containing two or more buffers for rendering and presenting onto the screen. */
	class VulkanSwapChain : INonCopyable
	{
	public:
		~VulkanSwapChain();

		/** 
		 * Rebuilds the swap chain with the provided properties. Destroys any previously existing swap chain. Caller must
		 * ensure the swap chain is not used at the device when this is called.
		 */
		void rebuild(const SPtr<VulkanDevice>& device, VkSurfaceKHR surface, UINT32 width, UINT32 height, bool vsync, 
			VkFormat colorFormat, VkColorSpaceKHR colorSpace, bool createDepth, VkFormat depthFormat);

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
		 * @param[in]	queue			Queue on which to queue the present operation. Must support present operations.
		 * @param[in]	semaphores		Optional semaphores to wait on before presenting the queue.
		 * @param[in]	numSemaphores	Number of semaphores in the @p semaphores array.
		 */
		void present(VkQueue queue, VkSemaphore* semaphores, UINT32 numSemaphores);

		/**
		 * Acquires a new back buffer image. Caller can retrieve the surface by calling getBackBuffer(). Caller must wait
		 * on the semaphore provided by the surface before rendering to it.
		 * 
		 * @note Must only be called once in-between present() calls, or before the first present() call.
		 */
		void acquireBackBuffer();

		/** Returns information describing the current back buffer. */
		const SwapChainSurface& getBackBuffer() { return mSurfaces[mCurrentBackBufferIdx]; }

		/** Returns the number of available color surfaces. */
		UINT32 getNumColorSurfaces() const { return (UINT32)mSurfaces.size(); }

		/** Returns an image view representing the color surface at the specified index. */
		VkImageView getColorView(UINT32 index) const { return mSurfaces[index].view; }

		/** Returns an image view representing the depth-stencil buffer, if any. */
		VkImageView getDepthStencilView() const { return mDepthStencilView; }
	private:
		/** Destroys current swap chain and depth stencil image (if any). */
		void clear(VkSwapchainKHR swapChain);

		SPtr<VulkanDevice> mDevice;
		VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;

		UINT32 mWidth = 0;
		UINT32 mHeight = 0;
		Vector<SwapChainSurface> mSurfaces;

		VkImage mDepthStencilImage = VK_NULL_HANDLE;
		VkImageView mDepthStencilView = VK_NULL_HANDLE;
		VkDeviceMemory mDepthStencilMemory = nullptr;

		UINT32 mCurrentSemaphoreIdx = 0;
		UINT32 mCurrentBackBufferIdx = 0;
	};

	/** @} */
}
