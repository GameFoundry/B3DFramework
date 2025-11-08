//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanBarrierHelper.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanUtility.h"

using namespace b3d;
using namespace b3d::render;

VulkanBarrierHelper::VulkanBarrierHelper(VulkanResourceTracker* resourceTracker)
	: mResourceTracker(resourceTracker)
{
}

void VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
{
	if(buffer == nullptr)
		return;

	const VulkanAccessStageFlags sourceAccessStageFlags = VulkanUtility::GetVulkanAccessStageFlags(sourceUsage);
	const VulkanAccessStageFlags destinationAccessStageFlags = VulkanUtility::GetVulkanAccessStageFlags(destinationUsage);

	AddBufferBarrier(buffer, sourceAccessStageFlags, sourceAccess, destinationAccessStageFlags, destinationAccess);
}

void VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
{
	if(buffer == nullptr)
		return;

	const VulkanResourceTracker::BufferTrackingState* bufferTrackingState = mResourceTracker->FindBufferTrackingState(buffer);
	if(bufferTrackingState == nullptr)
		return;

	AddBufferBarrier(buffer, *bufferTrackingState, destinationUsage, destinationAccess);
}

void VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, const VulkanResourceTracker::BufferTrackingState& bufferTrackingState, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
{
	if(buffer == nullptr)
		return;

	const VulkanAccessStageFlags destinationAccessStageFlags = VulkanUtility::GetVulkanAccessStageFlags(destinationUsage);

	VulkanAccessStageFlags sourceAccessStageFlags;
	GpuAccessFlags sourceAccessFlags;

#if B3D_HAZARD_TRACKING
	// WAW or RAW hazard
	const VulkanAccessStageFlags writeAccessStageFlags = bufferTrackingState.WriteHazardTracking->WriteAccessStages.GetUnsafeAccessStages(destinationAccessStageFlags);
	if(destinationAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		sourceAccessStageFlags |= writeAccessStageFlags;

		if(writeAccessStageFlags != VulkanAccessStageFlag::None)
			sourceAccessFlags |= GpuAccessFlag::Write;
	}

	// WAR hazard
	const VulkanAccessStageFlags readAccessStageFlags = bufferTrackingState.WriteHazardTracking->ReadAccessStages.GetUnsafeAccessStages(destinationAccessStageFlags);
	if(destinationAccess.IsSet(GpuAccessFlag::Write))
	{
		sourceAccessStageFlags |= readAccessStageFlags;

		if(readAccessStageFlags != VulkanAccessStageFlag::None)
			sourceAccessFlags |= GpuAccessFlag::Read;
	}
#endif

	if(sourceAccessFlags == GpuAccessFlag::None)
		return;

	AddBufferBarrier(buffer, sourceAccessStageFlags, sourceAccessFlags, destinationAccessStageFlags, destinationAccess);
}

void VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, VulkanAccessStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccessFlags, VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccessFlags)
{
	if(buffer == nullptr)
		return;

	VkPipelineStageFlags sourceStageMask, destinationStageMask;
	VkAccessFlags sourceAccessMask, destinationAccessMask;
	VulkanUtility::GetPipelineStageAndAccessMask(sourceAccessStageFlags, sourceAccessFlags, sourceStageMask, sourceAccessMask);
	VulkanUtility::GetPipelineStageAndAccessMask(destinationAccessStageFlags, destinationAccessFlags, destinationStageMask, destinationAccessMask);

	mCombinedSourceStages |= sourceStageMask;
	mCombinedDestinationStages |= destinationStageMask;
	mCombinedSourceAccess |= sourceAccessFlags;
	mCombinedDestinationAccess |= destinationAccessFlags;

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
	trackingInfo.SourceAccess = sourceAccessFlags;
	trackingInfo.SourceAccessStages = sourceAccessStageFlags;
	trackingInfo.DestinationAccess = destinationAccessFlags;
	trackingInfo.DestinationAccessStages = destinationAccessStageFlags;
	mBarrierTracking.push_back(trackingInfo);
#endif
}

void VulkanBarrierHelper::AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccessFlags, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccessFlags, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	const VulkanAccessStageFlags sourceAccessStageFlags = VulkanUtility::GetVulkanAccessStageFlags(sourceUsage);
	const VulkanAccessStageFlags destinationAccessStageFlags = VulkanUtility::GetVulkanAccessStageFlags(destinationUsage);

	AddImageBarrier(image, subresourceRange, sourceAccessStageFlags, sourceAccessFlags, destinationAccessStageFlags, destinationAccessFlags, oldLayout, newLayout);
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

		VulkanBarrierHelper& barrierHelper = *callbackParameters->BarrierHelper;
		barrierHelper.AddSubresourceBarrier(callbackParameters->Image, subresourceTrackingState, callbackParameters->DestinationUsage, callbackParameters->DestinationAccess, callbackParameters->NewLayout);
	}, &callbackParameters);
}

