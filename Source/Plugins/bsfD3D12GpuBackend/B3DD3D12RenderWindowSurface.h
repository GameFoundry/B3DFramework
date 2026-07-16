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

	/**
	 * Interface that acts as a bridge between Win32RenderWindow and D3D12SwapChain. Also handles headless surfaces,
	 * which differ only in the swap chain they create (offscreen textures instead of DXGI back buffers).
	 */
	class D3D12RenderWindowSurface : public IRenderWindowSurface
	{
	public:
		D3D12RenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation);
		~D3D12RenderWindowSurface();

		void RebuildSwapChain(u32 width, u32 height, bool vsync, u32 vsyncInterval) override;
		void SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask) override;
		void MarkSwapChainAsInvalid() override;
		void Destroy() override;
		TAsyncOp<TShared<PixelData>> ReadAsync(GpuCommandBuffer& commandBuffer) override;

		/** Returns the swap chain owned by the surface. */
		D3D12SwapChain* GetSwapChain() const { return mSwapChain; }

		/** Returns the framebuffer for the specified back buffer index. */
		D3D12Framebuffer* GetFramebuffer(u32 backBufferIndex) const;

		/**
		 * Returns the framebuffer for the swap chain image the render thread should render to this frame. If no image
		 * has been acquired yet (e.g. a fresh swap chain), an acquire is queued on the submit thread and the method
		 * returns the resulting framebuffer. Mirrors the Vulkan surface's GetActiveFramebuffer().
		 *
		 * @note	Render thread only.
		 */
		D3D12Framebuffer* GetActiveFramebuffer();

	private:
		/** Creates the swap chain via the device resource manager and kicks the initial image acquire. */
		void CreateSwapChainInternal(u32 width, u32 height, bool vsync, D3D12SwapChain* oldSwapChain);

		D3D12GpuDevice& mDevice;
		DXGI_FORMAT mColorFormat = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT mDepthFormat = DXGI_FORMAT_UNKNOWN;
		bool mCreateDepthBuffer = false;
		bool mUseHardwareSRGB = false;
		bool mIsHeadless = false;
		bool mVsync = false;
		u32 mVsyncInterval = 1;
		D3D12SwapChain* mSwapChain = nullptr;
		HWND mWindowHandle = nullptr;
		bool mIsDestroyed = false;
	};

	/** @} */
} // namespace b3d::render
