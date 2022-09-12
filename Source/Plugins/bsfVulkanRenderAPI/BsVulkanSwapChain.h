//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanFramebuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Description of a single swap chain surface. */
	struct SwapChainSurface
	{
		VulkanImage* image;
		VulkanSemaphore* sync;
		bool acquired;
		bool needsWait;

		VulkanFramebuffer* framebuffer;
		VULKAN_FRAMEBUFFER_DESC framebufferDesc;
	};

	/** Vulkan swap chain containing two or more buffers for rendering and presenting onto the screen. */
	class VulkanSwapChain : public VulkanResource, INonCopyable
	{
	public:
		/**
		 * Creates the swap chain with the provided properties. Destroys any previously existing swap chain. Caller must
		 * ensure the swap chain is not used at the device when this is called.
		 */
		VulkanSwapChain(VulkanResourceManager* owner, VkSurfaceKHR surface, UINT32 width, UINT32 height, bool vsync,
			VkFormat colorFormat, VkColorSpaceKHR colorSpace, bool createDepth, VkFormat depthFormat,
			VulkanSwapChain* oldSwapChain = nullptr);
		~VulkanSwapChain();

		/**
		 * Returns the actual width of the swap chain, in pixels. This might differ from the requested size in case it
		 * wasn't supported.
		 */
		UINT32 GetWidth() const { return mWidth; }

		/**
		 * Returns the actual height of the swap chain, in pixels. This might differ from the requested size in case it
		 * wasn't supported.
		 */
		UINT32 GetHeight() const { return mHeight; }

		/**
		 * Attempts to acquire a new back buffer image. Caller can retrieve the surface by calling getBackBuffer(). Caller
		 * must wait on the semaphore provided by the surface before rendering to it. Method might fail if the swap
		 * chain is no longer valid, and failure result will be returned.
		 *
		 * @note Must only be called once in-between present() calls, or before the first present() call.
		 */
		VkResult AcquireBackBuffer();

		/**
		 * Prepares the swap chain for the present operation.
		 *
		 * @param[out] backBufferIdx	Index of the image representing the current back buffer.
		 * @return						True if there is anything to present, false otherwise.
		 */
		bool PrepareForPresent(UINT32& backBufferIdx);

		/** Notifies the chain that the semaphore waiting for the back buffer to become available is being waited on. */
		void NotifyBackBufferWaitIssued();

		/** Returns information describing the current back buffer. */
		const SwapChainSurface& GetBackBuffer() { return mSurfaces[mCurrentBackBufferIdx]; }

		/** Returns the number of available color surfaces. */
		UINT32 GetNumColorSurfaces() const { return (UINT32)mSurfaces.Size(); }

		/** Returns the internal swap chain handle. */
		VkSwapchainKHR GetHandle() const { return mSwapChain; }
	private:
		/** Destroys current swap chain and depth stencil image (if any). */
		void Clear(VkSwapchainKHR swapChain);

		VkDevice mDevice = VK_NULL_HANDLE;
		VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;

		UINT32 mWidth = 0;
		UINT32 mHeight = 0;
		Vector<SwapChainSurface> mSurfaces;

		VulkanImage* mDepthStencilImage = nullptr;

		UINT32 mCurrentSemaphoreIdx = 0;
		UINT32 mCurrentBackBufferIdx = 0;
	};

	/** @} */
}}
