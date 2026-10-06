//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuExplicitBarrierValidator.h"

#if B3D_GPU_EXPLICIT_BARRIERS && B3D_BUILD_TYPE_DEVELOPMENT

#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

namespace b3d::render
{
	void GpuExplicitBarrierValidator::ValidateBarrier(IGpuBufferResource* buffer, const GpuExplicitBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split)
	{
		const char* const error = ApplyBarrier(mBuffers[buffer], barrier, phase, split, false);
		B3D_ENSURE_LOG(error == nullptr, "{0} Buffer: '{1}'.", error, buffer->GetDebugName());
	}

	void GpuExplicitBarrierValidator::ValidateBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, const GpuExplicitBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split)
	{
		const char* const error = ForEachSubresourceState(image, range, [&](ResourceState& state)
		{
			return ApplyBarrier(state, barrier, phase, split, true);
		});

		B3D_ENSURE_LOG(error == nullptr, "{0} Image: '{1}'.", error, image->GetDebugName());
	}

	void GpuExplicitBarrierValidator::ValidateAccess(IGpuBufferResource* buffer, GpuStageFlags stages, GpuAccessFlags access)
	{
		const char* const error = ApplyAccess(mBuffers[buffer], GpuImageLayout::Undefined, stages, access);
		B3D_ENSURE_LOG(error == nullptr, "{0} Buffer: '{1}'.", error, buffer->GetDebugName());
	}

	void GpuExplicitBarrierValidator::ValidateAccess(IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access)
	{
		const char* const error = ForEachSubresourceState(image, range, [&](ResourceState& state)
		{
			return ApplyAccess(state, layout, stages, access);
		});

		B3D_ENSURE_LOG(error == nullptr, "{0} Image: '{1}'.", error, image->GetDebugName());
	}

	void GpuExplicitBarrierValidator::Clear()
	{
		mBuffers.clear();
		mImages.clear();
	}

	const char* GpuExplicitBarrierValidator::ApplyBarrier(ResourceState& state, const GpuExplicitBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split, bool isImage)
	{
		const char* error = nullptr;
		const GpuAccessState& source = barrier.Source;
		if(state.PendingAcquire != nullptr)
		{
			// The release already checked the source
			if(phase != GpuBarrierPhase::Acquire || state.PendingAcquire != split)
				error = "An explicit barrier was recorded between the release and the acquire of a split barrier of the same resource.";
		}
		else if(barrier.Flags.IsSet(GpuBarrierFlag::AliasAcquire) && (state.IsDeclared || state.UsedAccess != GpuAccessFlag::None))
			error = "An alias acquire must precede every other use of the resource on the command buffer.";
		else if(isImage && state.IsDeclared && source.Layout != GpuImageLayout::Undefined && source.Layout != state.Declared.Layout)
			error = "The source layout of an explicit barrier differs from the destination layout of the previous barrier of the resource.";
		else if(!source.Stages.IsSetAll(state.UsedStages))
			error = "The source of an explicit barrier is missing stages that accessed the resource since the previous barrier.";
		else if(state.UsedAccess.IsSet(GpuAccessFlag::Write) && !source.Access.IsSet(GpuAccessFlag::Write))
			error = "The source of an explicit barrier is missing the writes of the resource since the previous barrier.";

		if(phase == GpuBarrierPhase::Release)
			state.PendingAcquire = split;
		else
		{
			state.PendingAcquire = nullptr;
			state.Declared = barrier.Destination;
			state.IsDeclared = true;
		}

		state.UsedStages = GpuStageFlag::None;
		state.UsedAccess = GpuAccessFlag::None;
		return error;
	}

	const char* GpuExplicitBarrierValidator::ApplyAccess(ResourceState& state, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access)
	{
		const char* error = nullptr;
		if(state.PendingAcquire != nullptr)
			error = "The resource was accessed between the release and the acquire of a split barrier.";
		else if(state.IsDeclared && !state.Declared.Stages.IsSetAll(stages))
			error = "The resource was accessed in stages that its last explicit barrier does not declare.";
		else if(state.IsDeclared && !state.Declared.Access.IsSetAll(access))
			error = "The resource was accessed with reads or writes that its last explicit barrier does not declare.";
		else if(state.IsDeclared && layout != GpuImageLayout::Undefined && layout != state.Declared.Layout)
			error = "The image was accessed in a different layout than its last explicit barrier declares.";

		state.UsedStages |= stages;
		state.UsedAccess |= access;
		return error;
	}

	template<class TFunction>
	const char* GpuExplicitBarrierValidator::ForEachSubresourceState(IGpuImageResource* image, const GpuTextureSubresourceRange& range, TFunction&& function)
	{
		const GpuTextureSubresourceRange& fullRange = image->GetRange();
		const GpuTextureSubresourceRange clampedRange = GpuBackendUtility::ClampRange(range, fullRange);

		Vector<ResourceState>& states = mImages[image];
		if(states.empty())
			states.resize(fullRange.MipLevelCount * fullRange.ArrayLayerCount * fullRange.GetAspectCount());

		const char* firstError = nullptr;
		u32 aspectIndex = 0;
		for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
		{
			if(!fullRange.AspectMask.IsSet(aspect))
				continue;

			if(clampedRange.AspectMask.IsSet(aspect))
			{
				for(u32 face = clampedRange.BaseArrayLayer; face < clampedRange.BaseArrayLayer + clampedRange.ArrayLayerCount; face++)
				{
					for(u32 mip = clampedRange.BaseMipLevel; mip < clampedRange.BaseMipLevel + clampedRange.MipLevelCount; mip++)
					{
						const u32 index = (aspectIndex * fullRange.ArrayLayerCount + face) * fullRange.MipLevelCount + mip;
						const char* const error = function(states[index]);
						if(firstError == nullptr)
							firstError = error;
					}
				}
			}

			aspectIndex++;
		}

		return firstError;
	}
} // namespace b3d::render

#endif
