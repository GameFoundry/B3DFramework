//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuHazards.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	struct WriteEpochTransition
	{
		GpuBarrierScope MemoryBarrier;
		GpuBarrierScope ExecutionBarrier;
		GpuResourceWriteEpochHazardState RemainingWriteEpochHazardState;
	};

	WriteEpochTransition BuildWriteEpochTransition(const GpuResourceWriteEpochHazardState& sourceWriteEpochHazardState, const GpuResourceHazardState& destinationHazardState)
	{
		WriteEpochTransition result;

		GpuResourceWriteEpochHazardState remainingWriteEpochHazardState = sourceWriteEpochHazardState;
		for(const GpuBarrierScope& leadingBarrier : destinationHazardState.LeadingBarriers)
			remainingWriteEpochHazardState.RecordBarrier(leadingBarrier);

		const GpuResourceWriteEpochHazardState boundaryWriteEpochHazardState = remainingWriteEpochHazardState;
		if(destinationHazardState.EntryReadStages != GpuStageFlag::None) // RAW
			result.MemoryBarrier = boundaryWriteEpochHazardState.GetRequiredBarrier(destinationHazardState.EntryReadStages, GpuAccessFlag::Read);

		if(destinationHazardState.HasWrite())
		{
			// WAW
			if(boundaryWriteEpochHazardState.WriteStages != GpuStageFlag::None)
			{
				result.MemoryBarrier.SourceStages |= boundaryWriteEpochHazardState.WriteStages;
				result.MemoryBarrier.SourceAccess |= GpuAccessFlag::Write;
				result.MemoryBarrier.DestinationStages |= destinationHazardState.FirstWriteStages;
				result.MemoryBarrier.DestinationAccess |= destinationHazardState.FirstWriteAccess;
			}

			// WAR
			if(boundaryWriteEpochHazardState.ReaderStages != GpuStageFlag::None)
			{
				result.ExecutionBarrier.SourceStages = boundaryWriteEpochHazardState.ReaderStages;
				result.ExecutionBarrier.SourceAccess = GpuAccessFlag::Read;
				result.ExecutionBarrier.DestinationStages = destinationHazardState.FirstWriteStages;
				result.ExecutionBarrier.DestinationAccess = destinationHazardState.FirstWriteAccess;
			}
		}

		if(destinationHazardState.HasWrite())
			result.RemainingWriteEpochHazardState = destinationHazardState.LastWriteEpochHazardState;
		else
		{
			if(destinationHazardState.EntryReadStages != GpuStageFlag::None)
			{
				remainingWriteEpochHazardState.RecordBarrier(result.MemoryBarrier);
				remainingWriteEpochHazardState.RecordAccess(destinationHazardState.EntryReadStages, GpuAccessFlag::Read);
			}

			result.RemainingWriteEpochHazardState = remainingWriteEpochHazardState;
		}

		return result;
	}
}

GpuBarrierScope GpuResourceWriteEpochHazardState::GetRequiredBarrier(GpuStageFlags stages, GpuAccessFlags access,
	GpuStageFlags broadenedReadStages) const
{
	GpuBarrierScope barrier;

	// If the access is write, we need to wait for all prior writes and reads to complete before we can write.
	if(access.IsSet(GpuAccessFlag::Write))
	{
		barrier.SourceStages = WriteStages | ReaderStages;
		if(barrier.SourceStages == GpuStageFlag::None)
			return barrier;

		if(WriteStages != GpuStageFlag::None)
			barrier.SourceAccess |= GpuAccessFlag::Write;

		if(ReaderStages != GpuStageFlag::None)
			barrier.SourceAccess |= GpuAccessFlag::Read;

		barrier.DestinationStages = stages;
		barrier.DestinationAccess = access;
		return barrier;
	}

	// If the access is read, we need to wait for all prior writes to complete before we can read.
	if(access.IsSet(GpuAccessFlag::Read))
	{
		const GpuStageFlags missingStages = stages & ~VisibleStages;
		if(WriteStages == GpuStageFlag::None || missingStages == GpuStageFlag::None)
			return barrier;

		barrier.SourceStages = WriteStages;
		barrier.SourceAccess = GpuAccessFlag::Write;
		barrier.DestinationStages = missingStages | (broadenedReadStages & ~VisibleStages);
		barrier.DestinationAccess = GpuAccessFlag::Read;
	}

	return barrier;
}

