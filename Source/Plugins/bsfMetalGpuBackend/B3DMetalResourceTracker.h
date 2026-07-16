//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuResourceTracker.h"

namespace b3d::render
{
	class MetalBarrierHelper;

	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	extern template class TGpuResourceTracker<MetalBarrierHelper>;

	/**
	 * Metal-specific resource tracker. Inherits the backend-agnostic tracking machinery from
	 * TGpuResourceTracker and adds the glue used around Metal render passes. Metal has no
	 * framebuffer / render-pass objects (attachments are plain textures referenced by the
	 * MTLRenderPassDescriptor built in BeginRenderPass) and no native image layouts — the layout
	 * fields tracked here are engine bookkeeping only, kept so hazard analysis and the core
	 * verification (@c B3D_VERIFY_BARRIERS) behave identically to other backends.
	 */
	class MetalResourceTracker : public TGpuResourceTracker<MetalBarrierHelper>
	{
	public:
		/**
		 * Lets the tracker know that the provided image will be used as a render-pass attachment on the
		 * associated command buffer. Call once per attachment before the render encoder is created;
		 * execute the barriers queued in @p barrierHelper before beginning the pass. This is the Metal
		 * analogue of VulkanResourceTracker::TrackFramebufferUsage, expressed per attachment because
		 * Metal has no framebuffer object to hang the aggregate call on.
		 *
		 * @param	image				Attachment image to track.
		 * @param	subresourceRange	Subresource range bound as the attachment (single mip/slice for
		 *								typical render targets).
		 * @param	layout				Layout the attachment must be in when the pass begins. Pass the
		 *								attachment layout when the pass loads existing contents, or
		 *								GpuImageLayout::Undefined when contents are cleared/discarded.
		 * @param	finalLayout			Layout the attachment is considered to be in after the pass.
		 * @param	useFlags			Attachment usage category (color / depth-stencil attachment).
		 * @param	accessFlags			Read for read-only attachments, Write otherwise.
		 * @param	barrierHelper		Receives any barriers required before the attachment can be used.
		 */
		void TrackAttachmentUsage(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, MetalBarrierHelper& barrierHelper);

		/** Clears framebuffer-related usage flags for all subresources of the specified image. Call after the render pass ends. */
		void ClearFramebufferFlagsForImage(IGpuImageResource* image);

		/** Clears shader-related usage flags for all image subresources that were used during the current render pass. Call after the render pass ends. */
		void ClearShaderFlagsForAllRenderPassImageSubresources();

		/**
		 * Returns the tracked layout of the specified image subresource as seen by the associated
		 * command buffer, or GpuImageLayout::Undefined when the subresource has not been tracked yet.
		 * The range's base mip/layer selects the tracked block that is queried.
		 */
		GpuImageLayout GetCurrentSubresourceLayout(IGpuImageResource* image, const GpuTextureSubresourceRange& range) const;

		/**
		 * Updates the tracked layout of every subresource of @p image that was bound as a render-pass
		 * attachment to its recorded post-pass layout. Call after the render pass ends, BEFORE
		 * ClearFramebufferFlagsForImage (which erases the attachment-use marker this keys off).
		 */
		void MoveAllAttachmentsToFinalLayouts(IGpuImageResource* image);
	};

	/** @} */
} // namespace b3d::render
