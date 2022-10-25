//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanFramebuffer.h"
#include "BsVulkanTexture.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"
#include "BsVulkanRenderPass.h"

namespace bs
{
	namespace ct
	{
		u32 VulkanFramebuffer::sNextValidId = 1;

		VulkanFramebuffer::VulkanFramebuffer(VulkanResourceManager* owner, VulkanRenderPass* renderPass, const VULKAN_FRAMEBUFFER_DESC& desc)
			: VulkanResource(owner, false), mRenderPass(renderPass), mWidth(desc.Width), mHeight(desc.Height), mNumLayers(desc.Layers)
		{
			mId = sNextValidId++;

			VkImageView attachmentViews[BS_MAX_MULTIPLE_RENDER_TARGETS + 1];
			VkFramebufferCreateInfo framebufferCI;

			u32 attachmentIdx = 0;
			for(u32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
			{
				if(desc.Color[i].Image == nullptr)
					continue;

				mColorAttachments[attachmentIdx].BaseLayer = desc.Color[i].BaseLayer;
				mColorAttachments[attachmentIdx].Image = desc.Color[i].Image;
				mColorAttachments[attachmentIdx].FinalLayout = renderPass->GetColorDesc(attachmentIdx).finalLayout;
				mColorAttachments[attachmentIdx].Index = i;
				mColorAttachments[attachmentIdx].Surface = desc.Color[i].Surface;

				if(desc.Color[i].Surface.NumMipLevels == 0)
					attachmentViews[attachmentIdx] = desc.Color[i].Image->GetView(true);
				else
					attachmentViews[attachmentIdx] = desc.Color[i].Image->GetView(desc.Color[i].Surface, true);

				attachmentIdx++;
			}

			if(renderPass->HasDepthAttachment())
			{
				mDepthStencilAttachment.BaseLayer = desc.Depth.BaseLayer;
				mDepthStencilAttachment.Image = desc.Depth.Image;
				mDepthStencilAttachment.FinalLayout = renderPass->GetDepthDesc().finalLayout;
				mDepthStencilAttachment.Index = 0;
				mDepthStencilAttachment.Surface = desc.Depth.Surface;

				if(desc.Depth.Surface.NumMipLevels == 0)
					attachmentViews[attachmentIdx] = desc.Depth.Image->GetView(true);
				else
					attachmentViews[attachmentIdx] = desc.Depth.Image->GetView(desc.Depth.Surface, true);

				attachmentIdx++;
			}

			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.pNext = nullptr;
			framebufferCI.flags = 0;
			framebufferCI.attachmentCount = renderPass->GetNumAttachments();
			framebufferCI.pAttachments = attachmentViews;
			framebufferCI.width = desc.Width;
			framebufferCI.height = desc.Height;
			framebufferCI.layers = desc.Layers;

			// Relying on the fact that compatible render passes can be used, and don't need to match exactly
			framebufferCI.renderPass = mRenderPass->GetVkRenderPass(RT_NONE, RT_NONE, CLEAR_NONE);

			VkDevice device = mOwner->GetDevice().GetLogical();
			VkResult result = vkCreateFramebuffer(device, &framebufferCI, gVulkanAllocator, &mVkFramebuffer);
			assert(result == VK_SUCCESS);
		}

		VulkanFramebuffer::~VulkanFramebuffer()
		{
			VkDevice device = mOwner->GetDevice().GetLogical();
			vkDestroyFramebuffer(device, mVkFramebuffer, gVulkanAllocator);
		}
	} // namespace ct
} // namespace bs
