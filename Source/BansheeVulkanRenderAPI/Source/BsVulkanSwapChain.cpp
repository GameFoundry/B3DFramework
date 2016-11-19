//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanSwapChain.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"

namespace BansheeEngine
{
	VulkanSwapChain::~VulkanSwapChain()
	{
		clear(mSwapChain);
	}

	void VulkanSwapChain::rebuild(const SPtr<VulkanDevice>& device, VkSurfaceKHR surface, UINT32 width, UINT32 height, 
		bool vsync, VkFormat colorFormat, VkColorSpaceKHR colorSpace, bool createDepth, VkFormat depthFormat)
	{
		mDevice = device;

		VkResult result;
		VkPhysicalDevice physicalDevice = device->getPhysical();

		// Determine swap chain dimensions
		VkSurfaceCapabilitiesKHR surfaceCaps;
		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps);
		assert(result == VK_SUCCESS);

		VkExtent2D swapchainExtent;
		// If width/height is 0xFFFFFFFF, we can manually specify width, height
		if (surfaceCaps.currentExtent.width == (uint32_t)-1 || surfaceCaps.currentExtent.height == (uint32_t)-1)
		{
			swapchainExtent.width = width;
			swapchainExtent.height = height;
		}
		else // Otherwise we must use the size we're given
			swapchainExtent = surfaceCaps.currentExtent;

		mWidth = swapchainExtent.width;
		mHeight = swapchainExtent.height;

