//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanRenderPass.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"

using namespace b3d;
using namespace b3d::render;

VulkanRenderPass::VariantKey::VariantKey(RenderSurfaceMask loadMask, RenderSurfaceMask readMask, RenderSurfaceMask clearMask)
	: LoadMask(loadMask), ReadMask(readMask), ClearMask(clearMask)
{}

size_t VulkanRenderPass::VariantKey::HashFunction::operator()(const VariantKey& v) const
{
	size_t hash = 0;
	B3DCombineHash(hash, v.ReadMask);
	B3DCombineHash(hash, v.LoadMask);
	B3DCombineHash(hash, v.ClearMask);

	return hash;
}

bool VulkanRenderPass::VariantKey::EqualFunction::operator()(
	const VariantKey& lhs, const VariantKey& rhs) const
{
	return lhs.LoadMask == rhs.LoadMask && lhs.ReadMask == rhs.ReadMask && lhs.ClearMask == rhs.ClearMask;
}

size_t VulkanRenderPassAttachmentCreateInformation::CalculateCompatibilityHash(bool isDepth) const
{
	size_t hash = 0;
	B3DCombineHash(hash, IsEnabled);
	B3DCombineHash(hash, Format);

	if(!isDepth)
		B3DCombineHash(hash, IsShaderReadAllowed);

	return hash;
}

bool VulkanRenderPassAttachmentCreateInformation::IsCompatible(bool isDepth, const VulkanRenderPassAttachmentCreateInformation& other) const
{
	return IsEnabled == other.IsEnabled && Format == other.Format && (isDepth || IsShaderReadAllowed == other.IsShaderReadAllowed);
}

size_t VulkanRenderPassCreateInformation::CalculateCompatibilityHash() const
{
	size_t hash = 0;
	B3DCombineHash(hash, IsOffscreenSurface);
	B3DCombineHash(hash, SampleCount);

	for(u32 i = 0; i < B3DSize(ColorAttachments); i++)
	{
		B3DCombineHash(hash, ColorAttachments[i].CalculateCompatibilityHash(false));
	}

	B3DCombineHash(hash, DepthAttachment.CalculateCompatibilityHash(true));

	return hash;
}

bool VulkanRenderPassCreateInformation::IsCompatible(const VulkanRenderPassCreateInformation& other) const
{
	if(IsOffscreenSurface != other.IsOffscreenSurface || SampleCount != other.SampleCount)
		return false;

	for(u32 i = 0; i < B3DSize(ColorAttachments); i++)
	{
		if(!ColorAttachments[i].IsCompatible(false, other.ColorAttachments[i]))
			return false;
	}

	if(!DepthAttachment.IsCompatible(true, other.DepthAttachment))
		return false;

	return true;
}


u32 VulkanRenderPass::sNextValidId = 1;

