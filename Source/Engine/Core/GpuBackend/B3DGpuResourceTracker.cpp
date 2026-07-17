//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuResourceTracker.h"

#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Utility/B3DBitwise.h"
#include "Debug/B3DDebug.h"

using namespace b3d;
using namespace b3d::render;

GpuHazardStageState::GpuHazardStageState()
{
	// Everything is safe to access by default
	std::fill(SafeAccess.begin(), SafeAccess.end(), GpuStageFlag::All);
}

void GpuHazardStageState::ClearStageSafeAccess(GpuStageFlags stages)
{
	u32 stagesAsInteger = (u32)stages;
	while(stagesAsInteger != 0)
	{
		const u32 stageFlagIndex = Bitwise::LeastSignificantBit(stagesAsInteger);
		SafeAccess[stageFlagIndex] = GpuStageFlag::None;

		stagesAsInteger &= ~(1 << stageFlagIndex);
	}
}

void GpuHazardStageState::AddStageSafeAccess(GpuStageFlags sourceStages, GpuStageFlags destinationStages)
{
	u32 sourceStagesAsInteger = (u32)sourceStages;
	while(sourceStagesAsInteger != 0)
	{
		const u32 stageFlagIndex = Bitwise::LeastSignificantBit(sourceStagesAsInteger);
		SafeAccess[stageFlagIndex] |= destinationStages;

		sourceStagesAsInteger &= ~(1 << stageFlagIndex);
	}
}

bool GpuHazardStageState::IsAccessSafe(GpuStageFlags stages) const
{
	for(const auto& entry : SafeAccess)
	{
		if((entry & stages) != stages)
			return false;
	}

	return true;
}

GpuStageFlags GpuHazardStageState::GetUnsafeAccessStages(GpuStageFlags stages) const
{
	GpuStageFlags unsafeStages = GpuStageFlag::None;

	for(u32 stageFlagIndex = 0; stageFlagIndex < (u32)SafeAccess.size(); stageFlagIndex++)
	{
		if((SafeAccess[stageFlagIndex] & stages) != stages)
			unsafeStages |= (GpuStageFlag)(1 << stageFlagIndex);
	}

	return unsafeStages;
}

void GpuHazardStageState::LogUnsafeAccess(GpuStageFlags stages, GpuAccessFlags currentAccessType, GpuAccessFlags previousAccessType) const
{
	StringStream stream;
	for(u32 stageIndex = 0; stageIndex < (u32)SafeAccess.size(); stageIndex++)
	{
		const GpuStageFlags& safeStages = SafeAccess[stageIndex];

		if((safeStages & stages) != stages)
		{
			const GpuStageFlag accessStageFlags = (GpuStageFlag)(1 << stageIndex);

			stream << "A resource was previously " << (previousAccessType.IsSet(GpuAccessFlag::Write) ? "WRITTEN" : "READ") << " ";
			stream << "on stage [" << GpuBackendUtility::GetAccessStageName(accessStageFlags) << "], ";

			stream << "and it's now being accessed for ";
			stream << (currentAccessType.IsSet(GpuAccessFlag::Write) ? "WRITE" : "READ") << " on stage(s) [";

			GpuBackendUtility::GetAccessStageNames(stages, stream);

			stream << "] without a barrier being issued. Issue a barrier with correct usage between those two accesses.";
		}
	}

	B3D_LOG(Warning, LogRenderBackend, "{0}", stream.str());
}

void GpuHazardState::ClearSafeAccess(GpuStageFlags stages, GpuAccessFlags access)
{
	if(access.IsSet(GpuAccessFlag::Read))
		ExecutionBarrierTracking.ClearStageSafeAccess(stages);

	if(access.IsSet(GpuAccessFlag::Write))
		MemoryBarrierTracking.ClearStageSafeAccess(stages);
}

void GpuHazardState::AddSafeAccess(const GpuHazardStageAndAccess& barrier)
{
	const bool needsMemoryBarrier = barrier.SourceAccess.IsSet(GpuAccessFlag::Write);
	const bool needsExecutionBarrier = barrier.SourceAccess.IsSet(GpuAccessFlag::Read) && barrier.DestinationAccess.IsSet(GpuAccessFlag::Write) || needsMemoryBarrier;

	if(needsExecutionBarrier)
		ExecutionBarrierTracking.AddStageSafeAccess(barrier.SourceStages, barrier.DestinationStages);

	if(needsMemoryBarrier)
		MemoryBarrierTracking.AddStageSafeAccess(barrier.SourceStages, barrier.DestinationStages);
}

