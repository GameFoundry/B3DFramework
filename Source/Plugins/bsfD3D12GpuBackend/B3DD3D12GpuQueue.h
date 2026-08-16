//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12GpuDevice.h"
#include "GpuBackend/B3DGpuTimelineFence.h"

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
			/** Creates a GPU queue wrapping @p d3d12Queue. */
			D3D12GpuQueue(D3D12GpuDevice& device, GpuQueueType type, u32 index, ID3D12CommandQueue* d3d12Queue);
			~D3D12GpuQueue() override;

			/**
			 * @name GpuQueue Interface
			 * @{
			 */

			void SubmitCommandBuffer(const GpuSubmissionInformation& information) override;
			void WaitUntilIdle() override;
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;

			/** @} */

			/** Returns the internal handle to the D3D12 queue object. */
			ID3D12CommandQueue* GetD3D12Handle() const { return mQueue.Get(); }

			/** Returns the device that owns the queue. */
			D3D12GpuDevice& GetDevice() const { return static_cast<D3D12GpuDevice&>(mGpuDevice); }

			/**
			 * Executes a previously recorded command buffer on the queue, waiting for work on other queues per
			 * @p syncMask and signaling the provided timeline fences when execution completes. Any queue transitions
			 * the submission carries are submitted to their own queues first, followed by this queue's transition
			 * command buffer, and only then the recorded command buffer itself.
			 *
			 * @note	Submit thread only.
			 */
			void ExecuteSubmitOnSubmitThread(const D3D12GpuCommandBufferSubmitInformation& submitInformation, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences);

			/**
			 * Checks if any of the active command buffers finished executing on the queue and updates their states
			 * accordingly (completion notifications are posted back to the command buffers' owning threads).
			 *
			 * @param	forceWait		Set to true if the system should wait until all command buffers finish executing.
			 * @param	lastSubmitIndex	Index of the last submitted command buffer which should be checked. If ~0u is
			 *							provided, all submitted command buffers will be checked.
			 *
			 * @note	Submit thread only.
			 */
			void RefreshCompletionState(bool forceWait, u32 lastSubmitIndex = ~0u);

			/**
			 * Returns the submit index of the most recently submitted work on this queue, or 0 if nothing has been
			 * submitted yet.
			 *
			 * @note	Submit thread only.
			 */
			u32 GetLastSubmitIndex() const { return mNextSubmitIndex - 1; }

			/**
			 * Checks if anything is currently executing on this queue.
			 *
			 * @note	This status is only updated after RefreshCompletionState has been called.
			 * @note	Submit thread only.
			 */
			bool IsExecuting() const;

			/**
			 * Returns the fence that is signaled after every submit on this queue. Used together with
			 * GetLastSignaledFenceValue() for cross-queue synchronization.
			 *
			 * @note	Submit thread only.
			 */
			ID3D12Fence* GetSubmitFence() const { return mFence.Get(); }

			/** Returns the value the submit fence was last signaled with. @note Submit thread only. */
			u64 GetLastSignaledFenceValue() const { return mLastSignaledFenceValue; }

			/**
			 * Submits command lists to the queue.
			 *
			 * @param commandLists		Array of command lists to submit.
			 * @param commandListCount	Number of command lists in the array.
			 *
			 * @note	Submit thread only.
			 */
			void ExecuteCommandLists(ID3D12CommandList* const* commandLists, u32 commandListCount);

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
			 * Presents the back buffer of the provided swap chain after waiting on any outstanding work on the queues
			 * in @p syncMask. The present uses the swap chain's own vsync interval.
			 *
			 * @param swapChain		Swap chain whose back buffer to present.
			 * @param syncMask		Mask that controls which other queues the present depends upon (if any).
			 * @return				Return code of the DXGI present operation.
			 *
			 * @note	Submit thread only.
			 */
			HRESULT Present(D3D12SwapChain* swapChain, GpuQueueMask syncMask = GpuQueueMask::kAll);

			/**
			 * Blocks the calling thread until all work submitted to the queue completes, using a native fence wait
			 * (bypassing the submit thread).
			 *
			 * @note	Only valid on the submit thread, or after the submit thread has been stopped.
			 */
			void WaitUntilIdleNative();

		private:
			/** Information about a command buffer submission on the queue. */
			struct QueueSubmissionInformation
			{
				/** Retains the command buffers belonging to submission @p submitIndex until completion. */
				QueueSubmissionInformation(const TShared<D3D12GpuCommandBuffer>& commandBuffer, const TShared<D3D12GpuCommandBuffer>& transitionCommandBuffer, u32 submitIndex) : CommandBuffer(commandBuffer), TransitionCommandBuffer(transitionCommandBuffer), SubmitIndex(submitIndex)
				{}

				TShared<D3D12GpuCommandBuffer> CommandBuffer; /**< Recorded command buffer this submission was made for. */
				TShared<D3D12GpuCommandBuffer> TransitionCommandBuffer; /**< Internal prologue command buffer to execute before @p CommandBuffer. */
				u32 SubmitIndex; /**< Value GetLastSubmitIndex() reported for this submission. */
			};

			ComPtr<ID3D12CommandQueue> mQueue;
			ComPtr<ID3D12Fence> mFence;
			u64 mNextFenceValue = 1;
			u64 mLastSignaledFenceValue = 0;
			HANDLE mFenceEvent = nullptr;

			mutable Mutex mMutex;
			List<QueueSubmissionInformation> mActiveSubmissions;
			TShared<D3D12GpuCommandBuffer> mLastSubmittedCommandBuffer;
			u32 mNextSubmitIndex = 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d
