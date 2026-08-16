//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuSwapChain.h"
#include "Threading/B3DSingleConsumerQueue.h"

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
			HWND WindowHandle = nullptr; /**< Native window handle used by the DXGI swap chain. */
			u32 Width = 0; /**< Back-buffer width in pixels. */
			u32 Height = 0; /**< Back-buffer height in pixels. */
			bool VSync = false; /**< Whether presentation waits for vertical synchronization. */
			u32 VsyncInterval = 1; /**< Number of vertical blanks between presents when VSync is enabled. */
			DXGI_FORMAT ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM; /**< Back-buffer format. */
			DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_UNKNOWN; /**< Optional depth-stencil format. */
			bool CreateDepthBuffer = false; /**< Whether to create a depth-stencil attachment. */
		};

		/**
		 * DirectX 12 implementation of a swap chain. Wraps a DXGI flip-model swap chain and integrates it with the
		 * core GpuSwapChain contract so acquires and presents flow through the GpuSubmitThread.
		 *
		 * @note	DXGI has no semaphore/fence based image acquire - AcquireImage() resolves to recording the DXGI
		 *			current back buffer index. Because the acquire is synchronous the acquired index is available to
		 *			the render thread immediately after AcquireImage() returns.
		 */
		class D3D12SwapChain : public GpuSwapChain
		{
			using Super = GpuSwapChain;
		public:
			/** Creates an uninitialized swap chain owned by @p owner. */
			D3D12SwapChain(D3D12ResourceManager* owner, const D3D12SwapChainCreateInformation& createInformation, D3D12GpuDevice& device);
			~D3D12SwapChain() override;

			/** Creates the native swap chain and all resources associated with its images. */
			void Initialize();

			void Destroy() override;
			SingleConsumerQueue& GetMessageQueue() override { return mMessageQueue; }

			/** Returns the current back buffer index. */
			u32 GetCurrentBackBufferIndex() const;

			/** Returns the image wrapping the back buffer at the specified index. */
			D3D12Image* GetBackBufferImage(u32 index) const;

			/** Returns the framebuffer for @p index. Render thread only. */
			D3D12Framebuffer* GetFramebufferForImage(u32 index) const;

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

			/** @name Submit thread
			 *  @{
			 */

			/** Executes the DXGI present using the swap chain's vsync interval. */
			HRESULT PresentDXGI();

			void AcquireImage() override;
			void Present(u32 imageIndex, GpuQueue& queue, GpuQueueMask syncMask) override;

			/** @} */

			/** @name Render thread
			 *  @{
			 */

			/**
			 * Blocks until every acquire operation queued via the submit thread has executed. Must be called before
			 * inspecting the acquired image indices, and before queuing a new acquire on the assumption that none are
			 * pending - otherwise the same back buffer can get acquired twice, permanently desyncing the
			 * acquire/present bookkeeping.
			 */
			void WaitForPendingAcquires();

			/** Marks the swap chain as retired. A retired swap chain can still present already-acquired images, but cannot acquire new ones. */
			void MarkAsRetired() { mIsRetired = true; }

			bool TryGetFirstAcquiredImageIndex(u32& outImageIndex) const override;
			void NotifyWasImageAcquireQueued() override;
			void NotifyWasPresentQueued(u32 imageIndex) override;
			bool IsRetired() const override { return mIsRetired; }

			/** @} */

		private:
			D3D12GpuDevice& mDevice;
			D3D12SwapChainCreateInformation mCreateInformation;
			ComPtr<IDXGISwapChain4> mSwapChain;

			ComPtr<ID3D12Resource> mBackBuffers[kD3D12MaximumBackBufferCount];
			D3D12Image* mBackBufferImages[kD3D12MaximumBackBufferCount] = {};

			D3D12Framebuffer* mFramebuffers[kD3D12MaximumBackBufferCount] = {};
			u32 mBackBufferCount = 0;

			/** The depth-stencil allocation is owned by its image wrapper. */
			ComPtr<ID3D12Resource> mDepthStencilBuffer;
			D3D12Image* mDepthStencilImage = nullptr;

			u32 mWidth = 0;
			u32 mHeight = 0;
			bool mVSync = false;
			u32 mVsyncInterval = 1;
			bool mIsInitialized = false;
			bool mIsRetired = false;

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
