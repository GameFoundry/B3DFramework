//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalFramebuffer.h"
#include "B3DMetalTexture.h"

namespace b3d
{
	namespace render
	{
		MetalFramebuffer::MetalFramebuffer(const MetalFramebufferCreateInformation& createInformation)
			: GpuFramebuffer(createInformation.Width, createInformation.Height)
		{
			auto fnGetImage = [](const MetalFramebufferAttachmentCreateInformation& attachment) -> MetalImage*
			{
				MetalImage* image = attachment.Texture != nullptr ? attachment.Texture->GetMetalResource() : nullptr;
				return image != nullptr && image->GetMetalHandle() != nil ? image : nullptr;
			};

			for (u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
			{
				const MetalFramebufferAttachmentCreateInformation& attachment = createInformation.ColorAttachments[colorIndex];
				MetalImage* image = fnGetImage(attachment);
				if (image == nullptr)
					continue;

				// A 3D texture is a single subresource, its depth slice is selected through the descriptor's depth plane
				const bool is3D = attachment.Texture->GetProperties().Type == TEX_TYPE_3D;
				const u32 face = is3D ? 0 : attachment.Face;
				mColorDepthPlanes[colorIndex] = is3D ? attachment.Face : 0;

				AddColorAttachment(*image, image->GetRange(TextureSurface(attachment.MipLevel, 1, face, 1)), colorIndex);
			}

			const MetalFramebufferAttachmentCreateInformation& depthStencil = createInformation.DepthStencilAttachment;
			MetalImage* depthStencilImage = fnGetImage(depthStencil);
			if (depthStencilImage != nullptr)
				AddDepthStencilAttachment(*depthStencilImage, depthStencilImage->GetRange(TextureSurface(depthStencil.MipLevel, 1, depthStencil.Face, 1)));
		}

		void MetalFramebuffer::ApplyAttachments(MTLRenderPassDescriptor* descriptor) const
		{
			for (const GpuFramebufferAttachment& attachment : GetAttachments())
			{
				id<MTLTexture> texture = static_cast<MetalImage*>(attachment.Image)->GetMetalHandle();
				const u32 mipLevel = attachment.Range.BaseMipLevel;
				const u32 slice = attachment.Range.BaseArrayLayer;

				if (attachment.IsColor())
				{
					const u32 colorIndex = attachment.GetIndex();
					MTLRenderPassColorAttachmentDescriptor* color = descriptor.colorAttachments[colorIndex];
					color.texture = texture;
					color.level = mipLevel;
					color.slice = slice;
					color.depthPlane = mColorDepthPlanes[colorIndex];
				}
				else
				{
					MTLRenderPassAttachmentDescriptor* depthOrStencil = attachment.Surface == RT_DEPTH
						? (MTLRenderPassAttachmentDescriptor*)descriptor.depthAttachment
						: (MTLRenderPassAttachmentDescriptor*)descriptor.stencilAttachment;
					depthOrStencil.texture = texture;
					depthOrStencil.level = mipLevel;
					depthOrStencil.slice = slice;
				}
			}
		}

		const GpuFramebufferLayoutPolicy& MetalFramebuffer::GetLayoutPolicy()
		{
			// Metal has no image layouts, these are only bookkeeping for the resource tracker
			static const GpuFramebufferLayoutPolicy policy(
				GpuRenderPassAttachmentLayout(GpuImageLayout::ColorAttachment),
				GpuRenderPassAttachmentLayout(GpuImageLayout::General, GpuImageLayout::General),
				GpuRenderPassAttachmentLayout(GpuImageLayout::DepthStencilAttachment),
				GpuRenderPassAttachmentLayout(GpuImageLayout::DepthReadOnlyStencilAttachment, GpuImageLayout::DepthReadOnlyStencilAttachment),
				GpuRenderPassAttachmentLayout(GpuImageLayout::DepthAttachmentStencilReadOnly, GpuImageLayout::DepthAttachmentStencilReadOnly),
				GpuRenderPassAttachmentLayout(GpuImageLayout::DepthStencilReadOnly, GpuImageLayout::DepthStencilReadOnly));

			return policy;
		}
	} // namespace render
} // namespace b3d
