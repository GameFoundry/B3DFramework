//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanSwapChain.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"

namespace BansheeEngine
{
	VulkanSwapChain::~VulkanSwapChain()
	{
		if (mSwapChain != VK_NULL_HANDLE)
		{
			VkDevice logicalDevice = mDevice->getLogical();
			for (auto& surface : mSurfaces)
				vkDestroyImageView(logicalDevice, surface.view, gVulkanAllocator);

			vkDestroySwapchainKHR(logicalDevice, mSwapChain, gVulkanAllocator);
		}
	}

	void VulkanSwapChain::rebuild(const SPtr<VulkanDevice>& device, VkSurfaceKHR surface, UINT32 width, UINT32 height, bool vsync,
		VkFormat colorFormat, VkColorSpaceKHR colorSpace)
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

		uint32_t numImages = std::min(surfaceCaps.minImageCount + 1, surfaceCaps.maxImageCount);

		VkSurfaceTransformFlagsKHR transform;
		if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else
			transform = surfaceCaps.currentTransform;

		VkSwapchainCreateInfoKHR swapChainCreateInfo;
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.pNext = nullptr;
		swapChainCreateInfo.flags = 0;
		swapChainCreateInfo.surface = surface;
		swapChainCreateInfo.minImageCount = numImages;
		swapChainCreateInfo.imageFormat = colorFormat;
		swapChainCreateInfo.imageColorSpace = colorSpace;
		swapChainCreateInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapChainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)transform;
		swapChainCreateInfo.imageArrayLayers = 1;
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = NULL;
		swapChainCreateInfo.presentMode = presentMode;
		swapChainCreateInfo.oldSwapchain = mSwapChain;
		swapChainCreateInfo.clipped = VK_TRUE;
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		VkSwapchainKHR oldSwapChain = mSwapChain;
		VkDevice logicalDevice = device->getLogical();
		result = vkCreateSwapchainKHR(logicalDevice, &swapChainCreateInfo, gVulkanAllocator, &mSwapChain);
		assert(result == VK_SUCCESS);

		if (oldSwapChain != VK_NULL_HANDLE)
		{
			for(auto& entry : mSurfaces)
				vkDestroyImageView(logicalDevice, entry.view, gVulkanAllocator);

			vkDestroySwapchainKHR(logicalDevice, oldSwapChain, gVulkanAllocator);
		}

		result = vkGetSwapchainImagesKHR(logicalDevice, mSwapChain, &numImages, nullptr);
		assert(result == VK_SUCCESS);

		// Get the swap chain images
		VkImage* images = bs_stack_alloc<VkImage>(numImages);
		result = vkGetSwapchainImagesKHR(logicalDevice, mSwapChain, &numImages, images);
		assert(result == VK_SUCCESS);

		mSurfaces.resize(numImages);
		for (UINT32 i = 0; i < numImages; i++)
		{
			VkImageViewCreateInfo colorAttachmentView;
			colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorAttachmentView.pNext = nullptr;
			colorAttachmentView.flags = 0;
			colorAttachmentView.format = colorFormat;
			colorAttachmentView.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorAttachmentView.subresourceRange.baseMipLevel = 0;
			colorAttachmentView.subresourceRange.levelCount = 1;
			colorAttachmentView.subresourceRange.baseArrayLayer = 0;
			colorAttachmentView.subresourceRange.layerCount = 1;
			colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorAttachmentView.image = images[i];

			mSurfaces[i].image = images[i];
			result = vkCreateImageView(logicalDevice, &colorAttachmentView, gVulkanAllocator, &mSurfaces[i].view);
			assert(result == VK_SUCCESS);
		}

		bs_stack_free(images);
	}
}