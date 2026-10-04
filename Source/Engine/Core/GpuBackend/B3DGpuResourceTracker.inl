//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//

#include "GpuBackend/B3DGpuBackendUtility.h"

// Template method definitions for TGpuResourceTracker. This file is not a translation unit of its own; include it after
// the concrete barrier helper and frame allocator headers before instantiating a tracker type.

namespace b3d
{
	namespace render
	{

template<class TDerived, class TBarrierHelper>
TDerived& TGpuResourceTracker<TDerived, TBarrierHelper>::GetDerived()
{
	return static_cast<TDerived&>(*this);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::ResolveSubmissionTransitions(GpuQueueId destinationQueueId, u32 frameIndex, GpuSubmissionTransitionVisitor& visitor)
{
	// Tracks resting reads in the submission state if the resource is at rest, so they need no transition. Returns false if the reads need a GpuSubmissionTransition::Build().
	auto fnTryTrackRestingRead = [destinationQueueId, frameIndex](GpuResourceSubmissionState& state, GpuStageFlags readStages)
	{
		// Clear state from last frame
		if(state.FrameIndex != frameIndex)
		{
			state.Clear();
			state.FrameIndex = frameIndex;
		}

		if(state.HasWriter)
			return false;

		// The read needs no synchronization. Track it, so later writes in this frame order after it.
		state.ReaderQueues |= destinationQueueId;
		state.ReaderStages |= readStages;
		return true;
	};

	// If a resource was alias acquired on the destination command buffer, we ignore its prior submission state and use a new one
	const GpuResourceSubmissionState newLifetimeSubmissionState;

	// Builds and visits the submission transition of an image range, then stores the post-submission state in @p stateResource
	auto fnVisitImageTransition = [destinationQueueId, frameIndex, &visitor, &newLifetimeSubmissionState](IGpuImageResource& image, const GpuTextureSubresourceRange& range, GpuImageSubresource& stateResource,
		GpuImageLayout initialLayout, GpuImageLayout finalLayout, GpuBarrierFlags barrierFlags, GpuQueueMask inFlightReadQueues, const GpuResourceHazardState& hazards, bool transitionsLayout)
	{
		// An alias acquire starts a new lifetime and command buffer already issued an inline barrier to sync with previous memory access.
		const bool aliasAcquired = barrierFlags.IsSet(GpuBarrierFlag::AliasAcquire);
		const GpuResourceSubmissionState& sourceState = aliasAcquired ? newLifetimeSubmissionState : stateResource.SubmissionState;

		GpuSubmissionImageTransition transition(image, range, stateResource.NativeState, initialLayout, finalLayout, barrierFlags,
			GpuSubmissionTransition::Build(sourceState, frameIndex, aliasAcquired ? GpuQueueMask::kNone : inFlightReadQueues, destinationQueueId, hazards, transitionsLayout));

		const u32 committedLayout = stateResource.NativeState.Layout;
		visitor.VisitImage(transition);

		// A changed native layout means the submission transitioned the image, which later accesses must order after like a write.
		if constexpr(TDerived::kLayoutTransitionsAreWrites)
		{
			if(stateResource.NativeState.Layout != committedLayout && !hazards.HasWrite())
			{
				// Submission does not track the stages of the transition, so it is a write from every stage of the queue, visible nowhere yet
				GpuResourceSubmissionState& postState = transition.PostTransitionSubmissionState;
				postState = GpuResourceSubmissionState();
				postState.WriterHazards = hazards.LastWriteEpochHazardState;
				postState.WriterHazards.WriteStages = GpuStageFlags(GpuStageFlag::All) & GpuBackendUtility::GetQueueStageFlags(destinationQueueId.GetType());
				postState.WriterQueueId = destinationQueueId;
				postState.AcquiredQueues = destinationQueueId;
				postState.HasWriter = true;
				postState.FrameIndex = frameIndex;
			}
		}

		stateResource.SubmissionState = std::move(transition.PostTransitionSubmissionState);
	};

	// Selects the hazards of a tracked image partition for one submitted range, and visits the range's submission transition
	auto fnResolveTrackedImageTransition = [this, &fnVisitImageTransition](IGpuImageResource* image, const GpuImageSubresourceTrackingState& trackingState, const GpuTextureSubresourceRange& range,
		GpuImageSubresource& stateResource, GpuQueueMask inFlightReadQueues)
	{
		const GpuResourceHazardState& hazards = GetDerived().ResolveImageSubmissionHazards(image, trackingState, stateResource);
		if(!hazards.HasSubmissionEffect())
			return;

		// Selected meta-data work contributes to both subresource and parent-image use flags.
		if(trackingState.MetadataState != nullptr && hazards.HasAccess())
		{
			GpuAccessFlags access;
			if(hazards.AllAccessScope.ReadStages != GpuStageFlag::None)
				access |= GpuAccessFlag::Read;

			if(hazards.HasWrite())
				access |= GpuAccessFlag::Write;

			TrackResourceUsage(&stateResource, access);
			GetImageTrackingState(image).UseHandle.Flags |= access;
		}

		fnVisitImageTransition(*image, range, stateResource, trackingState.InitialLayout, trackingState.CurrentLayout, trackingState.SubmissionBarrierFlags, inFlightReadQueues, hazards,
			TDerived::kLayoutTransitionsAreWrites && trackingState.TransitionsLayout);
	};

	for(const auto& entry : mBuffers)
	{
		IGpuBufferResource* const buffer = entry.first;
		const GpuBufferTrackingState& trackingState = entry.second;

		// The first access records the acquire's barrier. Without one, the next command buffer would use incorrect hazard tracking as its not aware of the pending alias acquire. We decided not to handle that case.
		B3D_ENSURE_LOG(!trackingState.IsAliasAcquirePending(), "An alias acquired GPU buffer must be accessed on the command buffer that acquires it.");

		const GpuResourceHazardState* hazards = trackingState.HazardState;
		GpuResourceHazardState restingReadHazardState;
		if(trackingState.HasOnlyRestingReads())
		{
			// At rest, the reads need no transition
			if(fnTryTrackRestingRead(buffer->GetSubmissionState(), trackingState.UseHandle.Stages))
				continue;

			// Resting -> tracked
			restingReadHazardState.RecordAccess(trackingState.UseHandle.Stages, GpuAccessFlag::Read);
			hazards = &restingReadHazardState;
		}
		else if(hazards == nullptr || !hazards->HasSubmissionEffect())
			continue;

		// An alias acquire starts a new lifetime (see the image case)
		const bool aliasAcquired = trackingState.SubmissionBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire);
		const GpuResourceSubmissionState& sourceState = aliasAcquired ? newLifetimeSubmissionState : buffer->GetSubmissionState();
		const GpuQueueMask inFlightReadQueues = aliasAcquired ? GpuQueueMask::kNone : buffer->GetUseInfo(GpuAccessFlag::Read);

		GpuSubmissionBufferTransition transition(*buffer, trackingState.SubmissionBarrierFlags, GpuSubmissionTransition::Build(sourceState, frameIndex, inFlightReadQueues, destinationQueueId, *hazards));
		visitor.VisitBuffer(transition);

		buffer->SetSubmissionState(std::move(transition.PostTransitionSubmissionState));
	}

	for(const auto& entry : mImages)
	{
		IGpuImageResource* const image = entry.first;
		const GpuImageTrackingState& imageTrackingState = mImageTrackingState[entry.second];

		// Resting case
		if(imageTrackingState.HasOnlyRestingReads())
		{
			const GpuTextureSubresourceRange& range = imageTrackingState.Range;
			const GpuStageFlags readStages = imageTrackingState.UseHandle.Stages;

			GpuImageSubresource& fullRangeSubresource = *image->GetFullRangeSubresource();
			if(image->HasUniformSubmissionState())
			{
				// Outside the resting layout, the reads need a transition
				if(fullRangeSubresource.NativeState.Layout == TDerived::kRestingNativeLayout && fnTryTrackRestingRead(fullRangeSubresource.SubmissionState, readStages))
					continue;
			}

			// Convert resting -> tracked state
			GpuResourceHazardState restingReadHazardState;
			restingReadHazardState.RecordAccess(readStages, GpuAccessFlag::Read);

			// Transitions below bypass ResolveImageSubmissionHazards(): images that can rest have no meta-data to resolve

			// In the uniform case, avoid splitting if the range covers the full image.
			if(image->HasUniformSubmissionState())
			{
				if(image->IsFullRange(range))
				{
					fnVisitImageTransition(*image, image->GetRange(), fullRangeSubresource, GpuImageLayout::ShaderReadOnly, GpuImageLayout::ShaderReadOnly, GpuBarrierFlag::None, image->GetUseInfo(GpuAccessFlag::Read), restingReadHazardState, false);
					continue;
				}

				// Split submission state as we'll need to track subresources individually
				image->SplitSubmissionState();
			}

			// A split image does not rest. Synchronize the surfaces that were read, then return to one state if they now share it.
			B3D_ASSERT(range.HasSingleAspect());

			const GpuQueueMask fullRangeReadQueues = fullRangeSubresource.GetUseInfo(GpuAccessFlag::Read);
			const u32 mipEnd = range.BaseMipLevel + range.MipLevelCount;
			const u32 faceEnd = range.BaseArrayLayer + range.ArrayLayerCount;
			for(u32 mipLevel = range.BaseMipLevel; mipLevel < mipEnd; ++mipLevel)
			{
				for(u32 face = range.BaseArrayLayer; face < faceEnd; ++face)
				{
					GpuImageSubresource& subresource = *image->GetSubresource(face, mipLevel, (GpuTextureAspectFlag)(u32)range.AspectMask);

					const GpuQueueMask inFlightReadQueues = subresource.GetUseInfo(GpuAccessFlag::Read) | fullRangeReadQueues;
					fnVisitImageTransition(*image, GpuTextureSubresourceRange(mipLevel, 1, face, 1, range.AspectMask), subresource, GpuImageLayout::ShaderReadOnly, GpuImageLayout::ShaderReadOnly, GpuBarrierFlag::None, inFlightReadQueues, restingReadHazardState, false);
				}
			}

			image->TryMergeSubmissionState(frameIndex);
			continue;
		}

		// Non-resting (tracked) case
		const TArrayView<const GpuImageSubresourceTrackingState> trackingStates = GetSubresourceTrackingStatesForImage(image);

		// The first access records the acquire's barrier. Without one, the next command buffer would use incorrect hazard tracking as its not aware of the pending alias acquire. We decided not to handle that case.
		for(const GpuImageSubresourceTrackingState& trackingState : trackingStates)
			B3D_ENSURE_LOG(!trackingState.IsAliasAcquirePending(), "Every subresource of an alias acquired GPU image must be accessed on the command buffer that acquires it.");

		const GpuImageSubresourceTrackingState* firstEffectiveTrackingState = nullptr;
		for(const GpuImageSubresourceTrackingState& trackingState : trackingStates)
		{
			if(trackingState.HazardState != nullptr && trackingState.HazardState->HasSubmissionEffect())
			{
				firstEffectiveTrackingState = &trackingState;
				break;
			}
		}

		// Nothing to transition, so the image's submission state stays as it is
		if(firstEffectiveTrackingState == nullptr)
			continue;

		GpuImageSubresource& fullRangeSubresource = *image->GetFullRangeSubresource();
		if(image->HasUniformSubmissionState())
		{
			// Partitions don't overlap, so a partition covering the full range is the only one
			if(image->IsFullRange(firstEffectiveTrackingState->Range))
			{
				fnResolveTrackedImageTransition(image, *firstEffectiveTrackingState, image->GetRange(), fullRangeSubresource, image->GetUseInfo(GpuAccessFlag::Read));
				continue;
			}

			image->SplitSubmissionState();
		}

		const GpuQueueMask fullRangeReadQueues = fullRangeSubresource.GetUseInfo(GpuAccessFlag::Read);
		for(const GpuImageSubresourceTrackingState& trackingState : trackingStates)
		{
			B3D_ASSERT(trackingState.Range.HasSingleAspect());

			if(trackingState.HazardState == nullptr || !trackingState.HazardState->HasSubmissionEffect())
				continue;

			const GpuTextureSubresourceRange& trackedRange = trackingState.Range;
			const u32 mipEnd = trackedRange.BaseMipLevel + trackedRange.MipLevelCount;
			const u32 faceEnd = trackedRange.BaseArrayLayer + trackedRange.ArrayLayerCount;
			for(u32 mipLevel = trackedRange.BaseMipLevel; mipLevel < mipEnd; ++mipLevel)
			{
				for(u32 face = trackedRange.BaseArrayLayer; face < faceEnd; ++face)
				{
					GpuImageSubresource& subresource = *image->GetSubresource(face, mipLevel, (GpuTextureAspectFlag)(u32)trackedRange.AspectMask);

					// Full-range reads cover every subresource
					const GpuQueueMask inFlightReadQueues = subresource.GetUseInfo(GpuAccessFlag::Read) | fullRangeReadQueues;
					fnResolveTrackedImageTransition(image, trackingState, GpuTextureSubresourceRange(mipLevel, 1, face, 1, trackedRange.AspectMask), subresource, inFlightReadQueues);
				}
			}
		}

		// Return to one state as soon as the subresources share it. Never within a command buffer, and never by adding synchronization.
		image->TryMergeSubmissionState(frameIndex);
	}
}

template<class TDerived, class TBarrierHelper>
GpuBufferTrackingState& TGpuResourceTracker<TDerived, TBarrierHelper>::GetOrCreateBufferTrackingState(IGpuBufferResource* buffer)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_ENSURE_LOG(!buffer->IsSupersededByAlias(), "A GPU buffer is used after another resource took over its memory with an alias acquire.");
#endif

