//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "Utility/B3DTArrayView.h"

namespace b3d
{
	class IGpuImageResource;

	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		/** Describes how one image subresource range is used as a render-pass attachment (mainly detecting if a render attachment is also sampled in a shader). */
		struct GpuRenderPassAttachmentUsage
		{
			GpuRenderPassAttachmentUsage() = default;

			IGpuImageResource* Image = nullptr; /**< Image bound by the attachment. */
			GpuTextureSubresourceRange Range; /**< Subresources covered by this attachment aspect. */
			RenderSurfaceMaskBits Surface = RT_NONE; /**< Logical render-target surface represented by this entry. */
			GpuResourceUseFlags UseFlags; /**< Describes attachment usage (color or depth attachment). */
			GpuAccessFlags Access; /**< Access performed through the attachment. */
			GpuImageBarrierFlags BarrierFlags; /**< Behavior requested from the barrier preceding attachment access. */
			GpuImageLayout Layout = GpuImageLayout::Undefined; /**< Layout used when the attachment isn't sampled. */
			TOptional<GpuImageLayout> ShaderReadLayout; /**< Layout supporting simultaneous attachment and shader reads, if available. */
			TOptional<GpuImageLayout> FinalLayout; /**< Layout established when the render pass ends, or empty to retain the resolved access layout. */
		};

		/** Render-pass attachment usage combined with any potential shader accesses. */
		struct GpuResolvedRenderPassAttachmentUsage
		{
			GpuResolvedRenderPassAttachmentUsage() = default;

			IGpuImageResource* Image = nullptr; /**< Image bound by the attachment. */
			GpuTextureSubresourceRange Range; /**< Subresources covered by this attachment aspect. */
			RenderSurfaceMaskBits Surface = RT_NONE; /**< Logical render-target surface represented by this entry. */
			GpuResourceUseFlags UseFlags; /**< Describes combined shader and attachment usage. */
			GpuAccessFlags Access; /**< Combined access performed during the render pass. */
			GpuImageBarrierFlags BarrierFlags; /**< Behavior requested from the barrier preceding attachment access. */
			GpuImageLayout Layout = GpuImageLayout::Undefined; /**< Layout selected for the render pass. */
			GpuImageLayout FinalLayout = GpuImageLayout::Undefined; /**< Layout established when the render pass ends. */
		};

		/** Inline storage for every color, depth, and stencil attachment aspect in one render pass. */
		using GpuRenderPassAttachmentUsageArray = TInlineArray<GpuRenderPassAttachmentUsage, B3D_MAXIMUM_RENDER_TARGET_COUNT + 2>;

		/** Describes one framebuffer attachment aspect. */
		struct B3D_EXPORT GpuFramebufferAttachment
		{
			/** Creates metadata for one logical attachment surface and aspect-specific image range. */
			GpuFramebufferAttachment(IGpuImageResource& image, const GpuTextureSubresourceRange& range, RenderSurfaceMaskBits surface, TOptional<GpuImageLayout> finalLayout);

			/** Returns true when this entry identifies a color surface. */
			bool IsColor() const;

			/** Returns the color slot index, or zero for depth and stencil surfaces. */
			u32 GetIndex() const;

			/** Returns color attachment or depth attachment usage depending on attachment type. */
			GpuResourceUseFlags GetUseFlags() const;

			IGpuImageResource* Image; /**< Image bound by the attachment. */
			GpuTextureSubresourceRange Range; /**< Exact aspect-specific subresource range. */
			RenderSurfaceMaskBits Surface; /**< Logical render-target surface represented by this entry. */
			TOptional<GpuImageLayout> FinalLayout; /**< Layout established when the render pass ends, or empty to retain the access layout. */
		};

		/** Attachment-only and optional attachment + sampled-read layout for one attachment. */
		struct B3D_EXPORT GpuRenderPassAttachmentLayout
		{
			GpuRenderPassAttachmentLayout(GpuImageLayout attachmentOnly = GpuImageLayout::Undefined, TOptional<GpuImageLayout> attachmentAndShaderRead = TOptional<GpuImageLayout>());

			GpuImageLayout AttachmentOnly; /**< Layout used for attachment access without shader reads. */
			TOptional<GpuImageLayout> AttachmentAndShaderRead; /**< Layout supporting simultaneous attachment and shader reads. */
		};

