//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DRenderWindow.h"

namespace b3d::render
{
	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/** Common interface for window-backed and headless D3D12 render-window surfaces. */
	class ID3D12RenderWindowSurface : public IRenderWindowSurface
	{
	public:
		/** Acquires the framebuffer for the next render pass. Render thread only. */
		virtual D3D12Framebuffer* GetActiveFramebuffer() = 0;

		/** Returns whether the surface can provide an image without first being rebuilt. */
		virtual bool IsSwapChainValid() const = 0;

		/** Returns the DXGI swap-chain wrapper, or null for a headless surface. */
		virtual D3D12SwapChain* GetSwapChain() const { return nullptr; }

		/** Returns the image containing the most recently rendered frame. */
		virtual D3D12Image* GetCurrentColorImage() const = 0;

		/** Returns the current surface width in pixels. */
		virtual u32 GetWidth() const = 0;

		/** Returns the current surface height in pixels. */
		virtual u32 GetHeight() const = 0;

		/** Returns the engine pixel format used for color readback. */
		virtual PixelFormat GetColorPixelFormat() const = 0;

		TAsyncOp<TShared<PixelData>> ReadAsync(GpuCommandBuffer& commandBuffer) override;
	};

	/** @} */
} // namespace b3d::render
