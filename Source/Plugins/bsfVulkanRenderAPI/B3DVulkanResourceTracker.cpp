//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanResourceTracker.h"

#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanSwapChain.h"
#include "B3DVulkanUtility.h"
#include "Utility/B3DBitwise.h"

using namespace b3d;
using namespace b3d::render;

#if B3D_HAZARD_TRACKING
WriteHazardPipelineTracking::WriteHazardPipelineTracking()
{
	// Everything is safe to access by default
	std::fill(SafeAccess.begin(), SafeAccess.end(), kAllPipelines);
}

void WriteHazardPipelineTracking::ClearStageSafeAccess(VkPipelineStageFlags stages)
{
	while(stages != 0)
	{
		const u32 stageFlagIndex = Bitwise::LeastSignificantBit(stages);
		SafeAccess[stageFlagIndex] = 0;

		stages &= ~(1 << stageFlagIndex);
	}
}

void WriteHazardPipelineTracking::AddStageSafeAccess(VkPipelineStageFlags sourceStages, VkPipelineStageFlags destinationStages)
{
	while(sourceStages != 0)
	{
		const u32 stageFlagIndex = Bitwise::LeastSignificantBit(sourceStages);
		SafeAccess[stageFlagIndex] |= destinationStages;

		sourceStages &= ~(1 << stageFlagIndex);
	}
}

bool WriteHazardPipelineTracking::IsAccessSafe(VkPipelineStageFlags stages) const
{
	for(const auto& entry : SafeAccess)
	{
		if((entry & stages) != stages)
			return false;
	}

	return true;
}

void WriteHazardPipelineTracking::LogUnsafeAccess(VkPipelineStageFlags stages, GpuAccessFlags currentAccessType, GpuAccessFlags previousAccessType) const
{
	StringStream stream;
	for(u32 stageIndex = 0; stageIndex < (u32)SafeAccess.size(); stageIndex++)
	{
		const VkPipelineStageFlags& safeStages = SafeAccess[stageIndex];

		if((safeStages & stages) != stages)
		{
			stream << "A resource was previously " << (previousAccessType.IsSet(GpuAccessFlag::Write) ? "WRITTEN" : "READ") << " ";
			stream << "on stage [" << VulkanUtility::GetPipelineStageName((VkPipelineStageFlagBits)(1 << stageIndex)) << "], ";

			stream << "and it's now being accessed for ";
			stream << (currentAccessType.IsSet(GpuAccessFlag::Write) ? "WRITE" : "READ") << " on stage(s) [";

			VulkanUtility::GetPipelineStageNames(stages, stream);

			stream << "] without a barrier being issued. Issue a barrier with correct usage between those two accesses.";
		}
	}

	B3D_LOG(Warning, RenderBackend, "{0}", stream.str());
}
#endif

VulkanResourceTracker::VulkanResourceTracker(VulkanGpuCommandBuffer* commandBuffer)
	: mCommandBuffer(commandBuffer)
{
}

VulkanResourceTracker::BufferTrackingState& VulkanResourceTracker::GetOrCreateBufferTrackingState(VulkanBuffer* buffer)
{
	auto insertResult = mBuffers.insert(std::make_pair(buffer, BufferTrackingState()));
	if(insertResult.second) // New element
	{
		BufferTrackingState& bufferTrackingState = insertResult.first->second;
		bufferTrackingState.UseFlags = GpuResourceUseFlag::Undefined;

		bufferTrackingState.UseHandle.Used = false;
		bufferTrackingState.UseHandle.Flags = GpuAccessFlag::None;

#if B3D_HAZARD_TRACKING
		bufferTrackingState.WriteHazardTracking = mWriteHazardPool.Construct<WriteHazardTracking>();
#endif

		buffer->NotifyBound();

		return bufferTrackingState;
	}
	else // Existing element
	{
		BufferTrackingState& bufferTrackingState = insertResult.first->second;
		return bufferTrackingState;
	}
}

