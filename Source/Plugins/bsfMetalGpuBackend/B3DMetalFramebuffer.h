//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuFramebuffer.h"

namespace b3d
{
	namespace render
	{
		class MetalTexture;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/** Texture and subresource used to create one framebuffer attachment. */
		struct MetalFramebufferAttachmentCreateInformation
		{
			MetalTexture* Texture = nullptr; /**< Texture rendered to, or null when the attachment is absent. */
			u32 Face = 0; /**< Array layer, cube face or 3D texture depth slice rendered to. */
			u32 MipLevel = 0; /**< Mip level rendered to. */
		};

		/** Textures and dimensions used to construct a framebuffer. */
		struct MetalFramebufferCreateInformation
		{
			MetalFramebufferAttachmentCreateInformation ColorAttachments[B3D_MAXIMUM_RENDER_TARGET_COUNT]; /**< Color attachments indexed by render-target slot. */
			MetalFramebufferAttachmentCreateInformation DepthStencilAttachment; /**< Optional depth/stencil attachment. */
			u32 Width = 0; /**< Framebuffer width in pixels. */
			u32 Height = 0; /**< Framebuffer height in pixels. */
		};

		/**
		 * Metal implementation of a framebuffer. Metal has no native framebuffer object, so this only holds the attachments
		 * a render pass descriptor is built from.
		 */
		class MetalFramebuffer : public GpuFramebuffer
		{
		public:
			explicit MetalFramebuffer(const MetalFramebufferCreateInformation& createInformation);

#ifdef __OBJC__
			/** Assigns the attachment textures and subresources to @p descriptor. Load and store actions are left to the caller. */
			void ApplyAttachments(MTLRenderPassDescriptor* descriptor) const;
#endif

			/** Returns the layout policy used for Metal render-pass tracking. */
			static const GpuFramebufferLayoutPolicy& GetLayoutPolicy();

		private:
			/** Depth slice rendered to, per color attachment of a 3D texture. Zero for other texture types. */
			u32 mColorDepthPlanes[B3D_MAXIMUM_RENDER_TARGET_COUNT] = {};
		};

		/** @} */
	} // namespace render
} // namespace b3d
