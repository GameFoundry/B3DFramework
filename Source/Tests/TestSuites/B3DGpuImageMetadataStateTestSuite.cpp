//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuImageMetadataStateTestSuite.h"
#include "GpuBackend/B3DGpuResourceTracker.h"
#include "GpuBackend/B3DGpuBarrierHelper.h"
#include "GpuBackend/B3DGpuBarrierHelper.inl"
#include "Allocators/B3DFrameAllocator.h"
#include "GpuBackend/B3DGpuResourceTracker.inl"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** CPU resource carrying a synthetic native encoding. */
	class NativeTestSubresource : public IGpuResource
	{
	public:
		NativeTestSubresource() = default;
		bool Compressed = true;
	};

	/** Two possible native executions used to exercise the core's optional state contract. */
	class NativeTestState : public GpuImageMetadataState
	{
	public:
		NativeTestState() = default;
		TShared<GpuImageMetadataState> Clone() const override { return B3DMakeShared<NativeTestState>(*this); }

		GpuResourceHazardState Expanded;
		GpuResourceHazardState Compressed;
	};

	/** Two faces and two aspects, with native state only on depth. */
	class NativeTestImage : public IGpuImageResource
	{
	public:
		NativeTestImage()
		{
			mFaceCount = 2;
			mMipLevelCount = 1;
			mFullRange = GpuTextureSubresourceRange(0, 1, 0, 2, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
			mSubresources = (IGpuResource**)B3DAllocate(sizeof(IGpuResource*) * GetSubresourceCount());
			for(u32 subresourceIndex = 0; subresourceIndex < GetSubresourceCount(); subresourceIndex++)
				mSubresources[subresourceIndex] = B3DNew<NativeTestSubresource>();
		}
		~NativeTestImage() override
		{
			for(u32 subresourceIndex = 0; subresourceIndex < GetSubresourceCount(); subresourceIndex++)
				B3DDelete(mSubresources[subresourceIndex]);
		}

	};

	class NativeTestTracker;

	/** No native GPU commands are needed for tracker contract tests. */
	class NativeTestBarrierHelper : public TGpuBarrierHelper<NativeTestBarrierHelper, NativeTestTracker>
	{
	public:
		explicit NativeTestBarrierHelper(NativeTestTracker* tracker);
		void Execute();

	private:
		friend class TGpuBarrierHelper<NativeTestBarrierHelper, NativeTestTracker>;
		void RecordNativeBufferBarrier(IGpuBufferResource*, const GpuBarrierScope&) { }
		void RecordNativeImageBarrier(IGpuImageResource*, const GpuTextureSubresourceRange&, const GpuBarrierScope&, GpuImageLayout&, GpuImageLayout, GpuImageBarrierFlags) { }
	};

	/** Exposes mutable state for synthetic native command recording. */
	class NativeTestTracker : public TGpuResourceTracker<NativeTestTracker, NativeTestBarrierHelper>
	{
	public:
		NativeTestTracker() = default;
		using Base = TGpuResourceTracker<NativeTestTracker, NativeTestBarrierHelper>;
		using Base::GetSubresourceTrackingState;
		using Base::UpdateHazardStateAfterBarrier;

		void ResolveAndQueueImageBarrier(IGpuImageResource* image, GpuImageSubresourceTrackingState& state, GpuResourceUseFlags usage, GpuAccessFlags access, GpuImageLayout layout, NativeTestBarrierHelper& helper, GpuImageBarrierFlags flags = GpuImageBarrierFlag::None)
		{
			Requirements++;
			Base::ResolveAndQueueImageBarrier(image, state, usage, access, layout, helper, flags);
		}

		u32 Requirements = 0;

		void RecordImageAccess(IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuStageFlags stages, GpuAccessFlags access)
		{
			Base::RecordImageAccess(image, range, stages, access);
			for(GpuImageSubresourceTrackingState& trackingState : GetSubresourceTrackingStatesForImage(image))
			{
				if(trackingState.MetadataState == nullptr || !GpuBackendUtility::RangeOverlaps(trackingState.Range, range))
					continue;

				NativeTestState& state = static_cast<NativeTestState&>(*trackingState.MetadataState);
				state.Expanded.RecordAccess(stages, access);
				state.Compressed.RecordAccess(stages, access);
			}
		}

		TShared<GpuImageMetadataState> CreateImageMetadataState(IGpuImageResource*, const GpuTextureSubresourceRange& range)
		{
			return range.AspectMask.IsSet(GpuTextureAspectFlag::Depth) ? B3DMakeShared<NativeTestState>() : nullptr;
		}

		void CommitPendingHazardRegistrations()
		{
			for(const PendingHazardRegistration& registration : mPendingHazardRegistrations)
			{
				if(registration.MetadataState == nullptr)
					continue;

				NativeTestState& state = static_cast<NativeTestState&>(*registration.MetadataState);
				state.Expanded.RecordAccess(registration.AccessStageFlags, registration.Access);
				state.Compressed.RecordAccess(registration.AccessStageFlags, registration.Access);
			}
			Base::CommitPendingHazardRegistrations();
		}

		void UpdateHazardStateAfterBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, const GpuBarrierScope& barrier)
		{
			Base::UpdateHazardStateAfterBarrier(image, range, barrier);
			for(GpuImageSubresourceTrackingState& trackingState : GetSubresourceTrackingStatesForImage(image))
			{
				if(trackingState.MetadataState == nullptr || !GpuBackendUtility::RangeOverlaps(trackingState.Range, range))
					continue;

				NativeTestState& state = static_cast<NativeTestState&>(*trackingState.MetadataState);
				state.Expanded.RecordBarrier(barrier);
				state.Compressed.RecordBarrier(barrier);
			}
		}

		const GpuResourceHazardState& ResolveImageSubmissionHazards(IGpuImageResource*, const GpuImageSubresourceTrackingState& trackingState, IGpuResource& resource)
		{
			if(trackingState.MetadataState == nullptr)
				return *trackingState.HazardState;

			const NativeTestState& state = static_cast<NativeTestState&>(*trackingState.MetadataState);
			NativeTestSubresource& subresource = static_cast<NativeTestSubresource&>(resource);
			const bool wasCompressed = subresource.Compressed;
			subresource.Compressed = false;
			return wasCompressed ? state.Compressed : state.Expanded;
		}
	};

	NativeTestBarrierHelper::NativeTestBarrierHelper(NativeTestTracker* tracker) : TGpuBarrierHelper(tracker) { }

	void NativeTestBarrierHelper::Execute()
	{
		ApplyPostBarrierTracking();
		mResourceTracker->CommitPendingHazardRegistrations();
		Clear();
	}

	/** Captures the ordinary submission result after native state selection. */
	class NativeTestVisitor : public GpuSubmissionTransitionVisitor
	{
	public:
		NativeTestVisitor() = default;
		void VisitBuffer(const GpuSubmissionBufferTransition&) override { }
		void VisitImage(const GpuSubmissionImageTransition& transition) override
		{
			Waits |= transition.ParallelAccessWaitMask;
			Writes += transition.DestinationAllAccessScope.WriteStages != GpuStageFlag::None ? 1 : 0;
		}
		GpuQueueMask Waits;
		u32 Writes = 0;
	};
}

