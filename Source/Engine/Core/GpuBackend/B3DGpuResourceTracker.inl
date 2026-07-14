//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//

// Template method definitions for TGpuResourceTracker. This file is not a translation unit of its own; it is included by
// the single backend source file that explicitly instantiates TGpuResourceTracker for its barrier-helper type, 
// after the concrete barrier helper, frame allocator and GpuBackendUtility headers.

namespace b3d
{
	namespace render
	{

template<class TBarrierHelper>
GpuBufferTrackingState& TGpuResourceTracker<TBarrierHelper>::GetOrCreateBufferTrackingState(IGpuBufferResource* buffer)
{
	auto insertResult = mBuffers.insert(std::make_pair(buffer, GpuBufferTrackingState()));
	if(insertResult.second) // New element
	{
		GpuBufferTrackingState& bufferTrackingState = insertResult.first->second;
		bufferTrackingState.UseFlags = GpuResourceUseFlag::Undefined;

		bufferTrackingState.UseHandle.Used = false;
		bufferTrackingState.UseHandle.Flags = GpuAccessFlag::None;
		bufferTrackingState.WriteHazardTracking = mHazardTrackingPool.Construct<GpuHazardTracking>();

		buffer->NotifyBound();

		return bufferTrackingState;
	}
	else // Existing element
	{
		GpuBufferTrackingState& bufferTrackingState = insertResult.first->second;
		return bufferTrackingState;
	}
}

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::TrackBufferUsage(IGpuBufferResource* buffer, GpuBufferTrackingState& bufferTrackingState, GpuResourceUseFlags useFlags, GpuAccessFlags access, TBarrierHelper& barrierHelper, u32 dynamicOffset)
{
	B3D_ASSERT(!bufferTrackingState.UseHandle.Used);

	const typename TBarrierHelper::BarrierTrackingInfo* barrierTrackingInfo = barrierHelper.AddBufferBarrier(buffer, bufferTrackingState, useFlags, access);

	const GpuStageFlags accessStageFlags = GpuBackendUtility::GetStageFlags(useFlags);
	GpuHazardTracking* const writeHazardTracking = bufferTrackingState.WriteHazardTracking;

#if B3D_VERIFY_BARRIERS
	// Make a copy as we need to apply the safe access from the barrier that was registered. We assume the caller will issue the barrier before using the buffer.
	GpuHazardTracking writeHazardTrackingCopy;
	writeHazardTrackingCopy.Access = writeHazardTracking->Access;
	writeHazardTrackingCopy.State = writeHazardTracking->State;

	if(barrierTrackingInfo != nullptr)
		writeHazardTrackingCopy.RegisterBarrier(barrierTrackingInfo->StageAndAccess);

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

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::TrackBufferUsage(IGpuBufferResource* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, u32 dynamicOffset)
{
	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	TrackBufferUsage(buffer, bufferTrackingState, useFlags, accessFlags, barrierHelper, dynamicOffset);
}

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::TrackImageUsage(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	B3D_ASSERT(!imageTrackingState.UseHandle.Used);
	imageTrackingState.UseHandle.Flags |= accessFlags;

	struct CallbackParameters
	{
		TGpuResourceTracker<TBarrierHelper>* Self;
		TBarrierHelper* BarrierHelper;
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
		TGpuResourceTracker<TBarrierHelper>* self = callbackParameters->Self;

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

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::TrackSubresourceUsage(IGpuImageResource* image, u32 globalSubresourceIndex, GpuImageLayout layout, GpuImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper)
{
	const bool isShaderUse = useFlags.IsSet(GpuResourceUseFlag::ShaderAccess);
	const bool isFramebufferUse = useFlags.IsSetAny(GpuResourceUseFlag::ColorAttachment | GpuResourceUseFlag::DepthStencilAttachment);
	const bool isTransferUse = useFlags.IsSetAny(GpuResourceUseFlag::Transfer);

	GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState[globalSubresourceIndex];
	if(subresourceTrackingState.Access == GpuAccessFlag::None) // New subresource
	{
		subresourceTrackingState.InitialLayout = layout;
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

	const typename TBarrierHelper::BarrierTrackingInfo* const barrierTrackingInfo = barrierHelper.AddSubresourceBarrier(image, subresourceTrackingState, useFlags, accessFlags, subresourceTrackingState.RequiredLayout);

	const GpuStageFlags accessStageFlags = GpuBackendUtility::GetStageFlags(useFlags);
	GpuHazardTracking* const writeHazardTracking = subresourceTrackingState.WriteHazardTracking;

#if B3D_VERIFY_BARRIERS
	// Make a copy as we need to apply the safe access from the barrier that was registered. We assume the caller will issue the barrier before using the image.
	GpuHazardTracking writeHazardTrackingCopy;
	writeHazardTrackingCopy.Access = writeHazardTracking->Access;
	writeHazardTrackingCopy.State = writeHazardTracking->State;

	if(barrierTrackingInfo != nullptr)
		writeHazardTrackingCopy.RegisterBarrier(barrierTrackingInfo->StageAndAccess);

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

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::TrackResourceUsage(IGpuResource* resource, GpuAccessFlags access)
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

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::TrackSwapChainUsage(IGpuSwapChainResource* swapChain)
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

template<class TBarrierHelper>
GpuImageTrackingState& TGpuResourceTracker<TBarrierHelper>::GetOrCreateImageTrackingState(IGpuImageResource* image)
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

template<class TBarrierHelper>
u32 TGpuResourceTracker<TBarrierHelper>::FindImageTrackingStateIndex(IGpuImageResource* image) const
{
	auto found = mImages.find(image);
	if(found == mImages.end())
		return ~0u;

	return found->second;
}

template<class TBarrierHelper>
const GpuImageTrackingState* TGpuResourceTracker<TBarrierHelper>::FindImageTrackingState(IGpuImageResource* image) const
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	if(imageTrackingIndex == ~0u)
		return nullptr;

	return &mImageTrackingState[imageTrackingIndex];
}

template<class TBarrierHelper>
const GpuImageTrackingState& TGpuResourceTracker<TBarrierHelper>::GetImageTrackingState(IGpuImageResource* image) const
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	B3D_ASSERT(imageTrackingIndex != ~0u);

	return mImageTrackingState[imageTrackingIndex];
}

template<class TBarrierHelper>
GpuImageTrackingState& TGpuResourceTracker<TBarrierHelper>::GetImageTrackingState(IGpuImageResource* image)
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	B3D_ASSERT(imageTrackingIndex != ~0u);

	return mImageTrackingState[imageTrackingIndex];
}

template<class TBarrierHelper>
TArrayView<const GpuImageSubresourceTrackingState> TGpuResourceTracker<TBarrierHelper>::GetSubresourceTrackingStatesForImage(IGpuImageResource* image) const
{
	const GpuImageTrackingState& imageTrackingState = GetImageTrackingState(image);
	if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
		return {};

	return TArrayView(&mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex], imageTrackingState.SubresourceInfoCount);
}

template<class TBarrierHelper>
TArrayView<GpuImageSubresourceTrackingState> TGpuResourceTracker<TBarrierHelper>::GetSubresourceTrackingStatesForImage(IGpuImageResource* image)
{
	GpuImageTrackingState& imageTrackingState = GetImageTrackingState(image);
	if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
		return {};

	return TArrayView(&mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex], imageTrackingState.SubresourceInfoCount);
}

template<class TBarrierHelper>
const GpuImageSubresourceTrackingState& TGpuResourceTracker<TBarrierHelper>::GetSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip) const
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

template<class TBarrierHelper>
const GpuImageSubresourceTrackingState* TGpuResourceTracker<TBarrierHelper>::FindSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip) const
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

