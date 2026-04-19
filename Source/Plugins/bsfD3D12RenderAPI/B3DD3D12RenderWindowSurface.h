//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "RenderAPI/B3DRenderWindow.h"

namespace b3d::render
{
	/** @addtogroup D3D12RenderAPI
	 *  @{
	 */

	/** Interface that acts as a bridge between Win32RenderWindow and D3D12SwapChain. */
	class D3D12RenderWindowSurface : public IRenderWindowSurface
	{
	public:
		D3D12RenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation);
		~D3D12RenderWindowSurface();

		void RebuildSwapChain(u32 width, u32 height, bool vsync) override;
		void MarkSwapChainAsInvalid() override;
		void Destroy() override;

		/** Returns the swap chain owned by the surface. */
		D3D12SwapChain* GetSwapChain() const { return mSwapChain.get(); }

		/** Returns the framebuffer for the specified back buffer index. */
		D3D12Framebuffer* GetFramebuffer(u32 backBufferIndex) const;

	private:
		D3D12GpuDevice& mDevice;
		DXGI_FORMAT mColorFormat = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT mDepthFormat = DXGI_FORMAT_UNKNOWN;
		bool mCreateDepthBuffer = false;
		bool mUseHardwareSRGB = false;
		UniquePtr<D3D12SwapChain> mSwapChain;
		HWND mWindowHandle = nullptr;
		bool mIsDestroyed = false;
	};

	/** @} */
} // namespace b3d::render
