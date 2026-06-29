//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanResourceTracker.h"

#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanSwapChain.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanUtility.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DVulkanBarrierHelper.h"

using namespace b3d;
using namespace b3d::render;

GpuBufferTrackingState& VulkanResourceTracker::GetOrCreateBufferTrackingState(IGpuBufferResource* buffer)
{
	auto insertResult = mBuffers.insert(std::make_pair(buffer, GpuBufferTrackingState()));
	if(insertResult.second) // New element
	{
		GpuBufferTrackingState& bufferTrackingState = insertResult.first->second;
		bufferTrackingState.UseFlags = GpuResourceUseFlag::Undefined;

		bufferTrackingState.UseHandle.Used = false;
		bufferTrackingState.UseHandle.Flags = GpuAccessFlag::None;
		bufferTrackingState.WriteHazardTracking = mWriteHazardPool.Construct<GpuWriteHazardTracking>();

		buffer->NotifyBound();

		return bufferTrackingState;
	}
	else // Existing element
	{
		GpuBufferTrackingState& bufferTrackingState = insertResult.first->second;
		return bufferTrackingState;
	}
}

void VulkanResourceTracker::TrackBufferUsage(IGpuBufferResource* buffer, GpuBufferTrackingState& bufferTrackingState, GpuResourceUseFlags useFlags, GpuAccessFlags access, VulkanBarrierHelper& barrierHelper, u32 dynamicOffset)
{
	B3D_ASSERT(!bufferTrackingState.UseHandle.Used);

	const VulkanBarrierHelper::BarrierTrackingInfo* barrierTrackingInfo = barrierHelper.AddBufferBarrier(buffer, bufferTrackingState, useFlags, access);

	const GpuStageFlags accessStageFlags = GpuBackendUtility::GetStageFlags(useFlags);
	GpuWriteHazardTracking* const writeHazardTracking = bufferTrackingState.WriteHazardTracking;

#if B3D_VERIFY_BARRIERS
	// Make a copy as we need to apply the safe access from the barrier that was registered. We assume the caller will issue the barrier before using the buffer.
	GpuWriteHazardTracking writeHazardTrackingCopy = *writeHazardTracking;

	if(barrierTrackingInfo != nullptr)
		writeHazardTrackingCopy.AddSafeAccess(barrierTrackingInfo->SourceAccessStages, barrierTrackingInfo->SourceAccess, barrierTrackingInfo->DestinationAccessStages, barrierTrackingInfo->DestinationAccess);

	writeHazardTrackingCopy.VerifySafeAccess(accessStageFlags, access);
#endif

	writeHazardTracking->Access |= access;

	// Defer registering hazards until after the barrier is issued, as the barrier helper clears any hazards that have been set
	if(access.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
		mPendingHazardRegistrations.push_back({ writeHazardTracking, accessStageFlags, access });

	bufferTrackingState.UseHandle.Flags |= access;
	bufferTrackingState.UseFlags |= useFlags;

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Calculate suballocation index from dynamic offset and track it
	const u32 suballocationIndex = buffer->GetSuballocationIndexForOffset(dynamicOffset);

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
#endif
}

void VulkanResourceTracker::TrackBufferUsage(IGpuBufferResource* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, VulkanBarrierHelper& barrierHelper, u32 dynamicOffset)
{
	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	TrackBufferUsage(buffer, bufferTrackingState, useFlags, accessFlags, barrierHelper, dynamicOffset);
}

void VulkanResourceTracker::TrackImageUsage(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, VulkanBarrierHelper& barrierHelper)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	B3D_ASSERT(!imageTrackingState.UseHandle.Used);
	imageTrackingState.UseHandle.Flags |= accessFlags;

	struct CallbackParameters
	{
		VulkanResourceTracker* Self;
		VulkanBarrierHelper* BarrierHelper;
		IGpuImageResource* Image;
		GpuImageLayout Layout;
		GpuImageLayout FinalLayout;
		GpuResourceUseFlags UseFlags;
		GpuAccessFlags AccessFlags;
	};

	CallbackParameters callbackParameters { this, &barrierHelper, image, layout, finalLayout, useFlags, accessFlags };
	IterateAndCreateOverlappingImageSubresourceTrackingState(imageTrackingState, *image, subresourceRange, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* const callbackParameters = (CallbackParameters*)userData;
		VulkanResourceTracker* self = callbackParameters->Self;

		self->TrackSubresourceUsage(callbackParameters->Image, globalSubresourceIndex, callbackParameters->Layout, callbackParameters->FinalLayout, callbackParameters->UseFlags, callbackParameters->AccessFlags, *callbackParameters->BarrierHelper);
	}, &callbackParameters);

	// Register any sub-resources
	B3D_ASSERT(subresourceRange.ArrayLayerCount != ~0u);
	B3D_ASSERT(subresourceRange.MipLevelCount != ~0u);

	for(u32 layerIndex = 0; layerIndex < subresourceRange.ArrayLayerCount; layerIndex++)
	{
		for(u32 levelIndex = 0; levelIndex < subresourceRange.MipLevelCount; levelIndex++)
		{
			const u32 layer = subresourceRange.BaseArrayLayer + layerIndex;
			const u32 mipLevel = subresourceRange.BaseMipLevel + levelIndex;

			TrackResourceUsage(image->GetSubresource(layer, mipLevel), accessFlags);
		}
	}
}

