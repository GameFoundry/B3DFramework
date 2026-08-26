//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12ResourceTracker.h"

#include "B3DD3D12Framebuffer.h"
#include "B3DD3D12BufferPool.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12Texture.h"
#include "Utility/B3DD3D12BarrierHelper.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Allocators/B3DFrameAllocator.h"

#include "GpuBackend/B3DGpuResourceTracker.inl"

template class b3d::render::TGpuResourceTracker<b3d::render::D3D12BarrierHelper>;

using namespace b3d;
using namespace b3d::render;

void D3D12ResourceTracker::TrackBufferUsage(IGpuBufferResource* buffer, GpuResourceUseFlags useFlags,
	GpuAccessFlags accessFlags, D3D12BarrierHelper& barrierHelper, u32 dynamicOffset)
{
	if(buffer == nullptr)
		return;

	D3D12BufferResource* const d3d12Buffer = static_cast<D3D12BufferResource*>(buffer);
	D3D12BufferPage* const page = d3d12Buffer->GetPage();
	if(page != nullptr && page != buffer && accessFlags.IsSet(GpuAccessFlag::Write))
	{
		TGpuResourceTracker<D3D12BarrierHelper>::TrackBufferUsage(page, useFlags, GpuAccessFlag::Write, barrierHelper);
	}

	TGpuResourceTracker<D3D12BarrierHelper>::TrackBufferUsage(buffer, useFlags, accessFlags, barrierHelper, dynamicOffset);
}

D3D12RenderPassResourceUsage::D3D12RenderPassResourceUsage(const D3D12FramebufferAttachment* attachments, u32 attachmentCount, RenderSurfaceMask readOnlyMask)
{
	for(u32 attachmentIndex = 0; attachmentIndex < attachmentCount; attachmentIndex++)
	{
		const D3D12FramebufferAttachment& attachment = attachments[attachmentIndex];
		const GpuTextureSubresourceRange attachmentRange = attachment.Image->GetRange(attachment.Surface);
		if(!attachment.IsDepthStencil)
		{
			const bool readOnly = readOnlyMask.IsSet((RenderSurfaceMaskBits)(1u << attachment.ColorIndex));

			D3D12RenderPassAttachmentUsage attachmentUsage;
			attachmentUsage.Image = attachment.Image;
			attachmentUsage.Range = attachmentRange;
			attachmentUsage.AttachmentUsage = GpuResourceUseFlag::ColorAttachment;
			attachmentUsage.AttachmentAccess = readOnly ? GpuAccessFlag::Read : GpuAccessFlag::Write;
			attachmentUsage.AttachmentLayout = readOnly ? GpuImageLayout::ShaderReadOnly : GpuImageLayout::ColorAttachment;

			if(readOnly)
				attachmentUsage.ShaderReadLayout = GpuImageLayout::ShaderReadOnly;

			Attachments.Add(std::move(attachmentUsage));
			continue;
		}

		if(attachmentRange.AspectMask.IsSet(GpuTextureAspectFlag::Depth))
		{
			const bool readOnly = readOnlyMask.IsSet(RT_DEPTH);

			D3D12RenderPassAttachmentUsage attachmentUsage;
			attachmentUsage.Image = attachment.Image;
			attachmentUsage.Range = attachmentRange;
			attachmentUsage.Range.AspectMask = GpuTextureAspectFlag::Depth;
			attachmentUsage.AttachmentUsage = GpuResourceUseFlag::DepthStencilAttachment;
			attachmentUsage.AttachmentAccess = readOnly ? GpuAccessFlag::Read : GpuAccessFlag::Write;
			attachmentUsage.AttachmentLayout = GpuImageLayout::DepthStencilAttachment;

			if(readOnly)
				attachmentUsage.ShaderReadLayout = GpuImageLayout::DepthStencilReadOnly;

			Attachments.Add(std::move(attachmentUsage));
		}

		if(attachmentRange.AspectMask.IsSet(GpuTextureAspectFlag::Stencil))
		{
			const bool readOnly = readOnlyMask.IsSet(RT_STENCIL);

			D3D12RenderPassAttachmentUsage attachmentUsage;
			attachmentUsage.Image = attachment.Image;
			attachmentUsage.Range = attachmentRange;
			attachmentUsage.Range.AspectMask = GpuTextureAspectFlag::Stencil;
			attachmentUsage.AttachmentUsage = GpuResourceUseFlag::DepthStencilAttachment;
			attachmentUsage.AttachmentAccess = readOnly ? GpuAccessFlag::Read : GpuAccessFlag::Write;
			attachmentUsage.AttachmentLayout = GpuImageLayout::DepthStencilAttachment;

			if(readOnly)
				attachmentUsage.ShaderReadLayout = GpuImageLayout::DepthStencilReadOnly;

			Attachments.Add(std::move(attachmentUsage));
		}
	}
}