void VulkanResourceTracker::TrackBufferUsage(BufferTrackingState& bufferTrackingState, GpuResourceUseFlags useFlags, GpuAccessFlags access)
{
	B3D_ASSERT(!bufferTrackingState.UseHandle.Used);

	const VkAccessFlags accessMask = VulkanUtility::GetAccessMaskFromUsage(useFlags, access);
	const VkPipelineStageFlags stages = VulkanUtility::GetPipelineStageFlags(useFlags, accessMask);

#if B3D_HAZARD_TRACKING
	WriteHazardTracking* const writeHazardTracking = bufferTrackingState.WriteHazardTracking;

	// If this buffer has been previously written to prevent read-after-write and write-after-read hazards
	if(access.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		// Read-after-write (or write-after-write)
		if(writeHazardTracking->Access.IsSet(GpuAccessFlag::Write))
		{
			// Triggers if user did not issue a RAW memory barrier between a previous write and this usage (or did not specify all the relevant stages in the barrier)
			if(!writeHazardTracking->WriteAccessStages.IsAccessSafe(stages))
			{
				writeHazardTracking->WriteAccessStages.LogUnsafeAccess(stages, access, GpuAccessFlag::Write);
				B3D_ENSURE(false);
			}
		}
	}

	if(access.IsSet(GpuAccessFlag::Write))
	{
		// Write-after-read
		if(writeHazardTracking->Access.IsSet(GpuAccessFlag::Read))
		{
			// Triggers if user did not issue a WAR memory barrier between a previous write and this usage (or did not specify all the relevant stages in the barrier)
			if(!writeHazardTracking->ReadAccessStages.IsAccessSafe(stages))
			{
				writeHazardTracking->ReadAccessStages.LogUnsafeAccess(stages, GpuAccessFlag::Write, GpuAccessFlag::Read);
				B3D_ENSURE(false);
			}
		}
	}

	writeHazardTracking->Access |= access;

	if(access.IsSet(GpuAccessFlag::Read))
		writeHazardTracking->ReadAccessStages.ClearStageSafeAccess(stages);

	if(access.IsSet(GpuAccessFlag::Write))
		writeHazardTracking->WriteAccessStages.ClearStageSafeAccess(stages);
#endif

	bufferTrackingState.UseHandle.Flags |= access;
	bufferTrackingState.UseFlags |= useFlags;
}

void VulkanResourceTracker::TrackBufferUsage(VulkanBuffer* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags access)
{
	BufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	TrackBufferUsage(bufferTrackingState, useFlags, access);
}

void VulkanResourceTracker::UpdateWriteHazardTrackingAfterBarrier(VulkanBuffer* buffer, GpuAccessFlags sourceAccess, VkPipelineStageFlags sourceStages, GpuAccessFlags destinationAccess, VkPipelineStageFlags destinationStages)
{
	const bool isReadOrWriteAfterWrite = sourceAccess.IsSet(GpuAccessFlag::Write);
	const bool isWriteAfterRead = sourceAccess.IsSet(GpuAccessFlag::Read) && destinationAccess.IsSet(GpuAccessFlag::Write);

	BufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	WriteHazardTracking* const writeHazardTracking = bufferTrackingState.WriteHazardTracking;

	if(isReadOrWriteAfterWrite || isWriteAfterRead)
		writeHazardTracking->ReadAccessStages.AddStageSafeAccess(sourceStages, destinationStages);

	if(isReadOrWriteAfterWrite)
		writeHazardTracking->WriteAccessStages.AddStageSafeAccess(sourceStages, destinationStages);
}

void VulkanResourceTracker::NotifyUsed(GpuQueueId queueId)
{
	for(auto& entry : mResources)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		ImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		ResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}

	for(auto& entry : mBuffers)
	{
		ResourceUseHandle& useHandle = entry.second.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}

	for(auto& entry : mSwapChains)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}
}

void VulkanResourceTracker::NotifyDone(GpuQueueId queueId)
{
	for(auto& entry : mResources)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(useHandle.Used);

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		ImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		ResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		B3D_ASSERT(useHandle.Used);

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}

	for(auto& entry : mBuffers)
	{
		ResourceUseHandle& useHandle = entry.second.UseHandle;
		B3D_ASSERT(useHandle.Used);

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}

	// Must be done after images & framebuffer because swap chain does error checking if those were freed
	for(auto& entry : mSwapChains)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(useHandle.Used);

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}
}

void VulkanResourceTracker::NotifyUnbound()
{
	for(auto& entry : mResources)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		entry.first->NotifyUnbound();
	}

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		ImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		ResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		entry.first->NotifyUnbound();
	}

	for(auto& entry : mBuffers)
	{
		ResourceUseHandle& useHandle = entry.second.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		entry.first->NotifyUnbound();
	}

	// Must be done after images & framebuffer because swap chain does error checking if those were freed
	for(auto& entry : mSwapChains)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		entry.first->NotifyUnbound();
	}
}

void VulkanResourceTracker::Clear()
{
#if B3D_HAZARD_TRACKING
	for(auto& entry : mBuffers)
	{
		if(entry.second.WriteHazardTracking != nullptr)
			mWriteHazardPool.Destruct(entry.second.WriteHazardTracking);
	}

	for(auto& entry : mSubresourceTrackingState)
	{
		if(entry.WriteHazardTracking != nullptr)
			mWriteHazardPool.Destruct(entry.WriteHazardTracking);
	}
#endif

	mResources.clear();
	mImages.clear();
	mBuffers.clear();
	mSwapChains.clear();
	mImageTrackingState.clear();
	mSubresourceTrackingState.clear();
	mShaderBoundSubresourceInfos.clear();
}
