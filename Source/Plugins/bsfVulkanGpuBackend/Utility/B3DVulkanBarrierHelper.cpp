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

void VulkanBarrierBatch::AddBufferBarrier(VkBuffer buffer, const GpuHazardStageAndAccess& barrier,
	u32 sourceQueueFamily, u32 destinationQueueFamily, VkDeviceSize offset, VkDeviceSize size)
{
	VkPipelineStageFlags sourceStages, destinationStages;
	VkAccessFlags sourceAccess, destinationAccess;
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.SourceStages, barrier.SourceAccess, sourceStages, sourceAccess);
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.DestinationStages, barrier.DestinationAccess, destinationStages, destinationAccess);

	AddBufferBarrier(buffer, sourceStages, sourceAccess, destinationStages, destinationAccess, sourceQueueFamily, destinationQueueFamily, offset, size);
}

void VulkanBarrierBatch::AddBufferBarrier(VkBuffer buffer, VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess, u32 sourceQueueFamily, u32 destinationQueueFamily, VkDeviceSize offset, VkDeviceSize size)
{
	mCombinedSourceStages |= sourceStages;
	mCombinedDestinationStages |= destinationStages;

	auto found = std::find_if(mBufferBarriers.begin(), mBufferBarriers.end(), [buffer, sourceQueueFamily, destinationQueueFamily, offset, size](const VkBufferMemoryBarrier& barrier)
	{
		return barrier.buffer == buffer && barrier.srcQueueFamilyIndex == sourceQueueFamily && barrier.dstQueueFamilyIndex == destinationQueueFamily && barrier.offset == offset && barrier.size == size;
	});

	if(found == mBufferBarriers.end())
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

		mBufferBarriers.Add(barrier);
	}
	else
	{
		found->srcAccessMask |= sourceAccess;
		found->dstAccessMask |= destinationAccess;
	}
}

VkImageLayout VulkanBarrierBatch::AddImageBarrier(VkImage image, const VkImageSubresourceRange& range,
	const GpuHazardStageAndAccess& barrier, VkImageLayout oldLayout, VkImageLayout newLayout,
	u32 sourceQueueFamily, u32 destinationQueueFamily)
{
	VkPipelineStageFlags sourceStages, destinationStages;
	VkAccessFlags sourceAccess, destinationAccess;
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.SourceStages, barrier.SourceAccess, sourceStages, sourceAccess);
	VulkanUtility::GetPipelineStageAndAccessMask(barrier.DestinationStages, barrier.DestinationAccess, destinationStages, destinationAccess);

	return AddImageBarrier(image, range, sourceStages, sourceAccess, destinationStages, destinationAccess, oldLayout, newLayout, sourceQueueFamily, destinationQueueFamily);
}

VkImageLayout VulkanBarrierBatch::AddImageBarrier(VkImage image, const VkImageSubresourceRange& range, VkPipelineStageFlags sourceStages, VkAccessFlags sourceAccess, VkPipelineStageFlags destinationStages, VkAccessFlags destinationAccess, VkImageLayout oldLayout, VkImageLayout newLayout, u32 sourceQueueFamily, u32 destinationQueueFamily)
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
		return oldLayout;
	}

	found->srcAccessMask |= sourceAccess;
	found->dstAccessMask |= destinationAccess;
	found->newLayout = newLayout;

	return found->oldLayout;
}

void VulkanBarrierBatch::AddExecutionBarrier(const GpuHazardStageAndAccess& barrier)
{
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

void VulkanBarrierBatch::Execute(VkCommandBuffer commandBuffer) const
{
	if(!HasBarriers())
		return;

	const VkPipelineStageFlags sourceStages = mCombinedSourceStages != 0 ? mCombinedSourceStages : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	const VkPipelineStageFlags destinationStages = mCombinedDestinationStages != 0 ? mCombinedDestinationStages : VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	vkCmdPipelineBarrier(commandBuffer, sourceStages, destinationStages, 0, 0, nullptr, (u32)mBufferBarriers.size(), mBufferBarriers.data(), (u32)mImageBarriers.size(), mImageBarriers.data());
}

void VulkanBarrierBatch::Clear()
{
	mBufferBarriers.Clear();
	mImageBarriers.Clear();
	mCombinedSourceStages = 0;
	mCombinedDestinationStages = 0;
	mHasExecutionBarrier = false;
}

bool VulkanBarrierBatch::HasBarriers() const
{
	return mHasExecutionBarrier || !mBufferBarriers.Empty() || !mImageBarriers.Empty();
}

VulkanBarrierHelper::VulkanBarrierHelper(VulkanResourceTracker* resourceTracker)
	: TGpuBarrierHelper<VulkanBarrierHelper>(resourceTracker)
{ }

void VulkanBarrierHelper::RecordBufferBarrier(IGpuBufferResource* buffer, const GpuHazardStageAndAccess& barrier)
{
	const VkBuffer bufferHandle = static_cast<VulkanBuffer*>(buffer)->GetVulkanHandle();

	if(barrier.SourceAccess.IsSet(GpuAccessFlag::Write))
		mBarrierBatch.AddBufferBarrier(bufferHandle, barrier);
	else if(barrier.SourceAccess.IsSet(GpuAccessFlag::Read) && barrier.DestinationAccess.IsSet(GpuAccessFlag::Write))
		mBarrierBatch.AddExecutionBarrier(barrier);
}

void VulkanBarrierHelper::RecordSubresourceBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, const GpuHazardStageAndAccess& barrier, GpuImageLayout& oldLayout, GpuImageLayout newLayout)
{
	const VkImage imageHandle = static_cast<VulkanImage*>(image)->GetVulkanHandle();
	const VkImageLayout vkOldLayout = VulkanUtility::ToVkImageLayout(oldLayout);
	const VkImageLayout vkNewLayout = VulkanUtility::ToVkImageLayout(newLayout);
	const VkImageSubresourceRange vkSubresourceRange = VulkanUtility::ToVkImageSubresourceRange(subresourceRange);

	const bool hasMemoryDependency = barrier.SourceAccess.IsSet(GpuAccessFlag::Write);
	const bool hasLayoutTransition = oldLayout != newLayout;
	if(hasMemoryDependency || hasLayoutTransition)
	{
		const VkImageLayout effectiveOldLayout = mBarrierBatch.AddImageBarrier(imageHandle, vkSubresourceRange, barrier,
			vkOldLayout, vkNewLayout);
		oldLayout = VulkanUtility::ToGpuImageLayout(effectiveOldLayout);
	}
	else if(barrier.SourceAccess.IsSet(GpuAccessFlag::Read) && barrier.DestinationAccess.IsSet(GpuAccessFlag::Write))
		mBarrierBatch.AddExecutionBarrier(barrier);
}

void VulkanBarrierHelper::Execute(VulkanGpuCommandBuffer& commandBuffer)
{
	if(HasBarriers())
	{
		mBarrierBatch.Execute(commandBuffer.GetVulkanHandle());
		ApplyPostBarrierTracking();
	}

	// Apply read/write hazard registrations deferred while tracking resource accesses
	mResourceTracker->CommitPendingHazardRegistrations();

	Clear();
}

void VulkanBarrierHelper::Clear()
{
	mBarrierBatch.Clear();

	TGpuBarrierHelper<VulkanBarrierHelper>::Clear();
}

bool VulkanBarrierHelper::HasBarriers() const
{
	return mBarrierBatch.HasBarriers();
}
