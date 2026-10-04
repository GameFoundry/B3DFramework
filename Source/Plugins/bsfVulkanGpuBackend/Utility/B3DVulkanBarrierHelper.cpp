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

template class b3d::render::TGpuBarrierHelper<b3d::render::VulkanBarrierHelper, b3d::render::VulkanResourceTracker>;

VulkanBarrierBatch::VulkanBarrierBatch()
{
	Clear();
}

void VulkanBarrierBatch::AddBufferBarrier(VkBuffer buffer, const GpuBarrierScope& barrier, u32 sourceQueueFamily, u32 destinationQueueFamily, VkDeviceSize offset, VkDeviceSize size)
{
	VkPipelineStageFlags sourceStages, destinationStages;
	VkAccessFlags sourceAccess, destinationAccess;

	// Only writes require Vulkan memory availability. Source reads are ordered through the source stage mask.
	const GpuAccessFlags sourceAvailabilityAccess = barrier.SourceAccess.IsSet(GpuAccessFlag::Write) ?  GpuAccessFlags(GpuAccessFlag::Write) : GpuAccessFlags(GpuAccessFlag::None);
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.SourceStages, sourceAvailabilityAccess, sourceStages, sourceAccess);
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.DestinationStages, barrier.DestinationAccess, destinationStages, destinationAccess);

	if(sourceQueueFamily != destinationQueueFamily)
		AddBufferBarrier(buffer, sourceStages, sourceAccess, destinationStages, destinationAccess, sourceQueueFamily, destinationQueueFamily, offset, size);
	else if(barrier.SourceAccess.IsSet(GpuAccessFlag::Write))
		AddMemoryBarrier(sourceStages, sourceAccess, destinationStages, destinationAccess);
	else if(barrier.SourceAccess.IsSet(GpuAccessFlag::Read) && barrier.DestinationAccess.IsSet(GpuAccessFlag::Write))
		AddExecutionBarrier(sourceStages, destinationStages);
}

void VulkanBarrierBatch::AddBufferBarrier(VkBuffer buffer, VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess, u32 sourceQueueFamily, u32 destinationQueueFamily, VkDeviceSize offset, VkDeviceSize size)
{
	mCombinedSourceStages |= sourceStages;
	mCombinedDestinationStages |= destinationStages;

	if(sourceQueueFamily == destinationQueueFamily)
	{
		AddMemoryBarrier(sourceStages, sourceAccess, destinationStages, destinationAccess);
		return;
	}

	auto found = std::find_if(mOwnershipBufferBarriers.begin(), mOwnershipBufferBarriers.end(), [buffer, sourceQueueFamily, destinationQueueFamily, offset, size](const VkBufferMemoryBarrier& barrier)
	{
		return barrier.buffer == buffer && barrier.srcQueueFamilyIndex == sourceQueueFamily && barrier.dstQueueFamilyIndex == destinationQueueFamily && barrier.offset == offset && barrier.size == size;
	});

	if(found == mOwnershipBufferBarriers.end())
	{
		VkBufferMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.srcAccessMask = sourceAccess;
		barrier.dstAccessMask = destinationAccess;
		barrier.srcQueueFamilyIndex = sourceQueueFamily;
		barrier.dstQueueFamilyIndex = destinationQueueFamily;
		barrier.buffer = buffer;
		barrier.offset = offset;
		barrier.size = size;

		mOwnershipBufferBarriers.Add(barrier);
	}
	else
	{
		found->srcAccessMask |= sourceAccess;
		found->dstAccessMask |= destinationAccess;
	}
}

