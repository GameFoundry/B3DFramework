//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanSwapChain.h"
#include "BsVulkanTexture.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "Managers/BsVulkanCommandBufferManager.h"

namespace bs { namespace ct
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
			swapchainExtent.width = Math::clamp(width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
			swapchainExtent.height = Math::clamp(height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
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

				if (presentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
					presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			}
		}
		else
		{
			// Mailbox comes with lower input latency than FIFO, but can waste GPU power by rendering frames that are never
			// displayed, especially if the app runs much faster than the refresh rate. This is a concern for mobiles.
#if BS_PLATFORM != BS_PLATFORM_ANDROID && BS_PLATFORM != BS_PLATFORM_IOS
			for (UINT32 i = 0; i < numPresentModes; i++)
			{

				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
			}
#endif
		}

		bs_stack_free(presentModes);

		uint32_t numImages = surfaceCaps.minImageCount;

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

		VulkanResourceManager& resManager = device->getResourceManager();

		VULKAN_IMAGE_DESC imageDesc;
		imageDesc.format = colorFormat;
		imageDesc.type = TEX_TYPE_2D;
		imageDesc.usage = TU_RENDERTARGET;
		imageDesc.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageDesc.numFaces = 1;
		imageDesc.numMipLevels = 1;
		imageDesc.allocation = VK_NULL_HANDLE;

		mSurfaces.resize(numImages);
		for (UINT32 i = 0; i < numImages; i++)
		{
			imageDesc.image = images[i];

			mSurfaces[i].acquired = false;
			mSurfaces[i].needsWait = false;
			mSurfaces[i].image = resManager.create<VulkanImage>(imageDesc, false);
			mSurfaces[i].sync = resManager.create<VulkanSemaphore>();
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

			VkImage depthStencilImage;
			result = vkCreateImage(logicalDevice, &depthStencilImageCI, gVulkanAllocator, &depthStencilImage);
			assert(result == VK_SUCCESS);

			imageDesc.image = depthStencilImage;
			imageDesc.usage = TU_DEPTHSTENCIL;
			imageDesc.format = depthFormat;
			imageDesc.allocation = mDevice->allocateMemory(depthStencilImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			mDepthStencilImage = resManager.create<VulkanImage>(imageDesc, true);
		}
		else
			mDepthStencilImage = nullptr;

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
			desc.color[0].image = mSurfaces[i].image;
			desc.color[0].surface = TextureSurface::COMPLETE;
			desc.color[0].baseLayer = 0;
			desc.depth.format = depthFormat;
			desc.depth.image = mDepthStencilImage;
			desc.depth.surface = TextureSurface::COMPLETE;
			desc.depth.baseLayer = 0;

			mSurfaces[i].framebuffer = resManager.create<VulkanFramebuffer>(desc);
		}
	}

	void VulkanSwapChain::acquireBackBuffer()
	{
		uint32_t imageIndex;

		VkResult result = vkAcquireNextImageKHR(mDevice->getLogical(), mSwapChain, UINT64_MAX,
			mSurfaces[mCurrentSemaphoreIdx].sync->getHandle(), VK_NULL_HANDLE, &imageIndex);
		assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

		// In case surfaces aren't being distributed in round-robin fashion the image and semaphore indices might not match,
		// in which case just move the semaphores
		if(imageIndex != mCurrentSemaphoreIdx)
			std::swap(mSurfaces[mCurrentSemaphoreIdx].sync, mSurfaces[imageIndex].sync);

		mCurrentSemaphoreIdx = (mCurrentSemaphoreIdx + 1) % mSurfaces.size();

		assert(!mSurfaces[imageIndex].acquired && "Same swap chain surface being acquired twice in a row without present().");
		mSurfaces[imageIndex].acquired = true;
		mSurfaces[imageIndex].needsWait = true;

		mCurrentBackBufferIdx = imageIndex;
	}

	bool VulkanSwapChain::prepareForPresent(UINT32& backBufferIdx)
	{
		if (!mSurfaces[mCurrentBackBufferIdx].acquired)
			return false;

		assert(mSurfaces[mCurrentBackBufferIdx].acquired && "Attempting to present an unacquired back buffer.");
		mSurfaces[mCurrentBackBufferIdx].acquired = false;

		backBufferIdx = mCurrentBackBufferIdx;
		return true;
	}

	void VulkanSwapChain::notifyBackBufferWaitIssued()
	{
		if (!mSurfaces[mCurrentBackBufferIdx].acquired)
			return;

		mSurfaces[mCurrentBackBufferIdx].needsWait = false;
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

				surface.image->destroy();
				surface.image = nullptr;

				surface.sync->destroy();
				surface.sync = nullptr;
			}

			vkDestroySwapchainKHR(logicalDevice, swapChain, gVulkanAllocator);
		}

		if (mDepthStencilImage != nullptr)
		{
			mDepthStencilImage->destroy();
			mDepthStencilImage = nullptr;
		}
	}
}}