	auto insertResult = mBuffers.insert(std::make_pair(buffer, GpuBufferTrackingState()));
	if(insertResult.second) // New element
	{
		GpuBufferTrackingState& bufferTrackingState = insertResult.first->second;

		bufferTrackingState.UseHandle.Used = false;
		bufferTrackingState.UseHandle.Flags = GpuAccessFlag::None;
		bufferTrackingState.UseHandle.Stages = GpuStageFlag::None;

		buffer->NotifyBound();

		return bufferTrackingState;
	}
	else // Existing element
	{
		GpuBufferTrackingState& bufferTrackingState = insertResult.first->second;
		return bufferTrackingState;
	}
}

template<class TDerived, class TBarrierHelper>
GpuResourceHazardState& TGpuResourceTracker<TDerived, TBarrierHelper>::GetOrCreateHazardState(GpuBufferTrackingState& bufferTrackingState)
{
	if(bufferTrackingState.HazardState != nullptr)
		return *bufferTrackingState.HazardState;

	bufferTrackingState.HazardState = mHazardStatePool.Construct<GpuResourceHazardState>();

	// Every access recorded so far was a resting read. They precede every access still pending registration, so they are recorded directly.
	if(bufferTrackingState.UseHandle.Stages != GpuStageFlag::None)
		bufferTrackingState.HazardState->RecordAccess(bufferTrackingState.UseHandle.Stages, GpuAccessFlag::Read);

	return *bufferTrackingState.HazardState;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::QueueRequiredBufferBarrier(IGpuBufferResource* buffer, const GpuBufferTrackingState& bufferTrackingState, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, TBarrierHelper& barrierHelper)
{
	if(buffer == nullptr)
		return;

	const GpuBarrierScope requiredBarrier = bufferTrackingState.HazardState->GetRequiredBarrier(destinationStages, destinationAccess);
	if(requiredBarrier.IsValid())
		barrierHelper.QueueResolvedBufferBarrier(buffer, requiredBarrier, bufferTrackingState.IsAliasAcquirePending() ? GpuBarrierFlag::AliasAcquire : GpuBarrierFlag::None);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackExplicitBufferBarrier(IGpuBufferResource* buffer, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, TBarrierHelper& barrierHelper)
{
	if(buffer == nullptr)
		return;

	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	GpuResourceHazardState& hazardState = GetOrCreateHazardState(bufferTrackingState);

	// The acquire's barrier is recorded by the first access, and an explicit barrier is not one and we don't handle that case.
	B3D_ENSURE_LOG(!bufferTrackingState.IsAliasAcquirePending(), "An alias acquired buffer must be written before an explicit barrier.");

	if(!hazardState.HasAccess())
	{
		hazardState.HasLeadingBarrier = true;
		return;
	}

	QueueRequiredBufferBarrier(buffer, bufferTrackingState, destinationStages, destinationAccess, barrierHelper);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackBufferAccess(IGpuBufferResource* buffer, GpuBufferTrackingState& bufferTrackingState, GpuStageFlags stages, GpuAccessFlags access, TBarrierHelper& barrierHelper, u32 dynamicOffset)
{
	B3D_ASSERT(!bufferTrackingState.UseHandle.Used);

	// The contents of a new lifetime are undefined
	const bool isFirstAccess = bufferTrackingState.UseHandle.Flags == GpuAccessFlag::None;
	B3D_ENSURE_LOG(!bufferTrackingState.SubmissionBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire) || !isFirstAccess || access.IsSet(GpuAccessFlag::Write), "The first access of an alias acquired buffer must write.");

	// Turns earlier resting reads into tracked reads, before the barrier is resolved against them
	GpuResourceHazardState* const hazardState = &GetOrCreateHazardState(bufferTrackingState);
	QueueRequiredBufferBarrier(buffer, bufferTrackingState, stages, access, barrierHelper);

	// Defer registering hazards until after the barrier is issued, as the barrier helper clears any hazards that have been set
	if(access.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		PendingHazardRegistration registration;
		registration.State = hazardState;
		registration.AccessStageFlags = stages;
		registration.Access = access;

		mPendingHazardRegistrations.push_back(registration);
	}

	bufferTrackingState.UseHandle.Flags |= access;
	bufferTrackingState.UseHandle.Stages |= stages;

#if B3D_BUILD_TYPE_DEVELOPMENT
	TrackBufferSuballocation(buffer, dynamicOffset);
#endif
}

#if B3D_BUILD_TYPE_DEVELOPMENT
template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackBufferSuballocation(IGpuBufferResource* buffer, u32 offset)
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

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackBufferAccess(IGpuBufferResource* buffer, GpuStageFlags stages, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, u32 dynamicOffset)
{
	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);

	// Resting read optimization
	if(bufferTrackingState.HazardState == nullptr && accessFlags == GpuAccessFlag::Read && stages != GpuStageFlag::None)
	{
		B3D_ASSERT(!bufferTrackingState.UseHandle.Used);

		bufferTrackingState.UseHandle.Stages |= stages;
		bufferTrackingState.UseHandle.Flags |= GpuAccessFlag::Read;

#if B3D_BUILD_TYPE_DEVELOPMENT
		TrackBufferSuballocation(buffer, dynamicOffset);
#endif
		return;
	}

	TrackBufferAccess(buffer, bufferTrackingState, stages, accessFlags, barrierHelper, dynamicOffset);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::PrepareRenderPass(TArrayView<const GpuRenderPassAttachmentUsage> attachments)
{
	if(!B3D_ENSURE(mRenderPassTrackingPhase == RenderPassTrackingPhase::Inactive))
		return;

	mRenderPassTrackingPhase = RenderPassTrackingPhase::Preparing;
	for(const GpuRenderPassAttachmentUsage& attachment : attachments)
	{
		if(attachment.Image == nullptr)
			continue;

		mPendingRenderPassAttachments.Add(PendingRenderPassAttachmentUsage(attachment));

		// Cut attachment subresource ranges
		IterateAndCreateOverlappingImageSubresourceTrackingState(attachment.Image, attachment.Range, [](u32, void*) { });
	}
}

template<class TDerived, class TBarrierHelper>
TArrayView<const GpuResolvedRenderPassAttachmentUsage> TGpuResourceTracker<TDerived, TBarrierHelper>::BeginRenderPass(TBarrierHelper& barrierHelper)
{
	if(!B3D_ENSURE(mRenderPassTrackingPhase == RenderPassTrackingPhase::Preparing))
		return TArrayView<const GpuResolvedRenderPassAttachmentUsage>();

	for(const PendingRenderPassAttachmentUsage& pendingAttachment : mPendingRenderPassAttachments)
	{
		const GpuRenderPassAttachmentUsage& attachment = pendingAttachment.Usage;

		GpuResolvedRenderPassAttachmentUsage resolvedAttachment;
		resolvedAttachment.Image = attachment.Image;
		resolvedAttachment.Range = attachment.Range;
		resolvedAttachment.Surface = attachment.Surface;
		resolvedAttachment.UseFlags = attachment.UseFlags | pendingAttachment.ShaderUseFlags;
		resolvedAttachment.Access = attachment.Access;
		resolvedAttachment.BarrierFlags = attachment.BarrierFlags;
		resolvedAttachment.Layout = attachment.Layout;

		if(pendingAttachment.ShaderUseFlags.IsSet(GpuResourceUseFlag::ShaderAccess))
		{
			B3D_ASSERT(attachment.Access == GpuAccessFlag::Read);
			B3D_ASSERT(attachment.ShaderReadLayout.has_value());

			resolvedAttachment.Access |= GpuAccessFlag::Read;
			resolvedAttachment.Layout = *attachment.ShaderReadLayout;
		}

		resolvedAttachment.FinalLayout = attachment.FinalLayout.value_or(resolvedAttachment.Layout);

		mActiveRenderPassAttachments.Add(std::move(resolvedAttachment));
	}

	mPendingRenderPassAttachments.Clear();
	mRenderPassTrackingPhase = RenderPassTrackingPhase::Active;

	for(const GpuResolvedRenderPassAttachmentUsage& attachment : mActiveRenderPassAttachments)
		TrackImageAccess(attachment.Image, attachment.Range, attachment.Layout, GpuBackendUtility::GetStageFlags(attachment.UseFlags), attachment.Access, barrierHelper, attachment.BarrierFlags);

	return mActiveRenderPassAttachments;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::EndRenderPass()
{
	if(!B3D_ENSURE(mRenderPassTrackingPhase == RenderPassTrackingPhase::Active))
		return;

	struct CallbackParameters
	{
		TGpuResourceTracker* Self;
		GpuImageLayout FinalLayout;
	};

	for(const GpuResolvedRenderPassAttachmentUsage& attachment : mActiveRenderPassAttachments)
	{
		CallbackParameters callbackParameters;
		callbackParameters.Self = this;
		callbackParameters.FinalLayout = attachment.FinalLayout;

		IterateAndCreateOverlappingImageSubresourceTrackingState(attachment.Image, attachment.Range, [](u32 globalSubresourceIndex, void* userData)
		{
			CallbackParameters* const callbackParameters = static_cast<CallbackParameters*>(userData);
			GpuImageSubresourceTrackingState& subresourceTrackingState = callbackParameters->Self->mSubresourceTrackingState[globalSubresourceIndex];

			if(callbackParameters->FinalLayout != GpuImageLayout::Undefined)
			{
				// The render pass transitions the attachment itself
				if(callbackParameters->FinalLayout != subresourceTrackingState.CurrentLayout)
					subresourceTrackingState.TransitionsLayout = true;

				subresourceTrackingState.CurrentLayout = callbackParameters->FinalLayout;
				subresourceTrackingState.RequiredLayout = callbackParameters->FinalLayout;
			}
		}, &callbackParameters);
	}

	mActiveRenderPassAttachments.Clear();
	mAttachmentsNeedingAccess = 0;
	mRenderPassTrackingPhase = RenderPassTrackingPhase::Inactive;
}

template<class TDerived, class TBarrierHelper>
const GpuResolvedRenderPassAttachmentUsage* TGpuResourceTracker<TDerived, TBarrierHelper>::FindRenderPassAttachment(IGpuImageResource* image, const GpuTextureSubresourceRange& range) const
{
	for(const GpuResolvedRenderPassAttachmentUsage& attachment : mActiveRenderPassAttachments)
	{
		if(attachment.Image == image && GpuBackendUtility::RangeOverlaps(attachment.Range, range))
			return &attachment;
	}

	return nullptr;
}

template<class TDerived, class TBarrierHelper>
GpuImageLayout TGpuResourceTracker<TDerived, TBarrierHelper>::ResolveShaderImageLayout(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout requestedLayout) const
{
	for(const PendingRenderPassAttachmentUsage& pendingAttachment : mPendingRenderPassAttachments)
	{
		if(pendingAttachment.Usage.Image == image && pendingAttachment.Usage.ShaderReadLayout.has_value() && GpuBackendUtility::RangeOverlaps(pendingAttachment.Usage.Range, subresourceRange))
			return *pendingAttachment.Usage.ShaderReadLayout;
	}

	const GpuResolvedRenderPassAttachmentUsage* const renderPassAttachment = FindRenderPassAttachment(image, subresourceRange);
	if(renderPassAttachment != nullptr)
		return renderPassAttachment->Layout;

	return requestedLayout;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::InvalidateRenderPassAttachmentAccess(IGpuImageResource* image)
{
	for(u32 attachmentIndex = 0; attachmentIndex < mActiveRenderPassAttachments.Size(); attachmentIndex++)
		if(mActiveRenderPassAttachments[attachmentIndex].Image == image)
			mAttachmentsNeedingAccess |= 1u << attachmentIndex;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackRenderPassAttachmentAccesses(TBarrierHelper& barrierHelper)
{
	if(mAttachmentsNeedingAccess == 0)
		return;

	const u32 pendingAttachments = mAttachmentsNeedingAccess;
	mAttachmentsNeedingAccess = 0;
	for(u32 attachmentIndex = 0; attachmentIndex < mActiveRenderPassAttachments.Size(); attachmentIndex++)
	{
		if((pendingAttachments & (1u << attachmentIndex)) == 0)
			continue;

		const GpuResolvedRenderPassAttachmentUsage& attachment = mActiveRenderPassAttachments[attachmentIndex];
		const GpuStageFlags stages = GpuBackendUtility::GetStageFlags(attachment.UseFlags);
		const GpuImageTrackingState& imageTrackingState = GetImageTrackingState(attachment.Image);
		for(u32 rangeIndex = 0; rangeIndex < imageTrackingState.SubresourceInfoCount; rangeIndex++)
		{
			const u32 subresourceIndex = imageTrackingState.FirstSubresourceInfoIndex + rangeIndex;
			const GpuImageSubresourceTrackingState& trackingState = mSubresourceTrackingState[subresourceIndex];
			if(!GpuBackendUtility::RangeOverlaps(trackingState.Range, attachment.Range))
				continue;

			const GpuResourceWriteEpochHazardState& hazards = trackingState.HazardState->LastWriteEpochHazardState;

			// Consecutive attachment writes are ordered by the render pass. An intervening reader or other writer ends that run and we must explicitly track usage.
			if(attachment.Access.IsSet(GpuAccessFlag::Write) && hazards.WriteStages == stages && hazards.ReaderStages == GpuStageFlag::None && hazards.VisibleStages == GpuStageFlag::None && trackingState.CurrentLayout == attachment.Layout)
				continue;

			TrackSubresourceUsage(attachment.Image, subresourceIndex, attachment.Layout, stages, attachment.Access, barrierHelper, GpuBarrierFlag::None);
		}
	}
}

template<class TDerived, class TBarrierHelper>
bool TGpuResourceTracker<TDerived, TBarrierHelper>::TrackShaderImageAccess(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper)
{
	if(image == nullptr)
		return true;

	if(!useFlags.IsSet(GpuResourceUseFlag::ShaderAccess))
	{
		TrackImageAccess(image, subresourceRange, layout, GpuBackendUtility::GetStageFlags(useFlags), accessFlags, barrierHelper);
		return true;
	}

	const GpuStageFlags stages = GpuBackendUtility::GetStageFlags(useFlags);
	if(TryTrackRestingImageRead(image, subresourceRange, layout, stages, accessFlags))
		return true;

	struct CallbackParameters
	{
		CallbackParameters() = default;

		TGpuResourceTracker<TDerived, TBarrierHelper>* Self;
		TBarrierHelper* BarrierHelper;
		IGpuImageResource* Image;
		GpuImageLayout Layout;
		GpuResourceUseFlags UseFlags;
		GpuAccessFlags AccessFlags;
		bool Valid = true;
	};

	CallbackParameters callbackParameters;
	callbackParameters.Self = this;
	callbackParameters.BarrierHelper = &barrierHelper;
	callbackParameters.Image = image;
	callbackParameters.Layout = layout;
	callbackParameters.UseFlags = useFlags;
	callbackParameters.AccessFlags = accessFlags;

	IterateAndCreateOverlappingImageSubresourceTrackingState(image, subresourceRange, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* const callbackParameters = (CallbackParameters*)userData;
		TGpuResourceTracker<TDerived, TBarrierHelper>* self = callbackParameters->Self;

		bool foldedIntoRenderPassAttachment = false;
		if(self->mRenderPassTrackingPhase == RenderPassTrackingPhase::Preparing)
		{
			const GpuTextureSubresourceRange& trackedRange = self->mSubresourceTrackingState[globalSubresourceIndex].Range;
			for(PendingRenderPassAttachmentUsage& pendingAttachment : self->mPendingRenderPassAttachments)
			{
				GpuRenderPassAttachmentUsage& attachment = pendingAttachment.Usage;
				if(attachment.Image != callbackParameters->Image || !GpuBackendUtility::RangeOverlaps(attachment.Range, trackedRange))
					continue;

				const bool supportsShaderRead = attachment.Access == GpuAccessFlag::Read && !callbackParameters->AccessFlags.IsSet(GpuAccessFlag::Write) && attachment.ShaderReadLayout.has_value();
				if(!supportsShaderRead)
				{
					B3D_LOG(Error, LogRenderBackend, "Framebuffer attachments sampled during a render pass must be marked read-only.");
					callbackParameters->Valid = false;
					return;
				}

				pendingAttachment.ShaderUseFlags |= callbackParameters->UseFlags;
				foldedIntoRenderPassAttachment = true;

				break;
			}
		}

		if(foldedIntoRenderPassAttachment)
			return;

		GpuImageSubresourceTrackingState& trackingState = self->mSubresourceTrackingState[globalSubresourceIndex];
		GpuImageLayout layout = callbackParameters->Layout;
		GpuResourceUseFlags useFlags = callbackParameters->UseFlags;
		const GpuResolvedRenderPassAttachmentUsage* const attachment = self->FindRenderPassAttachment(callbackParameters->Image, trackingState.Range);
		if(attachment != nullptr)
		{
			if(attachment->Access != GpuAccessFlag::Read || callbackParameters->AccessFlags.IsSet(GpuAccessFlag::Write))
			{
				B3D_LOG(Error, LogRenderBackend, "Framebuffer attachments sampled during a render pass must be marked read-only.");
				callbackParameters->Valid = false;
				return;
			}

			layout = attachment->Layout;
			useFlags |= attachment->UseFlags;
		}
		else if(trackingState.AccessEpoch == self->mEpoch && layout != GpuImageLayout::Undefined && trackingState.RequiredLayout != GpuImageLayout::Undefined && layout != trackingState.RequiredLayout)
			layout = GpuImageLayout::General;

		trackingState.AccessEpoch = self->mEpoch;
		self->TrackSubresourceUsage(callbackParameters->Image, globalSubresourceIndex, layout, GpuBackendUtility::GetStageFlags(useFlags), callbackParameters->AccessFlags, *callbackParameters->BarrierHelper, GpuBarrierFlag::None);

	}, &callbackParameters);

	if(callbackParameters.Valid)
		RegisterImageSubresources(image, subresourceRange, accessFlags, stages);

	return callbackParameters.Valid;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackImageAccess(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, GpuBarrierFlags barrierFlags, GpuImageTrackingFlags trackingFlags)
{
	if(image == nullptr)
		return;

	if(barrierFlags == GpuBarrierFlag::None && trackingFlags == GpuImageTrackingFlag::None && TryTrackRestingImageRead(image, subresourceRange, layout, stages, accessFlags))
		return;

	struct CallbackParameters
	{
		CallbackParameters() = default;

		TGpuResourceTracker<TDerived, TBarrierHelper>* Self;
		TBarrierHelper* BarrierHelper;
		IGpuImageResource* Image;
		GpuImageLayout Layout;
		GpuStageFlags Stages;
		GpuAccessFlags AccessFlags;
		GpuBarrierFlags BarrierFlags;
		GpuImageTrackingFlags TrackingFlags;
	};

	CallbackParameters callbackParameters;
	callbackParameters.Self = this;
	callbackParameters.BarrierHelper = &barrierHelper;
	callbackParameters.Image = image;
	callbackParameters.Layout = layout;
	callbackParameters.Stages = stages;
	callbackParameters.AccessFlags = accessFlags;
	callbackParameters.BarrierFlags = barrierFlags;
	callbackParameters.TrackingFlags = trackingFlags;

	IterateAndCreateOverlappingImageSubresourceTrackingState(image, subresourceRange, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* const callbackParameters = (CallbackParameters*)userData;
		TGpuResourceTracker<TDerived, TBarrierHelper>* self = callbackParameters->Self;

		self->TrackSubresourceUsage(callbackParameters->Image, globalSubresourceIndex, callbackParameters->Layout, callbackParameters->Stages, callbackParameters->AccessFlags, *callbackParameters->BarrierHelper, callbackParameters->BarrierFlags, callbackParameters->TrackingFlags);

	}, &callbackParameters);

	// For meta-data operations ignore access for now, just make sure the resource is registered. Since meta-data ops are conditional we patch the access during submission
	// (access for hazard tracking is updated right away though, conservatively)
	RegisterImageSubresources(image, subresourceRange, trackingFlags.IsSet(GpuImageTrackingFlag::MetadataOperation) ? GpuAccessFlags(GpuAccessFlag::Read) : accessFlags, stages);
}

template<class TDerived, class TBarrierHelper>
bool TGpuResourceTracker<TDerived, TBarrierHelper>::TryTrackRestingImageRead(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags accessFlags)
{
	if(accessFlags != GpuAccessFlag::Read || stages == GpuStageFlag::None || !image->CanRest() || layout != GpuImageLayout::ShaderReadOnly)
		return false;

	// Only uniform images rest, and only single-aspect images can be uniform
	B3D_ASSERT(image->GetRange().HasSingleAspect());

	const GpuTextureSubresourceRange range = GpuBackendUtility::ClampRange(subresourceRange, image->GetRange());
	if(!range.AspectMask)
		return false;

	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);
	if(imageTrackingState.SubresourceInfoCount != 0)
		return false;

	// Submission synchronizes the bounding range of the resting reads, so that is the range registered with the command buffer
	const bool isFirstAccess = imageTrackingState.UseHandle.Stages == GpuStageFlag::None;
	const GpuTextureSubresourceRange newRange = isFirstAccess ? range : GpuBackendUtility::GetBoundingRange(imageTrackingState.Range, range);
	if(isFirstAccess || !GpuBackendUtility::RangeContains(imageTrackingState.Range, newRange))
	{
		imageTrackingState.Range = newRange;
		RegisterImageSubresources(image, newRange, GpuAccessFlag::Read, stages);
	}
	else
		imageTrackingState.UseHandle.Stages |= stages;

	return true;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::RegisterImageSubresources(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuAccessFlags accessFlags, GpuStageFlags stages)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);
	B3D_ASSERT(!imageTrackingState.UseHandle.Used);
	imageTrackingState.UseHandle.Flags |= accessFlags;
	imageTrackingState.UseHandle.Stages |= stages;

	// Register any sub-resources
	B3D_ASSERT(subresourceRange.ArrayLayerCount != ~0u);
	B3D_ASSERT(subresourceRange.MipLevelCount != ~0u);

	if(image->IsFullRange(subresourceRange))
	{
		TrackResourceUsage(image->GetFullRangeSubresource(), accessFlags);
		return;
	}

	const GpuTextureAspectFlags trackedAspects = subresourceRange.AspectMask & image->GetRange().AspectMask;
	for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
	{
		if(!trackedAspects.IsSet(aspect))
			continue;

		for(u32 layerIndex = 0; layerIndex < subresourceRange.ArrayLayerCount; layerIndex++)
		{
			for(u32 levelIndex = 0; levelIndex < subresourceRange.MipLevelCount; levelIndex++)
			{
				const u32 layer = subresourceRange.BaseArrayLayer + layerIndex;
				const u32 mipLevel = subresourceRange.BaseMipLevel + levelIndex;

				TrackResourceUsage(image->GetSubresource(layer, mipLevel, aspect), accessFlags);
			}
		}
	}
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackExplicitImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, GpuImageLayout destinationLayout, TBarrierHelper& barrierHelper)
{
	if(image == nullptr)
		return;

	struct CallbackParameters
	{
		TGpuResourceTracker* Tracker;
		TBarrierHelper* BarrierHelper;
		IGpuImageResource* Image;
		GpuStageFlags DestinationStages;
		GpuAccessFlags DestinationAccess;
		GpuImageLayout DestinationLayout;
	};

	CallbackParameters callbackParameters { this, &barrierHelper, image, destinationStages, destinationAccess, destinationLayout };
	IterateAndCreateOverlappingImageSubresourceTrackingState(image, subresourceRange, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* const callbackParameters = static_cast<CallbackParameters*>(userData);
		GpuImageSubresourceTrackingState& subresourceTrackingState = callbackParameters->Tracker->mSubresourceTrackingState[globalSubresourceIndex];

		// The acquire's barrier is recorded by the first access, and an explicit barrier is not one and we dont' support this case
		B3D_ENSURE_LOG(!subresourceTrackingState.IsAliasAcquirePending(), "An alias acquired image must be written before an explicit barrier.");

		if(!subresourceTrackingState.HazardState->HasAccess())
		{
			subresourceTrackingState.HazardState->HasLeadingBarrier = true;

			if(callbackParameters->DestinationLayout != GpuImageLayout::Undefined)
			{
				subresourceTrackingState.InitialLayout = callbackParameters->DestinationLayout;
				subresourceTrackingState.CurrentLayout = callbackParameters->DestinationLayout;
				subresourceTrackingState.RequiredLayout = callbackParameters->DestinationLayout;
			}
		}

		callbackParameters->Tracker->RegisterImageSubresources(callbackParameters->Image, subresourceTrackingState.Range, GpuAccessFlag::None, GpuStageFlag::None);
		callbackParameters->Tracker->GetDerived().QueueRequiredImageBarrier(callbackParameters->Image, subresourceTrackingState, callbackParameters->DestinationStages, callbackParameters->DestinationAccess, callbackParameters->DestinationLayout, *callbackParameters->BarrierHelper);
	}, &callbackParameters);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::QueueRequiredImageBarrier(IGpuImageResource* image, GpuImageSubresourceTrackingState& subresourceTrackingState, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, GpuImageLayout destinationLayout, TBarrierHelper& barrierHelper, GpuBarrierFlags barrierFlags, GpuImageTrackingFlags trackingFlags)
{
	if(image == nullptr)
		return;

	if(destinationLayout == GpuImageLayout::Undefined)
		destinationLayout = subresourceTrackingState.CurrentLayout;

	// The first access after an alias acquire records the acquire's barrier, which always transitions from GpuImageLayout::Undefined
	const bool aliasAcquirePending = subresourceTrackingState.IsAliasAcquirePending();
	const bool needsLayoutTransition = subresourceTrackingState.CurrentLayout != destinationLayout || barrierFlags.IsSet(GpuBarrierFlag::DiscardContents) || aliasAcquirePending;

	// The first access defers its barrier to submission
	if(!subresourceTrackingState.HazardState->HasAccess() && !aliasAcquirePending)
	{
		if(needsLayoutTransition)
		{
			subresourceTrackingState.InitialLayout = destinationLayout;
			subresourceTrackingState.CurrentLayout = destinationLayout;
			subresourceTrackingState.RequiredLayout = destinationLayout;
		}

		return;
	}

	// If something other than attachment stages are using an attachment, we need to invalidate it so access is re-issued before next draw, so any barriers are correctly issued. This is a special case done only by some backends (e.g. a compute shader clear in the middle of a render pass)
	if(destinationStages != GpuBackendUtility::GetStageFlags(GpuResourceUseFlag::ColorAttachment) && destinationStages != GpuBackendUtility::GetStageFlags(GpuResourceUseFlag::DepthStencilAttachment))
		InvalidateRenderPassAttachmentAccess(image);

	// A layout transition is potentially a write operation, so it must be ordered after both earlier reads and writes,
	// even when the upcoming resource access itself is read-only.
	GpuAccessFlags hazardAccess = destinationAccess;
	if(needsLayoutTransition)
		hazardAccess |= GpuAccessFlag::Write;

	const GpuBarrierScope requiredBarrier = subresourceTrackingState.HazardState->GetRequiredBarrier(destinationStages, hazardAccess);
	if(!requiredBarrier.IsValid() && !needsLayoutTransition)
		return;

	GpuBarrierScope barrier = requiredBarrier;
	if(needsLayoutTransition)
	{
		// TransitionsLayout=true is used to mark the transition as a write during submission, so it's correctly ordered against other queues, but for 
		// aliased resources we already handle that, so don't unnecessarily set it
		if(!aliasAcquirePending)
			subresourceTrackingState.TransitionsLayout = true;

		// The synthetic write above only finds operations that must precede the transition. The native destination
		// scope describes the real access that consumes the image in its new layout.
		barrier.DestinationStages = destinationStages;
		barrier.DestinationAccess = destinationAccess;
	}

	if(aliasAcquirePending)
		barrierFlags |= GpuBarrierFlag::AliasAcquire | GpuBarrierFlag::DiscardContents;

	barrierHelper.QueueResolvedImageBarrier(image, subresourceTrackingState.Range, barrier, subresourceTrackingState.CurrentLayout, destinationLayout, barrierFlags);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackSubresourceUsage(IGpuImageResource* image, u32 globalSubresourceIndex, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, GpuBarrierFlags barrierFlags, GpuImageTrackingFlags trackingFlags)
{
	GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState[globalSubresourceIndex];

	// The contents of a new lifetime are undefined, so the first access must write
	const bool aliasAcquired = subresourceTrackingState.SubmissionBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire);
	if(aliasAcquired && subresourceTrackingState.Access == GpuAccessFlag::None)
	{
		const GpuStageFlags attachmentStages = GpuStageFlag::ColorAttachment | GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;
		const bool loadsAttachment = stages.IsSetAny(attachmentStages) && accessFlags.IsSet(GpuAccessFlag::Read) && !barrierFlags.IsSet(GpuBarrierFlag::DiscardContents);
		B3D_ENSURE_LOG(accessFlags.IsSet(GpuAccessFlag::Write) && !loadsAttachment, "The first access of an alias acquired image must write without reading the previous contents.");
	}

	if(subresourceTrackingState.Access == GpuAccessFlag::None)
		subresourceTrackingState.SubmissionBarrierFlags |= barrierFlags;

	// The first access defers its barrier to submission, except after an alias acquire, which records its barrier inline from GpuImageLayout::Undefined
	const bool hasLeadingBarrier = subresourceTrackingState.HazardState != nullptr && subresourceTrackingState.HazardState->HasLeadingBarrier;
	if(subresourceTrackingState.Access == GpuAccessFlag::None && !subresourceTrackingState.HazardState->HasAccess() && !hasLeadingBarrier && !aliasAcquired) // New subresource
	{
		subresourceTrackingState.InitialLayout = layout;
		subresourceTrackingState.CurrentLayout = layout;
		subresourceTrackingState.RequiredLayout = layout;
	}
	else if(layout != GpuImageLayout::Undefined)
		subresourceTrackingState.RequiredLayout = layout;

	GetDerived().QueueRequiredImageBarrier(image, subresourceTrackingState, stages, accessFlags, subresourceTrackingState.RequiredLayout, barrierHelper, barrierFlags, trackingFlags);

	GpuResourceHazardState* const hazardState = subresourceTrackingState.HazardState;

	// Defer registering hazards until after the barrier is issued, as the barrier helper clears any hazards that have been set. For metadata operations
	// we only register the hazard when we record the metadata transitions, as metadata transitions are conditionally enabled.
	if(!trackingFlags.IsSet(GpuImageTrackingFlag::MetadataOperation) && accessFlags.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		PendingHazardRegistration registration;
		registration.State = hazardState;
		registration.MetadataState = subresourceTrackingState.MetadataState.get();
		registration.AccessStageFlags = stages;
		registration.Access = accessFlags;

		mPendingHazardRegistrations.push_back(registration);
	}

	subresourceTrackingState.Access |= accessFlags;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackResourceUsage(IGpuResource* resource, GpuAccessFlags access)
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

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::AcquireAliased(IGpuBufferResource* buffer, const GpuAliasAcquire& acquire)
{
	if(buffer == nullptr)
		return;

	// Earlier tracking state would make the acquire's barrier order after this command buffer's accesses instead of the source
	if(!B3D_ENSURE_LOG(FindBufferTrackingState(buffer) == nullptr, "An alias acquire must precede every other use of the GPU buffer on the command buffer."))
		return;

#if B3D_BUILD_TYPE_DEVELOPMENT
	buffer->SetSupersededByAlias(false);
	for(IGpuResource* predecessor : acquire.Predecessors)
		predecessor->SetSupersededByAlias(true);
#endif

	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	bufferTrackingState.SubmissionBarrierFlags = GpuBarrierFlag::AliasAcquire;

	// Set the prior state of the resource, triggering an inline barrier on the next access
	GpuResourceWriteEpochHazardState& hazards = GetOrCreateHazardState(bufferTrackingState).LastWriteEpochHazardState;
	hazards.WriteStages = acquire.Source.WriteStages;
	hazards.ReaderStages = acquire.Source.ReadStages;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::AcquireAliased(IGpuImageResource* image, const GpuAliasAcquire& acquire)
{
	if(image == nullptr)
		return;

	// Earlier tracking state would make the acquire's barrier order after this command buffer's accesses instead of the source
	if(!B3D_ENSURE_LOG(FindImageTrackingStateIndex(image) == ~0u, "An alias acquire must precede every other use of the GPU image on the command buffer."))
		return;

#if B3D_BUILD_TYPE_DEVELOPMENT
	image->SetSupersededByAlias(false);
	for(IGpuResource* predecessor : acquire.Predecessors)
		predecessor->SetSupersededByAlias(true);
#endif

	struct CallbackParameters
	{
		TGpuResourceTracker* Tracker;
		const GpuAccessScope* Source;
	};

	// One tracking state per aspect, over the full range. The image never has only resting reads, so its first read cannot rest.
	CallbackParameters callbackParameters { this, &acquire.Source };
	IterateAndCreateOverlappingImageSubresourceTrackingState(image, image->GetRange(), [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* const callbackParameters = static_cast<CallbackParameters*>(userData);
		GpuImageSubresourceTrackingState& subresourceTrackingState = callbackParameters->Tracker->mSubresourceTrackingState[globalSubresourceIndex];

		// Layouts stay GpuImageLayout::Undefined, which the first access transitions from
		subresourceTrackingState.SubmissionBarrierFlags = GpuBarrierFlag::AliasAcquire;

		// Set the prior state of the resource, triggering an inline barrier on the next access
		GpuResourceWriteEpochHazardState& hazards = subresourceTrackingState.HazardState->LastWriteEpochHazardState;
		hazards.WriteStages = callbackParameters->Source->WriteStages;
		hazards.ReaderStages = callbackParameters->Source->ReadStages;
	}, &callbackParameters);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackSwapChainUsage(IGpuSwapChainResource* swapChain)
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

template<class TDerived, class TBarrierHelper>
GpuImageTrackingState& TGpuResourceTracker<TDerived, TBarrierHelper>::GetOrCreateImageTrackingState(IGpuImageResource* image)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_ENSURE_LOG(!image->IsSupersededByAlias(), "A GPU image is used after another resource took over its memory with an alias acquire.");
#endif

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
		imageTrackingState.UseHandle.Stages = GpuStageFlag::None;

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

template<class TDerived, class TBarrierHelper>
u32 TGpuResourceTracker<TDerived, TBarrierHelper>::FindImageTrackingStateIndex(IGpuImageResource* image) const
{
	auto found = mImages.find(image);
	if(found == mImages.end())
		return ~0u;

	return found->second;
}

template<class TDerived, class TBarrierHelper>
const GpuImageTrackingState* TGpuResourceTracker<TDerived, TBarrierHelper>::FindImageTrackingState(IGpuImageResource* image) const
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	if(imageTrackingIndex == ~0u)
		return nullptr;

	return &mImageTrackingState[imageTrackingIndex];
}

template<class TDerived, class TBarrierHelper>
const GpuImageTrackingState& TGpuResourceTracker<TDerived, TBarrierHelper>::GetImageTrackingState(IGpuImageResource* image) const
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	B3D_ASSERT(imageTrackingIndex != ~0u);

	return mImageTrackingState[imageTrackingIndex];
}

template<class TDerived, class TBarrierHelper>
GpuImageTrackingState& TGpuResourceTracker<TDerived, TBarrierHelper>::GetImageTrackingState(IGpuImageResource* image)
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	B3D_ASSERT(imageTrackingIndex != ~0u);

	return mImageTrackingState[imageTrackingIndex];
}

template<class TDerived, class TBarrierHelper>
TArrayView<const GpuImageSubresourceTrackingState> TGpuResourceTracker<TDerived, TBarrierHelper>::GetSubresourceTrackingStatesForImage(IGpuImageResource* image) const
{
	const GpuImageTrackingState& imageTrackingState = GetImageTrackingState(image);
	if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
		return {};

	return TArrayView(&mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex], imageTrackingState.SubresourceInfoCount);
}

template<class TDerived, class TBarrierHelper>
TArrayView<GpuImageSubresourceTrackingState> TGpuResourceTracker<TDerived, TBarrierHelper>::GetSubresourceTrackingStatesForImage(IGpuImageResource* image)
{
	GpuImageTrackingState& imageTrackingState = GetImageTrackingState(image);
	if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
		return {};

	return TArrayView(&mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex], imageTrackingState.SubresourceInfoCount);
}

template<class TDerived, class TBarrierHelper>
const GpuImageSubresourceTrackingState& TGpuResourceTracker<TDerived, TBarrierHelper>::GetSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip, GpuTextureAspectFlag aspect) const
{
	const GpuImageSubresourceTrackingState* const trackingState = FindSubresourceTrackingState(image, face, mip, aspect);
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

template<class TDerived, class TBarrierHelper>
const GpuImageSubresourceTrackingState* TGpuResourceTracker<TDerived, TBarrierHelper>::FindSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip, GpuTextureAspectFlag aspect) const
{
	const u32 imageTrackingIndex = mImages.find(image)->second;
	const GpuImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

	// An image with only resting reads has no subresource tracking states
	if(imageTrackingState.SubresourceInfoCount == 0)
		return nullptr;

	const GpuImageSubresourceTrackingState* const subresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];
	for(u32 localSubresourceIndex = 0; localSubresourceIndex < imageTrackingState.SubresourceInfoCount; localSubresourceIndex++)
	{
		const GpuImageSubresourceTrackingState& subresourceTrackingState = subresourceTrackingStates[localSubresourceIndex];

		if(subresourceTrackingState.Range.AspectMask.IsSet(aspect) &&
		   face >= subresourceTrackingState.Range.BaseArrayLayer && face < (subresourceTrackingState.Range.BaseArrayLayer + subresourceTrackingState.Range.ArrayLayerCount) &&
		   mip >= subresourceTrackingState.Range.BaseMipLevel && mip < (subresourceTrackingState.Range.BaseMipLevel + subresourceTrackingState.Range.MipLevelCount))
		{
			return &subresourceTrackingState;
		}
	}

	return nullptr;
}

