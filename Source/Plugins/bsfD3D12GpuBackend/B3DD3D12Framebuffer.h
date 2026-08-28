//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuFramebuffer.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** Image and subresource surface used to create one framebuffer attachment. */
		struct D3D12FramebufferAttachmentCreateInformation
		{
			D3D12Image* Image = nullptr; /**< Tracked image, or null when the attachment is absent. */
			TextureSurface Surface; /**< Face and mip range bound by the attachment. */
			TOptional<GpuImageLayout> FinalLayout; /**< Layout established when the render pass ends, or empty to retain the access layout. */
		};

		/** Images and dimensions used to construct a framebuffer. */
		struct D3D12FramebufferCreateInformation
		{
			D3D12FramebufferAttachmentCreateInformation ColorAttachments[B3D_MAXIMUM_RENDER_TARGET_COUNT]; /**< Color attachments indexed by render-target slot. */
			D3D12FramebufferAttachmentCreateInformation DepthStencilAttachment; /**< Optional depth/stencil attachment. */
			u32 Width = 0; /**< Framebuffer width in pixels. */
			u32 Height = 0; /**< Framebuffer height in pixels. */
		};

		/** DirectX 12 implementation of a framebuffer. */
		class D3D12Framebuffer : public GpuFramebuffer
		{
		public:
			/** Creates and owns native views for the supplied attachments. */
			explicit D3D12Framebuffer(const D3D12FramebufferCreateInformation& createInformation);
			~D3D12Framebuffer();

			/** Returns the render target views for the color attachments. */
			const D3D12_CPU_DESCRIPTOR_HANDLE* GetRenderTargetViews() const { return mRenderTargetViews; }

			/** Returns the depth-stencil view matching @p readOnlyMask, or nullptr if no depth-stencil attachment. */
			const D3D12_CPU_DESCRIPTOR_HANDLE* GetDepthStencilView(RenderSurfaceMask readOnlyMask) const;

			/** Returns the DXGI format of the color attachment at the given index (DXGI_FORMAT_UNKNOWN if not present). */
			DXGI_FORMAT GetColorFormat(u32 index) const { return mColorFormats[index]; }

			/** Returns the DXGI format of the depth-stencil attachment, or DXGI_FORMAT_UNKNOWN if there is none. */
			DXGI_FORMAT GetDepthStencilFormat() const { return mDepthStencilFormat; }

			/** Returns the sample count of the framebuffer attachments. */
			u32 GetSampleCount() const { return mSampleCount; }

			/** Returns the layout policy used for D3D12 render-pass tracking. */
			static const GpuFramebufferLayoutPolicy& GetLayoutPolicy();

		private:
			/** Creates a render-target view for @p attachment in the packed native slot @p attachmentIndex. */
			bool CreateRenderTargetView(u32 attachmentIndex, const D3D12FramebufferAttachmentCreateInformation& attachment);

			/** Creates every valid read-only variant of a depth-stencil descriptor. */
			bool CreateDepthStencilViews(const D3D12FramebufferAttachmentCreateInformation& attachment);

			static constexpr u32 kMaxColorAttachments = 8;
			static constexpr u32 kDepthStencilViewCount = 4;
			D3D12_CPU_DESCRIPTOR_HANDLE mRenderTargetViews[kMaxColorAttachments] = {};
			D3D12_CPU_DESCRIPTOR_HANDLE mDepthStencilViews[kDepthStencilViewCount] = {};

			DXGI_FORMAT mColorFormats[kMaxColorAttachments] = {};
			DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_UNKNOWN;
			u32 mSampleCount = 1;

			bool mDepthStencilHasStencil = false;
		};

		/** @} */
	} // namespace render
} // namespace b3d
