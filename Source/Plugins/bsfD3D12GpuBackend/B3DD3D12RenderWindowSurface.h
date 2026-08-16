//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DID3D12RenderWindowSurface.h"

namespace b3d::render
{
	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/** D3D12 render-window surface backed by an HWND and DXGI swap chain. */
	class D3D12RenderWindowSurface final : public ID3D12RenderWindowSurface
	{
	public:
		/** Creates a window-backed D3D12 surface and its initial swap chain. */
		D3D12RenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation);
		~D3D12RenderWindowSurface() override;

		void RebuildSwapChain(u32 width, u32 height, bool vsync, u32 vsyncInterval) override;
		void SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask) override;
		void MarkSwapChainAsInvalid() override;
		void Destroy() override;

		D3D12Framebuffer* GetActiveFramebuffer() override;
		bool IsSwapChainValid() const override { return mIsValid && mSwapChain != nullptr; }
		D3D12SwapChain* GetSwapChain() const override { return mSwapChain; }
		D3D12Image* GetCurrentColorImage() const override;
		u32 GetWidth() const override;
		u32 GetHeight() const override;
		PixelFormat GetColorPixelFormat() const override { return PF_RGBA8; }

	private:
		/** Creates the swap chain via the device resource manager and kicks the initial image acquire. */
		void CreateSwapChain(u32 width, u32 height, bool vsync);

		D3D12GpuDevice& mDevice;
		DXGI_FORMAT mColorFormat = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT mDepthFormat = DXGI_FORMAT_UNKNOWN;
		bool mCreateDepthBuffer = false;
		u32 mVsyncInterval = 1;
		D3D12SwapChain* mSwapChain = nullptr;
		HWND mWindowHandle = nullptr;
		bool mIsValid = false;
		bool mIsDestroyed = false;
	};

	/** @} */
} // namespace b3d::render