VulkanRenderPass::VulkanRenderPass(const VkDevice& device, const VulkanRenderPassCreateInformation& createInformation)
	: mDevice(device)
{
	mId = sNextValidId++;
	mSampleFlags = VulkanUtility::GetSampleFlags(createInformation.SampleCount);

	u32 sequentialAttachmentIndex = 0;
	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		const VulkanRenderPassAttachmentCreateInformation& attachmentCreateInformation = createInformation.ColorAttachments[i];

		VkAttachmentReference& vkAttachmentReference = mColorReferences[sequentialAttachmentIndex];

		if(attachmentCreateInformation.IsEnabled)
		{
			VkAttachmentDescription& vkAttachmentDescription = mAttachments[sequentialAttachmentIndex];
			vkAttachmentDescription.flags = 0;
			vkAttachmentDescription.format = attachmentCreateInformation.Format;
			vkAttachmentDescription.samples = mSampleFlags;
			vkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vkAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			vkAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vkAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			vkAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			if(createInformation.IsOffscreenSurface)
				mColorAttachmentFinalLayouts[sequentialAttachmentIndex] = GpuImageLayout::ColorAttachment;
			else
				mColorAttachmentFinalLayouts[sequentialAttachmentIndex] = GpuImageLayout::Present;

			vkAttachmentDescription.finalLayout = VulkanUtility::GetVulkanImageLayout(mColorAttachmentFinalLayouts[sequentialAttachmentIndex]);

			vkAttachmentReference.attachment = sequentialAttachmentIndex;
			vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			mColorAttachmentSequentialToAttachmentIndexMap[sequentialAttachmentIndex] = i;
			mIsShaderReadAllowedForColorAttachment[i] = attachmentCreateInformation.IsShaderReadAllowed;

			mMaximumColorAttachmentIndex = i;
			sequentialAttachmentIndex++;
		}
		else
		{
			vkAttachmentReference.attachment = VK_ATTACHMENT_UNUSED;
			vkAttachmentReference.layout = VK_IMAGE_LAYOUT_UNDEFINED;

			mIsShaderReadAllowedForColorAttachment[i] = false;
		}
	}

	mColorAttachmentCount = sequentialAttachmentIndex;
	mHasDepthAttachment = createInformation.DepthAttachment.IsEnabled;

	if(mHasDepthAttachment)
	{
		VkAttachmentDescription& vkAttachmentDescription = mAttachments[sequentialAttachmentIndex];
		vkAttachmentDescription.flags = 0;
		vkAttachmentDescription.format = createInformation.DepthAttachment.Format;
		vkAttachmentDescription.samples = mSampleFlags;
		vkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		vkAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		vkAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		mDepthAttachmentFinalLayout = GpuImageLayout::DepthStencilAttachment;
		vkAttachmentDescription.finalLayout = VulkanUtility::GetVulkanImageLayout(mDepthAttachmentFinalLayout);

		mDepthReference.attachment = sequentialAttachmentIndex;
		mDepthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		sequentialAttachmentIndex++;
	}

	mAttachmentCount = sequentialAttachmentIndex;

	mSubpassDescription.flags = 0;
	mSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	mSubpassDescription.colorAttachmentCount = mColorAttachmentCount > 0 ? (mMaximumColorAttachmentIndex + 1) : 0;
	mSubpassDescription.inputAttachmentCount = 0;
	mSubpassDescription.pInputAttachments = nullptr;
	mSubpassDescription.preserveAttachmentCount = 0;
	mSubpassDescription.pPreserveAttachments = nullptr;
	mSubpassDescription.pResolveAttachments = nullptr;
	mSubpassDescription.pColorAttachments = mColorReferences;

	if(mHasDepthAttachment)
		mSubpassDescription.pDepthStencilAttachment = &mDepthReference;
	else
		mSubpassDescription.pDepthStencilAttachment = nullptr;

	// No external memory barriers, we handle them explicitly
	mDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	mDependencies[0].dstSubpass = 0;
	mDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	mDependencies[0].srcAccessMask = 0;
	mDependencies[0].dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	mDependencies[0].dstAccessMask = 0;
	mDependencies[0].dependencyFlags = 0;

	mDependencies[1].srcSubpass = 0;
	mDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	mDependencies[1].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	mDependencies[1].srcAccessMask = 0;
	mDependencies[1].dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	mDependencies[1].dstAccessMask = 0;
	mDependencies[1].dependencyFlags = 0;

	// Any other use-case other than those above require an explicit barrier

	// Create render pass and frame buffer create infos
	mRenderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	mRenderPassCI.pNext = nullptr;
	mRenderPassCI.flags = 0;
	mRenderPassCI.attachmentCount = mAttachmentCount;
	mRenderPassCI.pAttachments = mAttachments;
	mRenderPassCI.subpassCount = 1;
	mRenderPassCI.pSubpasses = &mSubpassDescription;
	mRenderPassCI.dependencyCount = 2;
	mRenderPassCI.pDependencies = mDependencies;

	mDefault = CreateVariant(RT_NONE, RT_NONE, RT_NONE);
}

VulkanRenderPass::~VulkanRenderPass()
{
	Lock lock(mMutex);

	vkDestroyRenderPass(mDevice, mDefault, gVulkanAllocator);

	for(auto& entry : mVariants)
		vkDestroyRenderPass(mDevice, entry.second, gVulkanAllocator);
}

