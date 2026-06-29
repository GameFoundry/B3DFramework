//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanResourceTracker.h"

#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanSwapChain.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanUtility.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Allocators/B3DFrameAllocator.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DVulkanBarrierHelper.h"

// Generic tracker method definitions, followed by the explicit instantiation for the Vulkan barrier helper. Included
// here (after the complete VulkanBarrierHelper, GpuBackendUtility and frame allocator are available) so the single
// instantiation lives in this translation unit. The header carries a matching `extern template` to suppress implicit
// instantiation elsewhere.
#include "GpuBackend/B3DGpuResourceTracker.inl"

template class b3d::render::TGpuResourceTracker<b3d::render::VulkanBarrierHelper>;

using namespace b3d;
using namespace b3d::render;

void VulkanResourceTracker::TrackFramebufferUsage(VulkanFramebuffer* framebuffer, RenderSurfaceMask loadMask, RenderSurfaceMask readOnlyMask, VulkanBarrierHelper& barrierHelper)
{
	auto insertResult = mResources.insert(std::make_pair(framebuffer, GpuResourceUseHandle()));
	if(insertResult.second) // New element
	{
		GpuResourceUseHandle& useHandle = insertResult.first->second;
		useHandle.Used = false;
		useHandle.Flags = GpuAccessFlag::Write;

		framebuffer->NotifyBound();
	}
	else // Existing element
	{
		GpuResourceUseHandle& useHandle = insertResult.first->second;

		B3D_ASSERT(!useHandle.Used);
		useHandle.Flags |= GpuAccessFlag::Write;
	}

	// Register any sub-resources
	VulkanRenderPass* renderPass = framebuffer->GetRenderPass();
	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& attachment = framebuffer->GetColorAttachment(colorAttachmentIndex);

		// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter. We're using
		// these values because that's what VulkanFramebuffer expects as initialLayout.
		GpuImageLayout layout;
		if(loadMask.IsSet((RenderSurfaceMaskBits)(1 << colorAttachmentIndex)))
			layout = GpuImageLayout::ColorAttachment;
		else
			layout = GpuImageLayout::Undefined;

		GpuAccessFlag access = readOnlyMask.IsSet((RenderSurfaceMaskBits)(1 << colorAttachmentIndex)) ? GpuAccessFlag::Read : GpuAccessFlag::Write;

		GpuTextureSubresourceRange range = attachment.Image->GetRange(attachment.Surface);
		TrackImageUsage(attachment.Image, range, layout, attachment.FinalLayout, GpuResourceUseFlag::ColorAttachment, access, barrierHelper);
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& attachment = framebuffer->GetDepthStencilAttachment();

		// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter. We're using
		// these values because that's what VulkanFramebuffer expects as initialLayout.
		GpuImageLayout layout;
		if(loadMask.IsSet(RT_DEPTH) || loadMask.IsSet(RT_STENCIL)) // Can't load one without the other
			layout = GpuImageLayout::DepthStencilAttachment;
		else
			layout = GpuImageLayout::Undefined;

		// Note: We purposefully don't check read-only stencil here as generally access tracking doesn't matter for it, as it's always an attachment and shader can't read/write it directly
		const GpuAccessFlag access = readOnlyMask.IsSet(RT_DEPTH) ? GpuAccessFlag::Read : GpuAccessFlag::Write;

		GpuTextureSubresourceRange range = attachment.Image->GetRange(attachment.Surface);
		TrackImageUsage(attachment.Image, range, layout, attachment.FinalLayout, GpuResourceUseFlag::DepthStencilAttachment, access, barrierHelper);
	}
}

void VulkanResourceTracker::ClearFramebufferFlagsForImage(VulkanImage* image)
{
	const u32 imageTrackingIndex = mImages[image];
	GpuImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

	GpuImageSubresourceTrackingState* const subresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];
	for(u32 localSubresourceIndex = 0; localSubresourceIndex < imageTrackingState.SubresourceInfoCount; localSubresourceIndex++)
	{
		GpuImageSubresourceTrackingState& subresourceTrackingState = subresourceTrackingStates[localSubresourceIndex];
		subresourceTrackingState.FramebufferUse = GpuAccessFlag::None;
	}
}

void VulkanResourceTracker::ClearShaderFlagsForAllRenderPassImageSubresources()
{
	for(const auto& subresourceIndex : mRenderPassSubresources)
	{
		GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState[subresourceIndex];
		subresourceTrackingState.ShaderUse = GpuAccessFlag::None;
	}

	mRenderPassSubresources.clear();
}

