//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanFramebuffer.h"
#include "BsVulkanTexture.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"

namespace bs
{
	VulkanFramebuffer::VariantKey::VariantKey(RenderSurfaceMask loadMask, RenderSurfaceMask readMask)
		:loadMask(loadMask), readMask(readMask)
	{ }

	size_t VulkanFramebuffer::VariantKey::HashFunction::operator()(const VariantKey& v) const
	{
		size_t hash = 0;
		hash_combine(hash, v.readMask);
		hash_combine(hash, v.loadMask);

		return hash;
	}

	bool VulkanFramebuffer::VariantKey::EqualFunction::operator()(const VariantKey& lhs,
																			 const VariantKey& rhs) const
	{
		return lhs.loadMask == rhs.loadMask && lhs.readMask == rhs.readMask;
	}

	UINT32 VulkanFramebuffer::sNextValidId = 1;

	VulkanFramebuffer::VulkanFramebuffer(VulkanResourceManager* owner, const VULKAN_FRAMEBUFFER_DESC& desc)
		: VulkanResource(owner, false), mNumAttachments(0), mNumColorAttachments(0), mNumLayers(desc.layers)
		, mColorImages(), mDepthStencilImage(nullptr), mColorBaseLayers(), mDepthBaseLayer(0), mHasDepth(false)
		, mSampleFlags(VK_SAMPLE_COUNT_1_BIT)
	{
		mId = sNextValidId++;

		mSampleFlags = VulkanUtility::getSampleFlags(desc.numSamples);

		UINT32 attachmentIdx = 0;
		for(UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			if (desc.color[i].image == nullptr)
				continue;

			VkAttachmentDescription& attachmentDesc = mAttachments[attachmentIdx];
			attachmentDesc.flags = 0;
			attachmentDesc.format = desc.color[i].format;
			attachmentDesc.samples = mSampleFlags;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			// If offscreen we make the assumption the surface will be read by a shader
			if(desc.offscreen)
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			else
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			mColorBaseLayers[attachmentIdx] = desc.color[i].baseLayer;
			mColorImages[attachmentIdx] = desc.color[i].image;

			VkAttachmentReference& ref = mColorReferences[attachmentIdx];
			ref.attachment = attachmentIdx;
			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			mAttachmentViews[attachmentIdx] = desc.color[i].view;
			attachmentIdx++;
		}

		mNumColorAttachments = attachmentIdx;
		mHasDepth = desc.depth.view != VK_NULL_HANDLE;

		if (mHasDepth)
		{
			VkAttachmentDescription& attachmentDesc = mAttachments[attachmentIdx];
			attachmentDesc.flags = 0;
			attachmentDesc.format = desc.depth.format;
			attachmentDesc.samples = mSampleFlags;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			mDepthBaseLayer = desc.depth.baseLayer;
			mDepthStencilImage = desc.depth.image;

			VkAttachmentReference& ref = mDepthReference;
			ref.attachment = attachmentIdx;
			ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			mAttachmentViews[attachmentIdx] = desc.depth.view;
			attachmentIdx++;
		}

		mNumAttachments = attachmentIdx;

		mSubpassDesc.flags = 0;
		mSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		mSubpassDesc.colorAttachmentCount = mNumColorAttachments;
		mSubpassDesc.inputAttachmentCount = 0;
		mSubpassDesc.pInputAttachments = nullptr;
		mSubpassDesc.preserveAttachmentCount = 0;
		mSubpassDesc.pPreserveAttachments = nullptr;
		mSubpassDesc.pResolveAttachments = nullptr;

		if (mNumColorAttachments > 0)
			mSubpassDesc.pColorAttachments = mColorReferences;
		else
			mSubpassDesc.pColorAttachments = nullptr;

		if (mHasDepth)
			mSubpassDesc.pDepthStencilAttachment = &mDepthReference;
		else
			mSubpassDesc.pDepthStencilAttachment = nullptr;

		// Subpass dependencies for layout transitions
		mDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		mDependencies[0].dstSubpass = 0;
		mDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		mDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		mDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		mDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		mDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // Note: Is this really required?

		mDependencies[1].srcSubpass = 0;
		mDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		mDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		mDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		mDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		mDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		mDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;// Note: Is this really required?

		// Create render pass and frame buffer create infos
		mRenderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		mRenderPassCI.pNext = nullptr;
		mRenderPassCI.flags = 0;
		mRenderPassCI.attachmentCount = mNumAttachments;
		mRenderPassCI.pAttachments = mAttachments;
		mRenderPassCI.subpassCount = 1;
		mRenderPassCI.pSubpasses = &mSubpassDesc;
		mRenderPassCI.dependencyCount = 2;
		mRenderPassCI.pDependencies = mDependencies;

		mFramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		mFramebufferCI.pNext = nullptr;
		mFramebufferCI.flags = 0;
		mFramebufferCI.renderPass = VK_NULL_HANDLE;
		mFramebufferCI.attachmentCount = mNumAttachments;
		mFramebufferCI.pAttachments = mAttachmentViews;
		mFramebufferCI.width = desc.width;
		mFramebufferCI.height = desc.height;
		mFramebufferCI.layers = desc.layers;

		mDefault = createVariant(RT_NONE, RT_NONE);		
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		VkDevice device = mOwner->getDevice().getLogical();

		vkDestroyFramebuffer(device, mDefault.framebuffer, gVulkanAllocator);
		vkDestroyRenderPass(device, mDefault.renderPass, gVulkanAllocator);

		for(auto& entry : mVariants)
		{
			vkDestroyFramebuffer(device, entry.second.framebuffer, gVulkanAllocator);
			vkDestroyRenderPass(device, entry.second.renderPass, gVulkanAllocator);
		}
	}

