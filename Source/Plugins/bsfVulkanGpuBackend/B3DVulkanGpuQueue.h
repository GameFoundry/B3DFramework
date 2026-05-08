//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanGpuDevice.h"
#include "GpuBackend/B3DGpuTimelineFence.h"

namespace b3d
{
	namespace render
	{
		struct GpuCommandBufferSubmitInformation;
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Vulkan implementation of a GPU queue. */
		class VulkanGpuQueue : public GpuQueue
		{
		public:
			VulkanGpuQueue(VulkanGpuDevice& device, GpuQueueType type, u32 index, VkQueue vulkanQueue);

			void SubmitCommandBuffer(const GpuSubmissionInformation& information, bool flushTransferCommandBuffer = true) override;
			void WaitUntilIdle() override;
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;

			/**
			 * Submits a command buffer on the queue using information prepared by the command buffer.
			 *
			 * @param	submitInformation	Vulkan-specific per-CB submit info (transitions, semaphores).
			 * @param	syncMask			Inter-queue sync mask.
			 * @param	signalFences		Explicit list of GpuTimelineFence + value pairs to signal when the command buffer
			 *								finishes executing.
			 *
			 * @note	Submit thread only.
			 */
			void ExecuteSubmitOnSubmitThread(const GpuCommandBufferSubmitInformation& submitInformation, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences);

			/** Returns the internal handle to the Vulkan queue object. */
			VkQueue GetVulkanHandle() const { return mQueue; }

			/** Returns the device that owns the queue. */
			VulkanGpuDevice& GetDevice() const { return static_cast<VulkanGpuDevice&>(mGpuDevice); }

			/**
			 * Checks if anything is currently executing on this queue.
			 *
			 * @note	This status is only updated after RefreshCompletionStateOnSubmitThread has been called.
			 * @note	Submit thread only.
			 */
			bool IsExecuting() const;

			/**
			 * Presents the back buffer of the provided swap chain.
			 *
			 * @param	swapChain			Swap chain whose back buffer to present.
			 * @param	swapChainImageIndex	Index of the swap chain image to be presented. Must have been acquired previously.
			 * @param	waitSemaphores		Optional semaphores to wait on before presenting the queue.
			 * @return						Return code of the present operation.
			 *
			 * @note	Submit thread only.
			 */
			VkResult Present(VulkanSwapChain* swapChain, u32 swapChainImageIndex, TArrayView<VulkanSemaphore*> waitSemaphores);

			/**
			 * Checks if any of the active command buffers finished executing on the queue and updates their states accordingly. Note that you must follow this call
			 * with a call to RefreshCompletionStateOnRenderThread() in order for the states to correctly update if the command buffers are owned by the render thread.
			 *
			 * @param	forceWait				Set to true if the system should wait until all command buffers finish executing.
			 * @param	queueEmpty				Set to true if the caller guarantees the queue will be empty (e.g. on shutdown). This
			 *									allows the system to free all needed resources.
			 * @param	lastSubmitIndex			Index of the last submitted command buffer which should be checked. If ~0u is provided, all submitted command buffers will be checked.
			 *
			 * @note	Submit thread only.
			 */
			void RefreshCompletionStateOnSubmitThread(bool forceWait, bool queueEmpty = false, u32 lastSubmitIndex = ~0u);

			/**
			 * Returns the last command buffer that was submitted on this queue.
			 *
			 * @note	Submit thread only.
			 */
			TShared<VulkanGpuCommandBuffer> GetLastCommandBuffer() const { return mLastSubmittedCommandBuffer; }

		protected:
			/**
			 * Prepares a list of semaphores that can be provided to submit or present calls.
			 *
			 * @param		inSemaphores	External wait semaphores that need to be waited on.
			 * @param		outSemaphores	All semaphores (external ones, and possibly additional ones), as Vulkan handles. To be appended to this array.
			 * @return						Number of semaphores appended to the output array.
			 */
			u32 RegisterSemaphoresAndGetHandles(const TArrayView<VulkanSemaphore*>& inSemaphores, TInlineArray<VkSemaphore, 8>& outSemaphores);

			/** Information about one or multiple submitted command buffers on a queue. */
			struct QueueSubmissionInformation
			{
				QueueSubmissionInformation(const TShared<VulkanGpuCommandBuffer>& lastSubmittedCommandBuffer, u32 submitIndex, u32 commandBufferCount)
					: LastSubmittedCommandBuffer(lastSubmittedCommandBuffer), SubmitIndex(submitIndex), CommandBufferCount(commandBufferCount)
				{}

				QueueSubmissionInformation(VulkanSwapChain* swapChain, u32 submitIndex, u32 commandBufferCount)
					: PresentOperationSwapChain(swapChain), SubmitIndex(submitIndex), CommandBufferCount(commandBufferCount)
				{}

