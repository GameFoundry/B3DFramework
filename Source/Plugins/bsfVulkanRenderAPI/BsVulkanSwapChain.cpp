//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanSwapChain.h"
#include "BsVulkanTexture.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanGpuBackend.h"
#include "BsVulkanGpuQueue.h"
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanRenderPass.h"
#include "Threading/BsTaskScheduler.h"

using namespace bs;
using namespace bs::ct;

VulkanSurface::~VulkanSurface()
{
	vkDestroySurfaceKHR(GetVulkanGpuBackend().GetVkInstance(), mSurface, gVulkanAllocator);
}

VulkanSwapChain::VulkanSwapChain(VulkanResourceManager* owner, const SPtr<VulkanSurface>& surface, u32 width, u32 height, bool vsync, VkFormat colorFormat, VkColorSpaceKHR colorSpace, bool createDepth, VkFormat depthFormat, VulkanSwapChain* oldSwapChain)
	: VulkanResource(owner, false), mSurface(surface)
{
	VulkanGpuDevice& device = owner->GetDevice();
	mDevice = device.GetLogical();

	VkResult result;
	VkPhysicalDevice physicalDevice = device.GetPhysical();

	VkSurfaceKHR vkSurface = surface->GetVkHandle();

	// Determine swap chain dimensions
	VkSurfaceCapabilitiesKHR surfaceCaps;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vkSurface, &surfaceCaps);
	B3D_ASSERT(result == VK_SUCCESS);

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
	uint32_t presentModeCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModeCount, nullptr);
	B3D_ASSERT(result == VK_SUCCESS);
	B3D_ASSERT(presentModeCount > 0);

	VkPresentModeKHR* presentModes = B3DStackAllocate<VkPresentModeKHR>(presentModeCount);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModeCount, presentModes);
	B3D_ASSERT(result == VK_SUCCESS);

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if(!vsync)
	{
		for(u32 i = 0; i < presentModeCount; i++)
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
#if B3D_PLATFORM != B3D_PLATFORM_ID_ANDROID && B3D_PLATFORM != B3D_PLATFORM_ID_IOS
		for(u32 i = 0; i < presentModeCount; i++)
		{

			if(presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
		}
#endif
	}

	B3DStackFree(presentModes);

	// Note: Ideally we refactor the submit thread so it can work properly with two images
	constexpr u32 kPreferredImageCount = 3; // One extra required due to the separate submit thread.
	u32 imageCount = Math::Clamp(kPreferredImageCount, surfaceCaps.minImageCount, surfaceCaps.maxImageCount);

	if(imageCount < kPreferredImageCount)
		B3D_LOG(Error, RenderBackend, "Unable to allocate adequate number of swap chain images.");

	VkSurfaceTransformFlagsKHR transform;
	if(surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else
		transform = surfaceCaps.currentTransform;

	VkSwapchainCreateInfoKHR swapChainCI;
	swapChainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCI.pNext = nullptr;
	swapChainCI.flags = 0;
	swapChainCI.surface = vkSurface;
	swapChainCI.minImageCount = imageCount;
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
	B3D_ASSERT(result == VK_SUCCESS);

	result = vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
	B3D_ASSERT(result == VK_SUCCESS);

	// Get the swap chain images
	VkImage* images = B3DStackAllocate<VkImage>(imageCount);
	result = vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, images);
	B3D_ASSERT(result == VK_SUCCESS);

	VulkanImageCreateInformation imageDesc;
	imageDesc.Format = colorFormat;
	imageDesc.Type = TEX_TYPE_2D;
	imageDesc.Usage = TU_RENDERTARGET;
	imageDesc.Layout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageDesc.FaceCount = 1;
	imageDesc.MipLevelCount = 1;
	imageDesc.DepthSliceCount = 1;
	imageDesc.Allocation = VK_NULL_HANDLE;

	mSurfaces.resize(imageCount);
	for(u32 imageIndex = 0; imageIndex < imageCount; imageIndex++)
	{
		imageDesc.Image = images[imageIndex];

		mSurfaces[imageIndex].Acquired = false;
		mSurfaces[imageIndex].NeedsWait = false;
		mSurfaces[imageIndex].Image = owner->Create<VulkanImage>(imageDesc, false, false);
		mSurfaces[imageIndex].Semaphore = owner->Create<VulkanSemaphore>();

		if(mSurfaces[imageIndex].Image != nullptr)
		{
			mSurfaces[imageIndex].Image->SetName(StringUtil::Format("Color attachment #{0}", imageIndex));
		}
	}

	B3DStackFree(images);

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
		B3D_ASSERT(result == VK_SUCCESS);

		imageDesc.Image = depthStencilImage;
		imageDesc.Usage = TU_DEPTHSTENCIL;
		imageDesc.Format = depthFormat;
		imageDesc.Allocation = device.AllocateMemory(depthStencilImage, VMA_MEMORY_USAGE_GPU_ONLY);

		mDepthStencilImage = owner->Create<VulkanImage>(imageDesc, true, false);

		if(mDepthStencilImage != nullptr)
		{
			mDepthStencilImage->SetName("Depth-stencil attachment");
		}
	}
	else
		mDepthStencilImage = nullptr;

	// Create a render pass
	VulkanRenderPassCreateInformation rpDesc;
	rpDesc.SampleCount = 1;
	rpDesc.IsOffscreenSurface = false;
	rpDesc.ColorAttachments[0].Format = colorFormat;
	rpDesc.ColorAttachments[0].IsShaderReadAllowed = false;
	rpDesc.ColorAttachments[0].IsEnabled = true;

	if(mDepthStencilImage)
	{
		rpDesc.DepthAttachment.Format = depthFormat;
		rpDesc.DepthAttachment.IsShaderReadAllowed = false;
		rpDesc.DepthAttachment.IsEnabled = true;
	}

	VulkanRenderPass* renderPass = VulkanRenderPassCache::Instance().FindOrCreateRenderPass(mDevice, rpDesc);

	// Create a framebuffer for each swap chain buffer
	const u32 framebufferCount = (u32)mSurfaces.size();
	for(u32 framebufferIndex = 0; framebufferIndex < framebufferCount; framebufferIndex++)
	{
		VulkanFramebufferInformation& framebufferInformation = mSurfaces[framebufferIndex].FramebufferInformation;
		framebufferInformation.Width = mWidth;
		framebufferInformation.Height = mHeight;
		framebufferInformation.Layers = 1;
		framebufferInformation.Color[0].Image = mSurfaces[framebufferIndex].Image;
		framebufferInformation.Color[0].Surface = TextureSurface::kComplete;
		framebufferInformation.Color[0].BaseLayer = 0;
		framebufferInformation.Depth.Image = mDepthStencilImage;
		framebufferInformation.Depth.Surface = TextureSurface::kComplete;
		framebufferInformation.Depth.BaseLayer = 0;

		mSurfaces[framebufferIndex].Framebuffer = owner->Create<VulkanFramebuffer>(renderPass, framebufferInformation);
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
			B3D_ASSERT(!surface.Image->IsBound());
			B3D_ASSERT(!surface.Framebuffer->IsBound());

			surface.Framebuffer->Destroy();
			surface.Framebuffer = nullptr;

			surface.Image->Destroy();
			surface.Image = nullptr;

			surface.Semaphore->Destroy();
			surface.Semaphore = nullptr;
		}

		vkDestroySwapchainKHR(mDevice, mSwapChain, gVulkanAllocator);
	}

	if(mDepthStencilImage != nullptr)
	{
		mDepthStencilImage->Destroy();
		mDepthStencilImage = nullptr;
	}
}