void VulkanBarrierBatch::AddImageBarrier(VkImage image, const VkImageSubresourceRange& range, const GpuBarrierScope& barrier, VkImageLayout oldLayout, VkImageLayout newLayout, u32 sourceQueueFamily, u32 destinationQueueFamily)
{
	bool needsImageBarrier = oldLayout != newLayout || sourceQueueFamily != destinationQueueFamily;
	if(!needsImageBarrier)
	{
		// A request for the original layout can cancel an earlier transition in this batch.
		needsImageBarrier = std::any_of(mImageBarriers.begin(), mImageBarriers.end(), [image, &range, sourceQueueFamily, destinationQueueFamily](const VkImageMemoryBarrier& pendingBarrier)
		{
			return pendingBarrier.image == image && pendingBarrier.srcQueueFamilyIndex == sourceQueueFamily && pendingBarrier.dstQueueFamilyIndex == destinationQueueFamily && VulkanUtility::RangeEquals(pendingBarrier.subresourceRange, range);
		});
	}

	if(!needsImageBarrier)
	{
		if(barrier.SourceAccess.IsSet(GpuAccessFlag::Write))
			AddMemoryBarrier(barrier);
		else if(barrier.SourceAccess.IsSet(GpuAccessFlag::Read) && barrier.DestinationAccess.IsSet(GpuAccessFlag::Write))
			AddExecutionBarrier(barrier);
	}

	VkPipelineStageFlags sourceStages, destinationStages;
	VkAccessFlags sourceAccess, destinationAccess;

	// Only writes require Vulkan memory availability. Source reads are ordered through the source stage mask.
	const GpuAccessFlags sourceAvailabilityAccess = barrier.SourceAccess.IsSet(GpuAccessFlag::Write) ?  GpuAccessFlags(GpuAccessFlag::Write) : GpuAccessFlags(GpuAccessFlag::None);
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.SourceStages, sourceAvailabilityAccess, sourceStages, sourceAccess);
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.DestinationStages, barrier.DestinationAccess, destinationStages, destinationAccess);

	AddImageBarrier(image, range, sourceStages, sourceAccess, destinationStages, destinationAccess, oldLayout, newLayout, sourceQueueFamily, destinationQueueFamily);
}

void VulkanBarrierBatch::AddImageBarrier(VkImage image, const VkImageSubresourceRange& range, VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess, VkImageLayout oldLayout, VkImageLayout newLayout, u32 sourceQueueFamily, u32 destinationQueueFamily)
{
	mCombinedSourceStages |= sourceStages;
	mCombinedDestinationStages |= destinationStages;

	auto found = std::find_if(mImageBarriers.begin(), mImageBarriers.end(), [image, &range, sourceQueueFamily, destinationQueueFamily](const VkImageMemoryBarrier& barrier)
	{
		return barrier.image == image && barrier.srcQueueFamilyIndex == sourceQueueFamily && barrier.dstQueueFamilyIndex == destinationQueueFamily && VulkanUtility::RangeEquals(barrier.subresourceRange, range);
	});

	if(found == mImageBarriers.end())
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = sourceAccess;
		barrier.dstAccessMask = destinationAccess;
		barrier.srcQueueFamilyIndex = sourceQueueFamily;
		barrier.dstQueueFamilyIndex = destinationQueueFamily;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.image = image;
		barrier.subresourceRange = range;

		mImageBarriers.Add(barrier);
		return;
	}

	found->srcAccessMask |= sourceAccess;
	found->dstAccessMask |= destinationAccess;
	found->newLayout = newLayout;
	if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		found->oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void VulkanBarrierBatch::AddExecutionBarrier(const GpuBarrierScope& barrier)
{
	if(!barrier.IsValid())
		return;

	VkPipelineStageFlags sourceStages, destinationStages;
	VkAccessFlags unusedAccess;
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.SourceStages, barrier.SourceAccess, sourceStages, unusedAccess);
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.DestinationStages, barrier.DestinationAccess, destinationStages, unusedAccess);

	AddExecutionBarrier(sourceStages, destinationStages);
}

void VulkanBarrierBatch::AddExecutionBarrier(VkPipelineStageFlags sourceStages, VkPipelineStageFlags destinationStages)
{
	mCombinedSourceStages |= sourceStages;
	mCombinedDestinationStages |= destinationStages;
	mHasExecutionBarrier = true;
}

void VulkanBarrierBatch::AddMemoryBarrier(const GpuBarrierScope& barrier)
{
	if(!barrier.IsValid())
		return;

	VkPipelineStageFlags sourceStages, destinationStages;
	VkAccessFlags sourceAccess, destinationAccess;

	// Only writes require Vulkan memory availability. Source reads are ordered through an execution dependency.
	const GpuAccessFlags sourceAvailabilityAccess = barrier.SourceAccess.IsSet(GpuAccessFlag::Write) ?  GpuAccessFlags(GpuAccessFlag::Write) : GpuAccessFlags(GpuAccessFlag::None);
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.SourceStages, sourceAvailabilityAccess, sourceStages, sourceAccess);
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.DestinationStages, barrier.DestinationAccess, destinationStages, destinationAccess);

	AddMemoryBarrier(sourceStages, sourceAccess, destinationStages, destinationAccess);
}

