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
void TGpuResourceTracker<TDerived, TBarrierHelper>::ResolveSubmissionTransitions(GpuQueueId destinationQueueId, GpuSubmissionTransitionVisitor& visitor)
{
	for(const auto& entry : mBuffers)
	{
		IGpuBufferResource* const buffer = entry.first;
		const GpuBufferTrackingState& trackingState = entry.second;
		if(trackingState.HazardState == nullptr || !trackingState.HazardState->HasSubmissionEffect())
			continue;

		GpuSubmissionBufferTransition transition(*buffer, GpuSubmissionTransition::Build(*buffer, destinationQueueId, *trackingState.HazardState));
		visitor.VisitBuffer(transition);

		transition.StateResource->SetSubmissionState(std::move(transition.PostTransitionSubmissionState));
	}

	for(const auto& entry : mImages)
	{
		IGpuImageResource* const image = entry.first;
		const TArrayView<const GpuImageSubresourceTrackingState> trackingStates = GetSubresourceTrackingStatesForImage(image);

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
					IGpuResource* const subresource = image->GetSubresource(face, mipLevel, (GpuTextureAspectFlag)(u32)trackedRange.AspectMask);
					B3D_ASSERT(subresource != nullptr);

					const GpuResourceHazardState& hazards = GetDerived().ResolveImageSubmissionHazards(image, trackingState, *subresource);
					if(!hazards.HasSubmissionEffect())
						continue;

					// Selected meta-data work contributes to both subresource and parent-image use flags.
					if(trackingState.MetadataState != nullptr && hazards.HasAccess())
					{
						GpuAccessFlags access;
						if(hazards.AllAccessScope.ReadStages != GpuStageFlag::None)
							access |= GpuAccessFlag::Read;
						if(hazards.HasWrite())
							access |= GpuAccessFlag::Write;

						TrackResourceUsage(subresource, access);
						GetImageTrackingState(image).UseHandle.Flags |= access;
					}

					const GpuTextureSubresourceRange range(mipLevel, 1, face, 1, trackedRange.AspectMask);
					GpuSubmissionImageTransition transition(*image, range, trackingState.InitialLayout, trackingState.CurrentLayout, trackingState.SubmissionBarrierFlags, GpuSubmissionTransition::Build(*subresource, destinationQueueId, hazards));
					visitor.VisitImage(transition);

					transition.StateResource->SetSubmissionState(std::move(transition.PostTransitionSubmissionState));
				}
			}
		}
	}
}

