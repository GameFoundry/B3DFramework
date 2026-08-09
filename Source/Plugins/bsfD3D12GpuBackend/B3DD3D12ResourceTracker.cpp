//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12ResourceTracker.h"

#include "B3DD3D12Framebuffer.h"
#include "B3DD3D12Texture.h"
#include "Utility/B3DD3D12BarrierHelper.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Allocators/B3DFrameAllocator.h"

#include "GpuBackend/B3DGpuResourceTracker.inl"

template class b3d::render::TGpuResourceTracker<b3d::render::D3D12BarrierHelper>;

using namespace b3d;
using namespace b3d::render;

void D3D12ResourceTracker::TrackRenderTargetUsage(const D3D12FramebufferAttachment* attachments, u32 attachmentCount, RenderSurfaceMask readOnlyMask, D3D12BarrierHelper& barrierHelper)
{
	for (u32 attachmentIndex = 0; attachmentIndex < attachmentCount; attachmentIndex++)
	{
		const D3D12FramebufferAttachment& attachment = attachments[attachmentIndex];

		GpuAccessFlag access;
		GpuResourceUseFlag useFlags;
		GpuImageLayout layout;
		if (attachment.IsDepthStencil)
		{
			const bool hasStencil = attachment.Image->GetRange().AspectMask.IsSet(GpuTextureAspectFlag::Stencil);
			const bool depthReadOnly = readOnlyMask.IsSet(RT_DEPTH);
			const bool stencilReadOnly = !hasStencil || readOnlyMask.IsSet(RT_STENCIL);
			const bool fullyReadOnly = depthReadOnly && stencilReadOnly;
			access = fullyReadOnly ? GpuAccessFlag::Read : GpuAccessFlag::Write;
			useFlags = GpuResourceUseFlag::DepthStencilAttachment;
			if(fullyReadOnly)
				layout = GpuImageLayout::DepthStencilReadOnly;
			else if(depthReadOnly)
				layout = GpuImageLayout::DepthReadOnlyStencilAttachment;
			else if(stencilReadOnly && hasStencil)
				layout = GpuImageLayout::DepthAttachmentStencilReadOnly;
			else
				layout = GpuImageLayout::DepthStencilAttachment;
		}
		else
		{
			const RenderSurfaceMaskBits colorBit = (RenderSurfaceMaskBits)(1u << attachment.ColorIndex);
			const bool readOnly = readOnlyMask.IsSet(colorBit);
			access = readOnly ? GpuAccessFlag::Read : GpuAccessFlag::Write;
			useFlags = GpuResourceUseFlag::ColorAttachment;

			// A read-only color attachment stays shader-readable (PIXEL/NON_PIXEL SRV states).
			layout = readOnly ? GpuImageLayout::ShaderReadOnly : GpuImageLayout::ColorAttachment;
		}

		const GpuTextureSubresourceRange range = attachment.Image->GetRange(attachment.Surface);

		TrackImageUsage(attachment.Image, range, layout, layout, useFlags, access, barrierHelper);
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
