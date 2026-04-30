//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12GpuDevice.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** DirectX 12 implementation of a GPU queue. */
		class D3D12GpuQueue : public GpuQueue
		{
		public:
			D3D12GpuQueue(D3D12GpuDevice& device, GpuQueueUsage usage, u32 index, ID3D12CommandQueue* d3d12Queue);
			~D3D12GpuQueue();

			void SubmitCommandBuffer(const GpuSubmissionInformation& information, bool flushTransferCommandBuffer = true) override;
			void WaitUntilIdle() override;
			void PresentRenderWindow(const SPtr<RenderWindow>& renderWindow, u32 syncMask = 0xFFFFFFFF) override;

			/** Returns the internal handle to the D3D12 queue object. */
			ID3D12CommandQueue* GetD3D12Handle() const { return mQueue.Get(); }

			/** Returns the device that owns the queue. */
			D3D12GpuDevice& GetDevice() const { return static_cast<D3D12GpuDevice&>(mGpuDevice); }

			/**
			 * Submits a command list to the queue.
			 *
			 * @param commandLists		Array of command lists to submit.
			 * @param numCommandLists	Number of command lists in the array.
			 *
			 * @note	Submit thread only.
			 */
			void ExecuteCommandLists(ID3D12CommandList* const* commandLists, u32 numCommandLists);

			/**
			 * Signals the fence with the specified value when all previous work has completed.
			 *
			 * @param fence		Fence to signal.
			 * @param value		Value to signal with.
			 *
			 * @note	Submit thread only.
			 */
			void Signal(ID3D12Fence* fence, u64 value);

			/**
			 * Makes the queue wait until the fence reaches the specified value.
			 *
			 * @param fence		Fence to wait on.
			 * @param value		Value to wait for.
			 *
			 * @note	Submit thread only.
			 */
			void Wait(ID3D12Fence* fence, u64 value);

			/**
			 * Presents the back buffer of the provided swap chain.
			 *
			 * @param swapChain		Swap chain whose back buffer to present.
			 * @return				Return code of the present operation.
			 *
			 * @note	Submit thread only.
			 */
			HRESULT Present(D3D12SwapChain* swapChain);

		private:
			ComPtr<ID3D12CommandQueue> mQueue;
			ComPtr<ID3D12Fence> mFence;
			u64 mNextFenceValue = 1;
			HANDLE mFenceEvent = nullptr;
		};

		/** @} */
	} // namespace render
} // namespace b3d
