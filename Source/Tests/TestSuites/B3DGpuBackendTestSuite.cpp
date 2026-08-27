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
	struct SubmissionTestBarrierHelper
	{
		void QueueResolvedBufferBarrier(IGpuBufferResource*, const GpuBarrierScope&) { }
		void QueueResolvedImageBarrier(IGpuImageResource*, const GpuTextureSubresourceRange&, const GpuBarrierScope&, GpuImageLayout, GpuImageLayout) { }
	};

	class SubmissionTestBuffer : public IGpuBufferResource
	{
	public:
		SubmissionTestBuffer() = default;
	};

	class SubmissionTestImageSubresource : public IGpuResource
	{
	public:
		SubmissionTestImageSubresource() = default;
	};

	class SubmissionTestImage : public IGpuImageResource
	{
	public:
		SubmissionTestImage(u32 faceCount, u32 mipLevelCount, GpuTextureAspectFlags aspects)
		{
			mFaceCount = faceCount;
			mMipLevelCount = mipLevelCount;
			mFullRange = GpuTextureSubresourceRange(0, mipLevelCount, 0, faceCount, aspects);

			const u32 subresourceCount = GetSubresourceCount();
			mSubresources = (IGpuResource**)B3DAllocate(sizeof(IGpuResource*) * subresourceCount);
			for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
			{
				if(!aspects.IsSet(aspect))
					continue;

				for(u32 mipLevel = 0; mipLevel < mipLevelCount; mipLevel++)
				{
					for(u32 face = 0; face < faceCount; face++)
						mSubresources[GetSubresourceIndex(face, mipLevel, aspect)] = B3DNew<SubmissionTestImageSubresource>();
				}
			}
		}

		~SubmissionTestImage() override
		{
			const u32 subresourceCount = GetSubresourceCount();
			for(u32 subresourceIndex = 0; subresourceIndex < subresourceCount; subresourceIndex++)
				B3DDelete(mSubresources[subresourceIndex]);
		}
	};

	class SubmissionImageTestVisitor : public GpuSubmissionTransitionVisitor
	{
	public:
		void VisitBuffer(const GpuSubmissionBufferTransition&) override { }

		void VisitImage(const GpuSubmissionImageTransition& transition) override
		{
			B3D_ASSERT(transition.ImageRange.HasSingleAspect());
			VisitedAspects |= transition.ImageRange.AspectMask;
			StateResources.Add(transition.StateResource);
		}

		GpuTextureAspectFlags VisitedAspects;
		TInlineArray<IGpuResource*, 2> StateResources;
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

	class SubmissionTestTracker : public TGpuResourceTracker<SubmissionTestBarrierHelper>
	{
	public:
		using TGpuResourceTracker::GetSubresourceTrackingState;
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
	B3D_ADD_TEST(GpuBackendTestSuite::TestImageAspectTracking)
	B3D_ADD_TEST(GpuBackendTestSuite::TestRenderPassResourceTracking)
}

void GpuBackendTestSuite::TestImageAspectTracking()
{
	const GpuTextureSubresourceRange depthRange(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
	const GpuTextureSubresourceRange stencilRange(0, 1, 0, 1, GpuTextureAspectFlag::Stencil);
	B3D_TEST_ASSERT(!GpuBackendUtility::RangeOverlaps(depthRange, stencilRange))

	SubmissionTestImage image(2, 2, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
	SubmissionTestBarrierHelper barrierHelper;
	SubmissionTestTracker tracker;

	const GpuTextureSubresourceRange fullRange(0, 2, 0, 2, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
	u32 fullRangeCallbackCount = 0;
	tracker.IterateAndCreateOverlappingImageSubresourceTrackingState(&image, fullRange, [](u32, void* userData)
	{
		(*(u32*)userData)++;
	}, &fullRangeCallbackCount);
	B3D_TEST_ASSERT(fullRangeCallbackCount == 2)

	const TArrayView<const GpuImageSubresourceTrackingState> initialPartitions = tracker.GetSubresourceTrackingStatesForImage(&image);
	B3D_TEST_ASSERT(initialPartitions.Size() == 2)
	for(const GpuImageSubresourceTrackingState& partition : initialPartitions)
		B3D_TEST_ASSERT(partition.Range.HasSingleAspect())

	u32 partialRangeCallbackCount = 0;
	tracker.IterateAndCreateOverlappingImageSubresourceTrackingState(&image, depthRange, [](u32, void* userData)
	{
		(*(u32*)userData)++;
	}, &partialRangeCallbackCount);
	B3D_TEST_ASSERT(partialRangeCallbackCount == 1)

	const TArrayView<const GpuImageSubresourceTrackingState> partialPartitions = tracker.GetSubresourceTrackingStatesForImage(&image);
	u32 stencilPartitionCount = 0;
	for(const GpuImageSubresourceTrackingState& partition : partialPartitions)
	{
		B3D_TEST_ASSERT(partition.Range.HasSingleAspect())
		if(partition.Range.AspectMask.IsSet(GpuTextureAspectFlag::Stencil))
			stencilPartitionCount++;
	}
	B3D_TEST_ASSERT(stencilPartitionCount == 1)

	tracker.TrackImageUsage(&image, depthRange, GpuImageLayout::ShaderReadOnly, GpuImageLayout::ShaderReadOnly,
		GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader, GpuAccessFlag::Read, barrierHelper);
	tracker.TrackImageUsage(&image, stencilRange, GpuImageLayout::DepthStencilAttachment, GpuImageLayout::DepthStencilAttachment,
		GpuResourceUseFlag::DepthStencilAttachment, GpuAccessFlag::Write, barrierHelper);
	tracker.CommitPendingHazardRegistrations();

	const GpuImageSubresourceTrackingState& depthState = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	const GpuImageSubresourceTrackingState& stencilState = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Stencil);
	B3D_TEST_ASSERT(depthState.Access == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(depthState.CurrentLayout == GpuImageLayout::ShaderReadOnly)
	B3D_TEST_ASSERT(stencilState.Access == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(stencilState.CurrentLayout == GpuImageLayout::DepthStencilAttachment)
	B3D_TEST_ASSERT(depthState.HazardState != stencilState.HazardState)

	SubmissionImageTestVisitor visitor;
	tracker.ResolveSubmissionTransitions(GpuQueueId(GQT_GRAPHICS, 0), visitor);
	B3D_TEST_ASSERT(visitor.VisitedAspects == (GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil))
	B3D_TEST_ASSERT(visitor.StateResources.Size() == 2)
	B3D_TEST_ASSERT(visitor.StateResources[0] != visitor.StateResources[1])

	const TArrayView<const GpuImageSubresourceTrackingState> finalPartitions = tracker.GetSubresourceTrackingStatesForImage(&image);
	for(const GpuImageSubresourceTrackingState& partition : finalPartitions)
		B3D_TEST_ASSERT(partition.Range.HasSingleAspect())

	tracker.NotifyUnbound();
	tracker.Clear();

	SubmissionTestImage combinedUseImage(1, 1, GpuTextureAspectFlag::Depth);
	SubmissionTestTracker combinedUseTracker;
	const GpuResourceUseFlags combinedUseFlags = GpuResourceUseFlag::DepthStencilAttachment |
		GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader;
	combinedUseTracker.TrackImageUsage(&combinedUseImage, depthRange, GpuImageLayout::DepthStencilReadOnly,
		GpuImageLayout::DepthStencilReadOnly, combinedUseFlags, GpuAccessFlag::Read, barrierHelper);
	combinedUseTracker.CommitPendingHazardRegistrations();

	const GpuImageSubresourceTrackingState& combinedUseState = combinedUseTracker.GetSubresourceTrackingState(
		&combinedUseImage, 0, 0, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(combinedUseState.ShaderUse == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(combinedUseState.FramebufferUse == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(combinedUseState.CurrentLayout == GpuImageLayout::DepthStencilReadOnly)
	B3D_TEST_ASSERT(combinedUseState.HazardState->AllAccessScope.ReadStages ==
		(GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests | GpuStageFlag::FragmentShaderNonUniform))

	combinedUseTracker.NotifyUnbound();
	combinedUseTracker.Clear();
}

void GpuBackendTestSuite::TestRenderPassResourceTracking()
{
	SubmissionTestImage image(2, 2, GpuTextureAspectFlag::Color);
	SubmissionTestBarrierHelper barrierHelper;
	SubmissionTestTracker tracker;

	const GpuTextureSubresourceRange attachmentRange(0, 1, 0, 1, GpuTextureAspectFlag::Color);
	GpuRenderPassAttachmentUsage attachmentUsage;
	attachmentUsage.Image = &image;
	attachmentUsage.Range = attachmentRange;
	attachmentUsage.UseFlags = GpuResourceUseFlag::ColorAttachment;
	attachmentUsage.Access = GpuAccessFlag::Read;
	attachmentUsage.Layout = GpuImageLayout::ColorAttachment;
	attachmentUsage.ShaderReadLayout = GpuImageLayout::ShaderReadOnly;
	attachmentUsage.FinalLayout = GpuImageLayout::TransferSource;

	TInlineArray<GpuRenderPassAttachmentUsage, 1> attachments;
	attachments.Add(attachmentUsage);
	tracker.PrepareRenderPass(attachments);

	const GpuTextureSubresourceRange fullRange(0, 2, 0, 2, GpuTextureAspectFlag::Color);
	const GpuResourceUseFlags shaderUse = GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader;
	tracker.TrackImageUsage(&image, fullRange, GpuImageLayout::ShaderReadOnly, GpuImageLayout::ShaderReadOnly,
		shaderUse, GpuAccessFlag::Read, barrierHelper);
	B3D_TEST_ASSERT(tracker.GetRequiredImageLayout(&image, attachmentRange) == GpuImageLayout::ShaderReadOnly)

	const TArrayView<const GpuResolvedRenderPassAttachmentUsage> resolvedAttachments = tracker.BeginRenderPass(barrierHelper);
	GpuResourceUseFlags combinedUse = shaderUse;
	combinedUse |= GpuResourceUseFlag::ColorAttachment;
	B3D_TEST_ASSERT(resolvedAttachments.Size() == 1)
	B3D_TEST_ASSERT(resolvedAttachments[0].UseFlags == combinedUse)
	B3D_TEST_ASSERT(resolvedAttachments[0].Access == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(resolvedAttachments[0].Layout == GpuImageLayout::ShaderReadOnly)
	B3D_TEST_ASSERT(resolvedAttachments[0].FinalLayout == GpuImageLayout::TransferSource)
	B3D_TEST_ASSERT(tracker.GetRequiredImageLayout(&image, attachmentRange) == GpuImageLayout::ShaderReadOnly)

	tracker.CommitPendingHazardRegistrations();
	const GpuImageSubresourceTrackingState& attachmentStateBeforeEnd = tracker.GetSubresourceTrackingState(
		&image, 0, 0, GpuTextureAspectFlag::Color);
	const GpuImageSubresourceTrackingState& sampledOnlyStateBeforeEnd = tracker.GetSubresourceTrackingState(
		&image, 1, 1, GpuTextureAspectFlag::Color);
	B3D_TEST_ASSERT(attachmentStateBeforeEnd.ShaderUse == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(attachmentStateBeforeEnd.FramebufferUse == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(attachmentStateBeforeEnd.RequiredLayout == GpuImageLayout::ShaderReadOnly)
	B3D_TEST_ASSERT(sampledOnlyStateBeforeEnd.ShaderUse == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(sampledOnlyStateBeforeEnd.FramebufferUse == GpuAccessFlag::None)

	tracker.EndRenderPass();
	const GpuImageSubresourceTrackingState& attachmentStateAfterEnd = tracker.GetSubresourceTrackingState(
		&image, 0, 0, GpuTextureAspectFlag::Color);
	const GpuImageSubresourceTrackingState& sampledOnlyStateAfterEnd = tracker.GetSubresourceTrackingState(
		&image, 1, 1, GpuTextureAspectFlag::Color);
	B3D_TEST_ASSERT(attachmentStateAfterEnd.ShaderUse == GpuAccessFlag::None)
	B3D_TEST_ASSERT(attachmentStateAfterEnd.FramebufferUse == GpuAccessFlag::None)
	B3D_TEST_ASSERT(attachmentStateAfterEnd.CurrentLayout == GpuImageLayout::TransferSource)
	B3D_TEST_ASSERT(sampledOnlyStateAfterEnd.ShaderUse == GpuAccessFlag::None)

	tracker.NotifyUnbound();
	tracker.Clear();
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
	B3D_TEST_ASSERT(commandHazardState.AccessScopeBeforeFirstBarrier.ReadStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(commandHazardState.AccessScopeBeforeFirstBarrier.WriteStages == GpuStageFlag::None)
	B3D_TEST_ASSERT(commandHazardState.AllAccessScope.ReadStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(commandHazardState.AllAccessScope.WriteStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(commandHazardState.LastWriteEpochHazardState.WriteStages ==
		GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(commandHazardState.GetSubmissionBarrierAccessScope().GetStages() == GpuStageFlag::Transfer)

	GpuResourceHazardState readChainHazardState;
	readChainHazardState.RecordAccess(GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Read);
	readChainHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Read,
		GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	readChainHazardState.RecordAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(!readChainHazardState.GetRequiredBarrier(
		GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read).IsValid())

	GpuResourceHazardState writeChainHazardState;
	B3D_TEST_ASSERT(!ResolveTestAccess(writeChainHazardState,
		GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write).IsValid())
	const GpuBarrierScope fragmentReadBarrier = ResolveTestAccess(writeChainHazardState,
		GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(fragmentReadBarrier.SourceStages == GpuStageFlag::ComputeShaderNonUniform)
	const GpuBarrierScope unchainedVertexReadBarrier = writeChainHazardState.GetRequiredBarrier(
		GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(unchainedVertexReadBarrier.SourceStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(unchainedVertexReadBarrier.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(unchainedVertexReadBarrier.DestinationStages == GpuStageFlag::VertexShaderNonUniform)
	B3D_TEST_ASSERT(unchainedVertexReadBarrier.DestinationAccess == GpuAccessFlag::Read)

	GpuResourceHazardState leadingBarrierHazardState;
	leadingBarrierHazardState.HasLeadingBarrier = true;
	B3D_TEST_ASSERT(leadingBarrierHazardState.HasLeadingBarrier)
	B3D_TEST_ASSERT(!leadingBarrierHazardState.GetSubmissionBarrierAccessScope().IsValid())
	B3D_TEST_ASSERT(leadingBarrierHazardState.LastBarrier.SourceStages == GpuStageFlag::None)
	B3D_TEST_ASSERT(leadingBarrierHazardState.LastBarrier.DestinationStages == GpuStageFlag::None)
	leadingBarrierHazardState.RecordAccess(GpuStageFlag::Transfer, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(leadingBarrierHazardState.GetSubmissionBarrierAccessScope().ReadStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(leadingBarrierHazardState.LastBarrier.DestinationStages == GpuStageFlag::None)

	const GpuBarrierScope postAccessBarrier(GpuStageFlag::Transfer, GpuAccessFlag::Read,
		GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
	leadingBarrierHazardState.RecordBarrier(postAccessBarrier);
	leadingBarrierHazardState.RecordAccess(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(leadingBarrierHazardState.LastBarrier.DestinationStages == GpuStageFlag::ColorAttachment)
	B3D_TEST_ASSERT(leadingBarrierHazardState.LastBarrier.DestinationAccess == GpuAccessFlag::Write)

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
	B3D_TEST_ASSERT(readThenWriteTransition.MemoryBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(readThenWriteTransition.MemoryBarrier.DestinationAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(!readThenWriteTransition.ExecutionBarrier.IsValid())
	B3D_TEST_ASSERT(readThenWriteTransition.PostTransitionSubmissionState.WriterHazards.WriteStages ==
		GpuStageFlag::FragmentShaderNonUniform)

	GpuResourceWriteEpochHazardState sourceWithReader = sourceWriteEpochHazardState;
	ResolveTestAccess(sourceWithReader, GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Read);
	fnSetWriterState(sourceWithReader);
	GpuResourceHazardState chainedWriteHazardState;
	ResolveTestAccess(chainedWriteHazardState, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	ResolveTestAccess(chainedWriteHazardState, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Write);
	const GpuSubmissionTransition chainedWriteTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, chainedWriteHazardState);
	B3D_TEST_ASSERT(chainedWriteTransition.ExecutionBarrier.SourceStages == GpuStageFlag::VertexShaderNonUniform)
	B3D_TEST_ASSERT(chainedWriteTransition.ExecutionBarrier.SourceAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(chainedWriteTransition.ExecutionBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(chainedWriteTransition.ExecutionBarrier.DestinationAccess == GpuAccessFlag::Read)

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
	GpuResourceHazardState chainedReadHazardState;
	ResolveTestAccess(chainedReadHazardState, GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Read);
	chainedReadHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Read,
		GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	chainedReadHazardState.RecordAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	const GpuSubmissionTransition chainedReadTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, chainedReadHazardState);
	B3D_TEST_ASSERT(chainedReadTransition.MemoryBarrier.DestinationStages == GpuStageFlag::VertexShaderNonUniform)
	B3D_TEST_ASSERT(chainedReadTransition.PostTransitionSubmissionState.WriterHazards.ReaderStages ==
		(GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform))

	fnSetWriterState(sourceWriteEpochHazardState);
	GpuResourceHazardState exactLeadingBarrierHazardState;
	exactLeadingBarrierHazardState.HasLeadingBarrier = true;
	ResolveTestAccess(exactLeadingBarrierHazardState, GpuStageFlag::FragmentShaderNonUniform,
		GpuAccessFlag::Read);
	const GpuSubmissionTransition exactLeadingBarrierTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, exactLeadingBarrierHazardState);
	B3D_TEST_ASSERT(exactLeadingBarrierTransition.MemoryBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(exactLeadingBarrierTransition.MemoryBarrier.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)

	fnSetWriterState(sourceWriteEpochHazardState);
	GpuResourceHazardState leadingBarrierOnlyHazardState;
	leadingBarrierOnlyHazardState.HasLeadingBarrier = true;
	const GpuSubmissionTransition leadingBarrierOnlyTransition =
		GpuSubmissionTransition::Build(buffer, sourceQueueId, leadingBarrierOnlyHazardState);
	B3D_TEST_ASSERT(leadingBarrierOnlyHazardState.HasSubmissionEffect())
	B3D_TEST_ASSERT(!leadingBarrierOnlyTransition.SubmissionBarrierAccessScope.IsValid())
	B3D_TEST_ASSERT(!leadingBarrierOnlyTransition.MemoryBarrier.IsValid())
	B3D_TEST_ASSERT(leadingBarrierOnlyTransition.PostTransitionSubmissionState.WriterHazards.VisibleStages ==
		GpuStageFlag::None)
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
