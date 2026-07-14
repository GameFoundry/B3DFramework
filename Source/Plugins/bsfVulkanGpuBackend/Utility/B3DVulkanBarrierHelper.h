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
		/** Adds a buffer memory barrier described using backend-independent stages and access flags. */
		void AddBufferBarrier(VkBuffer buffer, const GpuHazardStageAndAccess& barrier, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

		/** Adds a buffer memory barrier and includes its stages in the batch dependency. */
		void AddBufferBarrier(VkBuffer buffer, VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

		/** Adds an image memory barrier described using backend-independent stages and access flags. */
		VkImageLayout AddImageBarrier(VkImage image, const VkImageSubresourceRange& range, const GpuHazardStageAndAccess& barrier, VkImageLayout oldLayout, VkImageLayout newLayout, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED);

		/** Adds an image memory barrier and returns the effective old layout if it merged with an existing barrier. */
		VkImageLayout AddImageBarrier(VkImage image, const VkImageSubresourceRange& range, VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess, VkImageLayout oldLayout, VkImageLayout newLayout, u32 sourceQueueFamily = VK_QUEUE_FAMILY_IGNORED, u32 destinationQueueFamily = VK_QUEUE_FAMILY_IGNORED);

		/** Adds an execution-only dependency described using backend-independent stages. */
		void AddExecutionBarrier(const GpuHazardStageAndAccess& barrier);

		/** Adds an execution-only dependency. */
		void AddExecutionBarrier(VkPipelineStageFlags sourceStages, VkPipelineStageFlags destinationStages);

		/** Emits the accumulated dependency into @p commandBuffer. */
		void Execute(VkCommandBuffer commandBuffer) const;

		/** Clears all accumulated barriers. */
		void Clear();

		/** Returns true if the batch contains a memory or execution dependency. */
		bool HasBarriers() const;

	private:
		TInlineArray<VkBufferMemoryBarrier, 4> mBufferBarriers;
		TInlineArray<VkImageMemoryBarrier, 4> mImageBarriers;
		VkPipelineStageFlags mCombinedSourceStages = 0;
		VkPipelineStageFlags mCombinedDestinationStages = 0;
		bool mHasExecutionBarrier = false;
	};

	/**
	 * Helper class for building and issuing Vulkan memory barriers.
	 *
	 * This class provides a convenient way to accumulate multiple barriers and issue them together.
	 * It works with low-level Vulkan resources (VulkanBuffer*, VulkanImage*) making it suitable
	 * for use in Copy operations and other low-level operations where IssueBarriers cannot be used.
	 *
	 * The helper automatically:
	 * - Converts resource usage and access flags to Vulkan access masks
	 * - Derives appropriate pipeline stages from access masks
	 * - Accumulates barriers for batch execution
	 * - Integrates with hazard tracking (when enabled)
	 *
	 * Typical usage:
	 * @code
	 * VulkanBarrierHelper helper(commandBuffer);
	 * helper.AddBufferBarrier(sourceBuffer, ...);
	 * helper.AddBufferBarrier(destBuffer, ...);
	 * helper.Execute();
	 * @endcode
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

		/** CRTP hook: accumulates the native Vulkan buffer barrier (dedup + combined stage/access masks). Called by the shared low-level path. */
		void RecordBufferBarrier(IGpuBufferResource* buffer, const GpuHazardStageAndAccess& barrier);

		/** CRTP hook: accumulates the native Vulkan image barrier; reconciles @p oldLayout from an already-merged barrier and flags layout transitions. Called by the shared low-level path. */
		void RecordSubresourceBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, const GpuHazardStageAndAccess& barrier, GpuImageLayout& oldLayout, GpuImageLayout newLayout);

		VulkanBarrierBatch mBarrierBatch;
	};

	extern template class TGpuBarrierHelper<VulkanBarrierHelper>;

	/** @} */
}