VkRenderPass VulkanRenderPass::CreateVariant(RenderSurfaceMask loadMask, RenderSurfaceMask readMask, RenderSurfaceMask clearMask) const
{
	for(u32 sequentialAttachmentIndex = 0; sequentialAttachmentIndex < mColorAttachmentCount; sequentialAttachmentIndex++)
	{
		const u32 attachmentIndex = mColorAttachmentSequentialToAttachmentIndexMap[sequentialAttachmentIndex];

		VkAttachmentDescription& vkAttachmentDescription = mAttachments[sequentialAttachmentIndex];
		VkAttachmentReference& vkAttachmentReference = mColorReferences[attachmentIndex];

		if(loadMask.IsSet((RenderSurfaceMaskBits)(1 << attachmentIndex)))
		{
			vkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			vkAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else if(clearMask.IsSet((RenderSurfaceMaskBits)(1 << attachmentIndex)))
		{
			vkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			vkAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		else
		{
			vkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vkAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		if(readMask.IsSet((RenderSurfaceMaskBits)(1 << attachmentIndex)))
			vkAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
		else
			vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if(mHasDepthAttachment)
	{
		VkAttachmentDescription& vkAttachmentDescription = mAttachments[mColorAttachmentCount];
		VkAttachmentReference& vkAttachmentReference = mDepthReference;

		if(loadMask.IsSet(RT_DEPTH) || loadMask.IsSet(RT_STENCIL))
		{
			vkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			vkAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			vkAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			if(clearMask.IsSet(RT_DEPTH))
				vkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			else
				vkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

			if(clearMask.IsSet(RT_STENCIL))
				vkAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			else
				vkAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

			vkAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		// When depth-stencil is readable it's up to the caller to ensure he doesn't try to write to it as well, so we
		// just assume a read-only layout.
		if(readMask.IsSet(RT_DEPTH))
		{
			if(readMask.IsSet(RT_STENCIL))
				vkAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			else // Depth readable but stencil isn't
				vkAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR;
		}
		else
		{
			if(readMask.IsSet(RT_STENCIL)) // Stencil readable but depth isn't
				vkAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;
			else
				vkAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
	}

	VkRenderPass output;
	VkResult result = vkCreateRenderPass(mDevice, &mRenderPassCI, gVulkanAllocator, &output);
	B3D_ASSERT(result == VK_SUCCESS);

	return output;
}

VkRenderPass VulkanRenderPass::GetVkRenderPass(RenderSurfaceMask loadMask, RenderSurfaceMask readMask, RenderSurfaceMask clearMask) const
{
	if(loadMask == RT_NONE && readMask == RT_NONE && clearMask == RT_NONE)
		return mDefault;

	Lock lock(mMutex);
	VariantKey key(loadMask, readMask, clearMask);
	auto iterFind = mVariants.find(key);
	if(iterFind != mVariants.end())
		return iterFind->second;

	VkRenderPass newVariant = CreateVariant(loadMask, readMask, clearMask);
	mVariants[key] = newVariant;

	return newVariant;
}

u32 VulkanRenderPass::GetClearEntryCount(RenderSurfaceMask clearMask) const
{
	if(clearMask == RT_NONE)
		return 0;
	else if(clearMask == RT_ALL)
		return GetAttachmentCount();
	else if(((u32)clearMask & (u32)(RT_DEPTH | RT_STENCIL)) != 0 && HasDepthAttachment())
		return GetAttachmentCount();

	u32 attachmentCount = 0;
	for(i32 i = B3D_MAXIMUM_RENDER_TARGET_COUNT - 1; i >= 0; i--)
	{
		if(((1 << i) & (u32)clearMask) != 0)
		{
			attachmentCount = i + 1;
			break;
		}
	}

	return std::min(attachmentCount, GetColorAttachmentCount());
}

VulkanRenderPassCache::~VulkanRenderPassCache()
{
	for(auto& entry : mVariants)
		B3DDelete(entry.second);
}

VulkanRenderPass* VulkanRenderPassCache::FindOrCreateRenderPass(const VkDevice& device, const VulkanRenderPassCreateInformation& desc)
{
	VariantKey key(device, desc);

	VulkanRenderPass* pass;
	{
		Lock lock(mMutex);

		auto iterFind = mVariants.find(key);
		if(iterFind != mVariants.end())
			return iterFind->second;

		pass = B3DNew<VulkanRenderPass>(device, desc);
		mVariants[key] = pass;
	}

	return pass;
}

VulkanRenderPassCache::VariantKey::VariantKey(const VkDevice& device, const VulkanRenderPassCreateInformation& createInformation)
	: Device(device), CreateInformation(createInformation)
{}

size_t VulkanRenderPassCache::VariantKey::HashFunction::operator()(const VariantKey& v) const
{
	size_t hash = 0;
	B3DCombineHash(hash, v.Device);
	B3DCombineHash(hash, v.CreateInformation.CalculateCompatibilityHash());

	return hash;
}

bool VulkanRenderPassCache::VariantKey::EqualFunction::operator()(const VariantKey& lhs, const VariantKey& rhs) const
{
	return lhs.Device == rhs.Device && lhs.CreateInformation.IsCompatible(rhs.CreateInformation);
}