GpuImageLayout VulkanResourceTracker::GetCurrentSubresourceLayout(VulkanImage* image, const GpuTextureSubresourceRange& range, VulkanFramebuffer* framebuffer, RenderSurfaceMask explicitReadOnlyMask) const
{
	const u32 face = range.BaseArrayLayer;
	const u32 mip = range.BaseMipLevel;

#if B3D_BUILD_TYPE_DEVELOPMENT
	const GpuImageTrackingState* const imageTrackingState = FindImageTrackingState(image);
	if(imageTrackingState == nullptr)
	{
		B3D_ASSERT(false);
		return GpuImageLayout::Undefined;
	}
#endif

	VulkanRenderPass* renderPass = nullptr;
	if(framebuffer != nullptr)
		renderPass = framebuffer->GetRenderPass();

	TArrayView<const GpuImageSubresourceTrackingState> subresourceTrackingStates = GetSubresourceTrackingStatesForImage(image);
	for(const auto& subresourceTrackingState : subresourceTrackingStates)
	{
		if(face >= subresourceTrackingState.Range.BaseArrayLayer && face < (subresourceTrackingState.Range.BaseArrayLayer + subresourceTrackingState.Range.ArrayLayerCount) &&
		   mip >= subresourceTrackingState.Range.BaseMipLevel && mip < (subresourceTrackingState.Range.BaseMipLevel + subresourceTrackingState.Range.MipLevelCount))
		{
			// If it's a FB attachment, retrieve its layout after the render pass begins
			if(subresourceTrackingState.FramebufferUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write) && framebuffer != nullptr)
			{
				RenderSurfaceMask readMask = GetFramebufferReadOnlyMask(framebuffer, explicitReadOnlyMask);

				// Is it a depth-stencil attachment?
				if(renderPass->HasDepthAttachment() && framebuffer->GetDepthStencilAttachment().Image == image)
				{
					if(readMask.IsSet(RT_DEPTH))
					{
						if(readMask.IsSet(RT_STENCIL))
							return GpuImageLayout::DepthStencilReadOnly;
						else // Depth readable but stencil isn't
							return GpuImageLayout::DepthReadOnlyStencilAttachment;
					}
					else
					{
						if(readMask.IsSet(RT_STENCIL)) // Stencil readable but depth isn't
							return GpuImageLayout::DepthAttachmentStencilReadOnly;
						else
							return GpuImageLayout::DepthStencilAttachment;
					}
				}
				else // It is a color attachment
				{
					const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
					for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
					{
						const VulkanFramebufferAttachment& attachment = framebuffer->GetColorAttachment(colorAttachmentIndex);

						if(attachment.Image == image)
						{
							if(readMask.IsSet((RenderSurfaceMaskBits)(1 << attachment.Index)))
								return GpuImageLayout::General;
							else
								return GpuImageLayout::ColorAttachment;
						}
					}
				}
			}

			return subresourceTrackingState.RequiredLayout;
		}
	}

	B3D_ASSERT(false);
	return GpuImageLayout::Undefined;
}

RenderSurfaceMask VulkanResourceTracker::GetFramebufferReadOnlyMask(VulkanFramebuffer* framebuffer, RenderSurfaceMask explicitReadOnlyMask) const
{
	// Check if any frame-buffer attachments are also used as shader inputs, in which case we make them read-only
	VulkanRenderPass* const renderPass = framebuffer->GetRenderPass();
	RenderSurfaceMask readMask = RT_NONE;

	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetColorAttachment(colorAttachmentIndex);
		const GpuImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		const bool readOnly = subresourceTrackingState.ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write); // Note: Should report error if shader write is used

		if(readOnly)
			readMask.Set((RenderSurfaceMaskBits)(1 << colorAttachmentIndex));
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetDepthStencilAttachment();
		const GpuImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		const bool readOnly = subresourceTrackingState.ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write); // Note: Should report error if shader write is used

		if(readOnly)
			readMask.Set(RT_DEPTH);

		if(explicitReadOnlyMask.IsSet(RT_DEPTH))
			readMask.Set(RT_DEPTH);

		if(explicitReadOnlyMask.IsSet(RT_STENCIL))
			readMask.Set(RT_STENCIL);
	}

	return readMask;
}

void VulkanResourceTracker::MoveAllFramebufferAttachmentsToFinalLayouts(VulkanFramebuffer* framebuffer)
{
	const VulkanRenderPass* const renderPass = framebuffer->GetRenderPass();
	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetColorAttachment(colorAttachmentIndex);
		GpuImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		subresourceTrackingState.CurrentLayout = subresourceTrackingState.RenderPassLayout;
		subresourceTrackingState.RequiredLayout = subresourceTrackingState.RenderPassLayout;
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetDepthStencilAttachment();
		GpuImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		subresourceTrackingState.CurrentLayout = subresourceTrackingState.RenderPassLayout;
		subresourceTrackingState.RequiredLayout = subresourceTrackingState.RenderPassLayout;
	}
}
