//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuResourceTracker.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

using namespace b3d;
using namespace b3d::render;

GpuBufferTrackingState& GpuResourceTracker::GetOrCreateBufferTrackingState(IGpuBufferResource* buffer)
{
	auto insertResult = mBuffers.insert(std::make_pair(buffer, GpuBufferTrackingState()));
	if(insertResult.second) // New element
		buffer->NotifyBound();

	GpuBufferTrackingState& bufferTrackingState = insertResult.first->second;
	B3D_ASSERT(!bufferTrackingState.UseHandle.Used);

	return bufferTrackingState;
}

void GpuResourceTracker::TrackBufferUsage(IGpuBufferResource* buffer, GpuAccessFlags access, [[maybe_unused]] u32 dynamicOffset)
{
	if(buffer == nullptr)
		return;

	GetOrCreateBufferTrackingState(buffer).UseHandle.Flags |= access;

#if B3D_BUILD_TYPE_DEVELOPMENT
	TrackBufferSuballocation(buffer, dynamicOffset);
#endif
}

#if B3D_BUILD_TYPE_DEVELOPMENT
void GpuResourceTracker::TrackBufferSuballocation(IGpuBufferResource* buffer, u32 offset)
{
	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	const u32 suballocationIndex = buffer->GetSuballocationIndexForOffset(offset);

	// Track this suballocation (avoid duplicates if same suballocation bound multiple times)
	bool alreadyTracked = false;
	for(u32 existingIndex : bufferTrackingState.BoundSuballocationIndices)
	{
		if(existingIndex == suballocationIndex)
		{
			alreadyTracked = true;
			break;
		}
	}

	if(!alreadyTracked)
	{
		bufferTrackingState.BoundSuballocationIndices.Add(suballocationIndex);
		buffer->NotifySuballocationBound(suballocationIndex);
	}
}
#endif

void GpuResourceTracker::BeginRenderPass(TArrayView<const GpuRenderPassAttachmentUsage> attachments)
{
	if(!B3D_ENSURE(!mIsInRenderPass))
		return;

	mIsInRenderPass = true;
	for(const GpuRenderPassAttachmentUsage& attachment : attachments)
	{
		if(attachment.Image == nullptr)
			continue;

		mRenderPassAttachments.Add(attachment);
		TrackImageUsage(attachment.Image, attachment.Range, attachment.Access);
	}
}

void GpuResourceTracker::EndRenderPass()
{
	if(!B3D_ENSURE(mIsInRenderPass))
		return;

	mRenderPassAttachments.Clear();
	mIsInRenderPass = false;
}

bool GpuResourceTracker::TrackShaderImageUsage(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuAccessFlags access)
{
	if(image == nullptr)
		return true;

	const GpuTextureSubresourceRange range = GpuBackendUtility::ClampRange(subresourceRange, image->GetRange());
	const GpuRenderPassAttachmentUsage* const attachment = GpuBackendUtility::FindRenderPassAttachment(mRenderPassAttachments, image, range);
	if(attachment != nullptr && (attachment->Access != GpuAccessFlag::Read || access.IsSet(GpuAccessFlag::Write)))
	{
		B3D_LOG(Error, LogRenderBackend, "Framebuffer attachments sampled during a render pass must be marked read-only.");
		return false;
	}

	TrackImageUsage(image, range, access);
	return true;
}

void GpuResourceTracker::TrackImageUsage(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuAccessFlags access)
{
	if(image == nullptr)
		return;

	auto insertResult = mImages.insert(std::make_pair(image, GpuResourceUseHandle()));
	if(insertResult.second) // New element
		image->NotifyBound();

	GpuResourceUseHandle& useHandle = insertResult.first->second;
	B3D_ASSERT(!useHandle.Used);
	useHandle.Flags |= access;

	// Register any sub-resources
	const GpuTextureSubresourceRange range = GpuBackendUtility::ClampRange(subresourceRange, image->GetRange());
	if(image->IsFullRange(range))
	{
		TrackResourceUsage(image->GetFullRangeSubresource(), access);
		return;
	}

	const GpuTextureAspectFlags trackedAspects = range.AspectMask & image->GetRange().AspectMask;
	for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
	{
		if(!trackedAspects.IsSet(aspect))
			continue;

		for(u32 layerIndex = 0; layerIndex < range.ArrayLayerCount; layerIndex++)
		{
			for(u32 levelIndex = 0; levelIndex < range.MipLevelCount; levelIndex++)
			{
				const u32 layer = range.BaseArrayLayer + layerIndex;
				const u32 mipLevel = range.BaseMipLevel + levelIndex;

				TrackResourceUsage(image->GetSubresource(layer, mipLevel, aspect), access);
			}
		}
	}
}

void GpuResourceTracker::TrackResourceUsage(IGpuResource* resource, GpuAccessFlags access)
{
	auto insertResult = mResources.insert(std::make_pair(resource, GpuResourceUseHandle()));
	if(insertResult.second) // New element
		resource->NotifyBound();

	GpuResourceUseHandle& useHandle = insertResult.first->second;
	B3D_ASSERT(!useHandle.Used);
	useHandle.Flags |= access;
}

