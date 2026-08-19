//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuBackendTestSuite.h"
#include "GpuBackend/B3DGpuHazards.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuResourceTracker.h"
#include "GpuBackend/B3DGpuResourceTracker.inl"

using namespace b3d;
using namespace b3d::render;

namespace
{
	struct SubmissionTestBarrierHelper { };

	class SubmissionTestBuffer : public IGpuBufferResource
	{
	public:
		SubmissionTestBuffer() = default;
	};

	class SubmissionTestVisitor : public GpuSubmissionTransitionVisitor
	{
	public:
		void VisitBuffer(const GpuSubmissionBufferTransition& transition) override
		{
			ParallelAccessWaitMask = transition.ParallelAccessWaitMask;
			ExclusiveAccessWaitMask = transition.ExclusiveAccessWaitMask;
			MemoryBarrier = transition.MemoryBarrier;
			ExecutionBarrier = transition.ExecutionBarrier;
		}

		void VisitImage(const GpuSubmissionImageTransition&) override
		{
			B3D_ASSERT(false);
		}

		GpuQueueMask ParallelAccessWaitMask = GpuQueueMask::kNone;
		GpuQueueMask ExclusiveAccessWaitMask = GpuQueueMask::kNone;
		GpuBarrierScope MemoryBarrier;
		GpuBarrierScope ExecutionBarrier;
	};

	struct SubmissionTestResult
	{
		GpuQueueMask ParallelAccessWaitMask;
		GpuQueueMask ExclusiveAccessWaitMask;
		GpuBarrierScope MemoryBarrier;
		GpuBarrierScope ExecutionBarrier;
	};

	template<class THazardState>
	GpuBarrierScope ResolveTestAccess(THazardState& state, GpuStageFlags stages, GpuAccessFlags access,
		GpuStageFlags broadenedReadStages = GpuStageFlag::None)
	{
		const GpuBarrierScope barrier = state.GetRequiredBarrier(stages, access, broadenedReadStages);
		state.RecordBarrier(barrier);
		state.RecordAccess(stages, access);
		return barrier;
	}

	SubmissionTestResult ResolveTestSubmission(SubmissionTestBuffer& buffer, GpuQueueId queueId, GpuStageFlags stages, GpuAccessFlags access)
	{
		GpuResourceHazardState hazardState;
		ResolveTestAccess(hazardState, stages, access);

		TGpuResourceTracker<SubmissionTestBarrierHelper> tracker;
		GpuBufferTrackingState trackingState;
		trackingState.HazardState = &hazardState;
		tracker.GetBuffers().insert(std::make_pair(&buffer, trackingState));

		SubmissionTestVisitor visitor;
		tracker.ResolveSubmissionTransitions(queueId, visitor);

		SubmissionTestResult result;
		result.ParallelAccessWaitMask = visitor.ParallelAccessWaitMask;
		result.ExclusiveAccessWaitMask = visitor.ExclusiveAccessWaitMask;
		result.MemoryBarrier = visitor.MemoryBarrier;
		result.ExecutionBarrier = visitor.ExecutionBarrier;
		return result;
	}

	void BeginTestRead(SubmissionTestBuffer& buffer, GpuQueueId queueId)
	{
		buffer.NotifyBound();
		buffer.NotifyUsed(queueId, GpuAccessFlag::Read);
	}

	void EndTestRead(SubmissionTestBuffer& buffer, GpuQueueId queueId)
	{
		buffer.NotifyDone(queueId, GpuAccessFlag::Read);
	}
}

GpuBackendTestSuite::GpuBackendTestSuite()
	: TestSuite("GpuBackendTestSuite")
{
	B3D_ADD_TEST(GpuBackendTestSuite::TestResourceHazardState)
	B3D_ADD_TEST(GpuBackendTestSuite::TestResourceTransition)
	B3D_ADD_TEST(GpuBackendTestSuite::TestSubmissionTransitionPlanning)
}

