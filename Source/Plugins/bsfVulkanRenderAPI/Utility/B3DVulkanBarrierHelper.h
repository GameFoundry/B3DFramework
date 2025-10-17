//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "Allocators/B3DFrameAllocator.h"

namespace b3d::render
{
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
	class VulkanBarrierHelper
	{
	public:
		/**
		 * Constructs a barrier helper associated with the provided command buffer.
		 *
		 * @param commandBuffer		Command buffer on which barriers will be issued.
		 * @param resourceTracker	Object responsible for tracking all resource usages on a command buffer.
		 */
		VulkanBarrierHelper(VulkanGpuCommandBuffer* commandBuffer, VulkanResourceTracker* resourceTracker);

		/**
		 * Adds a memory barrier for a buffer resource.
		 *
		 * @param buffer				Buffer to add barrier for.
		 * @param sourceUsage			How the buffer was used before the barrier.
		 * @param sourceAccess			Type of access (read/write) before the barrier.
		 * @param destinationUsage		How the buffer will be used after the barrier.
		 * @param destinationAccess		Type of access (read/write) after the barrier.
		 */
		void AddBufferBarrier(VulkanBuffer* buffer, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess);

		/**
		 * Adds a memory barrier for an image resource.
		 *
		 * @param image					Image to add barrier for.
		 * @param subresourceRange		Subresource range of the image to barrier.
		 * @param sourceUsage			How the image was used before the barrier.
		 * @param sourceAccess			Type of access (read/write) before the barrier.
		 * @param destinationUsage		How the image will be used after the barrier.
		 * @param destinationAccess		Type of access (read/write) after the barrier.
		 */
		void AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess);

		/**
		 * Executes all accumulated barriers by issuing a pipeline barrier command.
		 * After execution, all accumulated barriers are cleared.
		 *
		 * If no barriers have been accumulated, this is a no-op.
		 */
		void Execute();

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
#if B3D_HAZARD_TRACKING
		/** Information needed to update hazard tracking after barrier execution. */
		struct BarrierTrackingInfo
		{
			VulkanBuffer* Buffer = nullptr;
			VulkanImage* Image = nullptr;
			VkImageSubresourceRange ImageSubresourceRange{};
			GpuAccessFlags SourceAccess = GpuAccessFlag::None;
			VkPipelineStageFlags SourceStages = 0;
			GpuAccessFlags DestinationAccess = GpuAccessFlag::None;
			VkPipelineStageFlags DestinationStages = 0;
		};
#endif

		VulkanGpuCommandBuffer* mCommandBuffer;
		VulkanResourceTracker* mResourceTracker;

		FrameVector<VkBufferMemoryBarrier> mBufferBarriers;
		FrameVector<VkImageMemoryBarrier> mImageBarriers;

		VkPipelineStageFlags mCombinedSourceStages = 0;
		VkPipelineStageFlags mCombinedDestinationStages = 0;
		GpuAccessFlags mCombinedSourceAccess = GpuAccessFlag::None;
		GpuAccessFlags mCombinedDestinationAccess = GpuAccessFlag::None;

#if B3D_HAZARD_TRACKING
		FrameVector<BarrierTrackingInfo> mBarrierTracking;
#endif
	};

	/** @} */
}