void VulkanResourceTracker::TrackSubresourceUsage(IGpuImageResource* image, u32 globalSubresourceIndex, GpuImageLayout layout, GpuImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, VulkanBarrierHelper& barrierHelper)
{
	const bool isShaderUse = useFlags.IsSet(GpuResourceUseFlag::ShaderAccess);
	const bool isFramebufferUse = useFlags.IsSetAny(GpuResourceUseFlag::ColorAttachment | GpuResourceUseFlag::DepthStencilAttachment);
	const bool isTransferUse = useFlags.IsSetAny(GpuResourceUseFlag::Transfer);

	GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState[globalSubresourceIndex];
	if(subresourceTrackingState.Access == GpuAccessFlag::None) // New subresource
	{
		subresourceTrackingState.InitialLayout = layout;
		subresourceTrackingState.InitialReadOnly = !accessFlags.IsSet(GpuAccessFlag::Write);
		subresourceTrackingState.RenderPassLayout = finalLayout; // TODO - Handle this below
		subresourceTrackingState.CurrentLayout = layout; // TODO - Handle this below
		subresourceTrackingState.RequiredLayout = layout; // TODO - Handle this below
	}
	// TODO - Unify existing and new subresource paths
	else
	{
		// Determine required layout
		if(isShaderUse)
		{
			// Register the necessary layout transition, but only if the image isn't bound for framebuffer bind. If it is
			// then we are forced to use the layout that's expected by the framebuffer.
			if(subresourceTrackingState.FramebufferUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
			{
				// Currently the system doesn't support image being bound to framebuffer, yet being written to by the
				// shader. This seems like an unlikely scenario.
				B3D_ASSERT(!accessFlags.IsSet(GpuAccessFlag::Write));
			}
			else
			{
				// Check if the image had a layout previously assigned, and if so check if multiple different layouts
				// were requested. In that case we wish to transfer the image to GENERAL layout.

				const bool firstUseInRenderPass = !subresourceTrackingState.ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write)
					&& !subresourceTrackingState.FramebufferUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write);
				if(firstUseInRenderPass || subresourceTrackingState.RequiredLayout == GpuImageLayout::Undefined)
					subresourceTrackingState.RequiredLayout = layout;
				else if(subresourceTrackingState.RequiredLayout != layout)
					subresourceTrackingState.RequiredLayout = GpuImageLayout::General;
			}
		}
		else if(isFramebufferUse)
		{
			// Framebuffer expects a certain layout and we must respect it. In the case when the FB attachment is also bound
			// for shader reads, this will override the layout required for shader read (GENERAL or DEPTH_READ_ONLY), but that
			// is fine because those transitions are handled automatically by render-pass layout transitions.
			subresourceTrackingState.RequiredLayout = layout;
			subresourceTrackingState.RenderPassLayout = finalLayout;
		}
		else if(isTransferUse)
		{
			subresourceTrackingState.RequiredLayout = layout;
		}
	}

	const VulkanBarrierHelper::BarrierTrackingInfo* const barrierTrackingInfo = barrierHelper.AddSubresourceBarrier(image, subresourceTrackingState, useFlags, accessFlags, subresourceTrackingState.RequiredLayout);

	const GpuStageFlags accessStageFlags = GpuBackendUtility::GetStageFlags(useFlags);
	GpuWriteHazardTracking* const writeHazardTracking = subresourceTrackingState.WriteHazardTracking;

#if B3D_VERIFY_BARRIERS
	// Make a copy as we need to apply the safe access from the barrier that was registered. We assume the caller will issue the barrier before using the image.
	GpuWriteHazardTracking writeHazardTrackingCopy = *writeHazardTracking;

	if(barrierTrackingInfo != nullptr)
		writeHazardTrackingCopy.AddSafeAccess(barrierTrackingInfo->SourceAccessStages, barrierTrackingInfo->SourceAccess, barrierTrackingInfo->DestinationAccessStages, barrierTrackingInfo->DestinationAccess);

	writeHazardTrackingCopy.VerifySafeAccess(accessStageFlags, accessFlags);
#endif

	writeHazardTracking->Access |= accessFlags;

	// Defer registering hazards until after the barrier is issued, as the barrier helper clears any hazards that have been set
	if(accessFlags.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
		mPendingHazardRegistrations.push_back({ writeHazardTracking, accessStageFlags, accessFlags });

	subresourceTrackingState.Access |= accessFlags;

	if(isShaderUse)
	{
		subresourceTrackingState.ShaderUse |= accessFlags;
		mRenderPassSubresources.insert(globalSubresourceIndex);
	}
	else if(isFramebufferUse)
		subresourceTrackingState.FramebufferUse |= accessFlags;
}