		/** Defines backend layout choices used when building render-pass attachment usage. */
		struct B3D_EXPORT GpuFramebufferLayoutPolicy
		{
			GpuFramebufferLayoutPolicy(const GpuRenderPassAttachmentLayout& writableColor, const GpuRenderPassAttachmentLayout& readOnlyColor, const GpuRenderPassAttachmentLayout& writableDepthStencil, const GpuRenderPassAttachmentLayout& depthReadOnly, const GpuRenderPassAttachmentLayout& stencilReadOnly, const GpuRenderPassAttachmentLayout& readOnlyDepthStencil, TOptional<GpuImageLayout> unloadedLayout = TOptional<GpuImageLayout>());

			/** Returns the preferred color layout matching @p readOnly. */
			const GpuRenderPassAttachmentLayout& GetColorLayout(bool readOnly) const;

			/** Returns the preferred combined depth/stencil layout matching @p readOnlyMask. */
			const GpuRenderPassAttachmentLayout& GetDepthStencilLayout(RenderSurfaceMask readOnlyMask) const;

			GpuRenderPassAttachmentLayout WritableColor; /**< Writable color attachment layout. */
			GpuRenderPassAttachmentLayout ReadOnlyColor; /**< Read-only color attachment layout. */
			GpuRenderPassAttachmentLayout WritableDepthStencil; /**< Writable depth and stencil layout. */
			GpuRenderPassAttachmentLayout DepthReadOnly; /**< Read-only depth with writable stencil layout. */
			GpuRenderPassAttachmentLayout StencilReadOnly; /**< Writable depth with read-only stencil layout. */
			GpuRenderPassAttachmentLayout ReadOnlyDepthStencil; /**< Read-only depth and stencil layout. */
			TOptional<GpuImageLayout> UnloadedLayout; /**< Layout used for surfaces whose contents aren't preserved, or empty to retain their layout. */
		};

		/** Stores backend-neutral framebuffer attachment metadata without defining native lifetime semantics. */
		class B3D_EXPORT GpuFramebuffer
		{
		public:
			/** Returns the framebuffer width in pixels. */
			u32 GetWidth() const { return mWidth; }

			/** Returns the framebuffer height in pixels. */
			u32 GetHeight() const { return mHeight; }

			/** Returns the number of image layers addressed by the native framebuffer. */
			u32 GetLayerCount() const { return mLayerCount; }

			/** Returns all color + depth/stencil attachments. */
			TArrayView<const GpuFramebufferAttachment> GetAttachments() const { return mAttachments; }

			/** Returns the number of attachments (all color + depth and/or stencil). */
			u32 GetAttachmentCount() const { return (u32)mAttachments.Size(); }

			/** Returns color attachments. */
			TArrayView<const GpuFramebufferAttachment> GetColorAttachments() const;

			/** Returns the number of color attachments. */
			u32 GetColorAttachmentCount() const { return mColorAttachmentCount; }

			/** Finds the attachment associated with the render-target surface, or returns null. */
			const GpuFramebufferAttachment* FindAttachment(RenderSurfaceMaskBits surface) const;

			/** Builds the attachment usage array that can be used for tracking attachment usage in the resource tracker. */
			GpuRenderPassAttachmentUsageArray BuildRenderPassAttachmentUsages(RenderSurfaceMask readOnlyMask, RenderSurfaceMask loadMask, const GpuFramebufferLayoutPolicy& layoutPolicy) const;

		protected:
			GpuFramebuffer(u32 width, u32 height, u32 layerCount = 1);
			~GpuFramebuffer() = default;

			/** Adds one color attachment. Call only while constructing the native framebuffer. */
			void AddColorAttachment(IGpuImageResource& image, const GpuTextureSubresourceRange& range, u32 colorIndex, TOptional<GpuImageLayout> finalLayout = TOptional<GpuImageLayout>());

			/** Adds every depth/stencil aspect present in @p range. Call only while constructing the native framebuffer. */
			void AddDepthStencilAttachment(IGpuImageResource& image, const GpuTextureSubresourceRange& range, TOptional<GpuImageLayout> finalLayout = TOptional<GpuImageLayout>());

		private:
			TInlineArray<GpuFramebufferAttachment, B3D_MAXIMUM_RENDER_TARGET_COUNT + 2> mAttachments;
			u32 mColorAttachmentCount = 0;
			u32 mWidth;
			u32 mHeight;
			u32 mLayerCount;
		};

		/** @} */
	} // namespace render
} // namespace b3d