void VulkanBarrierBatch::AddMemoryBarrier(VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess)
{
	mCombinedSourceStages |= sourceStages;
	mCombinedDestinationStages |= destinationStages;
	mMemoryBarrier.srcAccessMask |= sourceAccess;
	mMemoryBarrier.dstAccessMask |= destinationAccess;
	mHasMemoryBarrier = true;
}

void VulkanBarrierBatch::Execute(VkCommandBuffer commandBuffer) const
{
	if(!HasBarriers())
		return;

	const VkPipelineStageFlags sourceStages = mCombinedSourceStages != 0 ? mCombinedSourceStages : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	const VkPipelineStageFlags destinationStages = mCombinedDestinationStages != 0 ? mCombinedDestinationStages : VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	vkCmdPipelineBarrier(commandBuffer, sourceStages, destinationStages, 0, mHasMemoryBarrier ? 1u : 0u, mHasMemoryBarrier ? &mMemoryBarrier : nullptr, (u32)mOwnershipBufferBarriers.size(), mOwnershipBufferBarriers.data(), (u32)mImageBarriers.size(), mImageBarriers.data());
}

void VulkanBarrierBatch::Clear()
{
	mMemoryBarrier = VkMemoryBarrier();
	mMemoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;

	mOwnershipBufferBarriers.Clear();
	mImageBarriers.Clear();
	mCombinedSourceStages = 0;
	mCombinedDestinationStages = 0;
	mHasMemoryBarrier = false;
	mHasExecutionBarrier = false;
}

bool VulkanBarrierBatch::HasBarriers() const
{
	return mHasMemoryBarrier || mHasExecutionBarrier || !mOwnershipBufferBarriers.Empty() || !mImageBarriers.Empty();
}

VulkanBarrierHelper::VulkanBarrierHelper(VulkanResourceTracker* resourceTracker)
	: TGpuBarrierHelper<VulkanBarrierHelper, VulkanResourceTracker>(resourceTracker)
{ }

void VulkanBarrierHelper::RecordNativeBufferBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier, GpuBarrierFlags barrierFlags)
{
	// Buffer barriers without an ownership transfer are recorded as global barriers already, which also covers an alias acquire's source
	(void)barrierFlags;

	const VkBuffer bufferHandle = static_cast<VulkanBuffer*>(buffer)->GetVulkanHandle();
	mBarrierBatch.AddBufferBarrier(bufferHandle, barrier);
}

void VulkanBarrierHelper::RecordNativeImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, const GpuBarrierScope& barrier, GpuImageLayout oldLayout, GpuImageLayout newLayout, GpuBarrierFlags barrierFlags)
{
	const VkImage imageHandle = static_cast<VulkanImage*>(image)->GetVulkanHandle();
	const VkImageLayout vkOldLayout = barrierFlags.IsSet(GpuBarrierFlag::DiscardContents) ? VK_IMAGE_LAYOUT_UNDEFINED : VulkanUtility::ToVkImageLayout(oldLayout);
	const VkImageLayout vkNewLayout = VulkanUtility::ToVkImageLayout(newLayout);
	const VkImageSubresourceRange vkSubresourceRange = VulkanUtility::ToVkImageSubresourceRange(subresourceRange);

	// The image barrier's memory dependency covers only this image, so an alias acquire adds a global memory barrier for them. The image barrier
	// still performs the transition from VK_IMAGE_LAYOUT_UNDEFINED.
	if(barrierFlags.IsSet(GpuBarrierFlag::AliasAcquire) && barrier.SourceAccess.IsSet(GpuAccessFlag::Write))
		mBarrierBatch.AddMemoryBarrier(barrier);

	mBarrierBatch.AddImageBarrier(imageHandle, vkSubresourceRange, barrier, vkOldLayout, vkNewLayout);
}

void VulkanBarrierHelper::Execute(VulkanGpuCommandBuffer& commandBuffer)
{
	if(HasBarriers())
	{
		mBarrierBatch.Execute(commandBuffer.GetVulkanHandle());
		ApplyPostBarrierTracking();
	}

	// Apply read/write hazard registrations deferred while tracking resource accesses
	mResourceTracker->CommitPendingAccesses();

	Clear();
}

void VulkanBarrierHelper::Clear()
{
	mBarrierBatch.Clear();

	TGpuBarrierHelper<VulkanBarrierHelper, VulkanResourceTracker>::Clear();
}

bool VulkanBarrierHelper::HasBarriers() const
{
	return mBarrierBatch.HasBarriers();
}
