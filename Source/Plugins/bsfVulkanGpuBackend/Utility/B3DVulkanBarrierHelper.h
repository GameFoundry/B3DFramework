//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "Allocators/B3DFrameAllocator.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuBarrierHelper.h"

namespace b3d::render
{
	class VulkanResourceTracker;
	class VulkanBuffer;
	class VulkanImage;

	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Accumulates native Vulkan barriers that can be emitted without resource-tracker bookkeeping. */
	class VulkanBarrierBatch
	{
	public:
		VulkanBarrierBatch();

		/** Adds a buffer memory barrier described using backend-independent stages and access flags. */
		void AddBufferBarrier(VkBuffer buffer, const GpuBarrierScope& barrier, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

		/** Adds a buffer memory barrier and includes its stages in the batch dependency. */
		void AddBufferBarrier(VkBuffer buffer, VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

		/** Adds an image memory barrier and returns the effective old layout if it merged with an existing barrier. */
		VkImageLayout AddImageBarrier(VkImage image, const VkImageSubresourceRange& range, const GpuBarrierScope& barrier, VkImageLayout oldLayout, VkImageLayout newLayout, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED);

		/** Adds an image memory barrier and returns the effective old layout if it merged with an existing barrier. */
		VkImageLayout AddImageBarrier(VkImage image, const VkImageSubresourceRange& range, VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess, VkImageLayout oldLayout, VkImageLayout newLayout, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED);

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

		/** Clears all accumulated barriers. */
		void Clear();

		/** Returns true if the batch contains a memory or execution dependency. */
		bool HasBarriers() const;

	private:
		VkMemoryBarrier mMemoryBarrier;
		TInlineArray<VkBufferMemoryBarrier, 2> mOwnershipBufferBarriers;
		TInlineArray<VkImageMemoryBarrier, 4> mImageBarriers;
		VkPipelineStageFlags mCombinedSourceStages = 0;
		VkPipelineStageFlags mCombinedDestinationStages = 0;
		bool mHasMemoryBarrier = false;
		bool mHasExecutionBarrier = false;
	};

	/**
	 * Helper class for building and issuing Vulkan memory barriers.
	 *
	 * Translates core barriers to Vulkan, accumulates them for one pipeline-barrier command, and reports
	 * their effects back to the tracker after emission.
	 */
	class VulkanBarrierHelper : public TGpuBarrierHelper<VulkanBarrierHelper>
	{
	public:
		/**
		 * Constructs a barrier helper associated with the provided command buffer.
		 *
		 * @param resourceTracker	Object responsible for tracking all resource usages on a command buffer. Used for determining current object state,
		 *							and notified with new state when barriers and layout transitions are executed.
		 */
		VulkanBarrierHelper(VulkanResourceTracker* resourceTracker);

		/**
		 * Executes all accumulated barriers by issuing a pipeline barrier command.
		 * After execution, all accumulated barriers are cleared.
		 *
		 * If no barriers have been accumulated, this is a no-op.
		 *
		 * @param commandBuffer		Command buffer on which barriers will be issued.
		 */
		void Execute(VulkanGpuCommandBuffer& commandBuffer);

		/**
		 * Clears all accumulated barriers without executing them.
		 * Useful if you need to reset the helper state without issuing barriers.
		 */
		void Clear();

		/**
		 * Returns true if there are any barriers accumulated and ready to execute.
		 */
		bool HasBarriers() const;

	private:
		friend class TGpuBarrierHelper<VulkanBarrierHelper>;

		/** Accumulates a resolved native Vulkan buffer barrier. */
		void RecordNativeBufferBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier);

		/** Accumulates a resolved native image barrier and reconciles @p oldLayout after barrier merging. */
		void RecordNativeImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, const GpuBarrierScope& barrier, GpuImageLayout& oldLayout, GpuImageLayout newLayout);

		VulkanBarrierBatch mBarrierBatch;
	};

	extern template class TGpuBarrierHelper<VulkanBarrierHelper>;

	/** @} */
}
