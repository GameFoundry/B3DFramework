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

	AddBufferBarrier(buffer, sourceAccessMask, sourceStages, destinationAccessMask, destinationStages);
}

void VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
{
	if(buffer == nullptr)
		return;

	const VulkanResourceTracker::BufferTrackingState* bufferTrackingState = mResourceTracker->FindBufferTrackingState(buffer);
	if(bufferTrackingState == nullptr)
		return;

	VkPipelineStageFlags stageMask = 0;
	VkAccessFlags accessMask = 0;

#if B3D_HAZARD_TRACKING
	// Stages that we have previously read or written the image on require a barrier
	const VkPipelineStageFlags writeStageMask = bufferTrackingState->WriteHazardTracking->WriteAccessStages.GetUnsafeAccessStages(VK_PIPELINE_STAGE_TRANSFER_BIT);
	const VkPipelineStageFlags readStageMask = bufferTrackingState->WriteHazardTracking->ReadAccessStages.GetUnsafeAccessStages(VK_PIPELINE_STAGE_TRANSFER_BIT);

	// We deduce access mask based on the pipeline stages the image was used on
	accessMask |= VulkanUtility::GetAccessMaskFromPipelineStages(writeStageMask, GpuAccessFlag::Write);
	accessMask |= VulkanUtility::GetAccessMaskFromPipelineStages(readStageMask, GpuAccessFlag::Read);

	stageMask = writeStageMask | readStageMask;
#endif

	if(stageMask == 0)
		return;

	const VkAccessFlags destinationAccessMask = VulkanUtility::GetAccessMaskFromUsage(destinationUsage, destinationAccess);
	const VkPipelineStageFlags destinationStageMask = VulkanUtility::GetPipelineStageFlags(destinationUsage, destinationAccessMask);

	AddBufferBarrier(buffer, accessMask, stageMask, destinationAccessMask, destinationStageMask);
}

void VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, VkAccessFlags sourceAccessMask, VkPipelineStageFlags sourceStageMask, VkAccessFlags destinationAccessMask, VkPipelineStageFlags destinationStageMask)
{
	if(buffer == nullptr)
		return;

	const GpuAccessFlags sourceAccess = VulkanUtility::GetAccessFlagsFromAccessMask(sourceAccessMask);
	const GpuAccessFlags destinationAccess = VulkanUtility::GetAccessFlagsFromAccessMask(destinationAccessMask);

	mCombinedSourceStages |= sourceStageMask;
	mCombinedDestinationStages |= destinationStageMask;
	mCombinedSourceAccess |= sourceAccess;
	mCombinedDestinationAccess |= destinationAccess;

	auto found = std::find_if(mBufferBarriers.begin(), mBufferBarriers.end(), [buffer](const VkBufferMemoryBarrier& barrier)
		{ return barrier.buffer == buffer->GetVulkanHandle(); } );
	if(found == mBufferBarriers.end())
	{
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
	}
	else
	{
		found->srcAccessMask |= sourceAccessMask;
		found->dstAccessMask |= destinationAccessMask;
	}


#if B3D_HAZARD_TRACKING
	BarrierTrackingInfo trackingInfo;
	trackingInfo.Buffer = buffer;
	trackingInfo.SourceAccess = sourceAccess;
	trackingInfo.SourceStages = sourceStageMask;
	trackingInfo.DestinationAccess = destinationAccess;
	trackingInfo.DestinationStages = destinationStageMask;
	mBarrierTracking.push_back(trackingInfo);
#endif
}

void VulkanBarrierHelper::AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	const VkAccessFlags sourceAccessMask = VulkanUtility::GetAccessMaskFromUsage(sourceUsage, sourceAccess);
	const VkAccessFlags destinationAccessMask = VulkanUtility::GetAccessMaskFromUsage(destinationUsage, destinationAccess);

	const VkPipelineStageFlags sourceStages = VulkanUtility::GetPipelineStageFlags(sourceUsage, sourceAccessMask);
	const VkPipelineStageFlags destinationStages = VulkanUtility::GetPipelineStageFlags(destinationUsage, destinationAccessMask);

	AddImageBarrier(image, subresourceRange, sourceAccessMask, sourceStages, destinationAccessMask, destinationStages, oldLayout, newLayout);
}