ImageAcquireResult VulkanSwapChain::AcquireImage()
{
	AssertIfNotVulkanSubmitThread();

	ImageAcquireResult output;

	// Ensure we don't have too many acquired images in flight
	const u32 acquiredImageCount = mAcquiredImageCountOnSubmitThread;
	const u32 allowedImageCount = (u32)mSurfaces.size() - 1u; // One reserved for the OS compositor
	if(acquiredImageCount >= allowedImageCount)
	{
		// If outdated ignore the error, everything should be back to normal once it is rebuilt.
		if(!mIsSwapChainOutdated)
		{
			B3D_LOG(Error, RenderBackend, "Unable to acquire a swap chain image. Maximum number of images has already been acquired.");
		}

		output = ImageAcquireResult(VK_ERROR_UNKNOWN, 0);

		Lock lock(mImageAcquireMutex);
		mImageAcquireResults.Add(output);
		mImageAcquireSignal.notify_all();

		return output;
	}

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mSurfaces[mLastAcquiredSemaphoreIndex].Semaphore->GetHandle(), VK_NULL_HANDLE, &imageIndex);

	// Return the error to the caller, so he can attempt to rebuild the swap chain
	if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		output = ImageAcquireResult(result, 0);

		Lock lock(mImageAcquireMutex);
		mImageAcquireResults.Add(output);
		mImageAcquireSignal.notify_all();

		return output;
	}

	// In case surfaces aren't being distributed in round-robin fashion the image and semaphore indices might not match,
	// in which case just move the semaphores
	if(imageIndex != mLastAcquiredSemaphoreIndex)
		std::swap(mSurfaces[mLastAcquiredSemaphoreIndex].Semaphore, mSurfaces[imageIndex].Semaphore);

	mLastAcquiredSemaphoreIndex = (mLastAcquiredSemaphoreIndex + 1) % mSurfaces.size();

	B3D_ASSERT(!mSurfaces[imageIndex].Acquired && "Swap chain image being acquired twice.");
	mSurfaces[imageIndex].Acquired = true;
	mSurfaces[imageIndex].NeedsWait = true;

	output = ImageAcquireResult(result, imageIndex);
	mAcquiredImageCountOnSubmitThread++;

	{
		Lock lock(mImageAcquireMutex);
		mImageAcquireResults.Add(output);
		mImageAcquireSignal.notify_all();
	}

	return output;
}

