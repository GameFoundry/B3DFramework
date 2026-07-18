//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuSwapChain.h"
#include "Threading/B3DSingleConsumerQueue.h"

#include <atomic>

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** Information used to create a D3D12 swap chain. */
		struct D3D12SwapChainCreateInformation
		{
			HWND WindowHandle = nullptr;
			u32 Width = 0;
			u32 Height = 0;
			bool VSync = false;
			u32 VsyncInterval = 1;
			DXGI_FORMAT ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_UNKNOWN;
			bool CreateDepthBuffer = false;
			bool Headless = false; /**< When true no DXGI swap chain is created; offscreen textures stand in for the back buffers. */
		};

		/**
		 * DirectX 12 implementation of a swap chain. Wraps a DXGI flip-model swap chain and integrates it with the
		 * core GpuSwapChain contract so acquires and presents flow through the GpuSubmitThread.
		 *
		 * In headless mode (see D3D12SwapChainCreateInformation::Headless) no DXGI object exists; the back buffers are
		 * plain committed textures and Present() just cycles the current index. All acquire/present bookkeeping flows
		 * through the same submit-thread paths as the windowed mode so downstream code needs no special casing.
		 *
		 * @note	DXGI has no semaphore/fence based image acquire - AcquireImage() resolves to recording the DXGI
		 *			current back buffer index. Because the acquire is synchronous the acquired index is available to
		 *			the render thread immediately after AcquireImage() returns.
		 */
		class D3D12SwapChain : public GpuSwapChain
		{
			using Super = GpuSwapChain;
		public:
			D3D12SwapChain(D3D12ResourceManager* owner, const D3D12SwapChainCreateInformation& createInfo, D3D12GpuDevice& device);
			~D3D12SwapChain() override;

			/** Sets the render target that owns this swap chain (for framebuffer creation). */
			void SetRenderTarget(const RenderTarget* renderTarget);

			/** Initialize the swap chain. Must be called after construction. */
			void Initialize();

			/** Destroy the swap chain and release resources. */
			void Destroy() override;

			/** @copydoc GpuSwapChain::GetMessageQueue */
			SingleConsumerQueue& GetMessageQueue() override { return mMessageQueue; }

			/** Returns the DXGI swap chain. */
			IDXGISwapChain4* GetDXGISwapChain() const { return mSwapChain.Get(); }

			/** Returns the current back buffer index. */
			u32 GetCurrentBackBufferIndex() const;

			/** Returns the back buffer texture at the specified index. */
			ID3D12Resource* GetBackBuffer(u32 index) const;

			/** Returns the image wrapping the back buffer at the specified index. */
			D3D12Image* GetBackBufferImage(u32 index) const;

			/** Returns the image wrapping the depth-stencil buffer, or null if no depth buffer was created. */
			D3D12Image* GetDepthStencilImage() const { return mDepthStencilImage; }

			/** Returns the render target view for the specified back buffer. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRTV(u32 index) const;

			/** Returns the depth stencil view if depth buffer was created. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const { return mDepthStencilView; }

			/** Returns the framebuffer for the specified back buffer. */
			D3D12Framebuffer* GetFramebuffer(u32 index) const;

			/** Returns the number of back buffers. */
			u32 GetBackBufferCount() const { return mBackBufferCount; }

			/** Returns the information the swap chain was created with. */
			const D3D12SwapChainCreateInformation& GetCreateInformation() const { return mCreateInfo; }

			/** Returns the width of the swap chain. */
			u32 GetWidth() const { return mWidth; }

			/** Returns the height of the swap chain. */
			u32 GetHeight() const { return mHeight; }

			/** Returns the vsync present interval this swap chain was created with (0 when vsync is disabled). */
			u32 GetSyncInterval() const { return mVSync ? mVsyncInterval : 0u; }

			/**
			 * Returns the index of the back buffer that was most recently queued for present, or the current back
			 * buffer if nothing was presented yet. This is the image holding the last fully rendered frame, which is
			 * what screen captures that run after the frame has been presented need to read.
			 *
			 * @note	Render thread only.
			 */
			u32 GetLastPresentedImageIndex() const;

			/**
			 * Executes the DXGI present using the swap chain's vsync interval. In headless mode simply advances the
			 * current back buffer index.
			 *
			 * @note	Submit thread only.
			 */
			HRESULT PresentDXGI();

			/** @name Submit thread
			 *  @{
			 */

			/** @copydoc GpuSwapChain::AcquireImage */
			void AcquireImage() override;

			/** @copydoc GpuSwapChain::Present */
			void Present(u32 imageIndex, GpuQueue& queue, GpuQueueMask syncMask) override;

			/** @} */

			/** @name Render thread
			 *  @{
			 */

			/** @copydoc GpuSwapChain::TryGetFirstAcquiredImageIndex */
			bool TryGetFirstAcquiredImageIndex(u32& outImageIndex) const override;

			/**
			 * Blocks until every acquire operation queued via the submit thread has executed. Must be called before
			 * inspecting the acquired image indices, and before queuing a new acquire on the assumption that none are
			 * pending - otherwise the same back buffer can get acquired twice, permanently desyncing the
			 * acquire/present bookkeeping.
			 */
			void WaitUntilFirstImageAcquired();

			/** @copydoc GpuSwapChain::NotifyWasImageAcquireQueued */
			void NotifyWasImageAcquireQueued() override;

			/** @copydoc GpuSwapChain::NotifyWasPresentQueued */
			void NotifyWasPresentQueued(u32 imageIndex) override;

			/** @copydoc GpuSwapChain::IsRetired */
			bool IsRetired() const override { return mIsRetired; }

			/** Marks the swap chain as retired. A retired swap chain can still present already-acquired images, but cannot acquire new ones. */
			void MarkAsRetired() { mIsRetired = true; }

			/** @} */

		private:
			/** Creates the swap chain. */
			void CreateSwapChain();

			/** Retrieves the back buffer resources. */
			void GetBackBufferResources();

			/** Creates offscreen textures that stand in for the back buffers in headless mode. */
			void CreateHeadlessBackBuffers();

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
			D3D12Image* mBackBufferImages[kMaxBackBuffers] = {};
			D3D12_CPU_DESCRIPTOR_HANDLE mBackBufferRTVs[kMaxBackBuffers];
			D3D12Framebuffer* mFramebuffers[kMaxBackBuffers];
			u32 mBackBufferCount = 0;

			// The depth-stencil buffer's ComPtr/allocation are owned by its D3D12Image wrapper.
			ComPtr<ID3D12Resource> mDepthStencilBuffer;
			D3D12Image* mDepthStencilImage = nullptr;
			D3D12_CPU_DESCRIPTOR_HANDLE mDepthStencilView;

			u32 mWidth = 0;
			u32 mHeight = 0;
			bool mVSync = false;
			u32 mVsyncInterval = 1;
			bool mIsInitialized = false;
			bool mIsRetired = false;
			bool mIsHeadless = false;

			/**
			 * Current back buffer index in headless mode (DXGI tracks it internally otherwise). Written on the submit
			 * thread when a present cycles buffers, read from the render thread and acquire workers.
			 */
			std::atomic<u32> mHeadlessBackBufferIndex{ 0 };

			/** Index of the most recently presented back buffer, or -1 if nothing was presented yet. Render thread only. */
			i32 mLastPresentedImageIndex = -1;

			/**
			 * Indices of images that have been acquired (via AcquireImage) but not yet queued for present, along with
			 * the number of acquire operations queued on the submit thread but not yet executed. Guarded by a mutex
			 * because AcquireImage runs on the submit thread (or its workers) while TryGetFirstAcquiredImageIndex /
			 * NotifyWasImageAcquireQueued / NotifyWasPresentQueued run on the render thread.
			 */
			mutable Mutex mAcquireMutex;
			Signal mAcquireSignal;
			TInlineArray<u32, 4> mAcquiredImageIndices;
			u32 mPendingAcquireCount = 0;

			SingleConsumerQueue mMessageQueue;
		};

		/** @} */
	} // namespace render
} // namespace b3d