void GpuResourceWriteEpochHazardState::RecordAccess(GpuStageFlags stages, GpuAccessFlags access)
{
	if(access.IsSet(GpuAccessFlag::Write))
	{
		WriteStages = stages;
		ReaderStages = GpuStageFlag::None;
		VisibleStages = GpuStageFlag::None;
		return;
	}

	ReaderStages |= stages;
}

void GpuResourceWriteEpochHazardState::RecordBarrier(const GpuBarrierScope& barrier)
{
	// Mark writes as visible to the destination stage if: 
	//  - Source access is write and destination is read (if destination is write, then a new epoch start and nothing is visible)
	//  - Barrier source stages cover all prior write stages
	if(barrier.SourceAccess.IsSet(GpuAccessFlag::Write) && barrier.DestinationAccess.IsSet(GpuAccessFlag::Read) && WriteStages != GpuStageFlag::None && (WriteStages & ~barrier.SourceStages) == GpuStageFlag::None)
		VisibleStages |= barrier.DestinationStages;
}

GpuBarrierScope GpuResourceHazardState::GetRequiredBarrier(GpuStageFlags stages, GpuAccessFlags access, GpuStageFlags broadenedReadStages) const
{
	return LastWriteEpochHazardState.GetRequiredBarrier(stages, access, broadenedReadStages);
}

void GpuResourceHazardState::RecordAccess(GpuStageFlags stages, GpuAccessFlags access)
{
	AllAccessScope.Add(stages, access);
	if(FirstWriteStages == GpuStageFlag::None)
	{
		if(access.IsSet(GpuAccessFlag::Write))
		{
			FirstWriteStages = stages;
			FirstWriteAccess = access;
		}
		else if(access.IsSet(GpuAccessFlag::Read))
			EntryReadStages |= stages;
	}

	LastWriteEpochHazardState.RecordAccess(stages, access);
}

void GpuResourceHazardState::RecordBarrier(const GpuBarrierScope& barrier)
{
	if(!HasAccess() && barrier.IsValid())
		LeadingBarriers.Add(barrier);

	LastWriteEpochHazardState.RecordBarrier(barrier);
	LastBarrier = barrier;
}

GpuAccessScope GpuResourceHazardState::GetFirstAccessScope() const
{
	GpuAccessScope scope;
	if(EntryReadStages != GpuStageFlag::None)
		scope.Add(EntryReadStages, GpuAccessFlag::Read);
	else if(HasWrite())
		scope.Add(FirstWriteStages, FirstWriteAccess);

	return scope;
}

GpuSubmissionTransition::GpuSubmissionTransition(IGpuResource& stateResource, const GpuAccessScope& destinationFirstAccessScope, const GpuAccessScope& destinationAllAccessScope)
	: StateResource(&stateResource), DestinationFirstAccessScope(destinationFirstAccessScope), DestinationAllAccessScope(destinationAllAccessScope)
{ }

