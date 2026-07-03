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
			 * A single framebuffer attachment together with the state-tracking information the command buffer needs
			 * to emit correct D3D12 resource-state transitions when the render pass begins/ends.
			 *
			 * For RenderTexture attachments both @c Resource and @c StateHolder point into the backing D3D12Texture
			 * (a D3D12Resource), so transitions update the texture's shared state. For swap-chain back buffers there
			 * is no D3D12Resource wrapper, so the framebuffer owns a lightweight state slot and StateHolder points at
			 * it. @c Resource may be null when the underlying ID3D12Resource* is unreachable from the framebuffer
			 * (currently the swap-chain depth buffer - see note in the .cpp), in which case the caller must skip the
			 * transition.
			 */
			struct Attachment
			{
				ID3D12Resource* Resource = nullptr;
				D3D12_RESOURCE_STATES* StateHolder = nullptr;
			};

			/** Returns the color attachment at the given index, for barrier/transition purposes. */
			const Attachment& GetColorAttachment(u32 index) const { return mColorAttachments[index]; }

			/** Returns the depth-stencil attachment, for barrier/transition purposes. Resource is null when absent or unreachable. */
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

			// Per-attachment resource + state-holder pointers. For swap-chain back buffers the framebuffer owns the
			// tracked states in mOwnedColorStates / mOwnedDepthStencilState (back buffers have no D3D12Resource wrapper).
			Attachment mColorAttachments[kMaxColorAttachments];
			Attachment mDepthStencilAttachment;
			D3D12_RESOURCE_STATES mOwnedColorStates[kMaxColorAttachments];
			D3D12_RESOURCE_STATES mOwnedDepthStencilState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

			DXGI_FORMAT mColorFormats[kMaxColorAttachments] = {};
			DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_UNKNOWN;
			u32 mSampleCount = 1;

			u32 mNumColorAttachments = 0;
			bool mHasDepthStencil = false;
			u32 mWidth = 0;
			u32 mHeight = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