template<class TDerived, class TBarrierHelper>
const GpuBufferTrackingState* TGpuResourceTracker<TDerived, TBarrierHelper>::FindBufferTrackingState(IGpuBufferResource* buffer) const
{
	auto found = mBuffers.find(buffer);
	if(found != mBuffers.end())
		return &found->second;

	return nullptr;
}

template<class TDerived, class TBarrierHelper>
GpuImageSubresourceTrackingState& TGpuResourceTracker<TDerived, TBarrierHelper>::GetSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip, GpuTextureAspectFlag aspect)
{
	// Delegate to 'const' version and re-cast
	return const_cast<GpuImageSubresourceTrackingState&>(const_cast<const TGpuResourceTracker*>(this)->GetSubresourceTrackingState(image, face, mip, aspect));
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::IterateAndCreateOverlappingImageSubresourceTrackingState(IGpuImageResource* image, GpuTextureSubresourceRange subresourceRange, void (*fnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	// Convert resting to tracked reads first if needed
	if(imageTrackingState.HasOnlyRestingReads())
	{
		const u32 firstSubresourceIndex = (u32)mSubresourceTrackingState.size();
		u32 subresourceCount = 0;
		for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
		{
			if(!imageTrackingState.Range.AspectMask.IsSet(aspect))
				continue;

			GpuTextureSubresourceRange aspectRange = imageTrackingState.Range;
			aspectRange.AspectMask = aspect;

			const u32 subresourceIndex = AddSubresourceTrackingState(image, aspectRange);
			GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState[subresourceIndex];

			subresourceTrackingState.InitialLayout = GpuImageLayout::ShaderReadOnly;
			subresourceTrackingState.CurrentLayout = GpuImageLayout::ShaderReadOnly;
			subresourceTrackingState.RequiredLayout = GpuImageLayout::ShaderReadOnly;
			subresourceTrackingState.Access = GpuAccessFlag::Read;
			subresourceTrackingState.AccessEpoch = mEpoch;

			// Resting reads precede every access still pending registration, so they are recorded directly
			subresourceTrackingState.HazardState->RecordAccess(imageTrackingState.UseHandle.Stages, GpuAccessFlag::Read);
			subresourceCount++;
		}

		// The range was registered when the resting reads were recorded
		imageTrackingState.FirstSubresourceInfoIndex = firstSubresourceIndex;
		imageTrackingState.SubresourceInfoCount = subresourceCount;
	}

	// Provide exact size as code below doesn't handle the "remaining" sentinel
	subresourceRange = GpuBackendUtility::ClampRange(subresourceRange, image->GetRange());
	B3D_ASSERT(subresourceRange.AspectMask);

	auto fnProcessAspectSubresourceRange = [this, image, &imageTrackingState, fnDoOnOverlappingSubresource, userData](const GpuTextureSubresourceRange& aspectSubresourceRange)
	{
		B3D_ASSERT(aspectSubresourceRange.HasSingleAspect());

		if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
		{
			const u32 subresourceIndex = AddSubresourceTrackingState(image, aspectSubresourceRange);
			imageTrackingState.FirstSubresourceInfoIndex = subresourceIndex;
			imageTrackingState.SubresourceInfoCount = 1;

			fnDoOnOverlappingSubresource(subresourceIndex, userData);
			return;
		}

		GpuImageSubresourceTrackingState* const existingSubresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];

		// First test for the simplest and most common case (same range or no overlap) to avoid more complex computations.
		for(u32 subresourceLocalIndex = 0; subresourceLocalIndex < imageTrackingState.SubresourceInfoCount; subresourceLocalIndex++)
		{
			GpuImageSubresourceTrackingState& existingSubresourceTrackingState = existingSubresourceTrackingStates[subresourceLocalIndex];
			if(!GpuBackendUtility::RangeOverlaps(existingSubresourceTrackingState.Range, aspectSubresourceRange))
				continue;

			if(GpuBackendUtility::RangeEquals(existingSubresourceTrackingState.Range, aspectSubresourceRange))
			{
				const u32 subresourceIndex = imageTrackingState.FirstSubresourceInfoIndex + subresourceLocalIndex;
				fnDoOnOverlappingSubresource(subresourceIndex, userData);
				return;
			}

			// This means there's a partial overlap which means there's no point searching further, we must subdivide
			break;
		}

		// Rebuild the image's contiguous tracking range. This is expected only for a few textures per frame.
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

				if(!GpuBackendUtility::RangeOverlaps(subresource.Range, aspectSubresourceRange))
					CopySubresourceTrackingStateWithNewRange(globalSubresourceIndex, subresource.Range);
				else // Need to cut
				{
					u32 cutRangeCount;
					GpuBackendUtility::CutRange(subresource.Range, aspectSubresourceRange, cutRanges, cutRangeCount);

					for(u32 cutRangeIndex = 0; cutRangeIndex < cutRangeCount; cutRangeIndex++)
					{
						// Create a copy of the original subresource with the new range
						const u32 newGlobalSubresourceIndex = CopySubresourceTrackingStateWithNewRange(globalSubresourceIndex, cutRanges[cutRangeIndex]);

						if(GpuBackendUtility::RangeOverlaps(cutRanges[cutRangeIndex], aspectSubresourceRange))
						{
							fnDoOnOverlappingSubresource(newGlobalSubresourceIndex, userData);

							// Keep track of the overlapping ranges for later
							cutOverlappingRanges.push_back((u32)mSubresourceTrackingState.size() - 1);
						}
					}
				}
			}

			// Our range doesn't overlap with any existing ranges, so just add it
			if(cutOverlappingRanges.empty())
			{
				const u32 newGlobalSubresourceIndex = AddSubresourceTrackingState(image, aspectSubresourceRange);
				fnDoOnOverlappingSubresource(newGlobalSubresourceIndex, userData);
			}
			else // Search if overlapping ranges fully cover the requested range, and insert non-covered regions
			{
				FrameQueue<GpuTextureSubresourceRange> sourceRanges;
				sourceRanges.push(aspectSubresourceRange);

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
					const u32 newGlobalSubresourceIndex = AddSubresourceTrackingState(image, sourceRanges.front());
					fnDoOnOverlappingSubresource(newGlobalSubresourceIndex, userData);
					sourceRanges.pop();
				}
			}

			imageTrackingState.FirstSubresourceInfoIndex = newSubresourceTrackingStateIndex;
			imageTrackingState.SubresourceInfoCount = (u32)mSubresourceTrackingState.size() - newSubresourceTrackingStateIndex;
		}
		B3DClearAllocatorFrame();
	};

	for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
	{
		if(!subresourceRange.AspectMask.IsSet(aspect))
			continue;

		GpuTextureSubresourceRange aspectSubresourceRange = subresourceRange;
		aspectSubresourceRange.AspectMask = aspect;

		fnProcessAspectSubresourceRange(aspectSubresourceRange);
	}
}