void VulkanBarrierHelper::AddSubresourceBarrier(VulkanImage* image, const VulkanResourceTracker::ImageSubresourceTrackingState& subresourceTrackingState, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, VkImageLayout newLayout)
{
	if(image == nullptr)
		return;
	
	const VulkanAccessStageFlags destinationAccessStageFlags = VulkanUtility::GetVulkanAccessStageFlags(destinationUsage);

	VulkanAccessStageFlags sourceAccessStageFlags;
	GpuAccessFlags sourceAccessFlags;

#if B3D_HAZARD_TRACKING
	// WAW or RAW hazard
	const VulkanAccessStageFlags writeAccessStageFlags = subresourceTrackingState.WriteHazardTracking->WriteAccessStages.GetUnsafeAccessStages(destinationAccessStageFlags);
	if(destinationAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		sourceAccessStageFlags |= writeAccessStageFlags;

		if(writeAccessStageFlags != VulkanAccessStageFlag::None)
			sourceAccessFlags |= GpuAccessFlag::Write;
	}

	// WAR hazard
	const VulkanAccessStageFlags readAccessStageFlags = subresourceTrackingState.WriteHazardTracking->ReadAccessStages.GetUnsafeAccessStages(destinationAccessStageFlags);
	if(destinationAccess.IsSet(GpuAccessFlag::Write))
	{
		sourceAccessStageFlags |= readAccessStageFlags;

		if(readAccessStageFlags != VulkanAccessStageFlag::None)
			sourceAccessFlags |= GpuAccessFlag::Read;
	}
#endif

	// TODO - Execution only barriers?
	// TODO - Stages need to be provided for layout transitions (again, for execution dependency)

	if(sourceAccessFlags == GpuAccessFlag::None && (subresourceTrackingState.CurrentLayout == newLayout || newLayout == VK_IMAGE_LAYOUT_UNDEFINED))
		return;

	AddImageBarrier(image, subresourceTrackingState.Range, sourceAccessStageFlags, sourceAccessFlags, destinationAccessStageFlags, destinationAccess, subresourceTrackingState.CurrentLayout, newLayout);
}

void VulkanBarrierHelper::AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, VulkanAccessStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccessFlags, VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccessFlags, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	if(image == nullptr)
		return;

	VkPipelineStageFlags sourceStageMask, destinationStageMask;
	VkAccessFlags sourceAccessMask, destinationAccessMask;
	VulkanUtility::GetPipelineStageAndAccessMask(sourceAccessStageFlags, sourceAccessFlags, sourceStageMask, sourceAccessMask);
	VulkanUtility::GetPipelineStageAndAccessMask(destinationAccessStageFlags, destinationAccessFlags, destinationStageMask, destinationAccessMask);

	mCombinedSourceStages |= sourceStageMask;
	mCombinedDestinationStages |= destinationStageMask;
	mCombinedSourceAccess |= sourceAccessFlags;
	mCombinedDestinationAccess |= destinationAccessFlags;

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

	auto foundTracking = std::find_if(mImageLayoutTracking.begin(), mImageLayoutTracking.end(), [image, &subresourceRange](const LayoutTrackingInfo& layoutTrackingInfo)
	{
		return layoutTrackingInfo.Image == image && VulkanUtility::RangeEquals(layoutTrackingInfo.SubresourceRange, subresourceRange);
	});

	if(oldLayout != newLayout)
	{
		if(foundTracking == mImageLayoutTracking.end())
		{
			LayoutTrackingInfo layoutTrackingInfo;
			layoutTrackingInfo.Image = image;
			layoutTrackingInfo.SubresourceRange = subresourceRange;
			layoutTrackingInfo.OldLayout = oldLayout;
			layoutTrackingInfo.NewLayout = newLayout;
			mImageLayoutTracking.push_back(layoutTrackingInfo);
		}
		else
		{
			B3D_ASSERT(foundTracking->OldLayout == oldLayout);
			foundTracking->NewLayout = newLayout;
		}

		// TODO - Use more specific stages for layout transitions?
		mCombinedSourceStages |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		mHasLayoutTransition = true;
	}

#if B3D_HAZARD_TRACKING
	BarrierTrackingInfo barrierTrackingInfo;
	barrierTrackingInfo.Image = image;
	barrierTrackingInfo.ImageSubresourceRange = subresourceRange;
	barrierTrackingInfo.SourceAccess = sourceAccessFlags;
	barrierTrackingInfo.SourceAccessStages = sourceAccessStageFlags;
	barrierTrackingInfo.DestinationAccess = destinationAccessFlags;
	barrierTrackingInfo.DestinationAccessStages = destinationAccessStageFlags;
	mBarrierTracking.push_back(barrierTrackingInfo);
#endif
}

void VulkanBarrierHelper::Execute(VulkanGpuCommandBuffer& commandBuffer)
{
	if(!HasBarriers())
		return;

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
				trackingInfo.SourceAccessStages,
				trackingInfo.SourceAccess,
				trackingInfo.DestinationAccessStages,
				trackingInfo.DestinationAccess);
		}
		else if(trackingInfo.Image != nullptr)
		{
			mResourceTracker->UpdateWriteHazardTrackingAfterBarrier(
				trackingInfo.Image,
				trackingInfo.ImageSubresourceRange,
				trackingInfo.SourceAccessStages,
				trackingInfo.SourceAccess,
				trackingInfo.DestinationAccessStages,
				trackingInfo.DestinationAccess);
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
