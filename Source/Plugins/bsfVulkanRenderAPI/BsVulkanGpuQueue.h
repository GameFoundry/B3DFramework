//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanGpuDevice.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Vulkan implementation of a GPU queue. */
		class VulkanGpuQueue : public GpuQueue
		{
		public:
			VulkanGpuQueue(VulkanGpuDevice& device, GpuQueueUsage usage, u32 index, VkQueue vulkanQueue);

			void SubmitCommandBuffer(const SPtr<ct::GpuCommandBuffer>& commandBuffer, u32 syncMask) override;
			void SubmitCommandBuffers(const ArrayView<SPtr<ct::GpuCommandBuffer>>& commandBuffer, u32 syncMask) override;

			/** Returns the internal handle to the Vulkan queue object. */
			VkQueue GetHandle() const { return mQueue; }

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
			 * Submits the provided command buffer on the queue. Returns a sequential index of the submit on the queue, or ~0u if nothing was submitted.
			 *
			 * @note	Submit thread only.
			 */
			u32 Submit(VulkanInternalCommandBuffer* commandBuffer, VulkanSemaphore** waitSemaphores, u32 semaphoresCount);

			/**
			 * Stores information about a submit internally, but doesn't actually execute it. The intended use is to queue
			 * multiple submits and execute them all at once using submitQueued(), ensuring better performance than queuing them
			 * all individually.
			 *
			 * @note	Submit thread only.
			 */
			void QueueSubmit(VulkanInternalCommandBuffer* commandBuffer, VulkanSemaphore** waitSemaphores, u32 semaphoresCount);

			/**
			 * Submits all previously queued commands buffers, as recorded by QueueSubmit(). Returns a sequential index of the last submitted buffer on the queue, or ~0u if nothing was submitted.
			 *
			 * @note	Submit thread only.
			 */
			u32 SubmitQueued();

			/**
			 * Presents the back buffer of the provided swap chain.
			 *
			 * @param	swapChain			Swap chain whose back buffer to present.
			 * @param	swapChainImageIndex	Index of the swap chain image to be presented. Must have been acquired previously.
			 * @param	waitSemaphores		Optional semaphores to wait on before presenting the queue.
			 * @param	semaphoresCount		Number of semaphores in the @p semaphores array.
			 * @return						Return code of the present operation.
			 *
			 * @note	Submit thread only.
			 */
			VkResult Present(VulkanSwapChain* swapChain, u32 swapChainImageIndex, VulkanSemaphore** waitSemaphores, u32 semaphoresCount);

			/**
			 * Blocks the calling thread until all operations on the queue finish.
			 *
			 * @note	Submit thread only.
			 */
			void WaitUntilIdle() const;

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
			 * Refreshes states of the command buffers that were marked as completed by the previous call to RefreshCompletionStateOnSubmitThread. This will mark as
			 * resources as done being used by the command buffer and reset the command buffer.
			 */
			void RefreshCompletionStateOnRenderThread();

			/**
			 * Returns the last command buffer that was submitted on this queue.
			 *
			 * @note	Submit thread only.
			 */
			VulkanInternalCommandBuffer* GetLastCommandBuffer() const { return mLastSubmittedCommandBuffer; }

		protected:
			/**
			 * Generates a submit-info structure that can be used for submitting the command buffer to the queue, but doesn't
			 * perform the actual submit.
			 */
			void GetSubmitInfo(VkCommandBuffer* vkCommandBuffer, VkSemaphore* signalSemaphores, u32 signalSemaphoreCount, VkSemaphore* waitSemaphores, u32 waitSemaphoreCount, VkSubmitInfo& submitInfo);

			/**
			 * Prepares a list of semaphores that can be provided to submit or present calls. *
			 *
			 * @param[in]		inSemaphores	External wait semaphores that need to be waited on.
			 * @param[out]		outSemaphores	All semaphores (external ones, and possibly additional ones), as Vulkan handles.
			 * @param[in, out]	semaphoresCount	Number of semaphores in @p inSemaphores when calling. When method returns this
			 *									will contain number of semaphores in @p outSemaphores.
			 */
			void PrepareSemaphores(VulkanSemaphore** inSemaphores, VkSemaphore* outSemaphores, u32& semaphoresCount);

			/** Information about one or multiple submitted command buffers on a queue. */
			struct QueueSubmissionInformation
			{
				QueueSubmissionInformation(VulkanInternalCommandBuffer* lastSubmittedCommandBuffer, u32 submitIndex, u32 commandBufferCount)
					: LastSubmittedCommandBuffer(lastSubmittedCommandBuffer), SubmitIndex(submitIndex), CommandBufferCount(commandBufferCount)
				{}

				QueueSubmissionInformation(VulkanSwapChain* swapChain, u32 submitIndex, u32 commandBufferCount)
					: PresentOperationSwapChain(swapChain), SubmitIndex(submitIndex), CommandBufferCount(commandBufferCount)
				{}

				VulkanInternalCommandBuffer* LastSubmittedCommandBuffer = nullptr; /**< Last command buffer that was submitted, if the submit operation had any command buffers. */
				VulkanSwapChain* PresentOperationSwapChain = nullptr; /**< Swap chain in case the submit operation was a present operation. */
				u32 SubmitIndex;
				u32 CommandBufferCount;
			};

			/** Information about a single submitted command buffer. */
			struct QueueSubmissionEntryInformation
			{
				QueueSubmissionEntryInformation(VulkanInternalCommandBuffer* commandBuffer, u32 semaphoreCount)
					: CommandBuffer(commandBuffer), SemaphoreCount(semaphoreCount)
				{}

				VulkanInternalCommandBuffer* CommandBuffer; /**< Submitted command buffer. If null, the submission is a present call. */
				u32 SemaphoreCount;
			};

			VkQueue mQueue;
			VkPipelineStageFlags mSubmitDstWaitMask[BS_MAX_UNIQUE_QUEUES];

			Vector<QueueSubmissionEntryInformation> mQueuedCommandBuffers;
			Vector<VulkanSemaphore*> mQueuedSemaphores;

			Queue<QueueSubmissionEntryInformation> mActiveCommandBuffers;
			Queue<VulkanSemaphore*> mActiveSemaphores;

			List<QueueSubmissionInformation> mActiveSubmissions;

			VulkanInternalCommandBuffer* mLastSubmittedCommandBuffer = nullptr;
			bool mLastCBSemaphoreUsed = false;
			u32 mNextSubmitIndex = 1;

			Vector<VkSemaphore> mSemaphoresTemp;

			Mutex mMutex;
			Vector<VulkanInternalCommandBuffer*> mCommandBuffersToResetOnRenderThread;
			Vector<VulkanSemaphore*> mSemaphoresToReleaseOnRenderThread;
			Vector<VulkanSwapChain*> mPresentedSwapChainsToUnbindOnRenderThread;
		};

		/** @} */
	} // namespace ct
} // namespace bs