void GpuResourceTracker::TrackSwapChainUsage(IGpuSwapChainResource* swapChain)
{
	auto insertResult = mSwapChains.insert(std::make_pair(swapChain, GpuResourceUseHandle()));
	if(insertResult.second) // New element
		swapChain->NotifyBound();

	GpuResourceUseHandle& useHandle = insertResult.first->second;
	B3D_ASSERT(!useHandle.Used);
	useHandle.Flags |= GpuAccessFlag::Write;
}

const GpuBufferTrackingState* GpuResourceTracker::FindBufferTrackingState(IGpuBufferResource* buffer) const
{
	auto found = mBuffers.find(buffer);
	if(found != mBuffers.end())
		return &found->second;

	return nullptr;
}

const GpuResourceUseHandle* GpuResourceTracker::FindImageUseHandle(IGpuImageResource* image) const
{
	auto found = mImages.find(image);
	if(found != mImages.end())
		return &found->second;

	return nullptr;
}

void GpuResourceTracker::NotifyUsed(GpuQueueId queueId)
{
	auto fnNotifyUsed = [queueId](auto& resources)
	{
		for(auto& entry : resources)
		{
			GpuResourceUseHandle& useHandle = entry.second;
			B3D_ASSERT(!useHandle.Used);

			if(useHandle.Flags == GpuAccessFlag::None)
				continue;

			useHandle.Used = true;
			entry.first->NotifyUsed(queueId, useHandle.Flags);
		}
	};

	fnNotifyUsed(mResources);
	fnNotifyUsed(mImages);

	for(auto& entry : mBuffers)
	{
		GpuBufferTrackingState& trackingState = entry.second;
		GpuResourceUseHandle& useHandle = trackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		if(useHandle.Flags == GpuAccessFlag::None)
			continue;

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);

#if B3D_BUILD_TYPE_DEVELOPMENT
		for(u32 suballocationIndex : trackingState.BoundSuballocationIndices)
			entry.first->NotifySuballocationUsed(suballocationIndex);
#endif
	}

	for(auto& entry : mSwapChains)
	{
		GpuResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}
}

void GpuResourceTracker::NotifyDone(GpuQueueId queueId)
{
	auto fnNotifyDone = [queueId](auto& resources)
	{
		for(auto& entry : resources)
		{
			GpuResourceUseHandle& useHandle = entry.second;
			if(useHandle.Flags == GpuAccessFlag::None)
			{
				B3D_ASSERT(!useHandle.Used);
				entry.first->NotifyUnbound();
				continue;
			}

			B3D_ASSERT(useHandle.Used);
			entry.first->NotifyDone(queueId, useHandle.Flags);
		}
	};

	fnNotifyDone(mResources);
	fnNotifyDone(mImages);

	for(auto& entry : mBuffers)
	{
		GpuBufferTrackingState& trackingState = entry.second;
		GpuResourceUseHandle& useHandle = trackingState.UseHandle;
		if(useHandle.Flags == GpuAccessFlag::None)
		{
			B3D_ASSERT(!useHandle.Used);
			entry.first->NotifyUnbound();
			continue;
		}

		B3D_ASSERT(useHandle.Used);
#if B3D_BUILD_TYPE_DEVELOPMENT
		for(u32 suballocationIndex : trackingState.BoundSuballocationIndices)
			entry.first->NotifySuballocationDone(suballocationIndex);
#endif

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}

	// Must be done after images & framebuffer because swap chain does error checking if those were freed
	for(auto& entry : mSwapChains)
	{
		GpuResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(useHandle.Used);

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}
}

void GpuResourceTracker::NotifyUnbound()
{
	auto fnNotifyUnbound = [](auto& resources)
	{
		for(auto& entry : resources)
		{
			B3D_ASSERT(!entry.second.Used);
			entry.first->NotifyUnbound();
		}
	};

	fnNotifyUnbound(mResources);
	fnNotifyUnbound(mImages);

	for(auto& entry : mBuffers)
	{
		GpuBufferTrackingState& trackingState = entry.second;
		B3D_ASSERT(!trackingState.UseHandle.Used);

#if B3D_BUILD_TYPE_DEVELOPMENT
		for(u32 suballocationIndex : trackingState.BoundSuballocationIndices)
			entry.first->NotifySuballocationUnbound(suballocationIndex);
#endif

		entry.first->NotifyUnbound();
	}

	// Must be done after images & framebuffer because swap chain does error checking if those were freed
	fnNotifyUnbound(mSwapChains);
}

void GpuResourceTracker::Clear()
{
	mResources.clear();
	mImages.clear();
	mBuffers.clear();
	mSwapChains.clear();
	mRenderPassAttachments.Clear();
	mIsInRenderPass = false;
}
