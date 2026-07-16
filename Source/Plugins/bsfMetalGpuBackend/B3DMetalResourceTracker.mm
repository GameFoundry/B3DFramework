//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalResourceTracker.h"

#include "B3DMetalBarrierHelper.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Allocators/B3DFrameAllocator.h"
#include "Utility/B3DBitwise.h"

// Generic tracker method definitions, followed by the explicit instantiation for the Metal barrier
// helper. Included here (after the complete MetalBarrierHelper, GpuBackendUtility and frame
// allocator are available) so the single instantiation lives in this translation unit. The header
// carries a matching `extern template` to suppress implicit instantiation elsewhere.
#include "GpuBackend/B3DGpuResourceTracker.inl"

template class b3d::render::TGpuResourceTracker<b3d::render::MetalBarrierHelper>;

namespace b3d::render
{
	void MetalResourceTracker::TrackAttachmentUsage(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, MetalBarrierHelper& barrierHelper)
	{
		// The generic per-subresource machinery (hazard analysis, layout bookkeeping, FramebufferUse /
		// RenderPassLayout stamping, render-pass subresource registration) is entirely backend-agnostic;
		// Metal needs no framebuffer-object glue on top of it. The named wrapper exists so render-pass
		// setup code reads symmetrically with the Vulkan backend's TrackFramebufferUsage.
		TrackImageUsage(image, subresourceRange, layout, finalLayout, useFlags, accessFlags, barrierHelper);
	}

	void MetalResourceTracker::ClearFramebufferFlagsForImage(IGpuImageResource* image)
	{
		if(FindImageTrackingState(image) == nullptr)
			return;

		TArrayView<GpuImageSubresourceTrackingState> subresourceStates = GetSubresourceTrackingStatesForImage(image);
		for(GpuImageSubresourceTrackingState& subresourceState : subresourceStates)
			subresourceState.FramebufferUse = GpuAccessFlag::None;
	}

	void MetalResourceTracker::ClearShaderFlagsForAllRenderPassImageSubresources()
	{
		for(u32 globalSubresourceIndex : mRenderPassSubresources)
			mSubresourceTrackingState[globalSubresourceIndex].ShaderUse = GpuAccessFlag::None;

		mRenderPassSubresources.clear();
	}

	GpuImageLayout MetalResourceTracker::GetCurrentSubresourceLayout(IGpuImageResource* image, const GpuTextureSubresourceRange& range) const
	{
		const GpuImageSubresourceTrackingState* subresourceState = FindSubresourceTrackingState(image, range.BaseArrayLayer, range.BaseMipLevel);
		if(subresourceState == nullptr)
			return GpuImageLayout::Undefined;

		return subresourceState->CurrentLayout;
	}

	void MetalResourceTracker::MoveAllAttachmentsToFinalLayouts(IGpuImageResource* image)
	{
		if(FindImageTrackingState(image) == nullptr)
			return;

		TArrayView<GpuImageSubresourceTrackingState> subresourceStates = GetSubresourceTrackingStatesForImage(image);
		for(GpuImageSubresourceTrackingState& subresourceState : subresourceStates)
		{
			// Only subresources actually bound as attachments in this pass carry a meaningful
			// RenderPassLayout; FramebufferUse is the marker TrackImageUsage stamps for those.
			if(!subresourceState.FramebufferUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
				continue;

			if(subresourceState.RenderPassLayout == GpuImageLayout::Undefined)
				continue;

			subresourceState.CurrentLayout = subresourceState.RenderPassLayout;
			subresourceState.RequiredLayout = subresourceState.RenderPassLayout;
		}
	}
} // namespace b3d::render
