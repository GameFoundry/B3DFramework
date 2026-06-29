//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanFramebuffer.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanRenderPass.h"

using namespace b3d;
using namespace b3d::render;

u32 VulkanFramebuffer::sNextValidId = 1;

VulkanFramebuffer::VulkanFramebuffer(VulkanResourceManager* owner, VulkanRenderPass* renderPass, const VulkanFramebufferInformation& desc, const StringView& name)
	: VulkanResource(owner, false, name), mRenderPass(renderPass), mWidth(desc.Width), mHeight(desc.Height), mNumLayers(desc.Layers)
{
	mId = sNextValidId++;

	VkImageView attachmentViews[B3D_MAXIMUM_RENDER_TARGET_COUNT + 1];
	VkFramebufferCreateInfo framebufferCI;

	u32 attachmentIdx = 0;
	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		if(desc.Color[i].Image == nullptr)
			continue;

		mColorAttachments[attachmentIdx].BaseLayer = desc.Color[i].BaseLayer;
		mColorAttachments[attachmentIdx].Image = desc.Color[i].Image;
		mColorAttachments[attachmentIdx].FinalLayout = renderPass->GetColorAttachmentFinalLayout(attachmentIdx);
		mColorAttachments[attachmentIdx].Index = i;
		mColorAttachments[attachmentIdx].Surface = desc.Color[i].Surface;

		if(desc.Color[i].Surface.MipLevelCount == 0)
			attachmentViews[attachmentIdx] = desc.Color[i].Image->GetView(true).Handle;
		else
			attachmentViews[attachmentIdx] = desc.Color[i].Image->GetView(desc.Color[i].Surface, true).Handle;

		attachmentIdx++;
	}

	if(renderPass->HasDepthAttachment())
	{
		mDepthStencilAttachment.BaseLayer = desc.Depth.BaseLayer;
		mDepthStencilAttachment.Image = desc.Depth.Image;
		mDepthStencilAttachment.FinalLayout = renderPass->GetDepthAttachmentFinalLayout();
		mDepthStencilAttachment.Index = 0;
		mDepthStencilAttachment.Surface = desc.Depth.Surface;

		if(desc.Depth.Surface.MipLevelCount == 0)
			attachmentViews[attachmentIdx] = desc.Depth.Image->GetView(true).Handle;
		else
			attachmentViews[attachmentIdx] = desc.Depth.Image->GetView(desc.Depth.Surface, true).Handle;

		attachmentIdx++;
	}

	framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCI.pNext = nullptr;
	framebufferCI.flags = 0;
	framebufferCI.attachmentCount = renderPass->GetAttachmentCount();
	framebufferCI.pAttachments = attachmentViews;
	framebufferCI.width = desc.Width;
	framebufferCI.height = desc.Height;
	framebufferCI.layers = desc.Layers;

	// Relying on the fact that compatible render passes can be used, and don't need to match exactly
	framebufferCI.renderPass = mRenderPass->GetVkRenderPass(RT_NONE, RT_NONE, RT_NONE);

	VkDevice device = mOwner->GetDevice().GetLogical();
	VkResult result = vkCreateFramebuffer(device, &framebufferCI, gVulkanAllocator, &mVkFramebuffer);
	B3D_ASSERT(result == VK_SUCCESS);
}

VulkanFramebuffer::~VulkanFramebuffer()
{
	VkDevice device = mOwner->GetDevice().GetLogical();
	vkDestroyFramebuffer(device, mVkFramebuffer, gVulkanAllocator);
}


VulkanFramebufferCache::~VulkanFramebufferCache()
{
	for(auto& entry : mCache)
		entry.second->Destroy();
}

VulkanFramebuffer* VulkanFramebufferCache::FindOrCreateFramebuffer(const VulkanGpuDevice& device, const VulkanFramebufferInformation& framebufferInformation, const VulkanRenderPassCreateInformation& renderPassInformation)
{
	FramebufferVariantKey key(device.GetLogical(), framebufferInformation, renderPassInformation);

	VulkanFramebuffer* framebuffer;
	{
		Lock lock(mMutex);

		const auto found = mCache.find(key);
		if(found != mCache.end())
			return found->second;

		VulkanRenderPass* const renderPass = VulkanRenderPassCache::Instance().FindOrCreateRenderPass(device.GetLogical(), renderPassInformation);
		framebuffer = device.GetResourceManager().Create<VulkanFramebuffer>(renderPass, framebufferInformation);
		mCache[key] = framebuffer;
	}

	return framebuffer;
}

