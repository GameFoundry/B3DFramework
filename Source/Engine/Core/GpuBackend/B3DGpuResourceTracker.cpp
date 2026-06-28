//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuResourceTracker.h"

#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Utility/B3DBitwise.h"
#include "Debug/B3DDebug.h"

using namespace b3d;
using namespace b3d::render;

GpuWriteHazardPipelineTracking::GpuWriteHazardPipelineTracking()
{
	// Everything is safe to access by default
	std::fill(SafeAccess.begin(), SafeAccess.end(), GpuStageFlag::All);
}

void GpuWriteHazardPipelineTracking::ClearStageSafeAccess(GpuStageFlags stages)
{
	u32 stagesAsInteger = (u32)stages;
	while(stagesAsInteger != 0)
	{
		const u32 stageFlagIndex = Bitwise::LeastSignificantBit(stagesAsInteger);
		SafeAccess[stageFlagIndex] = GpuStageFlag::None;

		stagesAsInteger &= ~(1 << stageFlagIndex);
	}
}

void GpuWriteHazardPipelineTracking::AddStageSafeAccess(GpuStageFlags sourceStages, GpuStageFlags destinationStages)
{
	u32 sourceStagesAsInteger = (u32)sourceStages;
	while(sourceStagesAsInteger != 0)
	{
		const u32 stageFlagIndex = Bitwise::LeastSignificantBit(sourceStagesAsInteger);
		SafeAccess[stageFlagIndex] |= destinationStages;

		sourceStagesAsInteger &= ~(1 << stageFlagIndex);
	}
}

bool GpuWriteHazardPipelineTracking::IsAccessSafe(GpuStageFlags stages) const
{
	for(const auto& entry : SafeAccess)
	{
		if((entry & stages) != stages)
			return false;
	}

	return true;
}

GpuStageFlags GpuWriteHazardPipelineTracking::GetUnsafeAccessStages(GpuStageFlags stages) const
{
	GpuStageFlags unsafeStages = GpuStageFlag::None;

	for(u32 stageFlagIndex = 0; stageFlagIndex < (u32)SafeAccess.size(); stageFlagIndex++)
	{
		if((SafeAccess[stageFlagIndex] & stages) != stages)
			unsafeStages |= (GpuStageFlag)(1 << stageFlagIndex);
	}

	return unsafeStages;
}

void GpuWriteHazardPipelineTracking::LogUnsafeAccess(GpuStageFlags stages, GpuAccessFlags currentAccessType, GpuAccessFlags previousAccessType) const
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

void GpuWriteHazardTracking::AddSafeAccess(GpuStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccess, GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess)
{
	const bool needsMemoryBarrier = sourceAccess.IsSet(GpuAccessFlag::Write);
	const bool needsExecutionBarrier = sourceAccess.IsSet(GpuAccessFlag::Read) && destinationAccess.IsSet(GpuAccessFlag::Write) || needsMemoryBarrier;

	if(needsExecutionBarrier)
		ExecutionBarrierTracking.AddStageSafeAccess(sourceAccessStageFlags, destinationAccessStageFlags);

	if(needsMemoryBarrier)
		MemoryBarrierTracking.AddStageSafeAccess(sourceAccessStageFlags, destinationAccessStageFlags);
}

#if B3D_VERIFY_BARRIERS
void GpuWriteHazardTracking::VerifySafeAccess(GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess) const
{
	// If this image has been previously used prevent read-after-write and write-after-read hazards
	if(destinationAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		// Read-after-write (and write-after-write, as little sense does that make)
		if(Access.IsSet(GpuAccessFlag::Write))
		{
			// Triggers if user did not issue a RAW memory barrier between a previous write and this usage (or did not specify all the relevant stages in the barrier)
			if(!MemoryBarrierTracking.IsAccessSafe(destinationAccessStageFlags))
			{
				MemoryBarrierTracking.LogUnsafeAccess(destinationAccessStageFlags, destinationAccess, GpuAccessFlag::Write);
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
				if(!ExecutionBarrierTracking.IsAccessSafe(destinationAccessStageFlags))
				{
					ExecutionBarrierTracking.LogUnsafeAccess(destinationAccessStageFlags, GpuAccessFlag::Write, GpuAccessFlag::Read);
					B3D_ENSURE(false);
				}
			}
		}
	}
}
#endif
