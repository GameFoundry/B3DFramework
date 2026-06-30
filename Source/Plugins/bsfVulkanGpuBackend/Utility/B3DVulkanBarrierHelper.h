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
		void RecordBufferBarrier(IGpuBufferResource* buffer, GpuStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccessFlags, GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccessFlags);

		/** CRTP hook: accumulates the native Vulkan image barrier; reconciles @p oldLayout from an already-merged barrier and flags layout transitions. Called by the shared low-level path. */
		void RecordSubresourceBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccessFlags, GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccessFlags, GpuImageLayout& oldLayout, GpuImageLayout newLayout);

		TInlineArray<VkBufferMemoryBarrier, 4> mBufferBarriers;
		TInlineArray<VkImageMemoryBarrier, 4> mImageBarriers;

		VkPipelineStageFlags mCombinedSourceStages = 0;
		VkPipelineStageFlags mCombinedDestinationStages = 0;
		GpuAccessFlags mCombinedSourceAccess = GpuAccessFlag::None;
		GpuAccessFlags mCombinedDestinationAccess = GpuAccessFlag::None;

		bool mHasLayoutTransition = false;
	};

	extern template class TGpuBarrierHelper<VulkanBarrierHelper>;

	/** @} */
}
