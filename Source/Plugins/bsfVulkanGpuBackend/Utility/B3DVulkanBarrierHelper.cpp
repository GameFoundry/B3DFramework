//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanBarrierHelper.h"
#include "B3DVulkanResourceTracker.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanUtility.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

using namespace b3d;
using namespace b3d::render;

// Generic barrier-helper method definitions, followed by the explicit instantiation for the Vulkan barrier helper.
// Included here (after the complete VulkanBarrierHelper and VulkanResourceTracker are available) so the single
// instantiation lives in this translation unit. The header carries a matching `extern template` to suppress implicit
// instantiation elsewhere.
#include "GpuBackend/B3DGpuBarrierHelper.inl"

template class b3d::render::TGpuBarrierHelper<b3d::render::VulkanBarrierHelper>;

VulkanBarrierHelper::VulkanBarrierHelper(VulkanResourceTracker* resourceTracker)
	: TGpuBarrierHelper<VulkanBarrierHelper>(resourceTracker)
{ }

void VulkanBarrierHelper::RecordBufferBarrier(IGpuBufferResource* buffer, GpuStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccessFlags, GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccessFlags)
{
	const VkBuffer bufferHandle = static_cast<VulkanBuffer*>(buffer)->GetVulkanHandle();

	VkPipelineStageFlags sourceStageMask, destinationStageMask;
	VkAccessFlags sourceAccessMask, destinationAccessMask;
	VulkanUtility::GetPipelineStageAndAccessMask(sourceAccessStageFlags, sourceAccessFlags, sourceStageMask, sourceAccessMask);
	VulkanUtility::GetPipelineStageAndAccessMask(destinationAccessStageFlags, destinationAccessFlags, destinationStageMask, destinationAccessMask);

	mCombinedSourceStages |= sourceStageMask;
	mCombinedDestinationStages |= destinationStageMask;
	mCombinedSourceAccess |= sourceAccessFlags;
	mCombinedDestinationAccess |= destinationAccessFlags;

	auto found = std::find_if(mBufferBarriers.begin(), mBufferBarriers.end(), [bufferHandle](const VkBufferMemoryBarrier& barrier)
		{ return barrier.buffer == bufferHandle; } );
	if(found == mBufferBarriers.end())
	{
		VkBufferMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = sourceAccessMask;
		barrier.dstAccessMask = destinationAccessMask;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = bufferHandle;
		barrier.offset = 0;
		barrier.size = VK_WHOLE_SIZE;

		mBufferBarriers.Add(barrier);
	}
	else
	{
		found->srcAccessMask |= sourceAccessMask;
		found->dstAccessMask |= destinationAccessMask;
	}
}

void VulkanBarrierHelper::RecordSubresourceBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccessFlags, GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccessFlags, GpuImageLayout& oldLayout, GpuImageLayout newLayout)
{
	const VkImage imageHandle = static_cast<VulkanImage*>(image)->GetVulkanHandle();
	const VkImageLayout vkOldLayout = VulkanUtility::ToVkImageLayout(oldLayout);
	const VkImageLayout vkNewLayout = VulkanUtility::ToVkImageLayout(newLayout);
	const VkImageSubresourceRange vkSubresourceRange = VulkanUtility::ToVkImageSubresourceRange(subresourceRange);

	VkPipelineStageFlags sourceStageMask, destinationStageMask;
	VkAccessFlags sourceAccessMask, destinationAccessMask;
	VulkanUtility::GetPipelineStageAndAccessMask(sourceAccessStageFlags, sourceAccessFlags, sourceStageMask, sourceAccessMask);
	VulkanUtility::GetPipelineStageAndAccessMask(destinationAccessStageFlags, destinationAccessFlags, destinationStageMask, destinationAccessMask);

	mCombinedSourceStages |= sourceStageMask;
	mCombinedDestinationStages |= destinationStageMask;
	mCombinedSourceAccess |= sourceAccessFlags;
	mCombinedDestinationAccess |= destinationAccessFlags;

	auto found = std::find_if(mImageBarriers.begin(), mImageBarriers.end(), [imageHandle, &vkSubresourceRange](const VkImageMemoryBarrier& barrier)
	{
		return barrier.image == imageHandle && VulkanUtility::RangeEquals(barrier.subresourceRange, vkSubresourceRange);
	});

	if(found == mImageBarriers.end())
	{
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = sourceAccessMask;
		barrier.dstAccessMask = destinationAccessMask;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.oldLayout = vkOldLayout;
		barrier.newLayout = vkNewLayout;
		barrier.image = imageHandle;
		barrier.subresourceRange = vkSubresourceRange;

		mImageBarriers.Add(barrier);
	}
	else
	{
		found->srcAccessMask |= sourceAccessMask;
		found->dstAccessMask |= destinationAccessMask;
		found->newLayout = vkNewLayout;

		// Reconcile the old layout with the one already recorded for the merged barrier, so the shared layout-tracking
		// bookkeeping (in TGpuBarrierHelper) observes the correct source layout.
		oldLayout = VulkanUtility::ToGpuImageLayout(found->oldLayout);
	}

	if(oldLayout != newLayout)
	{
		// TODO - Use more specific stages for layout transitions? Make sure layout transitions are only doing an execution barrier if memory barrier isn't needed
		mCombinedSourceStages |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		mHasLayoutTransition = true;
	}
}

void VulkanBarrierHelper::Execute(VulkanGpuCommandBuffer& commandBuffer)
{
	if(HasBarriers())
	{
		// Determine barrier type based on access patterns
		// Read-after-write or write-after-write requires memory barrier, or if there are any layout transitions queued
		if(mCombinedSourceAccess.IsSet(GpuAccessFlag::Write) || mHasLayoutTransition)
		{
			vkCmdPipelineBarrier(
				commandBuffer.GetVulkanHandle(),
				mCombinedSourceStages,
				mCombinedDestinationStages,
				0,
				0, nullptr,
				(u32)mBufferBarriers.size(), mBufferBarriers.data(),
				(u32)mImageBarriers.size(), mImageBarriers.data());
		}
		// Write-after-read requires only execution barrier
		else if(mCombinedSourceAccess.IsSet(GpuAccessFlag::Read) && mCombinedDestinationAccess.IsSet(GpuAccessFlag::Write))
		{
			vkCmdPipelineBarrier(
				commandBuffer.GetVulkanHandle(),
				mCombinedSourceStages,
				mCombinedDestinationStages,
				0,
				0, nullptr,
				0, nullptr,
				0, nullptr);
		}

		ApplyPostBarrierTracking();
	}

	// Apply the read/write hazard registrations that were deferred while tracking this dispatch/draw's resources
	mResourceTracker->CommitPendingHazardRegistrations();

	Clear();
}

void VulkanBarrierHelper::Clear()
{
	mBufferBarriers.Clear();
	mImageBarriers.Clear();
	mCombinedSourceStages = 0;
	mCombinedDestinationStages = 0;
	mCombinedSourceAccess = GpuAccessFlag::None;
	mCombinedDestinationAccess = GpuAccessFlag::None;
	mHasLayoutTransition = false;

	TGpuBarrierHelper<VulkanBarrierHelper>::Clear();
}

bool VulkanBarrierHelper::HasBarriers() const
{
	return !mBufferBarriers.Empty() || !mImageBarriers.Empty();
}