void VulkanResourceTracker::TrackResourceUsage(IGpuResource* resource, GpuAccessFlags access)
{
	auto insertResult = mResources.insert(std::make_pair(resource, GpuResourceUseHandle()));
	if(insertResult.second) // New element
	{
		GpuResourceUseHandle& useHandle = insertResult.first->second;
		useHandle.Used = false;
		useHandle.Flags = access;

		resource->NotifyBound();
	}
	else // Existing element
	{
		GpuResourceUseHandle& useHandle = insertResult.first->second;

		B3D_ASSERT(!useHandle.Used);
		useHandle.Flags |= access;
	}
}

void VulkanResourceTracker::TrackFramebufferUsage(VulkanFramebuffer* framebuffer, RenderSurfaceMask loadMask, RenderSurfaceMask readOnlyMask, VulkanBarrierHelper& barrierHelper)
{
	auto insertResult = mResources.insert(std::make_pair(framebuffer, GpuResourceUseHandle()));
	if(insertResult.second) // New element
	{
		GpuResourceUseHandle& useHandle = insertResult.first->second;
		useHandle.Used = false;
		useHandle.Flags = GpuAccessFlag::Write;

		framebuffer->NotifyBound();
	}
	else // Existing element
	{
		GpuResourceUseHandle& useHandle = insertResult.first->second;

		B3D_ASSERT(!useHandle.Used);
		useHandle.Flags |= GpuAccessFlag::Write;
	}

	// Register any sub-resources
	VulkanRenderPass* renderPass = framebuffer->GetRenderPass();
	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& attachment = framebuffer->GetColorAttachment(colorAttachmentIndex);

		// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter. We're using
		// these values because that's what VulkanFramebuffer expects as initialLayout.
		GpuImageLayout layout;
		if(loadMask.IsSet((RenderSurfaceMaskBits)(1 << colorAttachmentIndex)))
			layout = GpuImageLayout::ColorAttachment;
		else
			layout = GpuImageLayout::Undefined;

		GpuAccessFlag access = readOnlyMask.IsSet((RenderSurfaceMaskBits)(1 << colorAttachmentIndex)) ? GpuAccessFlag::Read : GpuAccessFlag::Write;

		GpuTextureSubresourceRange range = attachment.Image->GetRange(attachment.Surface);
		TrackImageUsage(attachment.Image, range, layout, attachment.FinalLayout, GpuResourceUseFlag::ColorAttachment, access, barrierHelper);
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& attachment = framebuffer->GetDepthStencilAttachment();

		// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter. We're using
		// these values because that's what VulkanFramebuffer expects as initialLayout.
		GpuImageLayout layout;
		if(loadMask.IsSet(RT_DEPTH) || loadMask.IsSet(RT_STENCIL)) // Can't load one without the other
			layout = GpuImageLayout::DepthStencilAttachment;
		else
			layout = GpuImageLayout::Undefined;

		// Note: We purposefully don't check read-only stencil here as generally access tracking doesn't matter for it, as it's always an attachment and shader can't read/write it directly
		const GpuAccessFlag access = readOnlyMask.IsSet(RT_DEPTH) ? GpuAccessFlag::Read : GpuAccessFlag::Write;

		GpuTextureSubresourceRange range = attachment.Image->GetRange(attachment.Surface);
		TrackImageUsage(attachment.Image, range, layout, attachment.FinalLayout, GpuResourceUseFlag::DepthStencilAttachment, access, barrierHelper);
	}
}

void VulkanResourceTracker::TrackSwapChainUsage(IGpuSwapChainResource* swapChain)
{
	auto insertResult = mSwapChains.insert(std::make_pair(swapChain, GpuResourceUseHandle()));
	if(insertResult.second) // New element
	{
		GpuResourceUseHandle& useHandle = insertResult.first->second;
		useHandle.Used = false;
		useHandle.Flags = GpuAccessFlag::Write;

		swapChain->NotifyBound();
	}
	else // Existing element
	{
		GpuResourceUseHandle& useHandle = insertResult.first->second;

		B3D_ASSERT(!useHandle.Used);
		useHandle.Flags |= GpuAccessFlag::Write;
	}
}