template<class TDerived, class TBarrierHelper>
GpuBufferTrackingState& TGpuResourceTracker<TDerived, TBarrierHelper>::GetOrCreateBufferTrackingState(IGpuBufferResource* buffer)
{
	auto insertResult = mBuffers.insert(std::make_pair(buffer, GpuBufferTrackingState()));
	if(insertResult.second) // New element
	{
		GpuBufferTrackingState& bufferTrackingState = insertResult.first->second;

		bufferTrackingState.UseHandle.Used = false;
		bufferTrackingState.UseHandle.Flags = GpuAccessFlag::None;
		bufferTrackingState.HazardState = mHazardStatePool.Construct<GpuResourceHazardState>();

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
void TGpuResourceTracker<TDerived, TBarrierHelper>::QueueRequiredBufferBarrier(IGpuBufferResource* buffer, const GpuBufferTrackingState& bufferTrackingState, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, TBarrierHelper& barrierHelper)
{
	if(buffer == nullptr)
		return;

	const GpuBarrierScope requiredBarrier = bufferTrackingState.HazardState->GetRequiredBarrier(destinationStages, destinationAccess);
	if(requiredBarrier.IsValid())
		barrierHelper.QueueResolvedBufferBarrier(buffer, requiredBarrier);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackExplicitBufferBarrier(IGpuBufferResource* buffer, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, TBarrierHelper& barrierHelper)
{
	if(buffer == nullptr)
		return;

	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	if(!bufferTrackingState.HazardState->HasAccess())
	{
		bufferTrackingState.HazardState->HasLeadingBarrier = true;
		return;
	}

	QueueRequiredBufferBarrier(buffer, bufferTrackingState, destinationStages, destinationAccess, barrierHelper);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackBufferAccess(IGpuBufferResource* buffer, GpuBufferTrackingState& bufferTrackingState, GpuStageFlags stages, GpuAccessFlags access, TBarrierHelper& barrierHelper, u32 dynamicOffset)
{
	B3D_ASSERT(!bufferTrackingState.UseHandle.Used);

	QueueRequiredBufferBarrier(buffer, bufferTrackingState, stages, access, barrierHelper);

	GpuResourceHazardState* const hazardState = bufferTrackingState.HazardState;

	// Defer registering hazards until after the barrier is issued, as the barrier helper clears any hazards that have been set
	if(access.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		PendingHazardRegistration registration;
		registration.Resource = buffer;
		registration.State = hazardState;
		registration.AccessStageFlags = stages;
		registration.Access = access;

		mPendingHazardRegistrations.push_back(registration);
	}

	bufferTrackingState.UseHandle.Flags |= access;

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

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackBufferAccess(IGpuBufferResource* buffer, GpuStageFlags stages, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, u32 dynamicOffset)
{
	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
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
				subresourceTrackingState.CurrentLayout = callbackParameters->FinalLayout;
				subresourceTrackingState.RequiredLayout = callbackParameters->FinalLayout;
			}
		}, &callbackParameters);
	}

	mActiveRenderPassAttachments.Clear();
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
GpuImageLayout TGpuResourceTracker<TDerived, TBarrierHelper>::GetRequiredImageLayout(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange) const
{
	for(const PendingRenderPassAttachmentUsage& pendingAttachment : mPendingRenderPassAttachments)
	{
		if(pendingAttachment.Usage.Image == image && pendingAttachment.ShaderUseFlags.IsSet(GpuResourceUseFlag::ShaderAccess) && GpuBackendUtility::RangeOverlaps(pendingAttachment.Usage.Range, subresourceRange))
		{
			B3D_ASSERT(pendingAttachment.Usage.ShaderReadLayout.has_value());
			return pendingAttachment.Usage.ShaderReadLayout.value_or(pendingAttachment.Usage.Layout);
		}
	}

	const GpuResolvedRenderPassAttachmentUsage* const renderPassAttachment = FindRenderPassAttachment(image, subresourceRange);
	if(renderPassAttachment != nullptr)
		return renderPassAttachment->Layout;

	for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
	{
		if(subresourceRange.AspectMask.IsSet(aspect))
			return GetSubresourceTrackingState(image, subresourceRange.BaseArrayLayer, subresourceRange.BaseMipLevel, aspect).RequiredLayout;
	}

	B3D_ASSERT(false);
	return GpuImageLayout::Undefined;
}

template<class TDerived, class TBarrierHelper>
bool TGpuResourceTracker<TDerived, TBarrierHelper>::TrackShaderAndAttachmentAccesses(GpuShaderBindings& bindings, TBarrierHelper& barrierHelper)
{
	const bool changed = bindings.Changed;
	if(changed)
	{
		bindings.Finalize();

		for(const GpuShaderBindings::ImageBinding& binding : bindings.Images)
			if(!TrackImageUsage(binding.Image, binding.Range, binding.Layout, binding.Usage, binding.Access, barrierHelper, true))
				return false;

		for(const GpuShaderBindings::BufferBinding& binding : bindings.Buffers)
			TrackBufferAccess(binding.Buffer, GpuBackendUtility::GetStageFlags(binding.Usage), binding.Access, barrierHelper, binding.Offset);
	}
	else
	{
		const TArrayView<const u32> imageIndicesWithWriteAccess = bindings.GetImageIndicesWithWriteAccess();
		const TArrayView<const u32> bufferIndicesWithWriteAccess = bindings.GetBufferIndicesWithWriteAccess();

		// Any images or buffer write write access set for shader bindings needs to be re-applied, each draw/dispatch is assumed to perform the write
		for(u32 bindingIndex : imageIndicesWithWriteAccess)
		{
			const GpuShaderBindings::ImageBinding& binding = bindings.Images[bindingIndex];
			if(!TrackImageUsage(binding.Image, binding.Range, binding.Layout, binding.Usage, binding.Access, barrierHelper, false))
				return false;
		}

		for(u32 bindingIndex : bufferIndicesWithWriteAccess)
		{
			const GpuShaderBindings::BufferBinding& binding = bindings.Buffers[bindingIndex];
			TrackBufferAccess(binding.Buffer, GpuBackendUtility::GetStageFlags(binding.Usage), binding.Access, barrierHelper, binding.Offset);
		}

		// Certain images/buffer can be written due without having declared write access directly. This can be caused by meta-data transitions or an internal operations such as executing a clear operating using a compute shader
		if(mShaderBindingsNeedRefresh && mInvalidatedShaderBindingResources.size() != 0)
		{
			// TODO: This needs a more optimized approach, so we don't iterate every single binding
			for(u32 bindingIndex = 0; bindingIndex < (u32)bindings.Images.size(); bindingIndex++)
			{
				const GpuShaderBindings::ImageBinding& binding = bindings.Images[bindingIndex];
				if(std::find(mInvalidatedShaderBindingResources.begin(), mInvalidatedShaderBindingResources.end(), binding.Image) != mInvalidatedShaderBindingResources.end() && std::find(imageIndicesWithWriteAccess.begin(), imageIndicesWithWriteAccess.end(), bindingIndex) == imageIndicesWithWriteAccess.end())
					if(!TrackImageUsage(binding.Image, binding.Range, binding.Layout, binding.Usage, binding.Access, barrierHelper, false))
						return false;
			}

			for(u32 bindingIndex = 0; bindingIndex < (u32)bindings.Buffers.size(); bindingIndex++)
			{
				const GpuShaderBindings::BufferBinding& binding = bindings.Buffers[bindingIndex];
				if(std::find(mInvalidatedShaderBindingResources.begin(), mInvalidatedShaderBindingResources.end(), binding.Buffer) != mInvalidatedShaderBindingResources.end() && std::find(bufferIndicesWithWriteAccess.begin(), bufferIndicesWithWriteAccess.end(), bindingIndex) == bufferIndicesWithWriteAccess.end())
					TrackBufferAccess(binding.Buffer, GpuBackendUtility::GetStageFlags(binding.Usage), binding.Access, barrierHelper, binding.Offset);
			}
		}
	}

	// Restore attachment access after intervening operations.
	if(changed || mShaderBindingsNeedRefresh)
	{
		for(const GpuResolvedRenderPassAttachmentUsage& attachment : mActiveRenderPassAttachments)
		{
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

				TrackSubresourceUsage(attachment.Image, subresourceIndex, attachment.Layout, stages, attachment.Access, barrierHelper, GpuImageBarrierFlag::None);
			}
		}
	}

	mIsShaderBindingEpoch = true; // True until we execute the barrier
	bindings.Changed = false;

	return true;
}

template<class TDerived, class TBarrierHelper>
bool TGpuResourceTracker<TDerived, TBarrierHelper>::TrackImageUsage(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, bool addUsedImage)
{
	if(image == nullptr)
		return true;

	if(!useFlags.IsSet(GpuResourceUseFlag::ShaderAccess))
	{
		TrackImageAccess(image, subresourceRange, layout, GpuBackendUtility::GetStageFlags(useFlags), accessFlags, barrierHelper);
		return true;
	}

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
		self->TrackSubresourceUsage(callbackParameters->Image, globalSubresourceIndex, layout, GpuBackendUtility::GetStageFlags(useFlags), callbackParameters->AccessFlags, *callbackParameters->BarrierHelper, GpuImageBarrierFlag::None);

	}, &callbackParameters);

	if(callbackParameters.Valid && addUsedImage)
		RegisterImageSubresources(image, subresourceRange, accessFlags);

	return callbackParameters.Valid;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackImageAccess(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, GpuImageBarrierFlags barrierFlags, GpuImageTrackingFlags trackingFlags)
{
	if(image == nullptr)
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
		GpuImageBarrierFlags BarrierFlags;
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
	RegisterImageSubresources(image, subresourceRange, trackingFlags.IsSet(GpuImageTrackingFlag::MetadataOperation) ? GpuAccessFlags(GpuAccessFlag::Read) : accessFlags);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::RegisterImageSubresources(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuAccessFlags accessFlags)
{
	GpuImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);
	B3D_ASSERT(!imageTrackingState.UseHandle.Used);
	imageTrackingState.UseHandle.Flags |= accessFlags;

	// Register any sub-resources
	B3D_ASSERT(subresourceRange.ArrayLayerCount != ~0u);
	B3D_ASSERT(subresourceRange.MipLevelCount != ~0u);

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

	// TODO - This unconditional invalidation can probably be removed: layout changes and native meta-data writes already invalidate their resources. Revisit after the current review and retest explicit barriers before removing it.
	// A layout-only barrier can invalidate a cached descriptor access without registering a write.
	if(std::find(mInvalidatedShaderBindingResources.begin(), mInvalidatedShaderBindingResources.end(), image) == mInvalidatedShaderBindingResources.end())
		mInvalidatedShaderBindingResources.Add(image);

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

		callbackParameters->Tracker->GetDerived().QueueRequiredImageBarrier(callbackParameters->Image, subresourceTrackingState, callbackParameters->DestinationStages, callbackParameters->DestinationAccess, callbackParameters->DestinationLayout, *callbackParameters->BarrierHelper);
	}, &callbackParameters);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::QueueRequiredImageBarrier(IGpuImageResource* image, GpuImageSubresourceTrackingState& subresourceTrackingState, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, GpuImageLayout destinationLayout, TBarrierHelper& barrierHelper, GpuImageBarrierFlags barrierFlags, GpuImageTrackingFlags trackingFlags)
{
	if(image == nullptr)
		return;

	if(destinationLayout == GpuImageLayout::Undefined)
		destinationLayout = subresourceTrackingState.CurrentLayout;

	const bool needsLayoutTransition = subresourceTrackingState.CurrentLayout != destinationLayout || barrierFlags.IsSet(GpuImageBarrierFlag::DiscardContents);
	if(!subresourceTrackingState.HazardState->HasAccess())
	{
		if(needsLayoutTransition)
		{
			subresourceTrackingState.InitialLayout = destinationLayout;
			subresourceTrackingState.CurrentLayout = destinationLayout;
			subresourceTrackingState.RequiredLayout = destinationLayout;
		}

		return;
	}

	// A copy source can change layout without a write registration. Its cached shader access must be restored too.
	if(needsLayoutTransition && std::find(mInvalidatedShaderBindingResources.begin(), mInvalidatedShaderBindingResources.end(), image) == mInvalidatedShaderBindingResources.end())
		mInvalidatedShaderBindingResources.Add(image);

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
		// The synthetic write above only finds operations that must precede the transition. The native destination
		// scope describes the real access that consumes the image in its new layout.
		barrier.DestinationStages = destinationStages;
		barrier.DestinationAccess = destinationAccess;
	}

	barrierHelper.QueueResolvedImageBarrier(image, subresourceTrackingState.Range, barrier, subresourceTrackingState.CurrentLayout, destinationLayout, barrierFlags);
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::TrackSubresourceUsage(IGpuImageResource* image, u32 globalSubresourceIndex, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, GpuImageBarrierFlags barrierFlags, GpuImageTrackingFlags trackingFlags)
{
	GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState[globalSubresourceIndex];
	if(subresourceTrackingState.Access == GpuAccessFlag::None)
		subresourceTrackingState.SubmissionBarrierFlags |= barrierFlags;

	const bool hasLeadingBarrier = subresourceTrackingState.HazardState != nullptr && subresourceTrackingState.HazardState->HasLeadingBarrier;
	if(subresourceTrackingState.Access == GpuAccessFlag::None && !subresourceTrackingState.HazardState->HasAccess() && !hasLeadingBarrier) // New subresource
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
		registration.Resource = image;
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
	// Provide exact size as code below doesn't handle the "remaining" sentinel
	if(subresourceRange.ArrayLayerCount == ~0u)
		subresourceRange.ArrayLayerCount = image->GetRange().ArrayLayerCount;

	if(subresourceRange.MipLevelCount == ~0u)
		subresourceRange.MipLevelCount = image->GetRange().MipLevelCount;

	subresourceRange.AspectMask &= image->GetRange().AspectMask;
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
	{
		registration.State->RecordAccess(registration.AccessStageFlags, registration.Access);
		if(!mIsShaderBindingEpoch && registration.Access.IsSet(GpuAccessFlag::Write) && std::find(mInvalidatedShaderBindingResources.begin(), mInvalidatedShaderBindingResources.end(), registration.Resource) == mInvalidatedShaderBindingResources.end())
			mInvalidatedShaderBindingResources.Add(registration.Resource);
	}

	mPendingHazardRegistrations.clear();
	mEpoch++;
	mShaderBindingsNeedRefresh = !mIsShaderBindingEpoch;
	if(mIsShaderBindingEpoch)
		mInvalidatedShaderBindingResources.clear();

	mIsShaderBindingEpoch = false;
}

template<class TDerived, class TBarrierHelper>
void TGpuResourceTracker<TDerived, TBarrierHelper>::UpdateHazardStateAfterBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier)
{
	GpuBufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	GpuResourceHazardState* const hazardState = bufferTrackingState.HazardState;

	hazardState->RecordBarrier(barrier);
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
	mRenderPassTrackingPhase = RenderPassTrackingPhase::Inactive;
	mEpoch = 1;
	mShaderBindingsNeedRefresh = true;
	mIsShaderBindingEpoch = false;
	mInvalidatedShaderBindingResources.clear();
}

	} // namespace render
} // namespace b3d
