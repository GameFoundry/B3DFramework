//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanBarrierHelper.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanUtility.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

using namespace b3d;
using namespace b3d::render;

VulkanBarrierHelper::VulkanBarrierHelper(VulkanResourceTracker* resourceTracker)
	: mResourceTracker(resourceTracker)
{ }

const VulkanBarrierHelper::BarrierTrackingInfo* VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
{
	if(buffer == nullptr)
		return nullptr;

	const GpuStageFlags sourceAccessStageFlags = GpuBackendUtility::GetStageFlags(sourceUsage);
	const GpuStageFlags destinationAccessStageFlags = GpuBackendUtility::GetStageFlags(destinationUsage);

	return AddBufferBarrier(buffer, sourceAccessStageFlags, sourceAccess, destinationAccessStageFlags, destinationAccess);
}

const VulkanBarrierHelper::BarrierTrackingInfo* VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
{
	if(buffer == nullptr)
		return nullptr;

	const VulkanResourceTracker::BufferTrackingState* bufferTrackingState = mResourceTracker->FindBufferTrackingState(buffer);
	if(bufferTrackingState == nullptr)
		return nullptr;

	return AddBufferBarrier(buffer, *bufferTrackingState, destinationUsage, destinationAccess);
}

const VulkanBarrierHelper::BarrierTrackingInfo* VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, const VulkanResourceTracker::BufferTrackingState& bufferTrackingState, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
{
	if(buffer == nullptr)
		return nullptr;

	const GpuStageFlags destinationAccessStageFlags = GpuBackendUtility::GetStageFlags(destinationUsage);

	GpuStageFlags sourceAccessStageFlags;
	GpuAccessFlags sourceAccessFlags;

	// WAW or RAW hazard
	const GpuStageFlags writeAccessStageFlags = bufferTrackingState.WriteHazardTracking->MemoryBarrierTracking.GetUnsafeAccessStages(destinationAccessStageFlags);
	if(destinationAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		sourceAccessStageFlags |= writeAccessStageFlags;

		if(writeAccessStageFlags != GpuStageFlag::None)
			sourceAccessFlags |= GpuAccessFlag::Write;
	}

	// WAR hazard
	const GpuStageFlags readAccessStageFlags = bufferTrackingState.WriteHazardTracking->ExecutionBarrierTracking.GetUnsafeAccessStages(destinationAccessStageFlags);
	if(destinationAccess.IsSet(GpuAccessFlag::Write))
	{
		sourceAccessStageFlags |= readAccessStageFlags;

		if(readAccessStageFlags != GpuStageFlag::None)
			sourceAccessFlags |= GpuAccessFlag::Read;
	}

	if(sourceAccessFlags == GpuAccessFlag::None)
		return nullptr;

	return AddBufferBarrier(buffer, sourceAccessStageFlags, sourceAccessFlags, destinationAccessStageFlags, destinationAccess);
}

const VulkanBarrierHelper::BarrierTrackingInfo* VulkanBarrierHelper::AddBufferBarrier(VulkanBuffer* buffer, GpuStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccessFlags, GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccessFlags)
{
	if(buffer == nullptr)
		return nullptr;

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

		mBufferBarriers.Add(barrier);
	}
	else
	{
		found->srcAccessMask |= sourceAccessMask;
		found->dstAccessMask |= destinationAccessMask;
	}

	BarrierTrackingInfo trackingInfo;
	trackingInfo.Buffer = buffer;
	trackingInfo.SourceAccess = sourceAccessFlags;
	trackingInfo.SourceAccessStages = sourceAccessStageFlags;
	trackingInfo.DestinationAccess = destinationAccessFlags;
	trackingInfo.DestinationAccessStages = destinationAccessStageFlags;
	mBarrierTracking.Add(trackingInfo);

	return &mBarrierTracking.back();
}

const VulkanBarrierHelper::BarrierTrackingInfo* VulkanBarrierHelper::AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccessFlags, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccessFlags, GpuImageLayout oldLayout, GpuImageLayout newLayout)
{
	const GpuStageFlags sourceAccessStageFlags = GpuBackendUtility::GetStageFlags(sourceUsage);
	const GpuStageFlags destinationAccessStageFlags = GpuBackendUtility::GetStageFlags(destinationUsage);

	const VkImageLayout vkOldLayout = VulkanUtility::GetImageLayout(oldLayout);
	const VkImageLayout vkNewLayout = VulkanUtility::GetImageLayout(newLayout);

	return AddSubresourceBarrier(image, subresourceRange, sourceAccessStageFlags, sourceAccessFlags, destinationAccessStageFlags, destinationAccessFlags, vkOldLayout, vkNewLayout);
}

