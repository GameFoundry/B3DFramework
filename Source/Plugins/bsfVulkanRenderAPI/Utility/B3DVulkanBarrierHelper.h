//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "Allocators/B3DFrameAllocator.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "B3DVulkanResourceTracker.h"

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
	class VulkanBarrierHelper
	{
	public:

		/** Information needed to update hazard tracking after barrier execution. */
		struct BarrierTrackingInfo
		{
			VulkanBuffer* Buffer = nullptr;
			VulkanImage* Image = nullptr;
			VkImageSubresourceRange ImageSubresourceRange{};
			GpuAccessFlags SourceAccess = GpuAccessFlag::None;
			VulkanAccessStageFlags SourceAccessStages = VulkanAccessStageFlag::None;
			GpuAccessFlags DestinationAccess = GpuAccessFlag::None;
			VulkanAccessStageFlags DestinationAccessStages = VulkanAccessStageFlag::None;
		};

		/**
		 * Constructs a barrier helper associated with the provided command buffer.
		 *
		 * @param resourceTracker	Object responsible for tracking all resource usages on a command buffer. Used for determining current object state,
		 *							and notified with new state when barriers and layout transitions are executed.
		 */
		VulkanBarrierHelper(VulkanResourceTracker* resourceTracker);

		/**
		 * Adds a memory barrier for a buffer resource.
		 *
		 * @param buffer				Buffer to add barrier for.
		 * @param sourceUsage			How the buffer was used before the barrier.
		 * @param sourceAccess			Type of access (read/write) before the barrier.
		 * @param destinationUsage		How the buffer will be used after the barrier.
		 * @param destinationAccess		Type of access (read/write) after the barrier.
		 * @return						Information about a barrier that was queued, or null if none was queued. Only valid until next call to Add/Execute/Clear.
		 */
		const BarrierTrackingInfo* AddBufferBarrier(VulkanBuffer* buffer, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess);

		/**
		 * Adds a memory barrier for a buffer resource. Automatically deduces source usage/access from current tracked state.
		 *
		 * @param buffer				Buffer to add barrier for.
		 * @param destinationUsage		How the buffer will be used after the barrier.
		 * @param destinationAccess		Type of access (read/write) after the barrier.
		 * @return						Information about a barrier that was queued, or null if none was queued. Only valid until next call to Add/Execute/Clear.
		 */
		const BarrierTrackingInfo* AddBufferBarrier(VulkanBuffer* buffer, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess);

		/**
		 * Adds a memory barrier for a buffer resource. Automatically deduces source usage/access from provided tracked state.
		 *
		 * @param buffer				Buffer to add barrier for.
		 * @param bufferTrackingState	Buffer tracking information as retrieved from VulkanResourceTracker.
		 * @param destinationUsage		How the buffer will be used after the barrier.
		 * @param destinationAccess		Type of access (read/write) after the barrier.
		 * @return						Information about a barrier that was queued, or null if none was queued. Only valid until next call to Add/Execute/Clear.
		 */
		const BarrierTrackingInfo* AddBufferBarrier(VulkanBuffer* buffer, const VulkanResourceTracker::BufferTrackingState& bufferTrackingState, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess);

		/**
		 * Adds a memory barrier for an image resource.
		 *
		 * @param image						Image to add barrier for.
		 * @param subresourceRange			Subresource range of the image to barrier.
		 * @param sourceUsage				How the image was used before the barrier.
		 * @param sourceAccessFlags			Type of access (read/write) before the barrier.
		 * @param destinationUsage			How the image will be used after the barrier.
		 * @param destinationAccessFlags	Type of access (read/write) after the barrier.
		 * @param oldLayout					Current layout of the image before the barrier.
		 * @param newLayout					Layout the image will be transitioned to after the barrier.
		 * @return							Information about a barrier that was queued, or null if none was queued. Only valid until next call to Add/Execute/Clear.
		 */
		const BarrierTrackingInfo* AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccessFlags, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccessFlags, ImageLayout oldLayout, ImageLayout newLayout);

		/**
		 * Adds a memory barrier for an image resource. Automatically deduces source usage/access and layout from current tracked state.
		 *
		 * @param image					Image to add barrier for.
		 * @param subresourceRange		Subresource range of the image to barrier.
		 * @param destinationUsage		How the image will be used after the barrier.
		 * @param destinationAccess		Type of access (read/write) after the barrier.
		 * @param newLayout				Layout the image will be transitioned to after the barrier.
		 * @return						Information about a barrier that was queued, or null if none was queued. Only valid until next call to Add/Execute/Clear.
		 */
		const BarrierTrackingInfo* AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, ImageLayout newLayout);

		/**
		 * Adds a memory barrier for an existing subresource of an image resource. Automatically deduces source usage/access and layout from provided tracked state.
		 *
		 * @param image							Image to add barrier for.
		 * @param subresourceTrackingState		Subresource tracking information as retrieved from VulkanResourceTracker.
		 * @param destinationUsage				How the image will be used after the barrier.
		 * @param destinationAccess				Type of access (read/write) after the barrier.
		 * @param newLayout						Layout the image will be transitioned to after the barrier.
		 * @return								Information about a barrier that was queued, or null if none was queued. Only valid until next call to Add/Execute/Clear.
		 */
		const BarrierTrackingInfo* AddSubresourceBarrier(VulkanImage* image, const VulkanResourceTracker::ImageSubresourceTrackingState& subresourceTrackingState, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, VkImageLayout newLayout);

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
		/** Low-level overload of AddImageBarrier that uses VulkanAccessStageFlags directly. */
		const BarrierTrackingInfo* AddSubresourceBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, VulkanAccessStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccessFlags, VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccessFlags, VkImageLayout oldLayout, VkImageLayout newLayout);

		/** Low-level overload of AddBufferBarrier that uses VulkanAccessStageFlags directly. */
		const BarrierTrackingInfo* AddBufferBarrier(VulkanBuffer* buffer, VulkanAccessStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccessFlags, VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccessFlags);

		/** Information needed to update layout after barrier execution. */
		struct LayoutTrackingInfo
		{
			VulkanImage* Image = nullptr;
			VkImageSubresourceRange SubresourceRange{};
			VkImageLayout OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkImageLayout NewLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		};

		VulkanResourceTracker* mResourceTracker;

		TInlineArray<VkBufferMemoryBarrier, 4> mBufferBarriers;
		TInlineArray<VkImageMemoryBarrier, 4> mImageBarriers;

		VkPipelineStageFlags mCombinedSourceStages = 0;
		VkPipelineStageFlags mCombinedDestinationStages = 0;
		GpuAccessFlags mCombinedSourceAccess = GpuAccessFlag::None;
		GpuAccessFlags mCombinedDestinationAccess = GpuAccessFlag::None;

		TInlineArray<LayoutTrackingInfo, 4> mImageLayoutTracking;
		bool mHasLayoutTransition = false;

		TInlineArray<BarrierTrackingInfo, 8> mBarrierTracking;
	};

	/** @} */
}