template<class TDerived, class TBarrierHelper>
u32 TGpuResourceTracker<TDerived, TBarrierHelper>::AddSubresourceTrackingState(IGpuImageResource* image, const GpuTextureSubresourceRange& range)
{
	B3D_ASSERT(range.HasSingleAspect());

	mSubresourceTrackingState.push_back(GpuImageSubresourceTrackingState());

	GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState.back();
	subresourceTrackingState.CurrentLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.InitialLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.RequiredLayout = GpuImageLayout::Undefined;
	subresourceTrackingState.Range = range;
	subresourceTrackingState.MetadataState = GetDerived().CreateImageMetadataState(image, range);
	subresourceTrackingState.HazardState = mHazardStatePool.Construct<GpuResourceHazardState>();

	return (u32)mSubresourceTrackingState.size() - 1;
}

template<class TDerived, class TBarrierHelper>
u32 TGpuResourceTracker<TDerived, TBarrierHelper>::CopySubresourceTrackingStateWithNewRange(u32 copyFromIndex, const GpuTextureSubresourceRange& newRange)
{
	B3D_ASSERT(newRange.HasSingleAspect());

	GpuImageSubresourceTrackingState* const copyFromSubresource = &mSubresourceTrackingState[copyFromIndex];

	GpuImageSubresourceTrackingState subresourceCopy = *copyFromSubresource;
	subresourceCopy.Range = newRange;
	if(subresourceCopy.MetadataState != nullptr)
		subresourceCopy.MetadataState = subresourceCopy.MetadataState->Clone();

	subresourceCopy.HazardState = mHazardStatePool.Construct<GpuResourceHazardState>();

	if(B3D_ENSURE(copyFromSubresource->HazardState != nullptr))
		*subresourceCopy.HazardState = *copyFromSubresource->HazardState;

	// Deferred accesses cover the source range and must remain associated with every partition created from it.
	const u32 pendingHazardRegistrationCount = (u32)mPendingHazardRegistrations.size();
	for(u32 registrationIndex = 0; registrationIndex < pendingHazardRegistrationCount; registrationIndex++)
	{
		if(mPendingHazardRegistrations[registrationIndex].State != copyFromSubresource->HazardState)
			continue;

		PendingHazardRegistration registrationCopy = mPendingHazardRegistrations[registrationIndex];
		registrationCopy.State = subresourceCopy.HazardState;
		registrationCopy.MetadataState = subresourceCopy.MetadataState.get();
		mPendingHazardRegistrations.push_back(registrationCopy);
	}

	mSubresourceTrackingState.push_back(subresourceCopy);
	return (u32)mSubresourceTrackingState.size() - 1;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::UpdateImageLayoutTrackingAfterBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuImageLayout oldLayout, GpuImageLayout newLayout)
{
	struct CallbackParameters
	{
		TGpuResourceTracker<TDerived, TBarrierHelper>* Self;
		GpuImageLayout OldLayout;
		GpuImageLayout NewLayout;
	};

	CallbackParameters callbackParameters = { this, oldLayout, newLayout };

	IterateAndCreateOverlappingImageSubresourceTrackingState(image, range, [](u32 globalSubresourceIndex, void* userData)
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

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::CommitPendingAccesses()
{
	for(const PendingHazardRegistration& registration : mPendingHazardRegistrations)
		registration.State->RecordAccess(registration.AccessStageFlags, registration.Access);

	mPendingHazardRegistrations.clear();
	mEpoch++;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::UpdateHazardStateAfterBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier)
{
	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	GetOrCreateHazardState(bufferTrackingState).RecordBarrier(barrier);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::UpdateHazardStateAfterBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, const GpuBarrierScope& barrier)
{
	struct CallbackParameters
	{
		TGpuResourceTracker<TDerived, TBarrierHelper>* Self;
		GpuBarrierScope Barrier;
	};

	CallbackParameters callbackParameters = { this, barrier };

	IterateAndCreateOverlappingImageSubresourceTrackingState(image, range, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* callbackParameters = (CallbackParameters*)userData;

		GpuImageSubresourceTrackingState& subresourceTrackingState = callbackParameters->Self->mSubresourceTrackingState[globalSubresourceIndex];
		GpuResourceHazardState* const hazardState = subresourceTrackingState.HazardState;

		hazardState->RecordBarrier(callbackParameters->Barrier);
	}, &callbackParameters);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::NotifyUsed(GpuQueueId queueId)
{
	for(auto& entry : mResources)
	{
		GpuResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		if(useHandle.Flags == GpuAccessFlag::None)
			continue;

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		GpuImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		GpuResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		if(useHandle.Flags == GpuAccessFlag::None)
			continue;

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}

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

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::NotifyDone(GpuQueueId queueId)
{
	for(auto& entry : mResources)
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

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		GpuImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		GpuResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		if(useHandle.Flags == GpuAccessFlag::None)
		{
			B3D_ASSERT(!useHandle.Used);
			entry.first->NotifyUnbound();
			continue;
		}

		B3D_ASSERT(useHandle.Used);
		entry.first->NotifyDone(queueId, useHandle.Flags);
	}

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

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::NotifyUnbound()
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

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::Clear()
{
	for(auto& entry : mBuffers)
	{
		if(entry.second.HazardState != nullptr)
			mHazardStatePool.Destruct(entry.second.HazardState);
	}

	for(auto& entry : mSubresourceTrackingState)
	{
		if(entry.HazardState != nullptr)
			mHazardStatePool.Destruct(entry.HazardState);
	}

	// Drop deferred registrations before destructing the hazard states they point at.
	mPendingHazardRegistrations.clear();

	mResources.clear();
	mImages.clear();
	mBuffers.clear();
	mSwapChains.clear();
	mImageTrackingState.clear();
	mSubresourceTrackingState.clear();
	mPendingRenderPassAttachments.Clear();
	mActiveRenderPassAttachments.Clear();
	mAttachmentsNeedingAccess = 0;
	mRenderPassTrackingPhase = RenderPassTrackingPhase::Inactive;
	mEpoch = 1;
}

	} // namespace render
} // namespace b3d
