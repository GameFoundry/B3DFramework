//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanFramebuffer.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"

namespace BansheeEngine
{
	UINT32 VulkanFramebuffer::sNextValidId = 1;

	VulkanFramebuffer::VulkanFramebuffer(VulkanResourceManager* owner, const VULKAN_FRAMEBUFFER_DESC& desc)
		: VulkanResource(owner, false), mNumAttachments(0), mNumColorAttachments(0), mHasDepth(false)
		, mSampleFlags(VK_SAMPLE_COUNT_1_BIT)
	{
		mId = sNextValidId++;

		// Create render state
		VkAttachmentDescription attachments[BS_MAX_MULTIPLE_RENDER_TARGETS + 1];
		VkImageView attachmentViews[BS_MAX_MULTIPLE_RENDER_TARGETS + 1];
		VkAttachmentReference colorReferences[BS_MAX_MULTIPLE_RENDER_TARGETS];
		VkAttachmentReference depthReference;

		mSampleFlags = VulkanUtility::getSampleFlags(desc.numSamples);

		UINT32 attachmentIdx = 0;
		for(UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			if (desc.color[i].view == VK_NULL_HANDLE)
				continue;

			VkAttachmentDescription& attachmentDesc = attachments[attachmentIdx];
			attachmentDesc.flags = 0;
			attachmentDesc.format = desc.color[i].format;
			attachmentDesc.samples = mSampleFlags;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			// If offscreen we make the assumption the surface will be read by a shader
			if(desc.offscreen)
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			else
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference& ref = colorReferences[attachmentIdx];
			ref.attachment = attachmentIdx;
			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachmentViews[attachmentIdx] = desc.color[i].view;
;			attachmentIdx++;
		}

		mNumColorAttachments = attachmentIdx;
		mHasDepth = desc.depth.view != VK_NULL_HANDLE;

		if (mHasDepth)
		{
			VkAttachmentDescription& attachmentDesc = attachments[attachmentIdx];
			attachmentDesc.flags = 0;
			attachmentDesc.format = desc.depth.format;
			attachmentDesc.samples = mSampleFlags;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference& ref = depthReference;
			ref.attachment = attachmentIdx;
			ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			attachmentViews[attachmentIdx] = desc.depth.view;
			attachmentIdx++;
		}

		mNumAttachments = attachmentIdx;

		VkSubpassDescription subpassDesc;
		subpassDesc.flags = 0;
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.colorAttachmentCount = mNumColorAttachments;
		subpassDesc.inputAttachmentCount = 0;
		subpassDesc.pInputAttachments = nullptr;
		subpassDesc.preserveAttachmentCount = 0;
		subpassDesc.pPreserveAttachments = nullptr;
		subpassDesc.pResolveAttachments = nullptr;

		if (mNumColorAttachments > 0)
			subpassDesc.pColorAttachments = colorReferences;
		else
			subpassDesc.pColorAttachments = nullptr;

		if (mHasDepth)
			subpassDesc.pDepthStencilAttachment = &depthReference;
		else
			subpassDesc.pDepthStencilAttachment = nullptr;

		// Subpass dependencies for layout transitions
		VkSubpassDependency dependencies[2];

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Note: Do we really need read access?
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // Note: Is this really required?

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;// Note: Is this really required?

		VkRenderPassCreateInfo renderPassCI;
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.pNext = nullptr;
		renderPassCI.flags = 0;
		renderPassCI.attachmentCount = mNumAttachments;
		renderPassCI.pAttachments = attachments;
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpassDesc;
		renderPassCI.dependencyCount = 2;
		renderPassCI.pDependencies = dependencies;

		VkDevice device = mOwner->getDevice().getLogical();

		VkResult result = vkCreateRenderPass(device, &renderPassCI, gVulkanAllocator, &mRenderPass);
		assert(result == VK_SUCCESS);

		// Create frame buffer
		VkFramebufferCreateInfo framebufferCI;
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.pNext = nullptr;
		framebufferCI.flags = 0;
		framebufferCI.renderPass = mRenderPass;
		framebufferCI.attachmentCount = mNumAttachments;
		framebufferCI.pAttachments = attachmentViews;
		framebufferCI.width = desc.width;
		framebufferCI.height = desc.height;
		framebufferCI.layers = desc.layers;

		result = vkCreateFramebuffer(device, &framebufferCI, gVulkanAllocator, &mFramebuffer);
		assert(result == VK_SUCCESS);
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		VkDevice device = mOwner->getDevice().getLogical();

		vkDestroyFramebuffer(device, mFramebuffer, gVulkanAllocator);
		vkDestroyRenderPass(device, mRenderPass, gVulkanAllocator);
	}
}