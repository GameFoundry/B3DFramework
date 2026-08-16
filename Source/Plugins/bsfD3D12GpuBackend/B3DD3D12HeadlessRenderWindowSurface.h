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

	/** D3D12 render-window surface backed by offscreen color and depth images. */
	class D3D12HeadlessRenderWindowSurface final : public ID3D12RenderWindowSurface
	{
	public:
		/** Creates an offscreen surface with the requested color and depth attachments. */
		D3D12HeadlessRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation);
		~D3D12HeadlessRenderWindowSurface() override;

		void RebuildSwapChain(u32 width, u32 height, bool vsync, u32 vsyncInterval) override;
		void SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask) override;
		void MarkSwapChainAsInvalid() override;
		void Destroy() override;

		D3D12Framebuffer* GetActiveFramebuffer() override;
		bool IsSwapChainValid() const override { return mIsValid && mColorImages[mCurrentImageIndex] != nullptr; }
		D3D12Image* GetCurrentColorImage() const override { return mColorImages[mCurrentImageIndex]; }
		u32 GetWidth() const override { return mWidth; }
		u32 GetHeight() const override { return mHeight; }
		PixelFormat GetColorPixelFormat() const override { return PF_RGBA8; }

	private:
		/** Creates the offscreen images and framebuffers. */
		void CreateSurfaceResources();

		/** Releases every framebuffer and image owned by the surface. */
		void DestroySurfaceResources();

		D3D12GpuDevice& mDevice;
		const u32 mBackBufferCount;
		u32 mWidth = 0;
		u32 mHeight = 0;
		bool mVSync = false;
		u32 mVsyncInterval = 1;
		bool mCreateDepthBuffer = false;
		DXGI_FORMAT mColorFormat = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT mDepthFormat = DXGI_FORMAT_UNKNOWN;
		bool mImageAdvancePending = false;
		bool mIsValid = false;
		bool mIsDestroyed = false;
		u32 mCurrentImageIndex = 0;

		ComPtr<ID3D12Resource> mColorBuffers[kD3D12MaximumBackBufferCount];
		D3D12Image* mColorImages[kD3D12MaximumBackBufferCount] = {};
		D3D12Framebuffer* mFramebuffers[kD3D12MaximumBackBufferCount] = {};
		ComPtr<ID3D12Resource> mDepthStencilBuffer;
		D3D12Image* mDepthStencilImage = nullptr;
	};

	/** @} */
} // namespace b3d::render
