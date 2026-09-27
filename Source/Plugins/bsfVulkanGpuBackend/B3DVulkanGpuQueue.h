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
		struct VulkanGpuCommandBufferSubmitInformation;

		/** @addtogroup Vulkan
		 *  @{
		 */

		/**
		 * Vulkan implementation of a GPU queue.
		 *
		 * All engine-internal ordering is expressed through a single timeline semaphore owned by each queue: every
		 * submission signals the queue's timeline with a monotonically increasing value, and inter-queue dependencies
		 * wait on the source queue's timeline at its last submitted value. Binary semaphores appear only at the
		 * swapchain boundary (image acquisition and present), where the Vulkan API requires them.
		 *
		 * Unless a method notes otherwise, all queue state is confined to the submit thread and requires no locking.
		 * Submissions retire strictly in FIFO order.
		 */
		class VulkanGpuQueue : public GpuQueue
		{
		public:
			VulkanGpuQueue(VulkanGpuDevice& device, GpuQueueType type, u32 index, VkQueue vulkanQueue);
			~VulkanGpuQueue() override;

			void SubmitCommandBuffer(const GpuSubmissionInformation& information) override;
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
			void ExecuteSubmitOnSubmitThread(const VulkanGpuCommandBufferSubmitInformation& submitInformation, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences);

			/** Returns the internal handle to the Vulkan queue object. */
			VkQueue GetVulkanHandle() const { return mQueue; }

			/** Returns the device that owns the queue. */
			VulkanGpuDevice& GetDevice() const { return static_cast<VulkanGpuDevice&>(mGpuDevice); }

			/**
			 * Presents the back buffer of the provided swap chain, after waiting for the provided semaphores and for the
			 * queues selected by @p syncMask.
			 *
			 * @param	swapChain			Swap chain whose back buffer to present.
			 * @param	swapChainImageIndex	Index of the swap chain image to be presented. Must have been acquired previously.
			 * @param	syncMask			Inter-queue sync mask.
			 * @param	waitSemaphores		Optional semaphores to wait on before presenting.
			 * @return						Return code of the present operation.
			 *
			 * @note	Submit thread only.
			 */
			VkResult Present(VulkanSwapChain* swapChain, u32 swapChainImageIndex, GpuQueueMask syncMask, TArrayView<VulkanSemaphore* const> waitSemaphores);

			/**
			 * Checks if any of the active command buffers finished executing on the queue and updates their states accordingly. Note that you must follow this call
			 * with a call to RefreshCompletionStateOnRenderThread() in order for the states to correctly update if the command buffers are owned by the render thread.
			 *
			 * @param	forceWait		Set to true if the system should wait until all command buffers finish executing.
			 * @param	lastSubmitIndex	Index of the last submitted command buffer which should be checked. If ~0u is provided, all submitted command buffers will be checked.
			 *
			 * @note	Submit thread only.
			 */
			void RefreshCompletionState(bool forceWait, u32 lastSubmitIndex = ~0u);

			/**
			 * Returns the submit index of the most recently submitted work on this queue, or 0 if nothing has been
			 * submitted yet. Capture this at a frame boundary and pass it to RefreshCompletionState() to wait for all
			 * of that frame's work to complete.
			 *
			 * @note	Submit thread only.
			 */
			u32 GetLastSubmitIndex() const { return mNextSubmitIndex - 1; }

			/**
			 * Returns the progress timeline value of the most recent submission on this queue, or 0 if nothing has been
			 * submitted yet.
			 *
			 * @note	Submit thread only.
			 */
			u64 GetLastSubmittedProgressValue() const { return mLastSubmittedProgressValue; }

		protected:
			/**
			 * Scratch storage backing a single VkSubmitInfo (or VkPresentInfoKHR). Its data must stay untouched until the
			 * corresponding vkQueueSubmit/vkQueuePresentKHR call returns.
			 */
			struct SubmitWorkBuffer
			{
				TInlineArray<VkSemaphore, 8> WaitSemaphores;
				TInlineArray<u64, 8> WaitValues;
				TInlineArray<VkPipelineStageFlags, 8> WaitStages;
				TInlineArray<VkSemaphore, 8> SignalSemaphores;
				TInlineArray<u64, 8> SignalValues;
				TInlineArray<VkCommandBuffer, 2> CommandBuffers;
				VkTimelineSemaphoreSubmitInfo TimelineSubmitInfo = {};

				/** Appends a wait operation, keeping all wait arrays in lockstep. Use a zero @p value for binary semaphores. */
				void AddWait(VkSemaphore semaphore, u64 value = 0, VkPipelineStageFlags stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

				/** Appends a signal operation, keeping both signal arrays in lockstep. Use a zero @p value for binary semaphores. */
				void AddSignal(VkSemaphore semaphore, u64 value = 0);

				void Clear();
			};

			/** A submitted command buffer batch together with everything that must stay alive until the GPU finishes executing it. */
			struct SubmissionRecord
			{
				u32 SubmitIndex = 0;
				TInlineArray<TShared<VulkanGpuCommandBuffer>, 2> CommandBuffers; /**< Command buffers in submission order. The last one owns the fence used to detect completion. */
				TInlineArray<VulkanSemaphore*, 4> RetainedSemaphores; /**< Managed semaphores the submission waits on, released on retirement. */
				TInlineArray<TShared<GpuTimelineFence>, 2> RetainedFences; /**< Keeps fences the submission signals alive until the signal has executed. */
			};

			/** Appends timeline waits covering the last submission of every source queue selected by @p syncMask. Submit thread only. */
			void AppendSyncMaskWaits(GpuQueueMask syncMask, SubmitWorkBuffer& outWorkBuffer) const;

			/**
			 * Appends the waits of the frame fence if this is the queue's first submission since the frame boundary. Skips
			 * other queues in @p syncMask, whose latest value AppendSyncMaskWaits() already waits on. Submit thread only.
			 */
			void AppendFrameFenceWaits(GpuQueueMask syncMask, SubmitWorkBuffer& outWorkBuffer);

			/** Marks the semaphores as in use by this queue and stores them in @p outRecord for release on retirement. Submit thread only. */
			void RetainSemaphores(SubmissionRecord& outRecord, TArrayView<VulkanSemaphore* const> semaphores);

			/** Wires @p outWorkBuffer's arrays into a VkSubmitInfo. The returned structure points into @p outWorkBuffer and shares its lifetime. */
			VkSubmitInfo BuildVkSubmitInfo(SubmitWorkBuffer& outWorkBuffer) const;

			VkQueue mQueue;
			VkSemaphore mProgressTimeline = VK_NULL_HANDLE;
			u64 mNextProgressValue = 1;
			u64 mLastSubmittedProgressValue = 0; /**< Timeline value of the most recent successful submission, or 0 if nothing was submitted yet. */

			u32 mNextSubmitIndex = 1;
			Deque<SubmissionRecord> mActiveSubmissions;

			Array<SubmitWorkBuffer, 2> mSubmitWorkBuffers; /**< Slot 0 backs the transition submit (or a present), slot 1 the primary submit. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