void VulkanBarrierHelper::AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, VkImageLayout newLayout)
{
	if(image == nullptr)
		return;

	const VulkanResourceTracker::ImageTrackingState* imageTrackingState = mResourceTracker->FindImageTrackingState(image);
	if(imageTrackingState == nullptr)
		return;

	struct CallbackParameters
	{
		VulkanBarrierHelper* BarrierHelper;
		VulkanResourceTracker* ResourceTracker;
		VulkanImage* Image;
		GpuResourceUseFlags DestinationUsage;
		GpuAccessFlags DestinationAccess;
		VkImageLayout NewLayout;
	};

	CallbackParameters callbackParameters { this, mResourceTracker, image, destinationUsage, destinationAccess, newLayout };
	mResourceTracker->IterateAndCreateOverlappingImageSubresourceTrackingState(image, subresourceRange, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* const callbackParameters = static_cast<CallbackParameters*>(userData);

		VulkanResourceTracker& resourceTracker = *callbackParameters->ResourceTracker;
		const VulkanResourceTracker::ImageSubresourceTrackingState& subresourceTrackingState = resourceTracker.GetSubresourceTrackingStateAtIndex(globalSubresourceIndex);

		VkPipelineStageFlags stageMask = 0;
		VkAccessFlags accessMask = 0;

#if B3D_HAZARD_TRACKING
		// Stages that we have previously read or written the image on require a barrier
		const VkPipelineStageFlags writeStageMask = subresourceTrackingState.WriteHazardTracking->WriteAccessStages.GetUnsafeAccessStages(VK_PIPELINE_STAGE_TRANSFER_BIT);
		const VkPipelineStageFlags readStageMask = subresourceTrackingState.WriteHazardTracking->ReadAccessStages.GetUnsafeAccessStages(VK_PIPELINE_STAGE_TRANSFER_BIT);

		// We deduce access mask based on the pipeline stages the image was used on
		accessMask |= VulkanUtility::GetAccessMaskFromPipelineStages(writeStageMask, GpuAccessFlag::Write);
		accessMask |= VulkanUtility::GetAccessMaskFromPipelineStages(readStageMask, GpuAccessFlag::Read);

		stageMask = writeStageMask | readStageMask;
#endif

		if(stageMask == 0 && subresourceTrackingState.CurrentLayout == callbackParameters->NewLayout)
			return;

		VulkanBarrierHelper& barrierHelper = *callbackParameters->BarrierHelper;

		const VkAccessFlags destinationAccessMask = VulkanUtility::GetAccessMaskFromUsage(callbackParameters->DestinationUsage, callbackParameters->DestinationAccess);
		const VkPipelineStageFlags destinationStageMask = VulkanUtility::GetPipelineStageFlags(callbackParameters->DestinationUsage, destinationAccessMask);

		barrierHelper.AddImageBarrier(callbackParameters->Image, subresourceTrackingState.Range, accessMask, stageMask, destinationAccessMask,
			destinationStageMask, subresourceTrackingState.CurrentLayout, callbackParameters->NewLayout);
	}, &callbackParameters);
}

void VulkanBarrierHelper::AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, VkAccessFlags sourceAccessMask, VkPipelineStageFlags sourceStageMask, VkAccessFlags destinationAccessMask, VkPipelineStageFlags destinationStageMask, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	if(image == nullptr)
		return;

	const GpuAccessFlags sourceAccess = VulkanUtility::GetAccessFlagsFromAccessMask(sourceAccessMask);
	const GpuAccessFlags destinationAccess = VulkanUtility::GetAccessFlagsFromAccessMask(destinationAccessMask);

	mCombinedSourceStages |= sourceStageMask;
	mCombinedDestinationStages |= destinationStageMask;
	mCombinedSourceAccess |= sourceAccess;
	mCombinedDestinationAccess |= destinationAccess;

	auto found = std::find_if(mImageBarriers.begin(), mImageBarriers.end(), [image, &subresourceRange](const VkImageMemoryBarrier& barrier)
	{
		return barrier.image == image->GetVulkanHandle() && VulkanUtility::RangeEquals(barrier.subresourceRange, subresourceRange);
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
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.image = image->GetVulkanHandle();
		barrier.subresourceRange = subresourceRange;

		mImageBarriers.push_back(barrier);
	}
	else
	{
		found->srcAccessMask |= sourceAccessMask;
		found->dstAccessMask |= destinationAccessMask;
		found->newLayout = newLayout;

		oldLayout = found->oldLayout;
	}

	if(oldLayout != newLayout)
	{
		LayoutTrackingInfo layoutTrackingInfo;
		layoutTrackingInfo.Image = image;
		layoutTrackingInfo.SubresourceRange = subresourceRange;
		layoutTrackingInfo.OldLayout = oldLayout;
		layoutTrackingInfo.NewLayout = newLayout;
		mImageLayoutTracking.push_back(layoutTrackingInfo);

		mHasLayoutTransition = true;
	}

#if B3D_HAZARD_TRACKING
	BarrierTrackingInfo barrierTrackingInfo;
	barrierTrackingInfo.Image = image;
	barrierTrackingInfo.ImageSubresourceRange = subresourceRange;
	barrierTrackingInfo.SourceAccess = sourceAccess;
	barrierTrackingInfo.SourceStages = sourceStageMask;
	barrierTrackingInfo.DestinationAccess = destinationAccess;
	barrierTrackingInfo.DestinationStages = destinationStageMask;
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
