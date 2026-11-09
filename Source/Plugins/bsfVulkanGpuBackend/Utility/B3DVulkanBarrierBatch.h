//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"

namespace b3d::render
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Accumulates native Vulkan barriers and emits them as one dependency. */
	class VulkanBarrierBatch
	{
	public:
		/** Creates a batch for barriers recorded on @p device. Barriers described with backend-independent stages leave out stages it doesn't have. */
		VulkanBarrierBatch(const VulkanGpuDevice& device);

		/** Adds a buffer memory barrier described using backend-independent stages and access flags. */
		void AddBufferBarrier(VkBuffer buffer, const GpuBarrierScope& barrier, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

		/** Adds a buffer memory barrier and includes its stages in the batch dependency. */
		void AddBufferBarrier(VkBuffer buffer, VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

		/** Adds or merges an image barrier. An undefined old layout discards contents. */
		void AddImageBarrier(VkImage image, const VkImageSubresourceRange& range, const GpuBarrierScope& barrier, VkImageLayout oldLayout, VkImageLayout newLayout, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED);

		/** Adds or merges an image barrier. An undefined old layout discards contents. */
		void AddImageBarrier(VkImage image, const VkImageSubresourceRange& range, VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess, VkImageLayout oldLayout, VkImageLayout newLayout, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED);

		/** Adds an execution-only dependency described using backend-independent stages. */
		void AddExecutionBarrier(const GpuBarrierScope& barrier);

		/** Adds an execution-only dependency. */
		void AddExecutionBarrier(VkPipelineStageFlags sourceStages, VkPipelineStageFlags destinationStages);

		/** Adds a global memory dependency described using backend-independent stages and access flags. */
		void AddMemoryBarrier(const GpuBarrierScope& barrier);

		/** Adds a global memory dependency to the batch. */
		void AddMemoryBarrier(VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess);

		/** Emits the accumulated dependency into @p commandBuffer. */
		void Execute(VkCommandBuffer commandBuffer) const;

		/** Emits the accumulated dependency as the release of a split barrier, which performs the layout transitions and signals @p event. */
		void SetEvent(VkCommandBuffer commandBuffer, VkEvent event) const;

		/**
		 * Emits the accumulated dependency as the acquire of a split barrier, which waits for @p event. The release must have emitted
		 * the same dependency with SetEvent().
		 */
		void WaitEvent(VkCommandBuffer commandBuffer, VkEvent event) const;

		/** Clears all accumulated barriers. */
		void Clear();

		/** Returns true if the batch contains a memory or execution dependency. */
		bool HasBarriers() const;

	private:
		/** Converts the accumulated dependency to its synchronization2 form and passes it to @p fnRecord. */
		template<class TRecord>
		void RecordDependencyInformation(TRecord fnRecord) const;

		const VulkanGpuDevice& mDevice;
		VkMemoryBarrier mMemoryBarrier;
		TInlineArray<VkBufferMemoryBarrier, 2> mOwnershipBufferBarriers;
		TInlineArray<VkImageMemoryBarrier, 4> mImageBarriers;
		VkPipelineStageFlags mCombinedSourceStages = 0;
		VkPipelineStageFlags mCombinedDestinationStages = 0;
		bool mHasMemoryBarrier = false;
		bool mHasExecutionBarrier = false;
	};

	/** @} */
}