void VulkanSwapChain::WaitUntilFirstImageAcquired()
{
	Lock lock(mImageAcquireMutex);

	// Nothing queued
	if(mQueuedImageAcquireOperationCount == 0)
		return;

	// Wait until all the acquire operations finish
	while(mQueuedImageAcquireOperationCount != (u32)mImageAcquireResults.size())
	{
		TaskScheduler::Instance().AddWorker();
		mImageAcquireSignal.wait(lock);
		TaskScheduler::Instance().RemoveWorker();
	}

	for(const auto& acquireResult : mImageAcquireResults)
	{
		if(acquireResult.ResultCode == VK_SUCCESS || acquireResult.ResultCode == VK_SUBOPTIMAL_KHR)
		{
			mAcquiredImageIndicesOnRenderThread.Add(acquireResult.AcquiredImageIndex);

			if(acquireResult.ResultCode == VK_SUBOPTIMAL_KHR)
				MarkAsInvalid();
		}
		else
		{
			MarkAsInvalid();
		}
	}

	mQueuedImageAcquireOperationCount = 0;
	mImageAcquireResults.clear();
}

void VulkanSwapChain::Present(u32 imageIndex, VulkanGpuQueue& queue, u32 syncMask)
{
	AssertIfNotVulkanSubmitThread();
	B3D_ASSERT(imageIndex <= (UINT32)mSurfaces.size());

	if(!mSurfaces[imageIndex].Acquired)
		return;

	// Ensure the image is in the correct layout
	VulkanImage *const image = mSurfaces[imageIndex].Image;
	VulkanImageSubresource* const imageSubresource = image->GetSubresource(0, 0);
	const VkImageLayout imageLayout = imageSubresource->GetLayout();

	if(imageLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		VulkanGpuDevice& device = queue.GetDevice();
		VulkanGpuCommandBufferPool& commandBufferPool = GetVulkanSubmitThread().GetCommandBufferPool(device.GetIndex(), queue.GetUsage());

		VulkanInternalCommandBuffer* const commandBuffer = commandBufferPool.GetBuffer();
		commandBuffer->SetName("Swap chain image layout transition");

		VkCommandBuffer vkCommandBuffer = commandBuffer->GetHandle();

		VkImageMemoryBarrier layoutTransitionBarrier;
		layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		layoutTransitionBarrier.pNext = nullptr;
		layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		layoutTransitionBarrier.image = image->GetHandle();
		layoutTransitionBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		layoutTransitionBarrier.subresourceRange.layerCount = 1;
		layoutTransitionBarrier.subresourceRange.levelCount = 1;
		layoutTransitionBarrier.subresourceRange.baseArrayLayer = 0;
		layoutTransitionBarrier.subresourceRange.baseMipLevel = 0;
		layoutTransitionBarrier.srcAccessMask = 0;
		layoutTransitionBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		layoutTransitionBarrier.oldLayout = imageLayout;
		layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		vkCmdPipelineBarrier(vkCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &layoutTransitionBarrier);

		commandBuffer->End();
		queue.Submit(commandBuffer, nullptr, 0);

		imageSubresource->SetLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	VulkanGpuDevice& presentDevice = queue.GetDevice();
	const u32 queueMask = presentDevice.GetQueueMask(queue.GetUsage(), queue.GetIndex());

	// Ignore myself as we handle this in VulkanGpuQueue::Present() already
	syncMask &= ~queueMask;

	const u32 deviceIndex = presentDevice.GetIndex();
	VulkanCommandBufferManager& commandBufferManager = GetVulkanCommandBufferManager();

	u32 semaphoreCount;
	commandBufferManager.GetSyncSemaphores(deviceIndex, syncMask, mSemaphoresBuffer, semaphoreCount);

	// Wait on present (i.e. until the back buffer becomes available), if we haven't already done so
	if(AppendWaitSemaphoreIfRequired(imageIndex, semaphoreCount, mSemaphoresBuffer))
		semaphoreCount++;

	const VkResult result = queue.Present(this, imageIndex, mSemaphoresBuffer, semaphoreCount);
	if(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
	{
		// As far as validation layers are concerned, when present fails the image is no longer considered as acquired.
		mSurfaces[imageIndex].Acquired = false;

		B3D_ASSERT(mAcquiredImageCountOnSubmitThread > 0);
		mAcquiredImageCountOnSubmitThread--;
	}
	else
	{
		mIsSwapChainOutdated = true;
	}
}

bool VulkanSwapChain::AppendWaitSemaphoreIfRequired(u32 imageIndex, u32 semaphoreIndex, VulkanSemaphore** outSemaphores)
{
	AssertIfNotVulkanSubmitThread();

	if(!mSurfaces[imageIndex].NeedsWait)
		return false;

	outSemaphores[semaphoreIndex] = mSurfaces[imageIndex].Semaphore;
	mSurfaces[imageIndex].NeedsWait = false;

	return true;
}

bool VulkanSwapChain::TryGetFirstAcquiredImageIndex(u32& outImageIndex) const
{
	if(mAcquiredImageIndicesOnRenderThread.Empty())
		return false;

	outImageIndex = mAcquiredImageIndicesOnRenderThread.Front();
	return true;
}

void VulkanSwapChain::NotifyWasImageAcquireQueued()
{
	{
		Lock lock(mImageAcquireMutex);
		mQueuedImageAcquireOperationCount++;
	}

	NotifyBound();
}

void VulkanSwapChain::NotifyWasPresentQueued(u32 imageIndex)
{
	const auto found = std::find(mAcquiredImageIndicesOnRenderThread.begin(), mAcquiredImageIndicesOnRenderThread.end(), imageIndex);
	if(found != mAcquiredImageIndicesOnRenderThread.end())
		mAcquiredImageIndicesOnRenderThread.erase(found);
	else
		B3D_ASSERT(false);

	NotifyBound();
}
