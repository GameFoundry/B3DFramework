//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanSwapChain.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanGpuQueue.h"
#include "B3DVulkanRenderPass.h"

using namespace b3d;
using namespace b3d::render;

VulkanSurface::~VulkanSurface()
{
	vkDestroySurfaceKHR(GetVulkanGpuBackend().GetVkInstance(), mSurface, gVulkanAllocator);
}

VulkanSwapChain::VulkanSwapChain(VulkanResourceManager* owner, const TShared<VulkanSurface>& surface, u32 width, u32 height, bool vsync, VkFormat colorFormat, VkColorSpaceKHR colorSpace, bool createDepth, VkFormat depthFormat, VulkanSwapChain* oldSwapChain, const StringView& name)
	: VulkanResource(owner, false, name), mSurface(surface)
{
	// Process messages related to this swap chain on this thread. Mostly these are notifies when a swap chain is done being used for a present operation. */
	Scheduler* const scheduler = Scheduler::Get();
	if(B3D_ENSURE(scheduler))
	{
		mMessageQueue.ScheduleRunUntilShutdown(*scheduler, true);
	}

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
#if !B3D_PLATFORM_ANDROID && !B3D_PLATFORM_IOS
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

	constexpr u32 kPreferredImageCount = 2;
	u32 imageCount = Math::Clamp(kPreferredImageCount, surfaceCaps.minImageCount, surfaceCaps.maxImageCount);

	if(imageCount < kPreferredImageCount)
		B3D_LOG(Error, LogRenderBackend, "Unable to allocate adequate number of swap chain images.");

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

	VulkanImageCreateInformation colorImageCreateInformation;
	colorImageCreateInformation.Format = colorFormat;
	colorImageCreateInformation.Usage = TextureUsageFlag::RenderTarget;
	colorImageCreateInformation.OwnsImage = false; // Owned by VkSwapchainKHR.
	colorImageCreateInformation.IsShaderReadAllowed = false;
	colorImageCreateInformation.DebugName = "SwapChainSurface";

	mSurfaces.resize(imageCount);
	for(u32 imageIndex = 0; imageIndex < imageCount; imageIndex++)
	{
		mSurfaces[imageIndex].Acquired = false;
		mSurfaces[imageIndex].NeedsWait = false;

		// Purposefully not setting parent so these images don't participate in defragmentation
		mSurfaces[imageIndex].Image = owner->Create<VulkanImage>(colorImageCreateInformation, images[imageIndex], VulkanAllocationResult{}, nullptr);

		if(mSurfaces[imageIndex].Image != nullptr)
		{
			mSurfaces[imageIndex].Image->SetName(StringUtility::Format("Color attachment #{0}", imageIndex));
		}
	}

	// One extra semaphore because we perform the acquire operation right after presenting an image. For 2 image swap chain this means
	// three semaphores will be in use:
	// - One for frame being executed on the GPU
	// - One for the image we're queuing a present for this frame
	// - One for the image we're queuing an acquire for, to present the next frame
	const u32 semaphoreCount = imageCount  + 1;

	mSemaphores.Resize(semaphoreCount);
	for(u32 imageIndex = 0; imageIndex < semaphoreCount; imageIndex++)
	{
		mSemaphores[imageIndex] = owner->Create<VulkanSemaphore>();
	}

	B3DStackFree(images);

	// Create depth stencil image
	if(createDepth)
	{
		VkImageCreateInfo depthImageVkCreateInformation;
		depthImageVkCreateInformation.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		depthImageVkCreateInformation.pNext = nullptr;
		depthImageVkCreateInformation.flags = 0;
		depthImageVkCreateInformation.imageType = VK_IMAGE_TYPE_2D;
		depthImageVkCreateInformation.format = depthFormat;
		depthImageVkCreateInformation.extent = { mWidth, mHeight, 1 };
		depthImageVkCreateInformation.mipLevels = 1;
		depthImageVkCreateInformation.arrayLayers = 1;
		depthImageVkCreateInformation.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthImageVkCreateInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		depthImageVkCreateInformation.samples = VK_SAMPLE_COUNT_1_BIT;
		depthImageVkCreateInformation.tiling = VK_IMAGE_TILING_OPTIMAL;
		depthImageVkCreateInformation.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depthImageVkCreateInformation.pQueueFamilyIndices = nullptr;
		depthImageVkCreateInformation.queueFamilyIndexCount = 0;

		VulkanImageCreateInformation depthImageCreateInformation;
		depthImageCreateInformation.Usage = TextureUsageFlag::DepthStencil;
		depthImageCreateInformation.Format = depthFormat;
		depthImageCreateInformation.CreateInfo = depthImageVkCreateInformation;
		depthImageCreateInformation.OwnsImage = true;
		depthImageCreateInformation.IsShaderReadAllowed = false;

		// Purposefully not setting parent so these images don't participate in defragmentation
		mDepthStencilImage = device.CreateImage(depthImageCreateInformation, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, GpuResourceKind::NonLinear, nullptr);

		if(mDepthStencilImage != nullptr)
		{
			mDepthStencilImage->SetName("Depth-stencil attachment");
		}
	}
	else
		mDepthStencilImage = nullptr;

	// Create a render pass
	VulkanRenderPassCreateInformation renderPassCreateInformation;
	renderPassCreateInformation.SampleCount = 1;
	renderPassCreateInformation.IsOffscreenSurface = false;
	renderPassCreateInformation.ColorAttachments[0].Format = colorFormat;
	renderPassCreateInformation.ColorAttachments[0].IsShaderReadAllowed = false;
	renderPassCreateInformation.ColorAttachments[0].IsEnabled = true;

	if(mDepthStencilImage)
	{
		renderPassCreateInformation.DepthAttachment.Format = depthFormat;
		renderPassCreateInformation.DepthAttachment.IsShaderReadAllowed = false;
		renderPassCreateInformation.DepthAttachment.IsEnabled = true;
	}

	VulkanRenderPass* renderPass = VulkanRenderPassCache::Instance().FindOrCreateRenderPass(mDevice, renderPassCreateInformation);

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
	mMessageQueue.PostRequestShutdownCommand(true);

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
		}

		for(auto semaphore : mSemaphores)
		{
			semaphore->Destroy();
		}
		mSemaphores.Clear();

		vkDestroySwapchainKHR(mDevice, mSwapChain, gVulkanAllocator);
	}

	if(mDepthStencilImage != nullptr)
	{
		mDepthStencilImage->Destroy();
		mDepthStencilImage = nullptr;
	}
}