				TShared<VulkanGpuCommandBuffer> LastSubmittedCommandBuffer; /**< Last command buffer that was submitted, if the submit operation had any command buffers. */
				VulkanSwapChain* PresentOperationSwapChain = nullptr; /**< Swap chain in case the submit operation was a present operation. */
				u32 SubmitIndex;
				u32 CommandBufferCount;
			};

			/** Information about a single submitted command buffer. */
			struct QueueSubmissionEntryInformation
			{
				QueueSubmissionEntryInformation(const TShared<VulkanGpuCommandBuffer>& commandBuffer, u32 semaphoreCount)
					: CommandBuffer(commandBuffer), SemaphoreCount(semaphoreCount)
				{}

				TShared<VulkanGpuCommandBuffer> CommandBuffer; /**< Submitted command buffer. If null, the submission is a present call. */
				u32 SemaphoreCount;
			};

			/**
			 * Per-submit scratch storage backing a single VkSubmitInfo (or VkPresentInfoKHR). Each submit owns
			 * its own SubmitWorkBuffer pulled from a pool.
			 */
			struct SubmitWorkBuffer
			{
				TInlineArray<VkSemaphore, 8> SignalSemaphores;
				TInlineArray<u64, 8> SignalValues;
				TInlineArray<VkSemaphore, 8> WaitSemaphores;
				TInlineArray<VkCommandBuffer, 8> CommandBuffers;
				VkTimelineSemaphoreSubmitInfo TimelineSubmitInfo = {};

				void Clear();
			};

			/**
			 * Registers the command buffer for submission and generates the VkSubmitInfo structure that can be submitted to the queue.
			 *
			 * @param	commandBuffer		Command buffer to be submitted.
			 * @param	waitSemaphores		Set of semaphores that should be waited on before the command buffers start executing.
			 * @param	signalFences		Optional timeline-fence + value pairs to also signal when this submit finishes.
			 */
			VkSubmitInfo RegisterSubmissionAndGenerateSubmitInfo(const TShared<VulkanGpuCommandBuffer>& commandBuffer, const TArrayView<VulkanSemaphore*>& waitSemaphores, TArrayView<const GpuTimelineFenceAndValue> signalFences = {});

			/**
			 * Registers the set of command buffers for submission and generates the VkSubmitInfo structure that can be submitted to the queue.
			 *
			 * @param	commandBuffers		One or multiple command buffers to be submitted.
			 * @param	waitSemaphores		Set of semaphores that should be waited on before the command buffers start executing.
			 * @param	signalFences		Optional timeline-fence + value pairs to also signal when this submit finishes.
			 */
			VkSubmitInfo RegisterSubmissionAndGenerateSubmitInfo(const TArrayView<TShared<VulkanGpuCommandBuffer>>& commandBuffers, const TArrayView<VulkanSemaphore*>& waitSemaphores, TArrayView<const GpuTimelineFenceAndValue> signalFences = {});

			/**
			 * Acquires a SubmitWorkBuffer for a new submit. Grows the pool on first use, but reuses existing
			 * heap-allocated buffers across frames (their previously grown capacity is retained). 
			 *
			 * @note	Submit thread only.
			 */
			SubmitWorkBuffer& AcquireSubmitWorkBuffer();

			/**
			 * Releases all currently acquired SubmitWorkBuffers back to the pool for reuse. Call after the
			 * submitted VkSubmitInfo array has been handed to vkQueueSubmit (or vkQueuePresentKHR), at which
			 * point Vulkan no longer references the buffers' data pointers.
			 *
			 * @note	Submit thread only.
			 */
			void ReleaseAllSubmitWorkBuffers();

			VkQueue mQueue;
			VkPipelineStageFlags mSubmitDstWaitMask[B3D_MAX_UNIQUE_QUEUES];
			mutable Mutex mMutex;

			Queue<QueueSubmissionEntryInformation> mActiveCommandBuffers;
			Queue<VulkanSemaphore*> mActiveSemaphores;

			List<QueueSubmissionInformation> mActiveSubmissions;

			TShared<VulkanGpuCommandBuffer> mLastSubmittedCommandBuffer;
			bool mLastCBSemaphoreUsed = false;
			u32 mNextSubmitIndex = 1;

			Vector<UPtr<SubmitWorkBuffer>> mSubmitWorkBufferPool;
			u32 mActiveSubmitWorkBufferCount = 0;

			TInlineArray<VulkanSemaphore*, 8> mWaitSemaphoreBuffer; // Input wrapper for ExecuteSubmitOnSubmitThread; cleared between adjacent submits.
		};

		/** @} */
	} // namespace render
} // namespace b3d