GpuImageTrackingState& VulkanResourceTracker::GetOrCreateImageTrackingState(IGpuImageResource* image)
{
	const u32 nextImageTrackingIndex = (u32)mImageTrackingState.size();

	auto insertResult = mImages.insert(std::make_pair(image, nextImageTrackingIndex));
	if(insertResult.second) // New element
	{
		mImageTrackingState.push_back(GpuImageTrackingState());

		GpuImageTrackingState& imageTrackingState = mImageTrackingState[nextImageTrackingIndex];
		imageTrackingState.FirstSubresourceInfoIndex = ~0u;
		imageTrackingState.SubresourceInfoCount = 0;

		imageTrackingState.UseHandle.Used = false;
		imageTrackingState.UseHandle.Flags = GpuAccessFlag::None;

		image->NotifyBound();
		return imageTrackingState;
	}
	else // Existing element
	{
		const u32 imageTrackingIndex = insertResult.first->second;
		GpuImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

		B3D_ASSERT(!imageTrackingState.UseHandle.Used);
		return imageTrackingState;
	}
}

u32 VulkanResourceTracker::FindImageTrackingStateIndex(IGpuImageResource* image) const
{
	auto found = mImages.find(image);
	if(found == mImages.end())
		return ~0u;

	return found->second;
}

const GpuImageTrackingState* VulkanResourceTracker::FindImageTrackingState(IGpuImageResource* image) const
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	if(imageTrackingIndex == ~0u)
		return nullptr;

	return &mImageTrackingState[imageTrackingIndex];
}

const GpuImageTrackingState& VulkanResourceTracker::GetImageTrackingState(IGpuImageResource* image) const
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	B3D_ASSERT(imageTrackingIndex != ~0u);

	return mImageTrackingState[imageTrackingIndex];
}

GpuImageTrackingState& VulkanResourceTracker::GetImageTrackingState(IGpuImageResource* image)
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	B3D_ASSERT(imageTrackingIndex != ~0u);

	return mImageTrackingState[imageTrackingIndex];
}

TArrayView<const GpuImageSubresourceTrackingState> VulkanResourceTracker::GetSubresourceTrackingStatesForImage(IGpuImageResource* image) const
{
	const GpuImageTrackingState& imageTrackingState = GetImageTrackingState(image);
	if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
		return {};

	return TArrayView(&mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex], imageTrackingState.SubresourceInfoCount);
}

TArrayView<GpuImageSubresourceTrackingState> VulkanResourceTracker::GetSubresourceTrackingStatesForImage(IGpuImageResource* image)
{
	GpuImageTrackingState& imageTrackingState = GetImageTrackingState(image);
	if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
		return {};

	return TArrayView(&mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex], imageTrackingState.SubresourceInfoCount);
}

const GpuImageSubresourceTrackingState& VulkanResourceTracker::GetSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip) const
{
	const GpuImageSubresourceTrackingState* const trackingState = FindSubresourceTrackingState(image, face, mip);
	if(!B3D_ENSURE(trackingState != nullptr))
	{
		// Fallback to first subresource
		const u32 imageTrackingIndex = mImages.find(image)->second;
		const GpuImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

		const GpuImageSubresourceTrackingState* const subresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];
		return subresourceTrackingStates[0];
	}

	return *trackingState;
}

const GpuImageSubresourceTrackingState* VulkanResourceTracker::FindSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip) const
{
	const u32 imageTrackingIndex = mImages.find(image)->second;
	const GpuImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

	const GpuImageSubresourceTrackingState* const subresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];
	for(u32 localSubresourceIndex = 0; localSubresourceIndex < imageTrackingState.SubresourceInfoCount; localSubresourceIndex++)
	{
		const GpuImageSubresourceTrackingState& subresourceTrackingState = subresourceTrackingStates[localSubresourceIndex];

		if(face >= subresourceTrackingState.Range.BaseArrayLayer && face < (subresourceTrackingState.Range.BaseArrayLayer + subresourceTrackingState.Range.ArrayLayerCount) &&
		   mip >= subresourceTrackingState.Range.BaseMipLevel && mip < (subresourceTrackingState.Range.BaseMipLevel + subresourceTrackingState.Range.MipLevelCount))
		{
			return &subresourceTrackingState;
		}
	}

	return nullptr;
}

const GpuBufferTrackingState* VulkanResourceTracker::FindBufferTrackingState(IGpuBufferResource* buffer) const
{
	auto found = mBuffers.find(buffer);
	if(found != mBuffers.end())
		return &found->second;

	return nullptr;
}

GpuImageSubresourceTrackingState& VulkanResourceTracker::GetSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip)
{
	// Delegate to 'const' version and re-cast
	return const_cast<GpuImageSubresourceTrackingState&>(const_cast<const VulkanResourceTracker*>(this)->GetSubresourceTrackingState(image, face, mip));
}

