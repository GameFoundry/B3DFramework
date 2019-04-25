//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper around a command buffer used specifically for transfer operations. */
	class VulkanTransferBuffer
	{
	public:
		VulkanTransferBuffer() = default;
		VulkanTransferBuffer(VulkanDevice* device, GpuQueueType type, UINT32 queueIdx);
		~VulkanTransferBuffer();

		/** 
		 * OR's the provided sync mask with the internal sync mask. The sync mask determines on which queues should
		 * the buffer wait on before executing. Sync mask is reset after a flush. See CommandSyncMask on how to generate
		 * a sync mask.
		 */
		void appendMask(UINT32 syncMask) { mSyncMask |= syncMask; }

		/** Resets the sync mask. */
		void clearMask() { mSyncMask = 0; }

		/** 
		 * Issues a pipeline barrier on the provided buffer. See vkCmdPipelineBarrier in Vulkan spec. for usage
		 * information.
		 */
		void memoryBarrier(VkBuffer buffer, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags,
						   VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

		/** 
		 * Issues a pipeline barrier on the provided image, changing its layout. See vkCmdPipelineBarrier in Vulkan spec. 
		 * for usage information.
		 */
		void setLayout(VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, 
			VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& range);

		/**
		 * Issues one or multiple pipeline barrier on the provided image, changing the layout of its subresources. 
		 * Automatically determines original layout for individual sub-resources, groups the pipeline barriers and issues
		 * them.
		 */
		void setLayout(VulkanImage* image, const VkImageSubresourceRange& range, VkAccessFlags newAccessMask, 
					   VkImageLayout newLayout);

		/** 
		 * Submits the command buffer on the queue. 
		 * 
		 *	@param[in]	wait	If true, the caller thread will wait until all device operations on the command buffer's
		 *						queue complete.	
		 */
		void flush(bool wait);

		/** Returns the internal command buffer. */
		VulkanCmdBuffer* getCB() const { return mCB; }
	private:
		friend class VulkanCommandBufferManager;

		/** Allocates a new internal command buffer. */
		void allocate();

		VulkanDevice* mDevice = nullptr;
		GpuQueueType mType = GQT_GRAPHICS;
		UINT32 mQueueIdx = 0;
		VulkanQueue* mQueue = nullptr;
		UINT32 mQueueMask = 0;

		VulkanCmdBuffer* mCB = nullptr;
		UINT32 mSyncMask = 0;

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

		/** @copydoc CommandBufferManager::createInternal() */
		SPtr<CommandBuffer> createInternal(GpuQueueType type, UINT32 deviceIdx = 0, UINT32 queueIdx = 0,
			bool secondary = false) override;

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
		void getSyncSemaphores(UINT32 deviceIdx, UINT32 syncMask, VulkanSemaphore** semaphores, UINT32& count);

		/** 
		 * Returns an command buffer that can be used for executing transfer operations on the specified queue. 
		 * Transfer buffers are automatically flushed (submitted) whenever a new (normal) command buffer is about to
		 * execute.
		 */
		VulkanTransferBuffer* getTransferBuffer(UINT32 deviceIdx, GpuQueueType type, UINT32 queueIdx);

		/** Submits all transfer command buffers, ensuring all queued transfer operations get executed. */
		void flushTransferBuffers(UINT32 deviceIdx);

	private:
		/** Contains command buffers specific to one device. */
		struct PerDeviceData
		{
			VulkanTransferBuffer transferBuffers[GQT_COUNT][BS_MAX_QUEUES_PER_TYPE];
		};

		const VulkanRenderAPI& mRapi;

		PerDeviceData* mDeviceData;
		UINT32 mNumDevices;
	};

	/**	Provides easy access to the VulkanCommandBufferManager. */
	VulkanCommandBufferManager& gVulkanCBManager();

	/** @} */
}}