	VulkanFramebuffer::Variant VulkanFramebuffer::createVariant(RenderSurfaceMask loadMask, 
		RenderSurfaceMask readMask) const
	{
		for (UINT32 i = 0; i < mNumColorAttachments; i++)
		{
			VkAttachmentDescription& attachmentDesc = mAttachments[i];
			VkAttachmentReference& attachmentRef = mColorReferences[i];

			if (loadMask.isSet((RenderSurfaceMaskBits)i))
			{
				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			else
			{
				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			}

			if(readMask.isSet((RenderSurfaceMaskBits)i))
				attachmentRef.layout = VK_IMAGE_LAYOUT_GENERAL;
			else
				attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		if (mHasDepth)
		{
			VkAttachmentDescription& attachmentDesc = mAttachments[mNumColorAttachments];
			VkAttachmentReference& attachmentRef = mDepthReference;

			if (loadMask.isSet(RT_DEPTH))
			{
				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			else
			{
				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			}

			if (readMask.isSet(RT_DEPTH))
				attachmentRef.layout = VK_IMAGE_LAYOUT_GENERAL;
			else
				attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkDevice device = mOwner->getDevice().getLogical();

		Variant variant;
		VkResult result = vkCreateRenderPass(device, &mRenderPassCI, gVulkanAllocator, &variant.renderPass);
		assert(result == VK_SUCCESS);

		mFramebufferCI.renderPass = variant.renderPass;

		result = vkCreateFramebuffer(device, &mFramebufferCI, gVulkanAllocator, &variant.framebuffer);
		assert(result == VK_SUCCESS);

		return variant;
	}

	VkRenderPass VulkanFramebuffer::getRenderPass(RenderSurfaceMask loadMask, RenderSurfaceMask readMask) const
	{
		if (loadMask == RT_NONE && readMask == RT_NONE)
			return mDefault.renderPass;

		VariantKey key(loadMask, readMask);
		auto iterFind = mVariants.find(key);
		if (iterFind != mVariants.end())
			return iterFind->second.renderPass;

		Variant newVariant = createVariant(loadMask, readMask);
		mVariants[key] = newVariant;

		return newVariant.renderPass;
	}

	VkFramebuffer VulkanFramebuffer::getFramebuffer(RenderSurfaceMask loadMask, RenderSurfaceMask readMask) const
	{
		if (loadMask == RT_NONE && readMask == RT_NONE)
			return mDefault.framebuffer;

		VariantKey key(loadMask, readMask);
		auto iterFind = mVariants.find(key);
		if (iterFind != mVariants.end())
			return iterFind->second.framebuffer;

		Variant newVariant = createVariant(loadMask, readMask);
		mVariants[key] = newVariant;

		return newVariant.framebuffer;
	}
}