const VulkanBarrierHelper::BarrierTrackingInfo* VulkanBarrierHelper::AddImageBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, GpuImageLayout newLayout)
{
	if(image == nullptr)
		return nullptr;

	const VulkanResourceTracker::ImageTrackingState* imageTrackingState = mResourceTracker->FindImageTrackingState(image);
	if(imageTrackingState == nullptr)
		return nullptr;

	struct CallbackParameters
	{
		VulkanBarrierHelper* BarrierHelper;
		VulkanResourceTracker* ResourceTracker;
		VulkanImage* Image;
		GpuResourceUseFlags DestinationUsage;
		GpuAccessFlags DestinationAccess;
		GpuImageLayout NewLayout;
		const BarrierTrackingInfo* OutTrackingInfo;
	};

	CallbackParameters callbackParameters { this, mResourceTracker, image, destinationUsage, destinationAccess, newLayout, nullptr };
	mResourceTracker->IterateAndCreateOverlappingImageSubresourceTrackingState(image, subresourceRange, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* const callbackParameters = static_cast<CallbackParameters*>(userData);

		VulkanResourceTracker& resourceTracker = *callbackParameters->ResourceTracker;
		const VulkanResourceTracker::ImageSubresourceTrackingState& subresourceTrackingState = resourceTracker.GetSubresourceTrackingStateAtIndex(globalSubresourceIndex);

		const VkImageLayout vkNewLayout = VulkanUtility::GetImageLayout(callbackParameters->NewLayout);

		VulkanBarrierHelper& barrierHelper = *callbackParameters->BarrierHelper;
		callbackParameters->OutTrackingInfo = barrierHelper.AddSubresourceBarrier(callbackParameters->Image, subresourceTrackingState, callbackParameters->DestinationUsage, callbackParameters->DestinationAccess, vkNewLayout);
	}, &callbackParameters);

	return callbackParameters.OutTrackingInfo;
}

const VulkanBarrierHelper::BarrierTrackingInfo* VulkanBarrierHelper::AddSubresourceBarrier(VulkanImage* image, const VulkanResourceTracker::ImageSubresourceTrackingState& subresourceTrackingState, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, VkImageLayout newLayout)
{
	if(image == nullptr)
		return nullptr;
	
	const GpuStageFlags destinationAccessStageFlags = GpuBackendUtility::GetStageFlags(destinationUsage);

	GpuStageFlags sourceAccessStageFlags;
	GpuAccessFlags sourceAccessFlags;

	// WAW or RAW hazard
	const GpuStageFlags writeAccessStageFlags = subresourceTrackingState.WriteHazardTracking->MemoryBarrierTracking.GetUnsafeAccessStages(destinationAccessStageFlags);
	if(destinationAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		sourceAccessStageFlags |= writeAccessStageFlags;

		if(writeAccessStageFlags != GpuStageFlag::None)
			sourceAccessFlags |= GpuAccessFlag::Write;
	}

	// WAR hazard
	const GpuStageFlags readAccessStageFlags = subresourceTrackingState.WriteHazardTracking->ExecutionBarrierTracking.GetUnsafeAccessStages(destinationAccessStageFlags);
	if(destinationAccess.IsSet(GpuAccessFlag::Write))
	{
		sourceAccessStageFlags |= readAccessStageFlags;

		if(readAccessStageFlags != GpuStageFlag::None)
			sourceAccessFlags |= GpuAccessFlag::Read;
	}

	// No layout transition if destination layout is undefined
	if(newLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		newLayout = subresourceTrackingState.CurrentLayout;

	if(sourceAccessFlags == GpuAccessFlag::None && subresourceTrackingState.CurrentLayout == newLayout)
		return nullptr;

	return AddSubresourceBarrier(image, subresourceTrackingState.Range, sourceAccessStageFlags, sourceAccessFlags, destinationAccessStageFlags, destinationAccess, subresourceTrackingState.CurrentLayout, newLayout);
}

const VulkanBarrierHelper::BarrierTrackingInfo* VulkanBarrierHelper::AddSubresourceBarrier(VulkanImage* image, const VkImageSubresourceRange& subresourceRange, GpuStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccessFlags, GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccessFlags, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	if(image == nullptr)
		return nullptr;

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

		mImageBarriers.Add(barrier);
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
			mImageLayoutTracking.Add(layoutTrackingInfo);
		}
		else
		{
			B3D_ASSERT(foundTracking->OldLayout == oldLayout);
			foundTracking->NewLayout = newLayout;
		}

		// TODO - Use more specific stages for layout transitions? Make sure layout transitions are only doing an execution barrier if memory barrier isn't needed
		mCombinedSourceStages |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		mHasLayoutTransition = true;
	}

	BarrierTrackingInfo barrierTrackingInfo;
	barrierTrackingInfo.Image = image;
	barrierTrackingInfo.ImageSubresourceRange = subresourceRange;
	barrierTrackingInfo.SourceAccess = sourceAccessFlags;
	barrierTrackingInfo.SourceAccessStages = sourceAccessStageFlags;
	barrierTrackingInfo.DestinationAccess = destinationAccessFlags;
	barrierTrackingInfo.DestinationAccessStages = destinationAccessStageFlags;
	mBarrierTracking.Add(barrierTrackingInfo);

	return &mBarrierTracking.back();
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

		// Update hazard tracking for all barriers
		for(const auto& trackingInfo : mBarrierTracking)
		{
			// TODO - SourceAccess/DestinationAccess should probably be the combined source/destination access, as the only thing that
			// matters if memory barrier was executed or not.
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

	mImageLayoutTracking.Clear();
	mHasLayoutTransition = false;

	mBarrierTracking.Clear();
}

bool VulkanBarrierHelper::HasBarriers() const
{
	return !mBufferBarriers.Empty() || !mImageBarriers.Empty();
}