void VulkanResourceTracker::IterateAndCreateOverlappingImageSubresourceTrackingState(IGpuImageResource* image, GpuTextureSubresourceRange subresourceRange, void (*FnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	IterateAndCreateOverlappingImageSubresourceTrackingState(imageTrackingState, *image, subresourceRange, FnDoOnOverlappingSubresource, userData);
}

void VulkanResourceTracker::IterateAndCreateOverlappingImageSubresourceTrackingState(GpuImageTrackingState& imageTrackingState, const IGpuImageResource& image, GpuTextureSubresourceRange subresourceRange, void(*FnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData)
{
	// Provide exact size as code below doesn't handle the "remaining" sentinel
	if(subresourceRange.ArrayLayerCount == ~0u)
		subresourceRange.ArrayLayerCount = image.GetRange().ArrayLayerCount;

	if(subresourceRange.MipLevelCount == ~0u)
		subresourceRange.MipLevelCount = image.GetRange().MipLevelCount;

	if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
	{
		const u32 subresourceIndex = AddSubresourceTrackingState(subresourceRange);
		imageTrackingState.FirstSubresourceInfoIndex = subresourceIndex;
		imageTrackingState.SubresourceInfoCount = 1;

		FnDoOnOverlappingSubresource(subresourceIndex, userData);
		return;
	}

	GpuImageSubresourceTrackingState* const existingSubresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];

	// First test for the simplest and most common case (same range or no overlap) to avoid more complex computations.
	bool foundRange = false;
	for(u32 subresourceLocalIndex = 0; subresourceLocalIndex < imageTrackingState.SubresourceInfoCount; subresourceLocalIndex++)
	{
		GpuImageSubresourceTrackingState& existingSubresourceTrackingState = existingSubresourceTrackingStates[subresourceLocalIndex];
		if(GpuBackendUtility::RangeOverlaps(existingSubresourceTrackingState.Range, subresourceRange))
		{
			if(existingSubresourceTrackingState.Range.ArrayLayerCount == subresourceRange.ArrayLayerCount &&
			   existingSubresourceTrackingState.Range.MipLevelCount == subresourceRange.MipLevelCount &&
			   existingSubresourceTrackingState.Range.BaseArrayLayer == subresourceRange.BaseArrayLayer &&
			   existingSubresourceTrackingState.Range.BaseMipLevel == subresourceRange.BaseMipLevel)
			{
				const u32 subresourceIndex = imageTrackingState.FirstSubresourceInfoIndex + subresourceLocalIndex;
				FnDoOnOverlappingSubresource(subresourceIndex, userData);
				return;
			}

			// This means there's a partial overlap which means there's no point searching further, we must subdivide
			break;
		}
	}

	// We'll need to update subresource ranges or add new ones. The hope is that this code is trigger VERY rarely
	// (for just a few specific textures per frame).
	if(!foundRange)
	{
		std::array<GpuTextureSubresourceRange, 5> cutRanges;

		B3DMarkAllocatorFrame();
		{
			// We orphan previously allocated memory (we reset after command buffer is done executing anyway)
			u32 newSubresourceTrackingStateIndex = (u32)mSubresourceTrackingState.size();

			FrameVector<u32> cutOverlappingRanges;
			for(u32 subresourceLocalIndex = 0; subresourceLocalIndex < imageTrackingState.SubresourceInfoCount; subresourceLocalIndex++)
			{
				const u32 globalSubresourceIndex = imageTrackingState.FirstSubresourceInfoIndex + subresourceLocalIndex;
				GpuImageSubresourceTrackingState& subresource = mSubresourceTrackingState[globalSubresourceIndex];

				if(!GpuBackendUtility::RangeOverlaps(subresource.Range, subresourceRange))
					CopySubresourceTrackingStateWithNewRange(globalSubresourceIndex, subresource.Range);
				else // Need to cut
				{
					u32 cutRangeCount;
					GpuBackendUtility::CutRange(subresource.Range, subresourceRange, cutRanges, cutRangeCount);

					for(u32 cutRangeIndex = 0; cutRangeIndex < cutRangeCount; cutRangeIndex++)
					{
						// Create a copy of the original subresource with the new range
						const u32 newGlobalSubresourceIndex = CopySubresourceTrackingStateWithNewRange(globalSubresourceIndex, cutRanges[cutRangeIndex]);

						if(GpuBackendUtility::RangeOverlaps(cutRanges[cutRangeIndex], subresourceRange))
						{
							FnDoOnOverlappingSubresource(newGlobalSubresourceIndex, userData);

							// Keep track of the overlapping ranges for later
							cutOverlappingRanges.push_back((u32)mSubresourceTrackingState.size() - 1);
						}
					}
				}
			}

			// Our range doesn't overlap with any existing ranges, so just add it
			if(cutOverlappingRanges.empty())
			{
				const u32 newGlobalSubresourceIndex = AddSubresourceTrackingState(subresourceRange);
				FnDoOnOverlappingSubresource(newGlobalSubresourceIndex, userData);
			}
			else // Search if overlapping ranges fully cover the requested range, and insert non-covered regions
			{
				FrameQueue<GpuTextureSubresourceRange> sourceRanges;
				sourceRanges.push(subresourceRange);

				for(auto& entry : cutOverlappingRanges)
				{
					GpuTextureSubresourceRange& overlappingRange = mSubresourceTrackingState[entry].Range;

					const u32 sourceRangeCount = (u32)sourceRanges.size();
					for(u32 sourceRangeIndex = 0; sourceRangeIndex < sourceRangeCount; sourceRangeIndex++)
					{
						GpuTextureSubresourceRange sourceRange = sourceRanges.front();
						sourceRanges.pop();

						u32 cutRangeCount;
						GpuBackendUtility::CutRange(sourceRange, overlappingRange, cutRanges, cutRangeCount);

						for(u32 cutRangeIndex = 0; cutRangeIndex < cutRangeCount; cutRangeIndex++)
						{
							// We only care about ranges outside of the ones we already covered
							if(!GpuBackendUtility::RangeOverlaps(cutRanges[cutRangeIndex], overlappingRange))
								sourceRanges.push(cutRanges[cutRangeIndex]);
						}
					}
				}

				// Any remaining range hasn't been covered yet
				while(!sourceRanges.empty())
				{
					AddSubresourceTrackingState(sourceRanges.front());
					sourceRanges.pop();
				}
			}

			imageTrackingState.FirstSubresourceInfoIndex = newSubresourceTrackingStateIndex;
			imageTrackingState.SubresourceInfoCount = (u32)mSubresourceTrackingState.size() - newSubresourceTrackingStateIndex;
		}
		B3DClearAllocatorFrame();
	}
}

u32 VulkanResourceTracker::AddSubresourceTrackingState(const GpuTextureSubresourceRange& range)
{
	mSubresourceTrackingState.push_back(GpuImageSubresourceTrackingState());

	GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState.back();
	subresourceTrackingState.CurrentLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.InitialLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.RequiredLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.RenderPassLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.Range = range;
	subresourceTrackingState.WriteHazardTracking = mWriteHazardPool.Construct<GpuWriteHazardTracking>();

	return (u32)mSubresourceTrackingState.size() - 1;
}

u32 VulkanResourceTracker::CopySubresourceTrackingStateWithNewRange(u32 copyFromIndex, const GpuTextureSubresourceRange& newRange)
{
	GpuImageSubresourceTrackingState* const copyFromSubresource = &mSubresourceTrackingState[copyFromIndex];

	GpuImageSubresourceTrackingState subresourceCopy = *copyFromSubresource;
	subresourceCopy.Range = newRange;

	subresourceCopy.WriteHazardTracking = mWriteHazardPool.Construct<GpuWriteHazardTracking>();

	if(B3D_ENSURE(copyFromSubresource->WriteHazardTracking != nullptr))
		*subresourceCopy.WriteHazardTracking = *copyFromSubresource->WriteHazardTracking;

	const u32 newSubresourceIndex = (u32)mSubresourceTrackingState.size();
	if(copyFromSubresource->ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
		mRenderPassSubresources.insert(newSubresourceIndex);

	mSubresourceTrackingState.push_back(subresourceCopy);
	return (u32)mSubresourceTrackingState.size() - 1;
}

void VulkanResourceTracker::UpdateImageLayoutTrackingAfterBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuImageLayout oldLayout, GpuImageLayout newLayout)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	struct CallbackParameters
	{
		VulkanResourceTracker* Self;
		GpuImageLayout OldLayout;
		GpuImageLayout NewLayout;
	};

	CallbackParameters callbackParameters = { this, oldLayout, newLayout };

	IterateAndCreateOverlappingImageSubresourceTrackingState(imageTrackingState, *image, range, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* callbackParameters = (CallbackParameters*)userData;

		GpuImageSubresourceTrackingState& subresourceTrackingState = callbackParameters->Self->mSubresourceTrackingState[globalSubresourceIndex];

		if(subresourceTrackingState.CurrentLayout != callbackParameters->OldLayout)
		{
			B3D_LOG(Warning, LogRenderBackend, "Image layout transition failed: current layout does not match expected old layout. "
				"Current layout: {0}, Expected old layout: {1}. The barrier's old layout must match the image's current layout.",
				GpuBackendUtility::GetImageLayoutName(subresourceTrackingState.CurrentLayout), GpuBackendUtility::GetImageLayoutName(callbackParameters->OldLayout));
		}

		B3D_ENSURE(subresourceTrackingState.CurrentLayout == callbackParameters->OldLayout);
		subresourceTrackingState.CurrentLayout = callbackParameters->NewLayout;
		subresourceTrackingState.RequiredLayout = callbackParameters->NewLayout; // TODO - RequiredLayout should no longer be necessary with explicit transitions
	}, &callbackParameters);
}

