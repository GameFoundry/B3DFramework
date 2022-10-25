//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanSwapChain.h"
#include "BsVulkanTexture.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanRenderPass.h"

namespace bs
{
	namespace ct
	{
		VulkanSwapChain::VulkanSwapChain(VulkanResourceManager* owner, VkSurfaceKHR surface, u32 width, u32 height, bool vsync, VkFormat colorFormat, VkColorSpaceKHR colorSpace, bool createDepth, VkFormat depthFormat, VulkanSwapChain* oldSwapChain)
			: VulkanResource(owner, false)
		{
			VulkanDevice& device = owner->GetDevice();
			mDevice = device.GetLogical();

			VkResult result;
			VkPhysicalDevice physicalDevice = device.GetPhysical();

			// Determine swap chain dimensions
			VkSurfaceCapabilitiesKHR surfaceCaps;
			result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps);
			assert(result == VK_SUCCESS);

			VkExtent2D swapchainExtent;
			// If width/height is 0xFFFFFFFF, we can manually specify width, height
			if(surfaceCaps.currentExtent.width == (uint32_t)-1 || surfaceCaps.currentExtent.height == (uint32_t)-1)
			{
				swapchainExtent.width = Math::Clamp(width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
				swapchainExtent.height = Math::Clamp(height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
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
				for(u32 i = 0; i < numPresentModes; i++)
				{
					if(presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
					{
						presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
						break;
					}

					if(presentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
						presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
				}
			}
			else
			{
				// Mailbox comes with lower input latency than FIFO, but can waste GPU power by rendering frames that are never
				// displayed, especially if the app runs much faster than the refresh rate. This is a concern for mobiles.
#if BS_PLATFORM != BS_PLATFORM_ANDROID && BS_PLATFORM != BS_PLATFORM_IOS
				for(u32 i = 0; i < numPresentModes; i++)
				{

					if(presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
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
			if(surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
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
			swapChainCI.pQueueFamilyIndices = nullptr;
			swapChainCI.presentMode = presentMode;
			swapChainCI.oldSwapchain = oldSwapChain ? oldSwapChain->mSwapChain : VK_NULL_HANDLE;
			swapChainCI.clipped = VK_TRUE;
			swapChainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

			result = vkCreateSwapchainKHR(mDevice, &swapChainCI, gVulkanAllocator, &mSwapChain);
			assert(result == VK_SUCCESS);

			result = vkGetSwapchainImagesKHR(mDevice, mSwapChain, &numImages, nullptr);
			assert(result == VK_SUCCESS);

			// Get the swap chain images
			VkImage* images = bs_stack_alloc<VkImage>(numImages);
			result = vkGetSwapchainImagesKHR(mDevice, mSwapChain, &numImages, images);
			assert(result == VK_SUCCESS);

			VULKAN_IMAGE_DESC imageDesc;
			imageDesc.Format = colorFormat;
			imageDesc.Type = TEX_TYPE_2D;
			imageDesc.Usage = TU_RENDERTARGET;
			imageDesc.Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageDesc.NumFaces = 1;
			imageDesc.NumMipLevels = 1;
			imageDesc.Allocation = VK_NULL_HANDLE;

			mSurfaces.resize(numImages);
			for(u32 i = 0; i < numImages; i++)
			{
				imageDesc.Image = images[i];

				mSurfaces[i].Acquired = false;
				mSurfaces[i].NeedsWait = false;
				mSurfaces[i].Image = owner->Create<VulkanImage>(imageDesc, false);
				mSurfaces[i].Sync = owner->Create<VulkanSemaphore>();
			}

			bs_stack_free(images);

			// Create depth stencil image
			if(createDepth)
			{
				VkImageCreateInfo depthStencilImageCI;
				depthStencilImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				depthStencilImageCI.pNext = nullptr;
				depthStencilImageCI.flags = 0;
				depthStencilImageCI.imageType = VK_IMAGE_TYPE_2D;
				depthStencilImageCI.format = depthFormat;
				depthStencilImageCI.extent = { mWidth, mHeight, 1 };
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
				result = vkCreateImage(mDevice, &depthStencilImageCI, gVulkanAllocator, &depthStencilImage);
				assert(result == VK_SUCCESS);

				imageDesc.Image = depthStencilImage;
				imageDesc.Usage = TU_DEPTHSTENCIL;
				imageDesc.Format = depthFormat;
				imageDesc.Allocation = device.AllocateMemory(depthStencilImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

				mDepthStencilImage = owner->Create<VulkanImage>(imageDesc, true);
			}
			else
				mDepthStencilImage = nullptr;

			// Create a render pass
			VULKAN_RENDER_PASS_DESC rpDesc;
			rpDesc.NumSamples = 1;
			rpDesc.Offscreen = false;
			rpDesc.Color[0].Format = colorFormat;
			rpDesc.Color[0].Enabled = true;

			if(mDepthStencilImage)
			{
				rpDesc.Depth.Format = depthFormat;
				rpDesc.Depth.Enabled = true;
			}

			VulkanRenderPass* renderPass = VulkanRenderPasses::Instance().Get(mDevice, rpDesc);

			// Create a framebuffer for each swap chain buffer
			u32 numFramebuffers = (u32)mSurfaces.size();
			for(u32 i = 0; i < numFramebuffers; i++)
			{
				VULKAN_FRAMEBUFFER_DESC& desc = mSurfaces[i].FramebufferDesc;
				desc.Width = mWidth;
				desc.Height = mHeight;
				desc.Layers = 1;
				desc.Color[0].Image = mSurfaces[i].Image;
				desc.Color[0].Surface = TextureSurface::COMPLETE;
				desc.Color[0].BaseLayer = 0;
				desc.Depth.Image = mDepthStencilImage;
				desc.Depth.Surface = TextureSurface::COMPLETE;
				desc.Depth.BaseLayer = 0;

				mSurfaces[i].Framebuffer = owner->Create<VulkanFramebuffer>(renderPass, desc);
			}
		}

		VulkanSwapChain::~VulkanSwapChain()
		{
			if(mSwapChain != VK_NULL_HANDLE)
			{
				for(auto& surface : mSurfaces)
				{
					// Swap chain images only live as long as the swap chain, so its invalid if they are being used somewhere,
					// and same goes for the framebuffer since it depends on those images.
					assert(!surface.Image->IsBound());
					assert(!surface.Framebuffer->IsBound());

					surface.Framebuffer->Destroy();
					surface.Framebuffer = nullptr;

					surface.Image->Destroy();
					surface.Image = nullptr;

					surface.Sync->Destroy();
					surface.Sync = nullptr;
				}

				vkDestroySwapchainKHR(mDevice, mSwapChain, gVulkanAllocator);
			}

			if(mDepthStencilImage != nullptr)
			{
				mDepthStencilImage->Destroy();
				mDepthStencilImage = nullptr;
			}
		}

		VkResult VulkanSwapChain::AcquireBackBuffer()
		{
			uint32_t imageIndex;

			VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mSurfaces[mCurrentSemaphoreIdx].Sync->GetHandle(), VK_NULL_HANDLE, &imageIndex);

			if(result != VK_SUCCESS)
				return result;

			// In case surfaces aren't being distributed in round-robin fashion the image and semaphore indices might not match,
			// in which case just move the semaphores
			if(imageIndex != mCurrentSemaphoreIdx)
				std::swap(mSurfaces[mCurrentSemaphoreIdx].Sync, mSurfaces[imageIndex].Sync);

			mCurrentSemaphoreIdx = (mCurrentSemaphoreIdx + 1) % mSurfaces.size();

			assert(!mSurfaces[imageIndex].Acquired && "Same swap chain surface being acquired twice in a row without present().");
			mSurfaces[imageIndex].Acquired = true;
			mSurfaces[imageIndex].NeedsWait = true;

			mCurrentBackBufferIdx = imageIndex;

			return VK_SUCCESS;
		}

		bool VulkanSwapChain::PrepareForPresent(u32& backBufferIdx)
		{
			if(!mSurfaces[mCurrentBackBufferIdx].Acquired)
				return false;

			assert(mSurfaces[mCurrentBackBufferIdx].Acquired && "Attempting to present an unacquired back buffer.");
			mSurfaces[mCurrentBackBufferIdx].Acquired = false;

			backBufferIdx = mCurrentBackBufferIdx;
			return true;
		}

		void VulkanSwapChain::NotifyBackBufferWaitIssued()
		{
			if(!mSurfaces[mCurrentBackBufferIdx].Acquired)
				return;

			mSurfaces[mCurrentBackBufferIdx].NeedsWait = false;
		}
	} // namespace ct
} // namespace bs
