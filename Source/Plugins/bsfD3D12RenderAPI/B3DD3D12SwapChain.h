//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
		 *  @{
		 */

		/** Information used to create a D3D12 swap chain. */
		struct D3D12SwapChainCreateInformation
		{
			HWND WindowHandle = nullptr;
			u32 Width = 0;
			u32 Height = 0;
			bool VSync = false;
			DXGI_FORMAT ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_UNKNOWN;
			bool CreateDepthBuffer = false;
		};

		/** DirectX 12 implementation of a swap chain. */
		class D3D12SwapChain
		{
		public:
			D3D12SwapChain(const D3D12SwapChainCreateInformation& createInfo, D3D12GpuDevice& device);
			~D3D12SwapChain();

			/** Sets the render target that owns this swap chain (for framebuffer creation). */
			void SetRenderTarget(const RenderTarget* renderTarget);

			/** Initialize the swap chain. Must be called after construction. */
			void Initialize();

			/** Destroy the swap chain and release resources. */
			void Destroy();

			/** Returns the DXGI swap chain. */
			IDXGISwapChain4* GetDXGISwapChain() const { return mSwapChain.Get(); }

			/** Returns the current back buffer index. */
			u32 GetCurrentBackBufferIndex() const;

			/** Returns the back buffer texture at the specified index. */
			ID3D12Resource* GetBackBuffer(u32 index) const;

			/** Returns the render target view for the specified back buffer. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRTV(u32 index) const;

			/** Returns the depth stencil view if depth buffer was created. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const { return mDepthStencilView; }

			/** Returns the framebuffer for the specified back buffer. */
			D3D12Framebuffer* GetFramebuffer(u32 index) const;

			/** Returns the number of back buffers. */
			u32 GetBackBufferCount() const { return mBackBufferCount; }

			/** Returns the width of the swap chain. */
			u32 GetWidth() const { return mWidth; }

			/** Returns the height of the swap chain. */
			u32 GetHeight() const { return mHeight; }

			/** Presents the current back buffer. */
			HRESULT Present(u32 syncInterval);

		private:
			/** Creates the swap chain. */
			void CreateSwapChain();

			/** Retrieves the back buffer resources. */
			void GetBackBufferResources();

			/** Creates render target views for back buffers. */
			void CreateRenderTargetViews();

			/** Creates depth stencil buffer and view. */
			void CreateDepthStencilBuffer();

			/** Creates framebuffers for all back buffers. */
			void CreateFramebuffers();

			D3D12GpuDevice& mDevice;
			D3D12SwapChainCreateInformation mCreateInfo;
			ComPtr<IDXGISwapChain4> mSwapChain;
			const RenderTarget* mRenderTarget = nullptr;

			static constexpr u32 kMaxBackBuffers = 3;
			ComPtr<ID3D12Resource> mBackBuffers[kMaxBackBuffers];
			D3D12_CPU_DESCRIPTOR_HANDLE mBackBufferRTVs[kMaxBackBuffers];
			UniquePtr<D3D12Framebuffer> mFramebuffers[kMaxBackBuffers];
			u32 mBackBufferCount = 0;

			ComPtr<ID3D12Resource> mDepthStencilBuffer;
			D3D12MA::Allocation* mDepthStencilAllocation = nullptr;
			D3D12_CPU_DESCRIPTOR_HANDLE mDepthStencilView;

			u32 mWidth = 0;
			u32 mHeight = 0;
			bool mIsInitialized = false;
		};

		/** @} */
	} // namespace render
} // namespace b3d