void GpuHazardState::Merge(const GpuHazardState& other)
{
	for(u32 stageIndex = 0; stageIndex < GpuHazardStageState::kPipelineStageCount; ++stageIndex)
	{
		ExecutionBarrierTracking.SafeAccess[stageIndex] &= other.ExecutionBarrierTracking.SafeAccess[stageIndex];
		MemoryBarrierTracking.SafeAccess[stageIndex] &= other.MemoryBarrierTracking.SafeAccess[stageIndex];
	}
}

GpuAccessScope GpuHazardStateWithHistory::GetFirstAccessScope() const
{
	for(const Epoch& epoch : mCompletedEpochs)
	{
		if(epoch.AccessScope.IsValid())
			return epoch.AccessScope;
	}

	return mCurrentAccessScope;
}

GpuHazardState::TransitionRecipe GpuHazardState::BuildTransitionRecipe(GpuQueueId sourceQueueId, const GpuHazardStateWithHistory& destinationCommandBufferHazards) const
{
	TransitionRecipe result;
	result.SourceQueueId = sourceQueueId;
	result.RemainingHazardState = *this;

	// Based on the provided access scope, determines which stages are unsafe to access with regards to the remaining hazards, 
	// and records necessary memory or execution barriers to make them safe. Then clears the hazards from the remaining hazards.
	auto fnGenerateBarriers = [&result](const GpuAccessScope& accessScope)
	{
		u32 stages = (u32)accessScope.GetStages();
		while(stages != 0)
		{
			const u32 stageIndex = Bitwise::LeastSignificantBit(stages);
			const GpuStageFlags destinationStage = (GpuStageFlag)(1 << stageIndex);
			const GpuAccessFlags destinationAccess = accessScope.GetAccess(destinationStage);

			const GpuStageFlags writeSourceStages = result.RemainingHazardState.MemoryBarrierTracking.GetUnsafeAccessStages(destinationStage);
			if(writeSourceStages != GpuStageFlag::None)
			{
				result.MemoryDependency.SourceStages |= writeSourceStages;
				result.MemoryDependency.SourceAccess |= GpuAccessFlag::Write;
				result.MemoryDependency.DestinationStages |= destinationStage;
				result.MemoryDependency.DestinationAccess |= destinationAccess;

				result.RemainingHazardState.AddSafeAccess(GpuHazardStageAndAccess(writeSourceStages, GpuAccessFlag::Write, destinationStage, destinationAccess));
			}

			if(destinationAccess.IsSet(GpuAccessFlag::Write))
			{
				const GpuStageFlags readSourceStages = result.RemainingHazardState.ExecutionBarrierTracking.GetUnsafeAccessStages(destinationStage);
				if(readSourceStages != GpuStageFlag::None)
				{
					result.ExecutionDependency.SourceStages |= readSourceStages;
					result.ExecutionDependency.SourceAccess |= GpuAccessFlag::Read;
					result.ExecutionDependency.DestinationStages |= destinationStage;
					result.ExecutionDependency.DestinationAccess |= GpuAccessFlag::Write;

					result.RemainingHazardState.AddSafeAccess(GpuHazardStageAndAccess(readSourceStages, GpuAccessFlag::Read, destinationStage, GpuAccessFlag::Write));
				}
			}

			stages &= ~(1 << stageIndex);
		}
	};

	// Clears remaining hazards based on inter-command-buffer barriers that were issued within the command buffer already.
	auto fnApplyExistingBarrier = [&result](const GpuHazardStageAndAccess& barrier)
	{
		// If an existing (already recorded) barrier is resolving some hazard we need to ensure we insert a dependency on the source command buffer's queue.
		// Normally this is done by if any new execution or memory barriers need to be issued, and this case covers the pre-existing barriers.
		if(barrier.SourceAccess.IsSet(GpuAccessFlag::Write))
		{
			const GpuStageFlags unsafeStages = result.RemainingHazardState.MemoryBarrierTracking.GetUnsafeAccessStages(barrier.DestinationStages) & barrier.SourceStages;
			result.RequiresCrossQueueDependency |= unsafeStages != GpuStageFlag::None;
		}

		if(barrier.SourceAccess.IsSet(GpuAccessFlag::Read) && barrier.DestinationAccess.IsSet(GpuAccessFlag::Write))
		{
			const GpuStageFlags unsafeStages = result.RemainingHazardState.ExecutionBarrierTracking.GetUnsafeAccessStages(barrier.DestinationStages) & barrier.SourceStages;
			result.RequiresCrossQueueDependency |= unsafeStages != GpuStageFlag::None;
		}

		result.RemainingHazardState.AddSafeAccess(barrier);
	};

	// Since the command buffer was recorded in isolation it had no knowledge of the hazards on the command buffer submitted before it.
	// So we need to replay all accesses and barriers in the order they were issued, against the hazards now that they are known. If an
	// existing inter-command-buffer barrier doesn't resolve the hazard, we must issue the barrier ourself before submitting the command buffer.
	for(const GpuHazardStateWithHistory::Epoch& epoch : destinationCommandBufferHazards.GetCompletedEpochs())
	{
		fnGenerateBarriers(epoch.AccessScope);
		fnApplyExistingBarrier(epoch.IssuedBarrier);
	}

	fnGenerateBarriers(destinationCommandBufferHazards.GetCurrentAccessScope());

	// TODO - When we call AddSafeAccess this clears the hazard. This is fine if all command buffers are on the same queue. It's also fine if source is on queue A and destination on queue B.
	// But in the cross-queue case, the hazard is only cleared from queue B's perspective. If we use the same resource on queue C, it will incorrectly think the hazard is gone. High level API
	// currently requires user to manually provide queue sync masks, so this is not relevant at the moment, but may be if we decide to fully rely on the automatic system.

	return result;
}

