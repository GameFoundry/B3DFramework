//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanGpuDevice.h"

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
			VulkanGpuQueue(VulkanGpuDevice& device, GpuQueueType usage, u32 index, VkQueue vulkanQueue);

			void SubmitCommandBuffer(const SPtr<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, bool flushTransferCommandBuffer) override;
			void WaitUntilIdle() override;
			void PresentRenderWindow(const SPtr<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;

			/**
			 * Submits a command buffer on the queue using information prepared by the command buffer.
			 *
			 * @note	Submit thread only.
			 */
			void ExecuteSubmitOnSubmitThread(const GpuCommandBufferSubmitInformation& submitInformation, GpuQueueMask syncMask);

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
			SPtr<VulkanGpuCommandBuffer> GetLastCommandBuffer() const { return mLastSubmittedCommandBuffer; }

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
				QueueSubmissionInformation(const SPtr<VulkanGpuCommandBuffer>& lastSubmittedCommandBuffer, u32 submitIndex, u32 commandBufferCount)
					: LastSubmittedCommandBuffer(lastSubmittedCommandBuffer), SubmitIndex(submitIndex), CommandBufferCount(commandBufferCount)
				{}

				QueueSubmissionInformation(VulkanSwapChain* swapChain, u32 submitIndex, u32 commandBufferCount)
					: PresentOperationSwapChain(swapChain), SubmitIndex(submitIndex), CommandBufferCount(commandBufferCount)
				{}

				SPtr<VulkanGpuCommandBuffer> LastSubmittedCommandBuffer; /**< Last command buffer that was submitted, if the submit operation had any command buffers. */
				VulkanSwapChain* PresentOperationSwapChain = nullptr; /**< Swap chain in case the submit operation was a present operation. */
				u32 SubmitIndex;
				u32 CommandBufferCount;
			};

			/** Information about a single submitted command buffer. */
			struct QueueSubmissionEntryInformation
			{
				QueueSubmissionEntryInformation(const SPtr<VulkanGpuCommandBuffer>& commandBuffer, u32 semaphoreCount)
					: CommandBuffer(commandBuffer), SemaphoreCount(semaphoreCount)
				{}

				SPtr<VulkanGpuCommandBuffer> CommandBuffer; /**< Submitted command buffer. If null, the submission is a present call. */
				u32 SemaphoreCount;
			};

			/**
			 * Registers the command buffer for submission and generates the VkSubmitInfo structure that can be submitted to the queue.
			 *
			 * @param	commandBuffer		Command buffer to be submitted.
			 * @param	waitSemaphores		Set of semaphores that should be waited on before the command buffers start executing.
			 */
			VkSubmitInfo RegisterSubmissionAndGenerateSubmitInfo(const SPtr<VulkanGpuCommandBuffer>& commandBuffer, const TArrayView<VulkanSemaphore*>& waitSemaphores);

			/**
			 * Registers the set of command buffers for submission and generates the VkSubmitInfo structure that can be submitted to the queue.
			 *
			 * @param	commandBuffers		One or multiple command buffers to be submitted.
			 * @param	waitSemaphores		Set of semaphores that should be waited on before the command buffers start executing.
			 */
			VkSubmitInfo RegisterSubmissionAndGenerateSubmitInfo(const TArrayView<SPtr<VulkanGpuCommandBuffer>>& commandBuffers, const TArrayView<VulkanSemaphore*>& waitSemaphores);

			VkQueue mQueue;
			VkPipelineStageFlags mSubmitDstWaitMask[B3D_MAX_UNIQUE_QUEUES];

			Queue<QueueSubmissionEntryInformation> mActiveCommandBuffers;
			Queue<VulkanSemaphore*> mActiveSemaphores;

			List<QueueSubmissionInformation> mActiveSubmissions;

			SPtr<VulkanGpuCommandBuffer> mLastSubmittedCommandBuffer;
			bool mLastCBSemaphoreUsed = false;
			u32 mNextSubmitIndex = 1;

			TInlineArray<VkSemaphore, 8> mSignalSemaphoreHandleBuffer; // Helper to avoid re-allocating memory
			TInlineArray<VkSemaphore, 8> mWaitSemaphoreHandleBuffer; // Helper to avoid re-allocating memory
			TInlineArray<VulkanSemaphore*, 8> mWaitSemaphoreBuffer; // Helper to avoid re-allocating memory
			TInlineArray<VkCommandBuffer, 8> mCommandBufferHandleBuffer; // Helper to avoid re-allocating memory
		};

		/** @} */
	} // namespace render
} // namespace b3d