GpuImageMetadataStateTestSuite::GpuImageMetadataStateTestSuite() : TestSuite("GpuImageMetadataStateTestSuite")
{
	B3D_ADD_TEST(GpuImageMetadataStateTestSuite::TestRangeSplits)
	B3D_ADD_TEST(GpuImageMetadataStateTestSuite::TestSubmissionSelection)
	B3D_ADD_TEST(GpuImageMetadataStateTestSuite::TestStaticDispatch)
	B3D_ADD_TEST(GpuImageMetadataStateTestSuite::TestInternalAccess)
	B3D_ADD_TEST(GpuImageMetadataStateTestSuite::TestAttachmentClear)
}

void GpuImageMetadataStateTestSuite::TestRangeSplits()
{
	NativeTestImage image;
	NativeTestTracker tracker;
	NativeTestBarrierHelper barrierHelper(&tracker);
	tracker.TrackImageUsage(&image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader, GpuAccessFlag::Read, barrierHelper);
	const GpuTextureSubresourceRange firstFace(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
	tracker.IterateAndCreateOverlappingImageSubresourceTrackingState(&image, firstFace, [](u32, void*) { });
	tracker.CommitPendingHazardRegistrations();

	const GpuImageSubresourceTrackingState& first = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	const GpuImageSubresourceTrackingState& second = tracker.GetSubresourceTrackingState(&image, 1, 0, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(first.MetadataState != second.MetadataState)
	B3D_TEST_ASSERT(tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Stencil).MetadataState == nullptr)
	NativeTestState& firstNative = static_cast<NativeTestState&>(*first.MetadataState);
	NativeTestState& secondNative = static_cast<NativeTestState&>(*second.MetadataState);
	B3D_TEST_ASSERT(firstNative.Compressed.AllAccessScope.ReadStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(secondNative.Compressed.AllAccessScope.ReadStages == GpuStageFlag::FragmentShaderNonUniform)

	const GpuBarrierScope barrier(GpuStageFlag::All, GpuAccessFlag::Write, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	tracker.UpdateHazardStateAfterBarrier(&image, firstFace, barrier);
	firstNative.Compressed.RecordAccess(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(firstNative.Compressed.LastWriteEpochHazardState.VisibleStages == GpuStageFlag::None)
	B3D_TEST_ASSERT(secondNative.Compressed.LastBarrier.DestinationStages == GpuStageFlag::None)
	tracker.UpdateHazardStateAfterBarrier(&image, firstFace, barrier);
	B3D_TEST_ASSERT(firstNative.Compressed.LastWriteEpochHazardState.VisibleStages == GpuStageFlag::ComputeShaderNonUniform)
	tracker.NotifyUnbound();
	tracker.Clear();
}

void GpuImageMetadataStateTestSuite::TestSubmissionSelection()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);
	for(bool enableRewrite : { false, true })
	{
		NativeTestImage image;
		const GpuTextureSubresourceRange range(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
		NativeTestSubresource& subresource = static_cast<NativeTestSubresource&>(*image.GetSubresource(0, 0, GpuTextureAspectFlag::Depth));
		NativeTestTracker tracker;
		NativeTestBarrierHelper barrierHelper(&tracker);
		tracker.TrackImageUsage(&image, range, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Read, barrierHelper);
		const GpuImageSubresourceTrackingState& trackingState = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
		NativeTestState& nativeState = static_cast<NativeTestState&>(*trackingState.MetadataState);
		nativeState.Compressed.RecordAccess(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
		tracker.CommitPendingHazardRegistrations();

		// The predecessor is assigned after recording; recording order must not select an execution.
		subresource.Compressed = enableRewrite;
		GpuResourceSubmissionState predecessor;
		predecessor.ReaderQueues = GpuQueueMask(graphics);
		predecessor.ReaderStages = GpuStageFlag::FragmentShaderNonUniform;
		subresource.SetSubmissionState(std::move(predecessor));
		subresource.NotifyBound();
		subresource.NotifyUsed(graphics, GpuAccessFlag::Read);
		NativeTestVisitor visitor;
		tracker.ResolveSubmissionTransitions(compute, visitor);
		B3D_TEST_ASSERT(visitor.Writes == (enableRewrite ? 1u : 0u))
		B3D_TEST_ASSERT(visitor.Waits.IsSet(graphics) == enableRewrite)
		B3D_TEST_ASSERT(subresource.GetSubmissionState().HasWriter == enableRewrite)
		B3D_TEST_ASSERT(subresource.GetSubmissionState().ReaderQueues.IsSet(compute) != enableRewrite)
		tracker.NotifyUsed(compute);
		B3D_TEST_ASSERT(subresource.GetUseInfo(GpuAccessFlag::Write).IsSet(compute) == enableRewrite)
		B3D_TEST_ASSERT(image.GetUseInfo(GpuAccessFlag::Write).IsSet(compute) == enableRewrite)
		tracker.NotifyDone(compute);
		subresource.NotifyDone(graphics, GpuAccessFlag::Read);
		tracker.Clear();
	}
}

void GpuImageMetadataStateTestSuite::TestStaticDispatch()
{
	NativeTestImage image;
	NativeTestTracker tracker;
	NativeTestBarrierHelper helper(&tracker);
	const GpuTextureSubresourceRange range(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
	GpuRenderPassAttachmentUsage attachment;
	attachment.Image = &image;
	attachment.Range = range;
	attachment.Surface = RT_DEPTH;
	attachment.UseFlags = GpuResourceUseFlag::DepthStencilAttachment;
	attachment.Access = GpuAccessFlag::Read;
	attachment.Layout = GpuImageLayout::DepthStencilReadOnly;
	attachment.ShaderReadLayout = GpuImageLayout::DepthStencilReadOnly;
	GpuRenderPassAttachmentUsageArray attachments;
	attachments.Add(attachment);
	tracker.PrepareRenderPass(attachments);
	tracker.TrackImageUsage(&image, range, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader, GpuAccessFlag::Read, helper);
	B3D_TEST_ASSERT(tracker.Requirements == 0)
	tracker.BeginRenderPass(helper);
	helper.Execute();
	B3D_TEST_ASSERT(tracker.Requirements == 1)
	const GpuImageSubresourceTrackingState& trackingState = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	const NativeTestState& state = static_cast<NativeTestState&>(*trackingState.MetadataState);
	B3D_TEST_ASSERT(state.Expanded.AllAccessScope.ReadStages.IsSet(GpuStageFlag::FragmentShaderNonUniform))
	B3D_TEST_ASSERT(state.Expanded.AllAccessScope.ReadStages.IsSet(GpuStageFlag::EarlyFragmentTests))
	tracker.EndRenderPass();

	// A call through the base type must still reach the resolved-range backend boundary.
	TGpuResourceTracker<NativeTestTracker, NativeTestBarrierHelper>& base = tracker;
	base.TrackExplicitImageBarrier(&image, range, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Write, GpuImageLayout::General, helper);
	helper.Execute();
	B3D_TEST_ASSERT(tracker.Requirements == 2)
	B3D_TEST_ASSERT(state.Expanded.LastBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	tracker.NotifyUnbound();
	tracker.Clear();
}

void GpuImageMetadataStateTestSuite::TestInternalAccess()
{
	NativeTestImage image;
	NativeTestTracker tracker;
	NativeTestBarrierHelper helper(&tracker);
	const GpuTextureSubresourceRange range(0, 1, 0, 2, GpuTextureAspectFlag::Depth);
	const GpuTextureSubresourceRange firstFace(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
	tracker.TrackExplicitImageBarrier(&image, range, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Write, GpuImageLayout::General, helper);
	helper.Execute();
	tracker.RecordImageAccess(&image, range, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
	tracker.TrackExplicitImageBarrier(&image, firstFace, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader, GpuAccessFlag::Read, GpuImageLayout::ShaderReadOnly, helper);
	helper.Execute();
	const GpuImageSubresourceTrackingState& first = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	const GpuImageSubresourceTrackingState& second = tracker.GetSubresourceTrackingState(&image, 1, 0, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(first.InitialLayout == GpuImageLayout::General)
	B3D_TEST_ASSERT(first.CurrentLayout == GpuImageLayout::ShaderReadOnly)
	B3D_TEST_ASSERT(first.HazardState->LastWriteEpochHazardState.VisibleStages.IsSet(GpuStageFlag::FragmentShaderNonUniform))
	B3D_TEST_ASSERT(!second.HazardState->LastWriteEpochHazardState.VisibleStages.IsSet(GpuStageFlag::FragmentShaderNonUniform))

	// A new write cannot inherit the earlier write's visibility.
	tracker.RecordImageAccess(&image, firstFace, GpuStageFlag::LateFragmentTests, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(first.HazardState->GetRequiredBarrier(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read).IsValid())
	B3D_TEST_ASSERT(static_cast<NativeTestState&>(*first.MetadataState).Expanded.GetRequiredBarrier(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read).IsValid())
	tracker.NotifyUnbound();
	tracker.Clear();

	// Abandoned/reset command buffers release data ownership and start with independent empty values.
	tracker.TrackImageUsage(&image, firstFace, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader, GpuAccessFlag::Read, helper);
	helper.Execute();
	B3D_TEST_ASSERT(!tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth).HazardState->HasWrite())
	tracker.NotifyUnbound();
	tracker.Clear();
}

void GpuImageMetadataStateTestSuite::TestAttachmentClear()
{
	NativeTestImage image;
	NativeTestTracker tracker;
	NativeTestBarrierHelper helper(&tracker);
	const GpuTextureSubresourceRange range(0, 1, 0, 2, GpuTextureAspectFlag::Depth);
	const GpuTextureSubresourceRange firstFace(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
	GpuRenderPassAttachmentUsage attachment;
	attachment.Image = &image;
	attachment.Range = range;
	attachment.Surface = RT_DEPTH;
	attachment.UseFlags = GpuResourceUseFlag::DepthStencilAttachment;
	attachment.Access = GpuAccessFlag::Write;
	attachment.Layout = GpuImageLayout::DepthStencilAttachment;
	GpuRenderPassAttachmentUsageArray attachments;
	attachments.Add(attachment);
	tracker.PrepareRenderPass(attachments);
	tracker.BeginRenderPass(helper);
	helper.Execute();

	tracker.TrackImageUsage(&image, firstFace, GpuImageLayout::Undefined, GpuResourceUseFlag::AttachmentClear | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Write, helper);
	const GpuImageSubresourceTrackingState& cleared = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	const GpuImageSubresourceTrackingState& untouched = tracker.GetSubresourceTrackingState(&image, 1, 0, GpuTextureAspectFlag::Depth);
	const GpuStageFlags depthStages = GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;
	B3D_TEST_ASSERT(cleared.HazardState->LastWriteEpochHazardState.WriteStages == depthStages)
	B3D_TEST_ASSERT(cleared.RequiredLayout == GpuImageLayout::DepthStencilAttachment)

	helper.Execute();
	B3D_TEST_ASSERT(cleared.CurrentLayout == GpuImageLayout::DepthStencilAttachment)
	B3D_TEST_ASSERT(cleared.HazardState->LastBarrier.SourceStages == depthStages)
	B3D_TEST_ASSERT(cleared.HazardState->LastBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(cleared.HazardState->LastWriteEpochHazardState.WriteStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(untouched.HazardState->LastWriteEpochHazardState.WriteStages == depthStages)
	const NativeTestState& state = static_cast<NativeTestState&>(*cleared.MetadataState);
	B3D_TEST_ASSERT(state.Expanded.LastWriteEpochHazardState.WriteStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(state.Compressed.LastWriteEpochHazardState.WriteStages == GpuStageFlag::ComputeShaderNonUniform)
	tracker.EndRenderPass();
	tracker.NotifyUnbound();
	tracker.Clear();
}