		// Find present mode
		uint32_t numPresentModes;
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &numPresentModes, nullptr);
		assert(result == VK_SUCCESS);
		assert(numPresentModes > 0);

		VkPresentModeKHR* presentModes = bs_stack_alloc<VkPresentModeKHR>(numPresentModes);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &numPresentModes, presentModes);
		assert(result == VK_SUCCESS);

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		if(!vsync)
		{
			for (UINT32 i = 0; i < numPresentModes; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
					break;
				}

				if (presentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
					presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			}
		}
		else
		{
			for (UINT32 i = 0; i < numPresentModes; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
			}
		}

		bs_stack_free(presentModes);

		uint32_t numImages = std::min(surfaceCaps.minImageCount + BS_NUM_BACK_BUFFERS, surfaceCaps.maxImageCount);

		VkSurfaceTransformFlagsKHR transform;
		if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else
			transform = surfaceCaps.currentTransform;

		VkSwapchainCreateInfoKHR swapChainCI;
		swapChainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCI.pNext = nullptr;
		swapChainCI.flags = 0;
		swapChainCI.surface = surface;
		swapChainCI.minImageCount = numImages;
		swapChainCI.imageFormat = colorFormat;
		swapChainCI.imageColorSpace = colorSpace;
		swapChainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
		swapChainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapChainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)transform;
		swapChainCI.imageArrayLayers = 1;
		swapChainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCI.queueFamilyIndexCount = 0;
		swapChainCI.pQueueFamilyIndices = NULL;
		swapChainCI.presentMode = presentMode;
		swapChainCI.oldSwapchain = mSwapChain;
		swapChainCI.clipped = VK_TRUE;
		swapChainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		VkSwapchainKHR oldSwapChain = mSwapChain;
		VkDevice logicalDevice = device->getLogical();
		result = vkCreateSwapchainKHR(logicalDevice, &swapChainCI, gVulkanAllocator, &mSwapChain);
		assert(result == VK_SUCCESS);

		clear(oldSwapChain);

		result = vkGetSwapchainImagesKHR(logicalDevice, mSwapChain, &numImages, nullptr);
		assert(result == VK_SUCCESS);

		// Get the swap chain images
		VkImage* images = bs_stack_alloc<VkImage>(numImages);
		result = vkGetSwapchainImagesKHR(logicalDevice, mSwapChain, &numImages, images);
		assert(result == VK_SUCCESS);

		mSurfaces.resize(numImages);
		for (UINT32 i = 0; i < numImages; i++)
		{
			VkImageViewCreateInfo colorViewCI;
			colorViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorViewCI.pNext = nullptr;
			colorViewCI.flags = 0;
			colorViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorViewCI.image = images[i];
			colorViewCI.format = colorFormat;
			colorViewCI.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			colorViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorViewCI.subresourceRange.baseMipLevel = 0;
			colorViewCI.subresourceRange.levelCount = 1;
			colorViewCI.subresourceRange.baseArrayLayer = 0;
			colorViewCI.subresourceRange.layerCount = 1;
			
			mSurfaces[i].acquired = false;
			mSurfaces[i].image = images[i];
			result = vkCreateImageView(logicalDevice, &colorViewCI, gVulkanAllocator, &mSurfaces[i].view);
			assert(result == VK_SUCCESS);

			VkSemaphoreCreateInfo semaphoreCI;
			semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCI.pNext = nullptr;
			semaphoreCI.flags = 0;

			result = vkCreateSemaphore(logicalDevice, &semaphoreCI, gVulkanAllocator, &mSurfaces[i].sync);
			assert(result == VK_SUCCESS);
		}

		bs_stack_free(images);

		// Create depth stencil image
		if (createDepth)
		{
			VkImageCreateInfo depthStencilImageCI;
			depthStencilImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			depthStencilImageCI.pNext = nullptr;
			depthStencilImageCI.flags = 0;
			depthStencilImageCI.imageType = VK_IMAGE_TYPE_2D;
			depthStencilImageCI.format = depthFormat;
			depthStencilImageCI.extent = { width, height, 1 };
			depthStencilImageCI.mipLevels = 1;
			depthStencilImageCI.arrayLayers = 1;
			depthStencilImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthStencilImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			depthStencilImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
			depthStencilImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
			depthStencilImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			depthStencilImageCI.pQueueFamilyIndices = nullptr;
			depthStencilImageCI.queueFamilyIndexCount = 0;

			result = vkCreateImage(logicalDevice, &depthStencilImageCI, gVulkanAllocator, &mDepthStencilImage);
			assert(result == VK_SUCCESS);

			mDepthStencilMemory = mDevice->allocateMemory(mDepthStencilImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VkImageViewCreateInfo depthStencilViewCI;
			depthStencilViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			depthStencilViewCI.pNext = nullptr;
			depthStencilViewCI.flags = 0;
			depthStencilViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			depthStencilViewCI.image = mDepthStencilImage;
			depthStencilViewCI.format = depthFormat;
			depthStencilViewCI.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			depthStencilViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			depthStencilViewCI.subresourceRange.baseMipLevel = 0;
			depthStencilViewCI.subresourceRange.levelCount = 1;
			depthStencilViewCI.subresourceRange.baseArrayLayer = 0;
			depthStencilViewCI.subresourceRange.layerCount = 1;

			result = vkCreateImageView(logicalDevice, &depthStencilViewCI, gVulkanAllocator, &mDepthStencilView);
			assert(result == VK_SUCCESS);
		}
		else
		{
			mDepthStencilImage = VK_NULL_HANDLE;
			mDepthStencilView = VK_NULL_HANDLE;
			mDepthStencilMemory = VK_NULL_HANDLE;
		}

		VulkanResourceManager& resManager = device->getResourceManager();

		// Create a framebuffer for each swap chain buffer
		UINT32 numFramebuffers = (UINT32)mSurfaces.size();
		for (UINT32 i = 0; i < numFramebuffers; i++)
		{
			VULKAN_FRAMEBUFFER_DESC& desc = mSurfaces[i].framebufferDesc;

			desc.width = getWidth();
			desc.height = getHeight();
			desc.layers = 1;
			desc.numSamples = 1;
			desc.offscreen = false;
			desc.color[0].format = colorFormat;
			desc.color[0].view = mSurfaces[i].view;
			desc.depth.format = depthFormat;
			desc.depth.view = mDepthStencilView;

			mSurfaces[i].framebuffer = resManager.create<VulkanFramebuffer>(desc);
		}
	}

	void VulkanSwapChain::present(VkQueue queue, VkSemaphore* semaphores, UINT32 numSemaphores)
	{
		assert(mSurfaces[mCurrentBackBufferIdx].acquired && "Attempting to present an unacquired back buffer.");
		mSurfaces[mCurrentBackBufferIdx].acquired = false;

		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &mSwapChain;
		presentInfo.pImageIndices = &mCurrentBackBufferIdx;
		presentInfo.pResults = nullptr;

		// Wait before presenting, if required
		if (numSemaphores > 0)
		{
			presentInfo.pWaitSemaphores = semaphores;
			presentInfo.waitSemaphoreCount = numSemaphores;
		}
		else
		{
			presentInfo.pWaitSemaphores = nullptr;
			presentInfo.waitSemaphoreCount = 0;
		}

		VkResult result = vkQueuePresentKHR(queue, &presentInfo);
		assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
	}

	void VulkanSwapChain::acquireBackBuffer()
	{
		uint32_t imageIndex;

		VkResult result = vkAcquireNextImageKHR(mDevice->getLogical(), mSwapChain, UINT64_MAX,
			mSurfaces[mCurrentSemaphoreIdx].sync, VK_NULL_HANDLE, &imageIndex);
		assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

		// In case surfaces aren't being distributed in round-robin fashion the image and semaphore indices might not match,
		// in which case just move the semaphores
		if(imageIndex != mCurrentSemaphoreIdx)
			std::swap(mSurfaces[mCurrentSemaphoreIdx].sync, mSurfaces[imageIndex].sync);

		mCurrentSemaphoreIdx = (mCurrentSemaphoreIdx + 1) % mSurfaces.size();

		assert(!mSurfaces[imageIndex].acquired && "Same swap chain surface being acquired twice in a row without present().");
		mSurfaces[imageIndex].acquired = true;

		mCurrentBackBufferIdx = imageIndex;
	}

	void VulkanSwapChain::clear(VkSwapchainKHR swapChain)
	{
		VkDevice logicalDevice = mDevice->getLogical();
		if (swapChain != VK_NULL_HANDLE)
		{
			for (auto& surface : mSurfaces)
			{
				surface.framebuffer->destroy();
				surface.framebuffer = nullptr;

				vkDestroySemaphore(logicalDevice, surface.sync, gVulkanAllocator);
				vkDestroyImageView(logicalDevice, surface.view, gVulkanAllocator);
			}

			vkDestroySwapchainKHR(logicalDevice, swapChain, gVulkanAllocator);
		}

		if (mDepthStencilImage != VK_NULL_HANDLE)
		{
			vkDestroyImageView(logicalDevice, mDepthStencilView, gVulkanAllocator);
			vkDestroyImage(logicalDevice, mDepthStencilImage, gVulkanAllocator);

			mDevice->freeMemory(mDepthStencilMemory);
			mDepthStencilImage = VK_NULL_HANDLE;
		}
	}
}