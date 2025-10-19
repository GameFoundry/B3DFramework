//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanBarrierHelper.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanUtility.h"

using namespace b3d;
using namespace b3d::render;

VulkanBarrierHelper::VulkanBarrierHelper(VulkanGpuCommandBuffer* commandBuffer, VulkanResourceTracker* resourceTracker)
	: mCommandBuffer(commandBuffer), mResourceTracker(resourceTracker)
{
}

void VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
{
	if(buffer == nullptr)
		return;

	const VkAccessFlags sourceAccessMask = VulkanUtility::GetAccessMaskFromUsage(sourceUsage, sourceAccess);
	const VkAccessFlags destinationAccessMask = VulkanUtility::GetAccessMaskFromUsage(destinationUsage, destinationAccess);

	const VkPipelineStageFlags sourceStages = VulkanUtility::GetPipelineStageFlags(sourceUsage, sourceAccessMask);
	const VkPipelineStageFlags destinationStages = VulkanUtility::GetPipelineStageFlags(destinationUsage, destinationAccessMask);

	mCombinedSourceStages |= sourceStages;
	mCombinedDestinationStages |= destinationStages;
	mCombinedSourceAccess |= sourceAccess;
	mCombinedDestinationAccess |= destinationAccess;

	VkBufferMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = sourceAccessMask;
	barrier.dstAccessMask = destinationAccessMask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = buffer->GetVulkanHandle();
	barrier.offset = 0;
	barrier.size = VK_WHOLE_SIZE;

	mBufferBarriers.push_back(barrier);

#if B3D_HAZARD_TRACKING
	BarrierTrackingInfo trackingInfo;
	trackingInfo.Buffer = buffer;
	trackingInfo.SourceAccess = sourceAccess;
	trackingInfo.SourceStages = sourceStages;
	trackingInfo.DestinationAccess = destinationAccess;
	trackingInfo.DestinationStages = destinationStages;
	mBarrierTracking.push_back(trackingInfo);
#endif
}

void VulkanBarrierHelper::AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	if(image == nullptr)
		return;

	const VkAccessFlags sourceAccessMask = VulkanUtility::GetAccessMaskFromUsage(sourceUsage, sourceAccess);
	const VkAccessFlags destinationAccessMask = VulkanUtility::GetAccessMaskFromUsage(destinationUsage, destinationAccess);

	const VkPipelineStageFlags sourceStages = VulkanUtility::GetPipelineStageFlags(sourceUsage, sourceAccessMask);
	const VkPipelineStageFlags destinationStages = VulkanUtility::GetPipelineStageFlags(destinationUsage, destinationAccessMask);

	mCombinedSourceStages |= sourceStages;
	mCombinedDestinationStages |= destinationStages;
	mCombinedSourceAccess |= sourceAccess;
	mCombinedDestinationAccess |= destinationAccess;

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = sourceAccessMask;
	barrier.dstAccessMask = destinationAccessMask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.image = image->GetVulkanHandle();
	barrier.subresourceRange = subresourceRange;

	mImageBarriers.push_back(barrier);

	LayoutTrackingInfo layoutTrackingInfo;
	layoutTrackingInfo.Image = image;
	layoutTrackingInfo.SubresourceRange = subresourceRange;
	layoutTrackingInfo.OldLayout = oldLayout;
	layoutTrackingInfo.NewLayout = newLayout;
	mImageLayoutTracking.push_back(layoutTrackingInfo);

	if(oldLayout != newLayout)
		mHasLayoutTransition = true;

#if B3D_HAZARD_TRACKING
	BarrierTrackingInfo barrierTrackingInfo;
	barrierTrackingInfo.Image = image;
	barrierTrackingInfo.ImageSubresourceRange = subresourceRange;
	barrierTrackingInfo.SourceAccess = sourceAccess;
	barrierTrackingInfo.SourceStages = sourceStages;
	barrierTrackingInfo.DestinationAccess = destinationAccess;
	barrierTrackingInfo.DestinationStages = destinationStages;
	mBarrierTracking.push_back(barrierTrackingInfo);
#endif
}

void VulkanBarrierHelper::Execute()
{
	if(!HasBarriers())
		return;

	// Determine barrier type based on access patterns
	// Read-after-write or write-after-write requires memory barrier, or if there are any layout transitions queued
	if(mCombinedSourceAccess.IsSet(GpuAccessFlag::Write) || mHasLayoutTransition)
	{
		vkCmdPipelineBarrier(
			mCommandBuffer->GetVulkanHandle(),
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
			mCommandBuffer->GetVulkanHandle(),
			mCombinedSourceStages,
			mCombinedDestinationStages,
			0,
			0, nullptr,
			0, nullptr,
			0, nullptr);
	}

	// Update layout for all image barriers
	for(const auto& trackingInfo : mImageLayoutTracking)
	{
		if(trackingInfo.Image == nullptr)
			continue;

		mResourceTracker->UpdateImageLayoutTrackingAfterBarrier(
			trackingInfo.Image,
			trackingInfo.SubresourceRange,
			trackingInfo.OldLayout,
			trackingInfo.NewLayout);
	}

#if B3D_HAZARD_TRACKING
	// Update hazard tracking for all barriers
	for(const auto& trackingInfo : mBarrierTracking)
	{
		if(trackingInfo.Buffer != nullptr)
		{
			mResourceTracker->UpdateWriteHazardTrackingAfterBarrier(
				trackingInfo.Buffer,
				trackingInfo.SourceAccess,
				trackingInfo.SourceStages,
				trackingInfo.DestinationAccess,
				trackingInfo.DestinationStages);
		}
		else if(trackingInfo.Image != nullptr)
		{
			mResourceTracker->UpdateWriteHazardTrackingAfterBarrier(
				trackingInfo.Image,
				trackingInfo.ImageSubresourceRange,
				trackingInfo.SourceAccess,
				trackingInfo.SourceStages,
				trackingInfo.DestinationAccess,
				trackingInfo.DestinationStages);
		}
	}
#endif

	Clear();
}

void VulkanBarrierHelper::Clear()
{
	mBufferBarriers.clear();
	mImageBarriers.clear();
	mCombinedSourceStages = 0;
	mCombinedDestinationStages = 0;
	mCombinedSourceAccess = GpuAccessFlag::None;
	mCombinedDestinationAccess = GpuAccessFlag::None;

	mImageLayoutTracking.clear();
	mHasLayoutTransition = false;

#if B3D_HAZARD_TRACKING
	mBarrierTracking.clear();
#endif
}

bool VulkanBarrierHelper::HasBarriers() const
{
	return !mBufferBarriers.empty() || !mImageBarriers.empty();
}
