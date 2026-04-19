//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "RenderAPI/B3DRenderTarget.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
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

		private:
			/** Creates the render target and depth-stencil views. */
			void CreateViews();

			const RenderTarget* mRenderTarget = nullptr;
			u32 mBackBufferIndex = 0;

			static constexpr u32 kMaxColorAttachments = 8;
			D3D12_CPU_DESCRIPTOR_HANDLE mRenderTargetViews[kMaxColorAttachments];
			D3D12_CPU_DESCRIPTOR_HANDLE mDepthStencilView;

			u32 mNumColorAttachments = 0;
			bool mHasDepthStencil = false;
			u32 mWidth = 0;
			u32 mHeight = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
