//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanHeadlessRenderWindowSurface.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanFramebuffer.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanRenderPass.h"
#include "B3DVulkanSubmitThread.h"
#include "B3DVulkanUtility.h"

using namespace b3d;
using namespace b3d::render;

VulkanHeadlessRenderWindowSurface::VulkanHeadlessRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation)
	: mWidth(createInformation.Width), mHeight(createInformation.Height), mVSync(createInformation.VSync), mCreateDepthBuffer(createInformation.CreateDepthBuffer), mUseHardwareSRGB(createInformation.UseHardwareSRGB)
{
	CreateSwapChainImages();
}

VulkanHeadlessRenderWindowSurface::~VulkanHeadlessRenderWindowSurface()
{
	if(!mIsDestroyed)
		Destroy();
}

void VulkanHeadlessRenderWindowSurface::CreateSwapChainImages()
{
	TShared<VulkanGpuDevice> presentDevice = GetVulkanGpuBackend().GetPresentDevice();
	VulkanResourceManager* resourceManager = &presentDevice->GetResourceManager();
	VkDevice device = presentDevice->GetLogical();

	// Determine color format
	VkFormat colorFormat = mUseHardwareSRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

	VkImageCreateInfo vkImageCreateInfo{};
	vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	vkImageCreateInfo.pNext = nullptr;
	vkImageCreateInfo.flags = 0;
	vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	vkImageCreateInfo.extent = { mWidth, mHeight, 1 };
	vkImageCreateInfo.mipLevels = 1;
	vkImageCreateInfo.arrayLayers = 1;
	vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	vkImageCreateInfo.pQueueFamilyIndices = nullptr;
	vkImageCreateInfo.queueFamilyIndexCount = 0;

	// Create color images for each buffer
	for(u32 imageIndex = 0; imageIndex < kImageCount; imageIndex++)
	{
		vkImageCreateInfo.format = colorFormat;
		vkImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		VulkanImageCreateInformation imageCreateInformation;
		imageCreateInformation.Usage = TextureUsageFlag::RenderTarget;
		imageCreateInformation.Format = colorFormat;
		imageCreateInformation.CreateInfo = vkImageCreateInfo;
		imageCreateInformation.OwnsImage = true;
		imageCreateInformation.IsShaderReadAllowed = false;

		// Purposefully not setting parent so these images don't participate in defragmentation
		mColorImages[imageIndex] = presentDevice->CreateImage(imageCreateInformation, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, GpuResourceKind::NonLinear, nullptr);
		if(mColorImages[imageIndex] != nullptr)
			mColorImages[imageIndex]->SetName("HeadlessSwapChainColor" + ToString(imageIndex));
	}

	// Create depth image if requested
	if(mCreateDepthBuffer)
	{
		vkImageCreateInfo.format = depthFormat;
		vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		VulkanImageCreateInformation imageCreateInformation;
		imageCreateInformation.Usage = TextureUsageFlag::DepthStencil;
		imageCreateInformation.Format = depthFormat;
		imageCreateInformation.CreateInfo = vkImageCreateInfo;
		imageCreateInformation.OwnsImage = true;
		imageCreateInformation.IsShaderReadAllowed = false;

		// Purposefully not setting parent so these images don't participate in defragmentation
		mDepthImage = presentDevice->CreateImage(imageCreateInformation, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, GpuResourceKind::NonLinear, nullptr);
		if(mDepthImage != nullptr)
			mDepthImage->SetName("HeadlessSwapChainDepth");
	}

	// Create render pass
	VulkanRenderPassCreateInformation renderPassCreateInformation;
	renderPassCreateInformation.SampleCount = 1;
	renderPassCreateInformation.IsOffscreenSurface = false;
	renderPassCreateInformation.ColorAttachments[0].Format = colorFormat;
	renderPassCreateInformation.ColorAttachments[0].IsShaderReadAllowed = false;
	renderPassCreateInformation.ColorAttachments[0].IsEnabled = true;

	if(mDepthImage != nullptr)
	{
		renderPassCreateInformation.DepthAttachment.Format = depthFormat;
		renderPassCreateInformation.DepthAttachment.IsShaderReadAllowed = false;
		renderPassCreateInformation.DepthAttachment.IsEnabled = true;
	}

	VulkanRenderPass* renderPass = VulkanRenderPassCache::Instance().FindOrCreateRenderPass(device, renderPassCreateInformation);

	// Create framebuffers for each image
	for(u32 imageIndex = 0; imageIndex < kImageCount; imageIndex++)
	{
		VulkanFramebufferInformation framebufferInformation;
		framebufferInformation.Width = mWidth;
		framebufferInformation.Height = mHeight;
		framebufferInformation.Layers = 1;
		framebufferInformation.Color[0].Image = mColorImages[imageIndex];
		framebufferInformation.Color[0].Surface = TextureSurface::kComplete;
		framebufferInformation.Color[0].BaseLayer = 0;
		framebufferInformation.Depth.Image = mDepthImage;
		framebufferInformation.Depth.Surface = TextureSurface::kComplete;
		framebufferInformation.Depth.BaseLayer = 0;

		mFramebuffers[imageIndex] = resourceManager->Create<VulkanFramebuffer>(renderPass, framebufferInformation);
	}

	mIsValid = true;
}

void VulkanHeadlessRenderWindowSurface::DestroySwapChainImages()
{
	GetVulkanSubmitThread().WaitUntilIdle();

	for(u32 imageIndex = 0; imageIndex < kImageCount; imageIndex++)
	{
		if(mFramebuffers[imageIndex] != nullptr)
		{
			mFramebuffers[imageIndex]->Destroy();
			mFramebuffers[imageIndex] = nullptr;
		}

		if(mColorImages[imageIndex] != nullptr)
		{
			mColorImages[imageIndex]->Destroy();
			mColorImages[imageIndex] = nullptr;
		}
	}

	if(mDepthImage != nullptr)
	{
		mDepthImage->Destroy();
		mDepthImage = nullptr;
	}
}

void VulkanHeadlessRenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync)
{
	if(mIsDestroyed)
		return;

	// Only rebuild if dimensions changed
	if(mWidth == width && mHeight == height && mVSync == vsync && mIsValid)
		return;

	mWidth = width;
	mHeight = height;
	mVSync = vsync;
	mCurrentImageIndex = 0;

	DestroySwapChainImages();
	CreateSwapChainImages();
}

void VulkanHeadlessRenderWindowSurface::MarkSwapChainAsInvalid()
{
	mIsValid = false;
}

void VulkanHeadlessRenderWindowSurface::Destroy()
{
	if(mIsDestroyed)
		return;

	DestroySwapChainImages();
	mIsDestroyed = true;
}

void VulkanHeadlessRenderWindowSurface::SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask)
{
	// We don't modify mCurrentImageIndex there in order to match the non-headless surface behaviour, which changes the active image index during the first subsequent GetActiveFramebuffer call.
	// This is important for scene captures that run /after/ the end of the frame (i.e. after SwapBuffers has been called). This way they will perform capture on the last image we rendered to, rather than a new empty one.
	mIsSwapQueued = true;
}

VulkanFramebuffer* VulkanHeadlessRenderWindowSurface::GetActiveFramebuffer(bool acquireIfUnavailable)
{
	if(mIsSwapQueued)
	{
		mCurrentImageIndex = (mCurrentImageIndex + 1) % kImageCount;
		mIsSwapQueued = false;
	}

	return mFramebuffers[mCurrentImageIndex];
}
