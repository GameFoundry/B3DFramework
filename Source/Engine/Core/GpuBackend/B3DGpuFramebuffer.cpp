//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuFramebuffer.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Utility/B3DBitwise.h"

namespace b3d::render
{
	GpuFramebufferAttachment::GpuFramebufferAttachment(IGpuImageResource& image, const GpuTextureSubresourceRange& range, RenderSurfaceMaskBits surface, TOptional<GpuImageLayout> finalLayout)
		: Image(&image), Range(range), Surface(surface), FinalLayout(finalLayout)
	{ }

	bool GpuFramebufferAttachment::IsColor() const
	{
		return ((u32)Surface & (u32)RT_COLOR_ALL) != 0;
	}

	u32 GpuFramebufferAttachment::GetIndex() const
	{
		return IsColor() ? Bitwise::LeastSignificantBit((u32)Surface) : 0;
	}

	GpuFramebuffer::GpuFramebuffer(u32 width, u32 height, u32 layerCount)
		: mWidth(width), mHeight(height), mLayerCount(layerCount)
	{ }

	TArrayView<const GpuFramebufferAttachment> GpuFramebuffer::GetColorAttachments() const
	{
		return TArrayView<const GpuFramebufferAttachment>(mAttachments.data(), mColorAttachmentCount);
	}

	const GpuFramebufferAttachment* GpuFramebuffer::FindAttachment(RenderSurfaceMaskBits surface) const
	{
		for(const GpuFramebufferAttachment& attachment : mAttachments)
		{
			if(attachment.Surface == surface)
				return &attachment;
		}

		return nullptr;
	}

	GpuRenderPassAttachmentUsageArray GpuFramebuffer::BuildRenderPassAttachmentUsages(RenderSurfaceMask readOnlyMask) const
	{
		GpuRenderPassAttachmentUsageArray output;
		for(const GpuFramebufferAttachment& attachment : mAttachments)
		{
			const bool readOnly = readOnlyMask.IsSet(attachment.Surface);

			GpuRenderPassAttachmentUsage usage;
			usage.Image = attachment.Image;
			usage.Range = attachment.Range;
			usage.Surface = attachment.Surface;
			usage.Access = readOnly ? GpuAccessFlags(GpuAccessFlag::Read) : GpuAccessFlag::Read | GpuAccessFlag::Write; // Writable attachments can also read through loads, blending and depth/stencil tests.
			usage.Layout = GpuBackendUtility::GetRenderPassAttachmentLayout(attachment.Surface, readOnlyMask);
			usage.FinalLayout = attachment.FinalLayout;

			output.Add(std::move(usage));
		}

		return output;
	}

	void GpuFramebuffer::AddColorAttachment(IGpuImageResource& image, const GpuTextureSubresourceRange& range, u32 colorIndex, TOptional<GpuImageLayout> finalLayout)
	{
		B3D_ASSERT(colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT);
		B3D_ASSERT(range.AspectMask == GpuTextureAspectFlag::Color);
		B3D_ASSERT(mAttachments.Size() == mColorAttachmentCount);

		mAttachments.Add(GpuFramebufferAttachment(image, range, (RenderSurfaceMaskBits)(RT_COLOR0 << colorIndex), finalLayout));
		mColorAttachmentCount++;
	}

	void GpuFramebuffer::AddDepthStencilAttachment(IGpuImageResource& image, const GpuTextureSubresourceRange& range, TOptional<GpuImageLayout> finalLayout)
	{
		if(range.AspectMask.IsSet(GpuTextureAspectFlag::Depth))
		{
			GpuTextureSubresourceRange depthRange = range;
			depthRange.AspectMask = GpuTextureAspectFlag::Depth;

			mAttachments.Add(GpuFramebufferAttachment(image, depthRange, RT_DEPTH, finalLayout));
		}

		if(range.AspectMask.IsSet(GpuTextureAspectFlag::Stencil))
		{
			GpuTextureSubresourceRange stencilRange = range;
			stencilRange.AspectMask = GpuTextureAspectFlag::Stencil;

			mAttachments.Add(GpuFramebufferAttachment(image, stencilRange, RT_STENCIL, finalLayout));
		}
	}
} // namespace b3d::render
