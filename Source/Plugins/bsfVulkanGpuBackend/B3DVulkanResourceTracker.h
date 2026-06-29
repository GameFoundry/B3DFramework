//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuResourceTracker.h"


namespace b3d::render
{
	class VulkanBarrierHelper;
	class VulkanRenderPass;
	class VulkanImage;
	class VulkanFramebuffer;

	/** @addtogroup Vulkan
	 *  @{
	 */

	extern template class TGpuResourceTracker<VulkanBarrierHelper>;

	/**
	 * Vulkan-specific resource tracker. Inherits the backend-agnostic tracking machinery from TGpuResourceTracker and
	 * adds the glue that depends on Vulkan framebuffers / render passes (which produce the per-attachment layouts the
	 * generic tracker stores).
	 */
	class VulkanResourceTracker : public TGpuResourceTracker<VulkanBarrierHelper>
	{
	public:
		/**
		 * Lets the tracker know that the provided framebuffer will be queued on the associated command buffer. All associated attachment images
		 * will be tracked as well, there's no need to track them separately. Call this before the framebuffer is used. Execute the barriers
		 * queued in @p barrierHelper before use.
		 */
		void TrackFramebufferUsage(VulkanFramebuffer* framebuffer, RenderSurfaceMask loadMask, RenderSurfaceMask readOnlyMask, VulkanBarrierHelper& barrierHelper);

		/** Clears framebuffer-related usage flags for all subresources of the specified image. Usually called after render pass ends. */
		void ClearFramebufferFlagsForImage(VulkanImage* image);

		/** Clears shader-related usage flags for all image subresources that were used during the current render pass. Usually called after render pass ends. */
		void ClearShaderFlagsForAllRenderPassImageSubresources();

		/**
		 * Returns the current layout of the specified image subresource, as seen by the associated command buffer. This is different from the
		 * global layout stored in VulkanImage itself, as it includes any transitions performed by the command buffer
		 * (at the current point in time), while the global layout is only updated after a command buffer as been submitted.
		 *
		 * @param	image						Image to lookup the layout for.
		 * @param	range						Subresource range of the image to lookup the layout for.
		 * @param	framebuffer					Optional framebuffer. If provided the method will assume we are currently executing a render pass with
		 *										the provided framebuffer, and will return the layout of the subresource after the render pass begins.
		 *										This may be different from the current layout if the image is used as a framebuffer attachment, in which
		 *										case the render pass may perform an automated layout transition when it begins.
		 * @param	explicitReadOnlyMask		Mask that specifies which attachments are forced to be read-only, regardless of shader use.
		 */
		GpuImageLayout GetCurrentSubresourceLayout(VulkanImage* image, const GpuTextureSubresourceRange& range, VulkanFramebuffer* framebuffer = nullptr, RenderSurfaceMask explicitReadOnlyMask = RT_NONE) const;

		/**
		 * Checks the subresource state of all associated framebuffer attachments and returns a mask of those that need to be read-only during the render pass,
		 * due to the fact they are also accessed by a shader during the same render pass.
		 */
		RenderSurfaceMask GetFramebufferReadOnlyMask(VulkanFramebuffer* framebuffer, RenderSurfaceMask explicitReadOnlyMask) const;

		/** Updates the current layout of all associated framebuffer attachments to their final layouts after a render pass has executed. */
		void MoveAllFramebufferAttachmentsToFinalLayouts(VulkanFramebuffer* framebuffer);
	};

	/** @} */
} // namespace b3d::render