void D3D12ResourceTracker::TrackSampledImageUsage(D3D12Image* image, const GpuTextureSubresourceRange& subresourceRange, GpuResourceUseFlags useFlags, D3D12BarrierHelper& barrierHelper, D3D12RenderPassResourceUsage* renderPassUsage)
{
	GpuTextureSubresourceRange shaderRange = subresourceRange;
	if(shaderRange.AspectMask.IsSet(GpuTextureAspectFlag::Depth))
		shaderRange.AspectMask = GpuTextureAspectFlag::Depth;

	if(renderPassUsage == nullptr)
	{
		TrackImageUsage(image, shaderRange, GpuImageLayout::ShaderReadOnly, GpuImageLayout::ShaderReadOnly, useFlags, GpuAccessFlag::Read, barrierHelper);
		return;
	}

	TInlineArray<GpuTextureSubresourceRange, B3D_MAXIMUM_RENDER_TARGET_COUNT + 2> untrackedRanges;
	untrackedRanges.Add(shaderRange);

	for(D3D12RenderPassAttachmentUsage& attachmentUsage : renderPassUsage->Attachments)
	{
		if(attachmentUsage.Image != image)
			continue;

		for(u32 rangeIndex = 0; rangeIndex < untrackedRanges.Size();)
		{
			const GpuTextureSubresourceRange range = untrackedRanges[rangeIndex];
			if(!GpuBackendUtility::RangeOverlaps(range, attachmentUsage.Range))
			{
				rangeIndex++;
				continue;
			}

			if(!B3D_ENSURE_LOG(attachmentUsage.ShaderReadLayout.has_value(),
				"D3D12 framebuffer attachments sampled during a render pass must be marked read-only."))
			{
				rangeIndex++;
				continue;
			}

			attachmentUsage.ShaderUsage |= useFlags;
			untrackedRanges.Erase(untrackedRanges.Begin() + rangeIndex);

			std::array<GpuTextureSubresourceRange, 5> splitRanges;
			u32 splitRangeCount = 0;
			GpuBackendUtility::CutRange(range, attachmentUsage.Range, splitRanges, splitRangeCount);

			for(u32 splitRangeIndex = 0; splitRangeIndex < splitRangeCount; splitRangeIndex++)
			{
				const GpuTextureSubresourceRange& splitRange = splitRanges[splitRangeIndex];

				if(!GpuBackendUtility::RangeOverlaps(splitRange, attachmentUsage.Range))
					untrackedRanges.Add(splitRange);
			}
		}
	}

	for(const GpuTextureSubresourceRange& range : untrackedRanges)
		TrackImageUsage(image, range, GpuImageLayout::ShaderReadOnly, GpuImageLayout::ShaderReadOnly, useFlags, GpuAccessFlag::Read, barrierHelper);
}

void D3D12ResourceTracker::TrackRenderTargetUsage(const D3D12RenderPassResourceUsage& renderPassUsage, D3D12BarrierHelper& barrierHelper)
{
	for(const D3D12RenderPassAttachmentUsage& attachmentUsage : renderPassUsage.Attachments)
	{
		GpuResourceUseFlags useFlags = attachmentUsage.AttachmentUsage;
		GpuImageLayout layout = attachmentUsage.AttachmentLayout;

		if(attachmentUsage.ShaderUsage.IsSet(GpuResourceUseFlag::ShaderAccess))
		{
			B3D_ASSERT(attachmentUsage.AttachmentAccess == GpuAccessFlag::Read);
			B3D_ASSERT(attachmentUsage.ShaderReadLayout.has_value());

			useFlags |= attachmentUsage.ShaderUsage;
			layout = *attachmentUsage.ShaderReadLayout;
		}

		TrackImageUsage(attachmentUsage.Image, attachmentUsage.Range, layout, layout, useFlags, attachmentUsage.AttachmentAccess, barrierHelper);
	}
}

void D3D12ResourceTracker::ClearRenderTargetFlagsForImage(D3D12Image* image)
{
	const u32 imageTrackingIndex = mImages[image];
	const GpuImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

	GpuImageSubresourceTrackingState* const subresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];
	for (u32 subresourceIndex = 0; subresourceIndex < imageTrackingState.SubresourceInfoCount; subresourceIndex++)
		subresourceTrackingStates[subresourceIndex].FramebufferUse = GpuAccessFlag::None;
}

void D3D12ResourceTracker::ClearShaderFlagsForAllRenderPassImageSubresources()
{
	for (u32 subresourceIndex : mRenderPassSubresources)
		mSubresourceTrackingState[subresourceIndex].ShaderUse = GpuAccessFlag::None;

	mRenderPassSubresources.clear();
}