GpuResourceRemainingHazards::TransitionRecipe GpuResourceRemainingHazards::BuildTransitionRecipe(const GpuHazardStateWithHistory& destinationCommandBufferHazards, GpuQueueId destinationQueueId) const
{
	TransitionRecipe transitionRecipe;
	transitionRecipe.SourceUnsafeAccessScope = GetUnsafeAccessScope();
	transitionRecipe.SourceQueueMask = GetQueueMask();

	for(const PerQueueHazards& perQueueHazard : mEntries)
	{
		transitionRecipe.PerQueueTransitionRecipes.Add(perQueueHazard.HazardState.BuildTransitionRecipe(perQueueHazard.QueueId, destinationCommandBufferHazards));

		if(perQueueHazard.QueueId.Id == destinationQueueId.Id)
		{
			transitionRecipe.RemainingHazards.Add(perQueueHazard.QueueId, transitionRecipe.PerQueueTransitionRecipes.back().RemainingHazardState);
		}
		else
		{
			// A release/acquire makes the source queue's hazards safe only for this destination queue. Preserve the
			// original source state so another queue that later consumes the resource still waits for the producer.
			// TODO(gpu-hazards): Track per-destination queue perspectives to avoid conservatively reacquiring the same
			// source hazard on repeated submissions to destinationQueueId.
			transitionRecipe.RemainingHazards.Add(perQueueHazard.QueueId, perQueueHazard.HazardState);
		}
	}

	return transitionRecipe;
}

#if B3D_VERIFY_BARRIERS
void GpuHazardStateWithHistory::VerifySafeAccess(GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess) const
{
	// If this image has been previously used prevent read-after-write and write-after-read hazards
	if(destinationAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		// Read-after-write (and write-after-write, as little sense does that make)
		if(Access.IsSet(GpuAccessFlag::Write))
		{
			// Triggers if user did not issue a RAW memory barrier between a previous write and this usage (or did not specify all the relevant stages in the barrier)
			if(!State.MemoryBarrierTracking.IsAccessSafe(destinationAccessStageFlags))
			{
				State.MemoryBarrierTracking.LogUnsafeAccess(destinationAccessStageFlags, destinationAccess, GpuAccessFlag::Write);
				B3D_ENSURE(false);
			}
		}
	}

	// No need to check for write-after-read barrier for framebuffer as it only needs an execution dependency and that is already handled by the render pass
	if(destinationAccessStageFlags.IsSetAny(GpuStageFlag::ColorAttachment | GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests))
	{
		if(destinationAccess.IsSet(GpuAccessFlag::Write))
		{
			// Write-after-read
			if(Access.IsSet(GpuAccessFlag::Read))
			{
				// Triggers if user did not issue a WAR memory barrier between a previous write and this usage (or did not specify all the relevant stages in the barrier)
				if(!State.ExecutionBarrierTracking.IsAccessSafe(destinationAccessStageFlags))
				{
					State.ExecutionBarrierTracking.LogUnsafeAccess(destinationAccessStageFlags, GpuAccessFlag::Write, GpuAccessFlag::Read);
					B3D_ENSURE(false);
				}
			}
		}
	}
}
#endif