template<class TBarrierHelper>
const GpuBufferTrackingState* TGpuResourceTracker<TBarrierHelper>::FindBufferTrackingState(IGpuBufferResource* buffer) const
{
	auto found = mBuffers.find(buffer);
	if(found != mBuffers.end())
		return &found->second;

	return nullptr;
}

template<class TBarrierHelper>
GpuImageSubresourceTrackingState& TGpuResourceTracker<TBarrierHelper>::GetSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip)
{
	// Delegate to 'const' version and re-cast
	return const_cast<GpuImageSubresourceTrackingState&>(const_cast<const TGpuResourceTracker*>(this)->GetSubresourceTrackingState(image, face, mip));
}

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::IterateAndCreateOverlappingImageSubresourceTrackingState(IGpuImageResource* image, GpuTextureSubresourceRange subresourceRange, void (*FnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	IterateAndCreateOverlappingImageSubresourceTrackingState(imageTrackingState, *image, subresourceRange, FnDoOnOverlappingSubresource, userData);
}

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::IterateAndCreateOverlappingImageSubresourceTrackingState(GpuImageTrackingState& imageTrackingState, const IGpuImageResource& image, GpuTextureSubresourceRange subresourceRange, void(*FnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData)
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

template<class TBarrierHelper>
u32 TGpuResourceTracker<TBarrierHelper>::AddSubresourceTrackingState(const GpuTextureSubresourceRange& range)
{
	mSubresourceTrackingState.push_back(GpuImageSubresourceTrackingState());

	GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState.back();
	subresourceTrackingState.CurrentLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.InitialLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.RequiredLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.RenderPassLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.Range = range;
	subresourceTrackingState.WriteHazardTracking = mHazardTrackingPool.Construct<GpuHazardTracking>();

	return (u32)mSubresourceTrackingState.size() - 1;
}

template<class TBarrierHelper>
u32 TGpuResourceTracker<TBarrierHelper>::CopySubresourceTrackingStateWithNewRange(u32 copyFromIndex, const GpuTextureSubresourceRange& newRange)
{
	GpuImageSubresourceTrackingState* const copyFromSubresource = &mSubresourceTrackingState[copyFromIndex];

	GpuImageSubresourceTrackingState subresourceCopy = *copyFromSubresource;
	subresourceCopy.Range = newRange;

	subresourceCopy.WriteHazardTracking = mHazardTrackingPool.Construct<GpuHazardTracking>();

	if(B3D_ENSURE(copyFromSubresource->WriteHazardTracking != nullptr))
		*subresourceCopy.WriteHazardTracking = *copyFromSubresource->WriteHazardTracking;

	const u32 newSubresourceIndex = (u32)mSubresourceTrackingState.size();
	if(copyFromSubresource->ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
		mRenderPassSubresources.insert(newSubresourceIndex);

	mSubresourceTrackingState.push_back(subresourceCopy);
	return (u32)mSubresourceTrackingState.size() - 1;
}

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::UpdateImageLayoutTrackingAfterBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuImageLayout oldLayout, GpuImageLayout newLayout)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	struct CallbackParameters
	{
		TGpuResourceTracker<TBarrierHelper>* Self;
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

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::CommitPendingHazardRegistrations()
{
	for(const PendingHazardRegistration& registration : mPendingHazardRegistrations)
	{
		registration.Tracking->RegisterStageAccess(registration.AccessStageFlags, registration.Access);
	}

	mPendingHazardRegistrations.clear();
}

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::UpdateWriteHazardTrackingAfterBarrier(IGpuBufferResource* buffer,
	const GpuHazardStageAndAccess& barrier)
{
	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	GpuHazardTracking* const writeHazardTracking = bufferTrackingState.WriteHazardTracking;

	writeHazardTracking->RegisterBarrier(barrier);
}

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::UpdateWriteHazardTrackingAfterBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, const GpuHazardStageAndAccess& barrier)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	struct CallbackParameters
	{
		TGpuResourceTracker<TBarrierHelper>* Self;
		GpuHazardStageAndAccess Barrier;
	};

	CallbackParameters callbackParameters = { this, barrier };

	IterateAndCreateOverlappingImageSubresourceTrackingState(imageTrackingState, *image, range, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* callbackParameters = (CallbackParameters*)userData;

		GpuImageSubresourceTrackingState& subresourceTrackingState = callbackParameters->Self->mSubresourceTrackingState[globalSubresourceIndex];
		GpuHazardTracking* const writeHazardTracking = subresourceTrackingState.WriteHazardTracking;

		writeHazardTracking->RegisterBarrier(callbackParameters->Barrier);

	}, &callbackParameters);
}

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::NotifyUsed(GpuQueueId queueId)
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

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::NotifyDone(GpuQueueId queueId)
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

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::NotifyUnbound()
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

template<class TBarrierHelper>
void TGpuResourceTracker<TBarrierHelper>::Clear()
{
	for(auto& entry : mBuffers)
	{
		if(entry.second.WriteHazardTracking != nullptr)
			mHazardTrackingPool.Destruct(entry.second.WriteHazardTracking);
	}

	for(auto& entry : mSubresourceTrackingState)
	{
		if(entry.WriteHazardTracking != nullptr)
			mHazardTrackingPool.Destruct(entry.WriteHazardTracking);
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

	} // namespace render
} // namespace b3d