void VulkanResourceTracker::CommitPendingHazardRegistrations()
{
	for(const PendingHazardRegistration& registration : mPendingHazardRegistrations)
	{
		if(registration.Access.IsSet(GpuAccessFlag::Read))
			registration.Tracking->ExecutionBarrierTracking.ClearStageSafeAccess(registration.AccessStageFlags);

		if(registration.Access.IsSet(GpuAccessFlag::Write))
			registration.Tracking->MemoryBarrierTracking.ClearStageSafeAccess(registration.AccessStageFlags);
	}

	mPendingHazardRegistrations.clear();
}

void VulkanResourceTracker::UpdateWriteHazardTrackingAfterBarrier(IGpuBufferResource* buffer, GpuStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccess, GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess)
{
	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	GpuWriteHazardTracking* const writeHazardTracking = bufferTrackingState.WriteHazardTracking;

	writeHazardTracking->AddSafeAccess(sourceAccessStageFlags, sourceAccess, destinationAccessStageFlags, destinationAccess);
}

void VulkanResourceTracker::UpdateWriteHazardTrackingAfterBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccess, GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	struct CallbackParameters
	{
		VulkanResourceTracker* Self;
		GpuStageFlags SourceAccessStageFlags;
		GpuAccessFlags SourceAccess;
		GpuStageFlags DestinationAccessStageFlags;
		GpuAccessFlags DestinationAccess;
	};

	CallbackParameters callbackParameters = { this, sourceAccessStageFlags, sourceAccess, destinationAccessStageFlags, destinationAccess };

	IterateAndCreateOverlappingImageSubresourceTrackingState(imageTrackingState, *image, range, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* callbackParameters = (CallbackParameters*)userData;

		GpuImageSubresourceTrackingState& subresourceTrackingState = callbackParameters->Self->mSubresourceTrackingState[globalSubresourceIndex];
		GpuWriteHazardTracking* const writeHazardTracking = subresourceTrackingState.WriteHazardTracking;

		writeHazardTracking->AddSafeAccess(callbackParameters->SourceAccessStageFlags, callbackParameters->SourceAccess, callbackParameters->DestinationAccessStageFlags, callbackParameters->DestinationAccess);

	}, &callbackParameters);
}