void VulkanFramebufferCache::NotifyImageDestroyed(const VulkanImage& image)
{
	Lock lock(mMutex);

	for(auto iter = mCache.begin(); iter != mCache.end();)
	{
		bool isEntryFound = false;
		
		const u32 colorAttachmentCount = (u32)B3DSize(iter->first.FramebufferInformation.Color);
		for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
		{
			const VulkanFramebufferAttachmentInformation& attachmentInformation = iter->first.FramebufferInformation.Color[colorAttachmentIndex];
			if(attachmentInformation.Image == nullptr)
				continue;

			const VulkanImage* const attachmentImage = attachmentInformation.Image;
			if(attachmentImage == &image)
			{
				isEntryFound = true;
				break;
			}
		}

		if(!isEntryFound)
		{
			const VulkanFramebufferAttachmentInformation& depthAttachmentInformation = iter->first.FramebufferInformation.Depth;

			if(depthAttachmentInformation.Image != nullptr)
			{
				const VulkanImage* const attachmentImage = depthAttachmentInformation.Image;
				if(attachmentImage == &image)
					isEntryFound = true;
			}
		}

		if(isEntryFound)
		{
			iter->second->Destroy();
			iter = mCache.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

VulkanFramebufferCache::FramebufferVariantKey::FramebufferVariantKey(const VkDevice& device, const VulkanFramebufferInformation& framebufferInformation, const VulkanRenderPassCreateInformation& renderPassInformation)
	: Device(device), FramebufferInformation(framebufferInformation), RenderPassInformation(renderPassInformation)
{}

size_t VulkanFramebufferCache::FramebufferVariantKey::HashFunction::operator()(const FramebufferVariantKey& value) const
{
	size_t hash = 0;
	B3DCombineHash(hash, value.Device);
	B3DCombineHash(hash, value.RenderPassInformation.CalculateCompatibilityHash());

	B3DCombineHash(hash, value.FramebufferInformation.Width);
	B3DCombineHash(hash, value.FramebufferInformation.Height);
	B3DCombineHash(hash, value.FramebufferInformation.Layers);

	if(value.FramebufferInformation.Depth.Image != nullptr)
	{
		B3DCombineHash(hash, value.FramebufferInformation.Depth.Image);
		B3DCombineHash(hash, value.FramebufferInformation.Depth.Surface);
	}

	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < B3DSize(value.FramebufferInformation.Color); colorAttachmentIndex++)
	{
		if(value.FramebufferInformation.Color[colorAttachmentIndex].Image == nullptr)
			continue;

		B3DCombineHash(hash, value.FramebufferInformation.Color[colorAttachmentIndex].Image);
		B3DCombineHash(hash, value.FramebufferInformation.Color[colorAttachmentIndex].Surface);
	}

	return hash;
}

bool VulkanFramebufferCache::FramebufferVariantKey::EqualFunction::operator()(const FramebufferVariantKey& lhs, const FramebufferVariantKey& rhs) const
{
	if(lhs.Device != rhs.Device || !lhs.RenderPassInformation.IsCompatible(rhs.RenderPassInformation))
		return false;

	if(lhs.FramebufferInformation.Depth.Image != rhs.FramebufferInformation.Depth.Image || lhs.FramebufferInformation.Depth.Surface != rhs.FramebufferInformation.Depth.Surface)
		return false;

	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < B3DSize(lhs.FramebufferInformation.Color); colorAttachmentIndex++)
	{
		if(lhs.FramebufferInformation.Color[colorAttachmentIndex].Image != rhs.FramebufferInformation.Color[colorAttachmentIndex].Image || lhs.FramebufferInformation.Color[colorAttachmentIndex].Surface != rhs.FramebufferInformation.Color[colorAttachmentIndex].Surface)
			return false;
	}

	return true;
}
