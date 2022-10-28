//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper around a command buffer used specifically for transfer operations. */
		class VulkanTransferBuffer
		{
		public:
			VulkanTransferBuffer() = default;
			VulkanTransferBuffer(VulkanDevice* device, GpuQueueType type, u32 queueIdx);
			~VulkanTransferBuffer();

			/**
			 * OR's the provided sync mask with the internal sync mask. The sync mask determines on which queues should
			 * the buffer wait on before executing. Sync mask is reset after a flush. See CommandSyncMask on how to generate
			 * a sync mask.
			 */
			void AppendMask(u32 syncMask) { mSyncMask |= syncMask; }

			/** Resets the sync mask. */
			void ClearMask() { mSyncMask = 0; }

			/**
			 * Issues a pipeline barrier on the provided buffer. See vkCmdPipelineBarrier in Vulkan spec. for usage
			 * information.
			 */
			void memoryBarrier(VkBuffer buffer, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

			/**
			 * Issues a pipeline barrier on the provided image, changing its layout. See vkCmdPipelineBarrier in Vulkan spec.
			 * for usage information.
			 */
			void SetLayout(VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& range);

			/**
			 * Issues one or multiple pipeline barrier on the provided image, changing the layout of its subresources.
			 * Automatically determines original layout for individual sub-resources, groups the pipeline barriers and issues
			 * them.
			 */
			void SetLayout(VulkanImage* image, const VkImageSubresourceRange& range, VkAccessFlags newAccessMask, VkImageLayout newLayout);

			/**
			 * Submits the command buffer on the queue.
			 *
			 *	@param[in]	wait	If true, the caller thread will wait until all device operations on the command buffer's
			 *						queue complete.
			 */
			void Flush(bool wait);

			/** Returns the internal command buffer. */
			VulkanCmdBuffer* GetCb() const { return mCB; }

		private:
			friend class VulkanCommandBufferManager;

			/** Allocates a new internal command buffer. */
			void Allocate();

			VulkanDevice* mDevice = nullptr;
			GpuQueueType mType = GQT_GRAPHICS;
			u32 mQueueIdx = 0;
			VulkanQueue* mQueue = nullptr;
			u32 mQueueMask = 0;

			VulkanCmdBuffer* mCB = nullptr;
			u32 mSyncMask = 0;

			Vector<VkImageMemoryBarrier> mBarriersTemp;
		};

		/**
		 * Handles creation of Vulkan command buffers. See CommandBuffer.
		 *
		 * @note Core thread only.
		 */
		class VulkanCommandBufferManager : public CommandBufferManager
		{
		public:
			VulkanCommandBufferManager(const VulkanRenderAPI& rapi);
			~VulkanCommandBufferManager();

			SPtr<CommandBuffer> CreateInternal(GpuQueueType type, u32 deviceIdx = 0, u32 queueIdx = 0, bool secondary = false) override;

			/**
			 * Returns a set of command buffer semaphores depending on the provided sync mask.
			 *
			 * @param[in]	deviceIdx	Index of the device to get the semaphores for.
			 * @param[in]	syncMask	Mask that has a bit enabled for each command buffer to retrieve the semaphore for.
			 *							If the command buffer is not currently executing, semaphore won't be returned.
			 * @param[out]	semaphores	List containing all the required semaphores. Semaphores are tightly packed at the
			 *							beginning of the array. Must be able to hold at least BS_MAX_UNIQUE_QUEUES entries.
			 * @param[out]	count		Number of semaphores provided in the @p semaphores array.
			 */
			void GetSyncSemaphores(u32 deviceIdx, u32 syncMask, VulkanSemaphore** semaphores, u32& count);

			/**
			 * Returns an command buffer that can be used for executing transfer operations on the specified queue.
			 * Transfer buffers are automatically flushed (submitted) whenever a new (normal) command buffer is about to
			 * execute.
			 */
			VulkanTransferBuffer* GetTransferBuffer(u32 deviceIdx, GpuQueueType type, u32 queueIdx);

			/** Submits all transfer command buffers, ensuring all queued transfer operations get executed. */
			void FlushTransferBuffers(u32 deviceIdx);

		private:
			/** Contains command buffers specific to one device. */
			struct PerDeviceData
			{
				VulkanTransferBuffer TransferBuffers[GQT_COUNT][BS_MAX_QUEUES_PER_TYPE];
			};

			const VulkanRenderAPI& mRapi;

			PerDeviceData* mDeviceData;
			u32 mNumDevices;
		};

		/**	Provides easy access to the VulkanCommandBufferManager. */
		VulkanCommandBufferManager& gVulkanCBManager();

		/** @} */
	} // namespace ct
} // namespace bs