void VulkanSwapChain::Destroy()
{
	// Ensure we process all pending messages as those could contain unbind notifies, so we can ensure the resource gets destroyed immediately (important for shutdown)
	mMessageQueue.RunUntilIdle();

	Base::Destroy();
}

ImageAcquireResult VulkanSwapChain::AcquireImage()
{
	// This should be called from the submit thread, or one of the helper workers (workers are utilized because of the wait in vkAcquireNextImageKHR,
	// otherwise the calling fiber cannot yield - we need to launch this in its own thread).
	AssertIfRenderThread();

	ImageAcquireResult output;

	// Ensure we don't have too many acquired images in flight
	const u32 acquiredImageCount = mAcquiredImageCountOnSubmitThread;
	const u32 allowedImageCount = (u32)mSurfaces.size() - 1u; // One reserved for the OS compositor
	if(acquiredImageCount >= allowedImageCount)
	{
		// If outdated ignore the error, everything should be back to normal once it is rebuilt.
		if(!mIsSwapChainOutdated)
		{
			B3D_LOG(Error, LogRenderBackend, "Unable to acquire a swap chain image. Maximum number of images has already been acquired.");
		}

		output = ImageAcquireResult(VK_ERROR_UNKNOWN, 0);

		Lock lock(mImageAcquireMutex);
		mImageAcquireResults.Add(output);
		mImageAcquireSignal.NotifyAll();

		return output;
	}

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mSemaphores[mLastAcquiredSemaphoreIndex]->GetHandle(), VK_NULL_HANDLE, &imageIndex);

	// Return the error to the caller, so he can attempt to rebuild the swap chain
	if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		output = ImageAcquireResult(result, 0);

		Lock lock(mImageAcquireMutex);
		mImageAcquireResults.Add(output);
		mImageAcquireSignal.NotifyAll();

		return output;
	}

	mSurfaces[imageIndex].WaitSemaphore = mSemaphores[mLastAcquiredSemaphoreIndex];
	mLastAcquiredSemaphoreIndex = (mLastAcquiredSemaphoreIndex + 1) % mSemaphores.size();

	B3D_ASSERT(!mSurfaces[imageIndex].Acquired && "Swap chain image being acquired twice.");
	mSurfaces[imageIndex].Acquired = true;
	mSurfaces[imageIndex].NeedsWait = true;

	output = ImageAcquireResult(result, imageIndex);
	mAcquiredImageCountOnSubmitThread++;

	{
		Lock lock(mImageAcquireMutex);
		mImageAcquireResults.Add(output);
		mImageAcquireSignal.NotifyAll();
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
	mImageAcquireSignal.Wait(lock, [this] { return mQueuedImageAcquireOperationCount == (u32)mImageAcquireResults.size(); });

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

void VulkanSwapChain::Present(u32 imageIndex, VulkanGpuQueue& queue, GpuQueueMask syncMask)
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
		VulkanGpuCommandBufferPool& commandBufferPool = GetVulkanSubmitThread().GetCommandBufferPool(queue.GetType());

		const TShared<VulkanGpuCommandBuffer> commandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(commandBufferPool.Create(GpuCommandBufferCreateInformation::Create("SwapChainImageLayoutTransition")));
		commandBuffer->SetName("Swap chain image layout transition");

		VkCommandBuffer vkCommandBuffer = commandBuffer->GetVulkanHandle();

		VkImageMemoryBarrier layoutTransitionBarrier;
		layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		layoutTransitionBarrier.pNext = nullptr;
		layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		layoutTransitionBarrier.image = image->GetVulkanHandle();
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

		GpuCommandBufferSubmitInformation submitInformation;
		submitInformation.PrimaryCommandBuffer = commandBuffer;
		queue.ExecuteSubmitOnSubmitThread(submitInformation, GpuQueueMask::kNone, {});

		imageSubresource->SetLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	VulkanGpuDevice& presentDevice = queue.GetDevice();
	const GpuQueueMask queueMask = queue.GetId();

	// Ignore myself as we handle this in VulkanGpuQueue::Present() already
	syncMask &= ~queueMask;

	B3D_ENSURE(mSemaphoresBuffer.Empty());
	presentDevice.GetSyncSemaphores(syncMask, mSemaphoresBuffer);

	// Wait on present (i.e. until the back buffer becomes available), if we haven't already done so
	AppendWaitSemaphoreIfRequired(imageIndex, mSemaphoresBuffer);

	const VkResult result = queue.Present(this, imageIndex, mSemaphoresBuffer);
	mSemaphoresBuffer.Clear();

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

bool VulkanSwapChain::AppendWaitSemaphoreIfRequired(u32 imageIndex, TInlineArray<VulkanSemaphore*, 8>& outSemaphores)
{
	AssertIfNotVulkanSubmitThread();

	if(!mSurfaces[imageIndex].NeedsWait)
		return false;

	if(B3D_ENSURE(mSurfaces[imageIndex].WaitSemaphore != nullptr))
		outSemaphores.Add(mSurfaces[imageIndex].WaitSemaphore);

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
