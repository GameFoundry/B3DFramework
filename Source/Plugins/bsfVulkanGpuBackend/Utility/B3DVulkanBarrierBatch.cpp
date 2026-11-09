//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanBarrierBatch.h"
#include "B3DVulkanUtility.h"

using namespace b3d;
using namespace b3d::render;

VulkanBarrierBatch::VulkanBarrierBatch(const VulkanGpuDevice& device)
	: mDevice(device)
{
	Clear();
}

void VulkanBarrierBatch::AddBufferBarrier(VkBuffer buffer, const GpuBarrierScope& barrier, u32 sourceQueueFamily, u32 destinationQueueFamily, VkDeviceSize offset, VkDeviceSize size)
{
	VkPipelineStageFlags sourceStages, destinationStages;
	VkAccessFlags sourceAccess, destinationAccess;

	// Only writes require Vulkan memory availability. Source reads are ordered through the source stage mask.
	const GpuAccessFlags sourceAvailabilityAccess = barrier.SourceAccess.IsSet(GpuAccessFlag::Write) ?  GpuAccessFlags(GpuAccessFlag::Write) : GpuAccessFlags(GpuAccessFlag::None);
	VulkanUtility::GetPipelineStageAndAccessMask(mDevice, barrier.SourceStages, sourceAvailabilityAccess, sourceStages, sourceAccess);
	VulkanUtility::GetPipelineStageAndAccessMask(mDevice, barrier.DestinationStages, barrier.DestinationAccess, destinationStages, destinationAccess);

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
	VulkanUtility::GetPipelineStageAndAccessMask(mDevice, barrier.SourceStages, sourceAvailabilityAccess, sourceStages, sourceAccess);
	VulkanUtility::GetPipelineStageAndAccessMask(mDevice, barrier.DestinationStages, barrier.DestinationAccess, destinationStages, destinationAccess);

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
	VulkanUtility::GetPipelineStageAndAccessMask(mDevice, barrier.SourceStages, barrier.SourceAccess, sourceStages, unusedAccess);
	VulkanUtility::GetPipelineStageAndAccessMask(mDevice, barrier.DestinationStages, barrier.DestinationAccess, destinationStages, unusedAccess);

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
	VulkanUtility::GetPipelineStageAndAccessMask(mDevice, barrier.SourceStages, sourceAvailabilityAccess, sourceStages, sourceAccess);
	VulkanUtility::GetPipelineStageAndAccessMask(mDevice, barrier.DestinationStages, barrier.DestinationAccess, destinationStages, destinationAccess);

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

template<class TRecord>
void VulkanBarrierBatch::RecordDependencyInformation(TRecord fnRecord) const
{
	// Like Execute(), every barrier uses the stages of the whole batch. The stage and access bits of synchronization 1 have the same
	// values in synchronization2.
	// Events reject the host stage in the source. Host accesses before the submission need no ordering: the submission makes host writes
	// visible, and the host reads a resource before the submission that next writes it, such as a readback buffer at rest.
	constexpr VkAccessFlags2 kHostAccess = VK_ACCESS_2_HOST_READ_BIT | VK_ACCESS_2_HOST_WRITE_BIT;
	const VkPipelineStageFlags2 sourceStages = mCombinedSourceStages & ~(VkPipelineStageFlags2)VK_PIPELINE_STAGE_2_HOST_BIT;
	const VkPipelineStageFlags2 destinationStages = mCombinedDestinationStages;

	// An execution barrier is a memory barrier without accesses
	VkMemoryBarrier2 memoryBarrier{};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
	memoryBarrier.srcStageMask = sourceStages;
	memoryBarrier.srcAccessMask = mMemoryBarrier.srcAccessMask & ~kHostAccess;
	memoryBarrier.dstStageMask = destinationStages;
	memoryBarrier.dstAccessMask = mMemoryBarrier.dstAccessMask;

	TInlineArray<VkBufferMemoryBarrier2, 2> bufferBarriers;
	for(const VkBufferMemoryBarrier& barrier : mOwnershipBufferBarriers)
	{
		VkBufferMemoryBarrier2 output{};
		output.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		output.srcStageMask = sourceStages;
		output.srcAccessMask = barrier.srcAccessMask & ~kHostAccess;
		output.dstStageMask = destinationStages;
		output.dstAccessMask = barrier.dstAccessMask;
		output.srcQueueFamilyIndex = barrier.srcQueueFamilyIndex;
		output.dstQueueFamilyIndex = barrier.dstQueueFamilyIndex;
		output.buffer = barrier.buffer;
		output.offset = barrier.offset;
		output.size = barrier.size;

		bufferBarriers.Add(output);
	}

	TInlineArray<VkImageMemoryBarrier2, 4> imageBarriers;
	for(const VkImageMemoryBarrier& barrier : mImageBarriers)
	{
		VkImageMemoryBarrier2 output{};
		output.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		output.srcStageMask = sourceStages;
		output.srcAccessMask = barrier.srcAccessMask & ~kHostAccess;
		output.dstStageMask = destinationStages;
		output.dstAccessMask = barrier.dstAccessMask;
		output.oldLayout = barrier.oldLayout;
		output.newLayout = barrier.newLayout;
		output.srcQueueFamilyIndex = barrier.srcQueueFamilyIndex;
		output.dstQueueFamilyIndex = barrier.dstQueueFamilyIndex;
		output.image = barrier.image;
		output.subresourceRange = barrier.subresourceRange;

		imageBarriers.Add(output);
	}

	const bool hasMemoryBarrier = mHasMemoryBarrier || mHasExecutionBarrier;

	VkDependencyInfo dependencyInformation{};
	dependencyInformation.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInformation.memoryBarrierCount = hasMemoryBarrier ? 1u : 0u;
	dependencyInformation.pMemoryBarriers = hasMemoryBarrier ? &memoryBarrier : nullptr;
	dependencyInformation.bufferMemoryBarrierCount = (u32)bufferBarriers.Size();
	dependencyInformation.pBufferMemoryBarriers = bufferBarriers.Data();
	dependencyInformation.imageMemoryBarrierCount = (u32)imageBarriers.Size();
	dependencyInformation.pImageMemoryBarriers = imageBarriers.Data();

	fnRecord(dependencyInformation);
}

void VulkanBarrierBatch::SetEvent(VkCommandBuffer commandBuffer, VkEvent event) const
{
	RecordDependencyInformation([commandBuffer, event](const VkDependencyInfo& dependencyInformation)
	{
		vkCmdSetEvent2(commandBuffer, event, &dependencyInformation);
	});
}

void VulkanBarrierBatch::WaitEvent(VkCommandBuffer commandBuffer, VkEvent event) const
{
	RecordDependencyInformation([commandBuffer, event](const VkDependencyInfo& dependencyInformation)
	{
		vkCmdWaitEvents2(commandBuffer, 1, &event, &dependencyInformation);
	});
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
