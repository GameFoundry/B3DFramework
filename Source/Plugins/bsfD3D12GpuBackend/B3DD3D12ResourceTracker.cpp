//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12ResourceTracker.h"

#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12BarrierUtility.h"
// COMPLETE D3D12BarrierHelper - load-bearing: the .inl below names TBarrierHelper::BarrierTrackingInfo and calls
// barrierHelper.AddBufferBarrier(...) / AddSubresourceBarrier(...), so the helper must be a complete type before the
// .inl is included. Do not reorder.
#include "Utility/B3DD3D12BarrierHelper.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Allocators/B3DFrameAllocator.h"

// Generic tracker method definitions, followed by the explicit instantiation for the D3D12 barrier helper. Included
// here (after the complete D3D12BarrierHelper, GpuBackendUtility and frame allocator are available) so the single
// instantiation lives in this translation unit. The header carries a matching `extern template` to suppress implicit
// instantiation elsewhere. This recipe mirrors the proven Vulkan TU (B3DVulkanResourceTracker.cpp) and is ODR-safe.
#include "GpuBackend/B3DGpuResourceTracker.inl"

template class b3d::render::TGpuResourceTracker<b3d::render::D3D12BarrierHelper>;

using namespace b3d;
using namespace b3d::render;

// ---------------------------------------------------------------------------------------------------------------------
// Render-target / render-pass glue. Mirrors VulkanResourceTracker's TrackFramebufferUsage, with the attachment layouts
// expressed as the GpuImageLayouts the D3D12 barrier helper translates into native states. D3D12 has no render-pass
// automatic transitions, so the post-pass (final) layout always equals the in-pass layout.
// ---------------------------------------------------------------------------------------------------------------------

void D3D12ResourceTracker::TrackRenderTargetUsage(const D3D12RenderTargetAttachment* attachments, u32 attachmentCount, RenderSurfaceMask readOnlyMask, D3D12BarrierHelper& barrierHelper)
{
	for (u32 i = 0; i < attachmentCount; i++)
	{
		const D3D12RenderTargetAttachment& attachment = attachments[i];

		GpuAccessFlag access;
		GpuResourceUseFlag useFlags;
		GpuImageLayout layout;
		if (attachment.IsDepthStencil)
		{
			const bool readOnly = readOnlyMask.IsSet(RT_DEPTH);
			access = readOnly ? GpuAccessFlag::Read : GpuAccessFlag::Write;
			useFlags = GpuResourceUseFlag::DepthStencilAttachment;
			layout = readOnly ? GpuImageLayout::DepthStencilReadOnly : GpuImageLayout::DepthStencilAttachment;
		}
		else
		{
			const RenderSurfaceMaskBits colorBit = (RenderSurfaceMaskBits)(1 << attachment.ColorIndex);
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

RenderSurfaceMask D3D12ResourceTracker::GetRenderTargetReadOnlyMask(const D3D12RenderTargetAttachment* attachments, u32 attachmentCount, RenderSurfaceMask explicitReadOnlyMask) const
{
	RenderSurfaceMask readMask = RT_NONE;

	for (u32 i = 0; i < attachmentCount; i++)
	{
		const D3D12RenderTargetAttachment& attachment = attachments[i];

		const GpuImageSubresourceTrackingState* subresourceTrackingState =
			FindSubresourceTrackingState(attachment.Image, attachment.Surface.Face, attachment.Surface.MipLevel);
		if (subresourceTrackingState == nullptr)
			continue;

		// A shader that also samples an attachment this pass forces it read-only (it cannot be written and sampled at
		// once). A shader *write* to an attachment would be an error, but is not policed here.
		if (!subresourceTrackingState->ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
			continue;

		if (attachment.IsDepthStencil)
			readMask.Set(RT_DEPTH);
		else
			readMask.Set((RenderSurfaceMaskBits)(1 << attachment.ColorIndex));
	}

	if (explicitReadOnlyMask.IsSet(RT_DEPTH))
		readMask.Set(RT_DEPTH);
	if (explicitReadOnlyMask.IsSet(RT_STENCIL))
		readMask.Set(RT_STENCIL);

	return readMask;
}

void D3D12ResourceTracker::ClearRenderTargetFlagsForImage(D3D12Image* image)
{
	const u32 imageTrackingIndex = mImages[image];
	const GpuImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

	GpuImageSubresourceTrackingState* const subresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];
	for (u32 i = 0; i < imageTrackingState.SubresourceInfoCount; i++)
		subresourceTrackingStates[i].FramebufferUse = GpuAccessFlag::None;
}

void D3D12ResourceTracker::ClearShaderFlagsForAllRenderPassImageSubresources()
{
	for (const auto& subresourceIndex : mRenderPassSubresources)
		mSubresourceTrackingState[subresourceIndex].ShaderUse = GpuAccessFlag::None;

	mRenderPassSubresources.clear();
}

void D3D12ResourceTracker::TrackBufferUsage(D3D12Buffer* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, D3D12BarrierHelper& barrierHelper, u32 dynamicOffset)
{
	// Shared bookkeeping first: registers the binding (keeping the buffer alive) and queues a hazard barrier when a
	// prior write requires one.
	TGpuResourceTracker<D3D12BarrierHelper>::TrackBufferUsage(buffer, useFlags, accessFlags, barrierHelper, dynamicOffset);

	// Keep the native state in sync with the usage. The transition (when one is needed) also provides the
	// execution/memory synchronization for any hazard registered above.
	barrierHelper.RequireBufferState(buffer, D3D12BarrierUtility::GetResourceState(useFlags, accessFlags, false));
}

void D3D12ResourceTracker::TrackImageUsage(D3D12Image* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, D3D12BarrierHelper& barrierHelper)
{
	// Shared bookkeeping first: registers the binding (keeping the image and its subresources alive), detects layout
	// changes/hazards and queues the matching native barriers via the helper's Record hooks.
	TGpuResourceTracker<D3D12BarrierHelper>::TrackImageUsage(image, subresourceRange, layout, finalLayout, useFlags, accessFlags, barrierHelper);

	// The first use of a subresource seeds the shared tracker's layout without invoking the barrier hooks, so the
	// native state may still not match. Ensure it does; this is a no-op for subresources the shared path already
	// transitioned (their native state was advanced by the Record hook).
	for(u32 layerIndex = 0; layerIndex < subresourceRange.ArrayLayerCount; layerIndex++)
	{
		for(u32 levelIndex = 0; levelIndex < subresourceRange.MipLevelCount; levelIndex++)
		{
			const u32 face = subresourceRange.BaseArrayLayer + layerIndex;
			const u32 mipLevel = subresourceRange.BaseMipLevel + levelIndex;

			const GpuImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(image, face, mipLevel);
			if(subresourceTrackingState.RequiredLayout == GpuImageLayout::Undefined)
				continue;

			barrierHelper.RequireSubresourceState(image, face, mipLevel,
				D3D12BarrierUtility::GetResourceStateFromLayout(subresourceTrackingState.RequiredLayout, accessFlags));
		}
	}
}
