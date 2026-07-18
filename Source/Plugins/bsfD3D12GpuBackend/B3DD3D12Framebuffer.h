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

		/** DirectX 12 implementation of a framebuffer. */
		class D3D12Framebuffer
		{
		public:
			D3D12Framebuffer(const RenderTarget* renderTarget, u32 backBufferIndex = 0);
			~D3D12Framebuffer();

			/** Returns the render target views for the color attachments. */
			const D3D12_CPU_DESCRIPTOR_HANDLE* GetRenderTargetViews() const { return mRenderTargetViews; }

			/** Returns the depth-stencil view, or nullptr if no depth-stencil attachment. */
			const D3D12_CPU_DESCRIPTOR_HANDLE* GetDepthStencilView() const { return mHasDepthStencil ? &mDepthStencilView : nullptr; }

			/** Returns the number of color attachments. */
			u32 GetNumColorAttachments() const { return mNumColorAttachments; }

			/** Returns the width of the framebuffer. */
			u32 GetWidth() const { return mWidth; }

			/** Returns the height of the framebuffer. */
			u32 GetHeight() const { return mHeight; }

			/**
			 * A single framebuffer attachment, resolved to the tracked image + subresource it binds. Built from either
			 * an offscreen render texture or a swap-chain surface, so the command buffer's resource tracking never
			 * needs to know which kind of render target produced it.
			 */
			struct Attachment
			{
				/** Tracked image backing the attachment, or null when the attachment is absent. */
				D3D12Image* Image = nullptr;

				/** Face/mip selection of the attachment within the image. */
				TextureSurface Surface;
			};

			/** Returns the color attachment at the given index, for tracking/barrier purposes. */
			const Attachment& GetColorAttachment(u32 index) const { return mColorAttachments[index]; }

			/** Returns the depth-stencil attachment, for tracking/barrier purposes. Image is null when absent. */
			const Attachment& GetDepthStencilAttachment() const { return mDepthStencilAttachment; }

			/** Returns the DXGI format of the color attachment at the given index (DXGI_FORMAT_UNKNOWN if not present). */
			DXGI_FORMAT GetColorFormat(u32 index) const { return mColorFormats[index]; }

			/** Returns the DXGI format of the depth-stencil attachment, or DXGI_FORMAT_UNKNOWN if there is none. */
			DXGI_FORMAT GetDepthStencilFormat() const { return mDepthStencilFormat; }

			/** Returns the sample count of the framebuffer attachments. */
			u32 GetSampleCount() const { return mSampleCount; }

		private:
			/** Creates the render target and depth-stencil views. */
			void CreateViews();

			const RenderTarget* mRenderTarget = nullptr;
			u32 mBackBufferIndex = 0;

			static constexpr u32 kMaxColorAttachments = 8;
			D3D12_CPU_DESCRIPTOR_HANDLE mRenderTargetViews[kMaxColorAttachments];
			D3D12_CPU_DESCRIPTOR_HANDLE mDepthStencilView;

			// Per-attachment image references. Resource state lives on the image (subresource) objects themselves.
			Attachment mColorAttachments[kMaxColorAttachments];
			Attachment mDepthStencilAttachment;

			DXGI_FORMAT mColorFormats[kMaxColorAttachments] = {};
			DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_UNKNOWN;
			u32 mSampleCount = 1;

			u32 mNumColorAttachments = 0;
			bool mHasDepthStencil = false;
			bool mOwnsViews = false; /**< True when the RTV/DSV descriptors were allocated by us (render texture path) rather than referenced from the swap chain. */
			u32 mWidth = 0;
			u32 mHeight = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