void VulkanResourceTracker::NotifyUsed(GpuQueueId queueId)
{
	for(auto& entry : mResources)
	{
		GpuResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		GpuImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		GpuResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}

	for(auto& entry : mBuffers)
	{
		GpuBufferTrackingState& trackingState = entry.second;
		GpuResourceUseHandle& useHandle = trackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

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

void VulkanResourceTracker::NotifyDone(GpuQueueId queueId)
{
	for(auto& entry : mResources)
	{
		GpuResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(useHandle.Used);

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		GpuImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		GpuResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		B3D_ASSERT(useHandle.Used);

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}

	for(auto& entry : mBuffers)
	{
		GpuBufferTrackingState& trackingState = entry.second;
		GpuResourceUseHandle& useHandle = trackingState.UseHandle;
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

void VulkanResourceTracker::NotifyUnbound()
{
	for(auto& entry : mResources)
	{
		GpuResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		entry.first->NotifyUnbound();
	}

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		GpuImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		GpuResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		entry.first->NotifyUnbound();
	}

	for(auto& entry : mBuffers)
	{
		GpuBufferTrackingState& trackingState = entry.second;
		GpuResourceUseHandle& useHandle = trackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

#if B3D_BUILD_TYPE_DEVELOPMENT
		for(u32 suballocationIndex : trackingState.BoundSuballocationIndices)
			entry.first->NotifySuballocationUnbound(suballocationIndex);
#endif

		entry.first->NotifyUnbound();
	}

	// Must be done after images & framebuffer because swap chain does error checking if those were freed
	for(auto& entry : mSwapChains)
	{
		GpuResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		entry.first->NotifyUnbound();
	}
}

void VulkanResourceTracker::Clear()
{
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

	// Drop deferred registrations before destructing the WriteHazardTracking objects they point at.
	mPendingHazardRegistrations.clear();

	mResources.clear();
	mImages.clear();
	mBuffers.clear();
	mSwapChains.clear();
	mImageTrackingState.clear();
	mSubresourceTrackingState.clear();
	mRenderPassSubresources.clear();
}

void VulkanResourceTracker::ClearFramebufferFlagsForImage(VulkanImage* image)
{
	const u32 imageTrackingIndex = mImages[image];
	GpuImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

	GpuImageSubresourceTrackingState* const subresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];
	for(u32 localSubresourceIndex = 0; localSubresourceIndex < imageTrackingState.SubresourceInfoCount; localSubresourceIndex++)
	{
		GpuImageSubresourceTrackingState& subresourceTrackingState = subresourceTrackingStates[localSubresourceIndex];
		subresourceTrackingState.FramebufferUse = GpuAccessFlag::None;
	}
}

void VulkanResourceTracker::ClearShaderFlagsForAllRenderPassImageSubresources()
{
	for(const auto& subresourceIndex : mRenderPassSubresources)
	{
		GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState[subresourceIndex];
		subresourceTrackingState.ShaderUse = GpuAccessFlag::None;
	}

	mRenderPassSubresources.clear();
}

GpuImageLayout VulkanResourceTracker::GetCurrentSubresourceLayout(VulkanImage* image, const GpuTextureSubresourceRange& range, VulkanFramebuffer* framebuffer, RenderSurfaceMask explicitReadOnlyMask) const
{
	const u32 face = range.BaseArrayLayer;
	const u32 mip = range.BaseMipLevel;

#if B3D_BUILD_TYPE_DEVELOPMENT
	const GpuImageTrackingState* const imageTrackingState = FindImageTrackingState(image);
	if(imageTrackingState == nullptr)
	{
		B3D_ASSERT(false);
		return GpuImageLayout::Undefined;
	}
#endif

	VulkanRenderPass* renderPass = nullptr;
	if(framebuffer != nullptr)
		renderPass = framebuffer->GetRenderPass();

	TArrayView<const GpuImageSubresourceTrackingState> subresourceTrackingStates = GetSubresourceTrackingStatesForImage(image);
	for(const auto& subresourceTrackingState : subresourceTrackingStates)
	{
		if(face >= subresourceTrackingState.Range.BaseArrayLayer && face < (subresourceTrackingState.Range.BaseArrayLayer + subresourceTrackingState.Range.ArrayLayerCount) &&
		   mip >= subresourceTrackingState.Range.BaseMipLevel && mip < (subresourceTrackingState.Range.BaseMipLevel + subresourceTrackingState.Range.MipLevelCount))
		{
			// If it's a FB attachment, retrieve its layout after the render pass begins
			if(subresourceTrackingState.FramebufferUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write) && framebuffer != nullptr)
			{
				RenderSurfaceMask readMask = GetFramebufferReadOnlyMask(framebuffer, explicitReadOnlyMask);

				// Is it a depth-stencil attachment?
				if(renderPass->HasDepthAttachment() && framebuffer->GetDepthStencilAttachment().Image == image)
				{
					if(readMask.IsSet(RT_DEPTH))
					{
						if(readMask.IsSet(RT_STENCIL))
							return GpuImageLayout::DepthStencilReadOnly;
						else // Depth readable but stencil isn't
							return GpuImageLayout::DepthReadOnlyStencilAttachment;
					}
					else
					{
						if(readMask.IsSet(RT_STENCIL)) // Stencil readable but depth isn't
							return GpuImageLayout::DepthAttachmentStencilReadOnly;
						else
							return GpuImageLayout::DepthStencilAttachment;
					}
				}
				else // It is a color attachment
				{
					const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
					for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
					{
						const VulkanFramebufferAttachment& attachment = framebuffer->GetColorAttachment(colorAttachmentIndex);

						if(attachment.Image == image)
						{
							if(readMask.IsSet((RenderSurfaceMaskBits)(1 << attachment.Index)))
								return GpuImageLayout::General;
							else
								return GpuImageLayout::ColorAttachment;
						}
					}
				}
			}

			return subresourceTrackingState.RequiredLayout;
		}
	}

	B3D_ASSERT(false);
	return GpuImageLayout::Undefined;
}

RenderSurfaceMask VulkanResourceTracker::GetFramebufferReadOnlyMask(VulkanFramebuffer* framebuffer, RenderSurfaceMask explicitReadOnlyMask) const
{
	// Check if any frame-buffer attachments are also used as shader inputs, in which case we make them read-only
	VulkanRenderPass* const renderPass = framebuffer->GetRenderPass();
	RenderSurfaceMask readMask = RT_NONE;

	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetColorAttachment(colorAttachmentIndex);
		const GpuImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		const bool readOnly = subresourceTrackingState.ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write); // Note: Should report error if shader write is used

		if(readOnly)
			readMask.Set((RenderSurfaceMaskBits)(1 << colorAttachmentIndex));
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetDepthStencilAttachment();
		const GpuImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		const bool readOnly = subresourceTrackingState.ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write); // Note: Should report error if shader write is used

		if(readOnly)
			readMask.Set(RT_DEPTH);

		if(explicitReadOnlyMask.IsSet(RT_DEPTH))
			readMask.Set(RT_DEPTH);

		if(explicitReadOnlyMask.IsSet(RT_STENCIL))
			readMask.Set(RT_STENCIL);
	}

	return readMask;
}

void VulkanResourceTracker::MoveAllFramebufferAttachmentsToFinalLayouts(VulkanFramebuffer* framebuffer)
{
	const VulkanRenderPass* const renderPass = framebuffer->GetRenderPass();
	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetColorAttachment(colorAttachmentIndex);
		GpuImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		subresourceTrackingState.CurrentLayout = subresourceTrackingState.RenderPassLayout;
		subresourceTrackingState.RequiredLayout = subresourceTrackingState.RenderPassLayout;
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetDepthStencilAttachment();
		GpuImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		subresourceTrackingState.CurrentLayout = subresourceTrackingState.RenderPassLayout;
		subresourceTrackingState.RequiredLayout = subresourceTrackingState.RenderPassLayout;
	}
}
