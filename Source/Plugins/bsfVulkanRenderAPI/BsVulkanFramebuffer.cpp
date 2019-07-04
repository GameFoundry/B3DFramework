//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanFramebuffer.h"
#include "BsVulkanTexture.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"
#include "BsVulkanRenderPass.h"

namespace bs { namespace ct
{
	UINT32 VulkanFramebuffer::sNextValidId = 1;

	VulkanFramebuffer::VulkanFramebuffer(VulkanResourceManager* owner, VulkanRenderPass* renderPass, 
		const VULKAN_FRAMEBUFFER_DESC& desc)
		: VulkanResource(owner, false), mRenderPass(renderPass), mWidth(desc.width), mHeight(desc.height)
		, mNumLayers(desc.layers)
	{
		mId = sNextValidId++;

		VkImageView attachmentViews[BS_MAX_MULTIPLE_RENDER_TARGETS + 1];
		VkFramebufferCreateInfo framebufferCI;

		UINT32 attachmentIdx = 0;
		for(UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			if (desc.color[i].image == nullptr)
				continue;

			mColorAttachments[attachmentIdx].baseLayer = desc.color[i].baseLayer;
			mColorAttachments[attachmentIdx].image = desc.color[i].image;
			mColorAttachments[attachmentIdx].finalLayout = renderPass->getColorDesc(attachmentIdx).finalLayout;
			mColorAttachments[attachmentIdx].index = i;
			mColorAttachments[attachmentIdx].surface = desc.color[i].surface;

			if (desc.color[i].surface.numMipLevels == 0)
				attachmentViews[attachmentIdx] = desc.color[i].image->getView(true);
			else
				attachmentViews[attachmentIdx] = desc.color[i].image->getView(desc.color[i].surface, true);

			attachmentIdx++;
		}

		if (renderPass->hasDepthAttachment())
		{
			mDepthStencilAttachment.baseLayer = desc.depth.baseLayer;
			mDepthStencilAttachment.image = desc.depth.image;
			mDepthStencilAttachment.finalLayout = renderPass->getDepthDesc().finalLayout;
			mDepthStencilAttachment.index = 0;
			mDepthStencilAttachment.surface = desc.depth.surface;

			if (desc.depth.surface.numMipLevels == 0)
				attachmentViews[attachmentIdx] = desc.depth.image->getView(true);
			else
				attachmentViews[attachmentIdx] = desc.depth.image->getView(desc.depth.surface, true);

			attachmentIdx++;
		}

		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.pNext = nullptr;
		framebufferCI.flags = 0;
		framebufferCI.attachmentCount = renderPass->getNumAttachments();
		framebufferCI.pAttachments = attachmentViews;
		framebufferCI.width = desc.width;
		framebufferCI.height = desc.height;
		framebufferCI.layers = desc.layers;

		// Relying on the fact that compatible render passes can be used, and don't need to match exactly
		framebufferCI.renderPass = mRenderPass->getVkRenderPass(RT_NONE, RT_NONE, CLEAR_NONE);

		VkDevice device = mOwner->getDevice().getLogical();
		VkResult result = vkCreateFramebuffer(device, &framebufferCI, gVulkanAllocator, &mVkFramebuffer);
		assert(result == VK_SUCCESS);
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		VkDevice device = mOwner->getDevice().getLogical();
		vkDestroyFramebuffer(device, mVkFramebuffer, gVulkanAllocator);
	}
}}