void GpuBackendTestSuite::TestResourceHazardState()
{
	GpuResourceWriteEpochHazardState state;
	B3D_TEST_ASSERT(!ResolveTestAccess(state, GpuStageFlag::Transfer, GpuAccessFlag::Write).IsValid())
	B3D_TEST_ASSERT(state.WriteStages == GpuStageFlag::Transfer)

	const GpuBarrierScope fragmentRead = ResolveTestAccess(state, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(fragmentRead.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(fragmentRead.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(fragmentRead.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(fragmentRead.DestinationAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(state.ReaderStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(state.VisibleStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(!ResolveTestAccess(state, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read).IsValid())

	const GpuBarrierScope computeRead = ResolveTestAccess(state, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(computeRead.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(computeRead.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(state.ReaderStages == (GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform))
	B3D_TEST_ASSERT(state.VisibleStages == state.ReaderStages)

	const GpuBarrierScope colorWrite = ResolveTestAccess(state, GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(colorWrite.SourceStages == (GpuStageFlag::Transfer | GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform))
	B3D_TEST_ASSERT(colorWrite.SourceAccess == (GpuAccessFlag::Read | GpuAccessFlag::Write))
	B3D_TEST_ASSERT(colorWrite.DestinationStages == GpuStageFlag::ColorAttachment)
	B3D_TEST_ASSERT(colorWrite.DestinationAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(state.WriteStages == GpuStageFlag::ColorAttachment)
	B3D_TEST_ASSERT(state.ReaderStages == GpuStageFlag::None)
	B3D_TEST_ASSERT(state.VisibleStages == GpuStageFlag::None)

	const GpuBarrierScope readAfterWrite = ResolveTestAccess(state, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(readAfterWrite.SourceStages == GpuStageFlag::ColorAttachment)

	GpuResourceWriteEpochHazardState barrierState;
	ResolveTestAccess(barrierState, GpuStageFlag::Transfer, GpuAccessFlag::Write);
	barrierState.RecordBarrier(GpuBarrierScope(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write));
	B3D_TEST_ASSERT(barrierState.GetRequiredBarrier(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read).IsValid())
	barrierState.RecordBarrier(GpuBarrierScope(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read));
	B3D_TEST_ASSERT(!barrierState.GetRequiredBarrier(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read).IsValid())

	GpuResourceWriteEpochHazardState layoutTransitionState;
	ResolveTestAccess(layoutTransitionState, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	const GpuBarrierScope layoutTransitionDependency = layoutTransitionState.GetRequiredBarrier(
		GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read | GpuAccessFlag::Write);
	B3D_TEST_ASSERT(layoutTransitionDependency.SourceStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(layoutTransitionDependency.SourceAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(layoutTransitionDependency.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(layoutTransitionDependency.DestinationAccess == (GpuAccessFlag::Read | GpuAccessFlag::Write))

	GpuResourceHazardState commandHazardState;
	B3D_TEST_ASSERT(!ResolveTestAccess(commandHazardState, GpuStageFlag::Transfer, GpuAccessFlag::Read).IsValid())
	const GpuBarrierScope internalWriteBarrier = ResolveTestAccess(commandHazardState,
		GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(internalWriteBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(internalWriteBarrier.SourceAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(internalWriteBarrier.DestinationAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(commandHazardState.EntryReadStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(commandHazardState.FirstWriteStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(commandHazardState.AllAccessScope.ReadStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(commandHazardState.AllAccessScope.WriteStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(commandHazardState.LastWriteEpochHazardState.WriteStages ==
		GpuStageFlag::ComputeShaderNonUniform)

	GpuResourceHazardState leadingBarrierHazardState;
	leadingBarrierHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::Transfer, GpuAccessFlag::Write,
		GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read));
	leadingBarrierHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::VertexShaderNonUniform,
		GpuAccessFlag::Write, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	B3D_TEST_ASSERT(leadingBarrierHazardState.LeadingBarriers.Size() == 2)
	B3D_TEST_ASSERT(leadingBarrierHazardState.LastBarrier.SourceStages == GpuStageFlag::VertexShaderNonUniform)
	B3D_TEST_ASSERT(leadingBarrierHazardState.LastBarrier.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(leadingBarrierHazardState.LastBarrier.DestinationAccess == GpuAccessFlag::Read)

	GpuResourceWriteEpochHazardState shaderWriteChainState;
	B3D_TEST_ASSERT(!ResolveTestAccess(shaderWriteChainState,
		GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write).IsValid())

	const GpuBarrierScope fragmentWriteBarrier = ResolveTestAccess(shaderWriteChainState,
		GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(fragmentWriteBarrier.SourceStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(fragmentWriteBarrier.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(fragmentWriteBarrier.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(fragmentWriteBarrier.DestinationAccess == GpuAccessFlag::Write)

	const GpuBarrierScope vertexReadBarrier = ResolveTestAccess(shaderWriteChainState,
		GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(vertexReadBarrier.SourceStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(vertexReadBarrier.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(vertexReadBarrier.DestinationStages == GpuStageFlag::VertexShaderNonUniform)
	B3D_TEST_ASSERT(vertexReadBarrier.DestinationAccess == GpuAccessFlag::Read)

	GpuResourceWriteEpochHazardState resolveWriteState;
	B3D_TEST_ASSERT(!ResolveTestAccess(resolveWriteState, GpuStageFlag::Resolve, GpuAccessFlag::Write).IsValid())
	const GpuBarrierScope resolveReadBarrier = ResolveTestAccess(resolveWriteState, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(resolveReadBarrier.SourceStages == GpuStageFlag::Resolve)
	B3D_TEST_ASSERT(resolveReadBarrier.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(resolveReadBarrier.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(resolveReadBarrier.DestinationAccess == GpuAccessFlag::Read)
}

void GpuBackendTestSuite::TestResourceTransition()
{
	const GpuQueueId sourceQueueId(GQT_GRAPHICS, 0);
	SubmissionTestBuffer buffer;

	auto fnSetWriterState = [&buffer, sourceQueueId](const GpuResourceWriteEpochHazardState& writerHazards)
	{
		GpuResourceSubmissionState submissionState;
		submissionState.WriterHazards = writerHazards;
		submissionState.WriterQueueId = sourceQueueId;
		submissionState.AcquiredQueues = sourceQueueId;
		submissionState.HasWriter = true;
		buffer.SetSubmissionState(std::move(submissionState));
	};

	GpuResourceWriteEpochHazardState sourceWriteEpochHazardState;
	ResolveTestAccess(sourceWriteEpochHazardState, GpuStageFlag::Transfer, GpuAccessFlag::Write);
	fnSetWriterState(sourceWriteEpochHazardState);

	GpuResourceHazardState fragmentReadHazardState;
	ResolveTestAccess(fragmentReadHazardState, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);

	const GpuSubmissionTransition fragmentReadTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, fragmentReadHazardState);
	B3D_TEST_ASSERT(fragmentReadTransition.StateResource == &buffer)
	B3D_TEST_ASSERT(fragmentReadTransition.MemoryBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(fragmentReadTransition.MemoryBarrier.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(fragmentReadTransition.MemoryBarrier.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(fragmentReadTransition.MemoryBarrier.DestinationAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(!fragmentReadTransition.ExecutionBarrier.IsValid())
	B3D_TEST_ASSERT(fragmentReadTransition.PostTransitionSubmissionState.WriterHazards.VisibleStages ==
		GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(fragmentReadTransition.PostTransitionSubmissionState.WriterHazards.ReaderStages ==
		GpuStageFlag::FragmentShaderNonUniform)

	GpuResourceHazardState computeReadHazardState;
	ResolveTestAccess(computeReadHazardState, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);

	GpuResourceSubmissionState fragmentReadPostState = fragmentReadTransition.PostTransitionSubmissionState;
	buffer.SetSubmissionState(std::move(fragmentReadPostState));
	const GpuSubmissionTransition computeReadTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, computeReadHazardState);
	B3D_TEST_ASSERT(computeReadTransition.MemoryBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(computeReadTransition.MemoryBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)

	fnSetWriterState(sourceWriteEpochHazardState);
	GpuResourceHazardState readThenWriteHazardState;
	ResolveTestAccess(readThenWriteHazardState, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	ResolveTestAccess(readThenWriteHazardState, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Write);

	const GpuSubmissionTransition readThenWriteTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, readThenWriteHazardState);
	B3D_TEST_ASSERT(readThenWriteTransition.MemoryBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(readThenWriteTransition.MemoryBarrier.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(readThenWriteTransition.MemoryBarrier.DestinationStages ==
		(GpuStageFlag::ComputeShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform))
	B3D_TEST_ASSERT(readThenWriteTransition.MemoryBarrier.DestinationAccess ==
		(GpuAccessFlag::Read | GpuAccessFlag::Write))
	B3D_TEST_ASSERT(!readThenWriteTransition.ExecutionBarrier.IsValid())
	B3D_TEST_ASSERT(readThenWriteTransition.PostTransitionSubmissionState.WriterHazards.WriteStages ==
		GpuStageFlag::FragmentShaderNonUniform)

	GpuResourceWriteEpochHazardState sourceWithReader = sourceWriteEpochHazardState;
	ResolveTestAccess(sourceWithReader, GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Read);
	fnSetWriterState(sourceWithReader);
	GpuResourceHazardState writeHazardState;
	ResolveTestAccess(writeHazardState, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
	const GpuSubmissionTransition writeTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, writeHazardState);
	B3D_TEST_ASSERT(writeTransition.MemoryBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(writeTransition.MemoryBarrier.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(writeTransition.ExecutionBarrier.SourceStages == GpuStageFlag::VertexShaderNonUniform)
	B3D_TEST_ASSERT(writeTransition.ExecutionBarrier.SourceAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(writeTransition.MemoryBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(writeTransition.ExecutionBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)

	fnSetWriterState(sourceWriteEpochHazardState);
	GpuResourceHazardState exactLeadingBarrierHazardState;
	exactLeadingBarrierHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::Transfer,
		GpuAccessFlag::Write, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	ResolveTestAccess(exactLeadingBarrierHazardState, GpuStageFlag::FragmentShaderNonUniform,
		GpuAccessFlag::Read);
	const GpuSubmissionTransition exactLeadingBarrierTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, exactLeadingBarrierHazardState);
	B3D_TEST_ASSERT(!exactLeadingBarrierTransition.MemoryBarrier.IsValid())

	fnSetWriterState(sourceWriteEpochHazardState);
	GpuResourceHazardState leadingBarrierOnlyHazardState;
	leadingBarrierOnlyHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::Transfer,
		GpuAccessFlag::Write, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	const GpuSubmissionTransition leadingBarrierOnlyTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, leadingBarrierOnlyHazardState);
	B3D_TEST_ASSERT(leadingBarrierOnlyHazardState.HasSubmissionEffect())
	B3D_TEST_ASSERT(leadingBarrierOnlyTransition.PostTransitionSubmissionState.WriterHazards.VisibleStages ==
		GpuStageFlag::FragmentShaderNonUniform)

	fnSetWriterState(sourceWriteEpochHazardState);
	GpuResourceHazardState mismatchedLeadingBarriersHazardState;
	mismatchedLeadingBarriersHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::Transfer,
		GpuAccessFlag::Write, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read));
	mismatchedLeadingBarriersHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::VertexShaderNonUniform,
		GpuAccessFlag::Write, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	ResolveTestAccess(mismatchedLeadingBarriersHazardState, GpuStageFlag::FragmentShaderNonUniform,
		GpuAccessFlag::Read);
	const GpuSubmissionTransition mismatchedLeadingBarriersTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, mismatchedLeadingBarriersHazardState);
	B3D_TEST_ASSERT(mismatchedLeadingBarriersTransition.MemoryBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(mismatchedLeadingBarriersTransition.MemoryBarrier.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)
}

void GpuBackendTestSuite::TestSubmissionTransitionPlanning()
{
	B3D_TEST_ASSERT(GpuBackendUtility::GetStageFlags(GpuResourceUseFlag::Host) == GpuStageFlag::Host)
	B3D_TEST_ASSERT(GpuBackendUtility::GetStageFlags(GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageVertexShader) == GpuStageFlag::VertexShaderNonUniform)
	B3D_TEST_ASSERT(GpuBackendUtility::GetStageFlags(GpuResourceUseFlag::Resolve) == GpuStageFlag::Resolve)
	B3D_TEST_ASSERT(String(GpuBackendUtility::GetAccessStageName(GpuStageFlag::Resolve)) == "Resolve")
	B3D_TEST_ASSERT(String(GpuBackendUtility::GetImageLayoutName(GpuImageLayout::ResolveSource)) == "ResolveSource")
	B3D_TEST_ASSERT(String(GpuBackendUtility::GetImageLayoutName(GpuImageLayout::ResolveDestination)) == "ResolveDestination")

	const GpuQueueId writerQueue(GQT_GRAPHICS, 0);
	const GpuQueueId firstReaderQueue(GQT_COMPUTE, 0);
	const GpuQueueId secondReaderQueue(GQT_TRANSFER, 0);
	const GpuQueueId nextWriterQueue(GQT_GRAPHICS, 1);
	SubmissionTestBuffer buffer;

	const SubmissionTestResult initialWrite = ResolveTestSubmission(buffer, writerQueue, GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(initialWrite.ParallelAccessWaitMask.IsEmpty())
	B3D_TEST_ASSERT(buffer.GetSubmissionState().HasWriter)
	B3D_TEST_ASSERT(buffer.GetSubmissionState().WriterQueueId.Id == writerQueue.Id)

	const SubmissionTestResult firstRead = ResolveTestSubmission(buffer, firstReaderQueue, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(firstRead.ParallelAccessWaitMask == GpuQueueMask(writerQueue))
	BeginTestRead(buffer, firstReaderQueue);

	const SubmissionTestResult repeatedRead = ResolveTestSubmission(buffer, firstReaderQueue, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(repeatedRead.ParallelAccessWaitMask.IsEmpty())
	BeginTestRead(buffer, firstReaderQueue);

	const SubmissionTestResult parallelRead = ResolveTestSubmission(buffer, secondReaderQueue, GpuStageFlag::Transfer, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(parallelRead.ParallelAccessWaitMask == GpuQueueMask(writerQueue))
	B3D_TEST_ASSERT(!parallelRead.ParallelAccessWaitMask.IsSet(firstReaderQueue))
	BeginTestRead(buffer, secondReaderQueue);

	const SubmissionTestResult nextWrite = ResolveTestSubmission(buffer, nextWriterQueue, GpuStageFlag::Transfer, GpuAccessFlag::Write);
	const GpuQueueMask readerMask = GpuQueueMask(firstReaderQueue) | GpuQueueMask(secondReaderQueue);
	B3D_TEST_ASSERT(nextWrite.ParallelAccessWaitMask == readerMask)
	B3D_TEST_ASSERT(nextWrite.ExclusiveAccessWaitMask == readerMask)
	B3D_TEST_ASSERT(!nextWrite.ParallelAccessWaitMask.IsSet(writerQueue))

	EndTestRead(buffer, firstReaderQueue);
	EndTestRead(buffer, firstReaderQueue);
	EndTestRead(buffer, secondReaderQueue);

	const SubmissionTestResult readAfterNewWrite = ResolveTestSubmission(buffer, firstReaderQueue, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(readAfterNewWrite.ParallelAccessWaitMask == GpuQueueMask(nextWriterQueue))

	SubmissionTestBuffer readOnlyBuffer;
	ResolveTestSubmission(readOnlyBuffer, firstReaderQueue, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	BeginTestRead(readOnlyBuffer, firstReaderQueue);

	// A late-bound point that resolves to NOP must leave the ordinary read/read submission state untouched.
	const SubmissionTestResult parallelNopRead = ResolveTestSubmission(readOnlyBuffer, secondReaderQueue,
		GpuStageFlag::Transfer, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(parallelNopRead.ParallelAccessWaitMask.IsEmpty())
	B3D_TEST_ASSERT(parallelNopRead.ExclusiveAccessWaitMask == GpuQueueMask(firstReaderQueue))
	BeginTestRead(readOnlyBuffer, secondReaderQueue);

	// A selected metadata rewrite is modelled with this same synthetic write hazard. It must drain a prior reader on
	// its own queue and wait for readers on other queues, while never trying to wait on its own fence.
	const SubmissionTestResult sameQueueWrite = ResolveTestSubmission(readOnlyBuffer, firstReaderQueue, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(sameQueueWrite.ParallelAccessWaitMask == GpuQueueMask(secondReaderQueue))
	B3D_TEST_ASSERT(sameQueueWrite.ExclusiveAccessWaitMask == GpuQueueMask(secondReaderQueue))
	B3D_TEST_ASSERT(!sameQueueWrite.ExclusiveAccessWaitMask.IsSet(firstReaderQueue))
	B3D_TEST_ASSERT(sameQueueWrite.ExecutionBarrier.IsValid())
	B3D_TEST_ASSERT(sameQueueWrite.ExecutionBarrier.SourceAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(!sameQueueWrite.MemoryBarrier.IsValid())

	// Publishing the selected rewrite as the latest writer makes a later reader acquire it even though the recording
	// that selected the rewrite may otherwise have contained only reads.
	const SubmissionTestResult readAfterSyntheticWrite = ResolveTestSubmission(readOnlyBuffer, secondReaderQueue,
		GpuStageFlag::Transfer, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(readAfterSyntheticWrite.ParallelAccessWaitMask == GpuQueueMask(firstReaderQueue))

	EndTestRead(readOnlyBuffer, firstReaderQueue);
	EndTestRead(readOnlyBuffer, secondReaderQueue);

	// Reads after a synthetic writer remain visible as a reader branch. A later write on another queue must therefore
	// wait for the queue that performed both the metadata rewrite and the following ordinary read.
	SubmissionTestBuffer rewriteThenReadBuffer;
	ResolveTestSubmission(rewriteThenReadBuffer, firstReaderQueue, GpuStageFlag::Transfer, GpuAccessFlag::Write);
	ResolveTestSubmission(rewriteThenReadBuffer, firstReaderQueue, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	BeginTestRead(rewriteThenReadBuffer, firstReaderQueue);
	const SubmissionTestResult writeAfterSyntheticWriterAndReader = ResolveTestSubmission(rewriteThenReadBuffer,
		secondReaderQueue, GpuStageFlag::Transfer, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(writeAfterSyntheticWriterAndReader.ParallelAccessWaitMask == GpuQueueMask(firstReaderQueue))
	B3D_TEST_ASSERT(writeAfterSyntheticWriterAndReader.ExclusiveAccessWaitMask == GpuQueueMask(firstReaderQueue))
	EndTestRead(rewriteThenReadBuffer, firstReaderQueue);

	// Reusing the ordinary write transition also preserves both halves of a prior same-queue write epoch: RAW/WAW
	// memory ordering for the writer and WAR execution ordering for its subsequent readers.
	SubmissionTestBuffer writerAndReaderBuffer;
	ResolveTestSubmission(writerAndReaderBuffer, writerQueue, GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
	ResolveTestSubmission(writerAndReaderBuffer, writerQueue, GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Read);
	BeginTestRead(writerAndReaderBuffer, writerQueue);
	const SubmissionTestResult rewriteAfterWriterAndReader = ResolveTestSubmission(writerAndReaderBuffer, writerQueue,
		GpuStageFlag::Transfer, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(rewriteAfterWriterAndReader.ParallelAccessWaitMask.IsEmpty())
	B3D_TEST_ASSERT(rewriteAfterWriterAndReader.MemoryBarrier.SourceStages == GpuStageFlag::ColorAttachment)
	B3D_TEST_ASSERT(rewriteAfterWriterAndReader.MemoryBarrier.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(rewriteAfterWriterAndReader.ExecutionBarrier.SourceStages == GpuStageFlag::VertexShaderNonUniform)
	B3D_TEST_ASSERT(rewriteAfterWriterAndReader.ExecutionBarrier.SourceAccess == GpuAccessFlag::Read)
	EndTestRead(writerAndReaderBuffer, writerQueue);
}
