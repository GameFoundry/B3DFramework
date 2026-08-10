//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DRenderTarget.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** One framebuffer attachment resolved to its tracked image range and render-target role. */
		struct D3D12FramebufferAttachment
		{
			D3D12Image* Image = nullptr; /**< Tracked image, or null when the attachment is absent. */
			TextureSurface Surface;      /**< Face and mip range bound by the attachment. */
			u32 ColorIndex = 0;          /**< Color attachment slot; unused when IsDepthStencil is true. */
			bool IsDepthStencil = false; /**< True for the depth/stencil attachment. */
		};

		/** DirectX 12 implementation of a framebuffer. */
		class D3D12Framebuffer
		{
		public:
			/** Resolves the views and tracked attachment ranges for @p renderTarget. */
			D3D12Framebuffer(const RenderTarget* renderTarget, u32 backBufferIndex = 0);
			~D3D12Framebuffer();

			/** Returns the render target views for the color attachments. */
			const D3D12_CPU_DESCRIPTOR_HANDLE* GetRenderTargetViews() const { return mRenderTargetViews; }

			/** Returns the depth-stencil view matching @p readOnlyMask, or nullptr if no depth-stencil attachment. */
			const D3D12_CPU_DESCRIPTOR_HANDLE* GetDepthStencilView(RenderSurfaceMask readOnlyMask) const;

			/** Returns the number of color attachments. */
			u32 GetColorAttachmentCount() const { return mColorAttachmentCount; }

			/** Returns the width of the framebuffer. */
			u32 GetWidth() const { return mWidth; }

			/** Returns the height of the framebuffer. */
			u32 GetHeight() const { return mHeight; }

			/** Returns the color attachment at @p index, which must be less than GetColorAttachmentCount(). */
			const D3D12FramebufferAttachment& GetColorAttachment(u32 index) const { return mAttachments[index]; }

			/** Returns the depth-stencil attachment, for tracking/barrier purposes. Image is null when absent. */
			const D3D12FramebufferAttachment& GetDepthStencilAttachment() const { return mAttachments[mColorAttachmentCount]; }

			/** Returns the contiguous color and depth/stencil attachments, valid for the framebuffer's lifetime. */
			const D3D12FramebufferAttachment* GetAttachments() const { return mAttachments; }

			/** Returns the number of populated entries returned by GetAttachments(). */
			u32 GetAttachmentCount() const { return mColorAttachmentCount + (GetDepthStencilAttachment().Image != nullptr ? 1u : 0u); }

			/** Returns the DXGI format of the color attachment at the given index (DXGI_FORMAT_UNKNOWN if not present). */
			DXGI_FORMAT GetColorFormat(u32 index) const { return mColorFormats[index]; }

			/** Returns the DXGI format of the depth-stencil attachment, or DXGI_FORMAT_UNKNOWN if there is none. */
			DXGI_FORMAT GetDepthStencilFormat() const { return mDepthStencilFormat; }

			/** Returns the sample count of the framebuffer attachments. */
			u32 GetSampleCount() const { return mSampleCount; }

		private:
			/** Creates the render target and depth-stencil views. */
			void CreateViews();

			/** Creates every valid read-only variant of a depth-stencil descriptor. */
			void CreateDepthStencilViews(ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& description, bool hasStencil);

			const RenderTarget* mRenderTarget = nullptr;
			u32 mBackBufferIndex = 0;

			static constexpr u32 kMaxColorAttachments = 8;
			static constexpr u32 kDepthStencilViewCount = 4;
			D3D12_CPU_DESCRIPTOR_HANDLE mRenderTargetViews[kMaxColorAttachments] = {};
			D3D12_CPU_DESCRIPTOR_HANDLE mDepthStencilViews[kDepthStencilViewCount] = {};

			/** Color attachments followed by the optional depth/stencil attachment. */
			D3D12FramebufferAttachment mAttachments[kMaxColorAttachments + 1];

			DXGI_FORMAT mColorFormats[kMaxColorAttachments] = {};
			DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_UNKNOWN;
			u32 mSampleCount = 1;

			u32 mColorAttachmentCount = 0;
			bool mDepthStencilHasStencil = false;
			bool mOwnsRenderTargetViews = false; /**< True when the RTV descriptors were allocated rather than borrowed from the swap chain. */
			u32 mWidth = 0;
			u32 mHeight = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