GpuSubmissionTransition GpuSubmissionTransition::Build(IGpuResource& stateResource, GpuQueueId destinationQueueId, const GpuResourceHazardState& destinationHazardState)
{
	const GpuResourceSubmissionState& sourceState = stateResource.GetSubmissionState();
	const GpuAccessScope& destinationAllAccessScope = destinationHazardState.AllAccessScope;
	const bool destinationReads = destinationAllAccessScope.ReadStages != GpuStageFlag::None;
	const bool destinationWrites = destinationAllAccessScope.WriteStages != GpuStageFlag::None;
	const GpuQueueMask destinationQueueMask(destinationQueueId);

	// Backends must preserve a waitable progress point for the latest submission on every queue. Cross-queue dependencies remain
	// required after the source submission completes because its memory dependency must still be acquired by a destination queue.
	const GpuQueueMask activeReaderQueues = sourceState.ReaderQueues & stateResource.GetUseInfo(GpuAccessFlag::Read);

	GpuSubmissionTransition transition(stateResource, destinationHazardState.GetFirstAccessScope(), destinationAllAccessScope);
	transition.PostTransitionSubmissionState = sourceState;
	transition.SourceAccessScope = sourceState.GetUnsafeAccessScope();

	GpuResourceWriteEpochHazardState sameQueueWriteEpochHazardState;

	// If accessing from the same queue as the previous writer, use its complete write-epoch hazard state.
	if(sourceState.HasWriter && sourceState.WriterQueueId.Id == destinationQueueId.Id)
		sameQueueWriteEpochHazardState = sourceState.WriterHazards;

	// Full per-stage hazards are only retained for the writer queue. If this queue has outstanding reads and now writes,
	// patch the same-queue state with the conservative reader-stage union carried by the submission state.
	if(destinationWrites && activeReaderQueues.IsSet(destinationQueueId))
		sameQueueWriteEpochHazardState.ReaderStages |= sourceState.ReaderStages;

	const WriteEpochTransition writeEpochTransition = BuildWriteEpochTransition(sameQueueWriteEpochHazardState, destinationHazardState);
	transition.MemoryBarrier = writeEpochTransition.MemoryBarrier;
	transition.ExecutionBarrier = writeEpochTransition.ExecutionBarrier;

	// In some cases the backend needs exclusive access to the resource (e.g. layout transition, ownership transfer). For that case we
	// build a mask that includes all prior writes AND reads (meaning no parallel access allowed). The backend gets to choose which mask to use.
	transition.ExclusiveAccessWaitMask = activeReaderQueues;
	transition.ExclusiveAccessWaitMask &= ~destinationQueueMask;

	// If a reader we are waiting on has already waited on the writer (i.e. is in the acquired queue list), no need to wait on the writer explicitly.
	const bool writerCoveredByReader = !(transition.ExclusiveAccessWaitMask & sourceState.AcquiredQueues).IsEmpty();

	// Wait on the writer if: it exists, is not the same queue as the destination, is not already acquired by the destination, and is not already covered by a reader we are waiting on.
	if(sourceState.HasWriter && sourceState.WriterQueueId.Id != destinationQueueId.Id && !sourceState.AcquiredQueues.IsSet(destinationQueueId) && !writerCoveredByReader)
		transition.ExclusiveAccessWaitMask |= sourceState.WriterQueueId;

	// Ordinary access: If destination is writer we need to wait on all readers, if destination is reader we need to wait on the writer.
	if(destinationWrites)
		transition.ParallelAccessWaitMask = transition.ExclusiveAccessWaitMask;
	else if(destinationReads && sourceState.HasWriter && sourceState.WriterQueueId.Id != destinationQueueId.Id && !sourceState.AcquiredQueues.IsSet(destinationQueueId))
	{
		transition.ParallelAccessWaitMask |= sourceState.WriterQueueId;
	}

	if(destinationWrites)
	{
		transition.PostTransitionSubmissionState = GpuResourceSubmissionState();
		transition.PostTransitionSubmissionState.WriterHazards = destinationHazardState.LastWriteEpochHazardState;
		transition.PostTransitionSubmissionState.WriterQueueId = destinationQueueId;
		transition.PostTransitionSubmissionState.AcquiredQueues = destinationQueueId;
		transition.PostTransitionSubmissionState.HasWriter = true;
	}
	else
	{
		if(sourceState.HasWriter && sourceState.WriterQueueId.Id == destinationQueueId.Id)
			transition.PostTransitionSubmissionState.WriterHazards = writeEpochTransition.RemainingWriteEpochHazardState;

		if(sourceState.HasWriter && destinationReads)
			transition.PostTransitionSubmissionState.AcquiredQueues |= destinationQueueId;

		if(destinationReads)
		{
			transition.PostTransitionSubmissionState.ReaderQueues |= destinationQueueId;
			transition.PostTransitionSubmissionState.ReaderStages |= destinationAllAccessScope.ReadStages;
		}
	}

	return transition;
}
