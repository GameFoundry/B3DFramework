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
	template<class TTracker, class TBarrierHelper>
	bool TrackImageBinding(TTracker& tracker, IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuResourceUseFlags usage, GpuAccessFlags access, TBarrierHelper& helper)
	{
		return tracker.TrackImageUsage(image, range, layout, usage, access, helper);
	}

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

		/** Returns whether any synchronization is queued. */
		bool HasBarriers() const { return mBarrierTracking.Size() != 0 || mImageLayoutTracking.Size() != 0; }

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

		void QueueRequiredImageBarrier(IGpuImageResource* image, GpuImageSubresourceTrackingState& state, GpuStageFlags usage, GpuAccessFlags access, GpuImageLayout layout, NativeTestBarrierHelper& helper, GpuImageBarrierFlags flags = GpuImageBarrierFlag::None, GpuImageTrackingFlags trackingFlags = GpuImageTrackingFlag::None)
		{
			Requirements++;
			LastRequiredStages = usage;
			Base::QueueRequiredImageBarrier(image, state, usage, access, layout, helper, flags, trackingFlags);
		}

		u32 Requirements = 0;
		GpuStageFlags LastRequiredStages;

		TShared<GpuImageMetadataState> CreateImageMetadataState(IGpuImageResource*, const GpuTextureSubresourceRange& range)
		{
			return range.AspectMask.IsSet(GpuTextureAspectFlag::Depth) ? B3DMakeShared<NativeTestState>() : nullptr;
		}

		void CommitPendingAccesses()
		{
			for(const PendingHazardRegistration& registration : mPendingHazardRegistrations)
			{
				if(registration.MetadataState == nullptr)
					continue;

				NativeTestState& state = static_cast<NativeTestState&>(*registration.MetadataState);
				state.Expanded.RecordAccess(registration.AccessStageFlags, registration.Access);
				state.Compressed.RecordAccess(registration.AccessStageFlags, registration.Access);
			}
			Base::CommitPendingAccesses();
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
		mResourceTracker->CommitPendingAccesses();
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
	B3D_ADD_TEST(GpuImageMetadataStateTestSuite::TestShaderBindingAccess)
	B3D_ADD_TEST(GpuImageMetadataStateTestSuite::TestComputeAccessTracking)
}

void GpuImageMetadataStateTestSuite::TestRangeSplits()
{
	NativeTestImage image;
	NativeTestTracker tracker;
	NativeTestBarrierHelper barrierHelper(&tracker);
	tracker.TrackImageAccess(&image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, barrierHelper);
	const GpuTextureSubresourceRange firstFace(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
	tracker.IterateAndCreateOverlappingImageSubresourceTrackingState(&image, firstFace, [](u32, void*) { });
	tracker.CommitPendingAccesses();

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
		tracker.TrackImageAccess(&image, range, GpuImageLayout::ShaderReadOnly, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read, barrierHelper);
		const GpuImageSubresourceTrackingState& trackingState = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
		NativeTestState& nativeState = static_cast<NativeTestState&>(*trackingState.MetadataState);
		nativeState.Compressed.RecordAccess(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
		tracker.CommitPendingAccesses();

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
	TrackImageBinding(tracker, &image, range, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader, GpuAccessFlag::Read, helper);
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
	base.TrackExplicitImageBarrier(&image, range, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, GpuImageLayout::General, helper);
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
	tracker.TrackImageAccess(&image, range, GpuImageLayout::General, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, helper);
	helper.Execute();
	tracker.TrackExplicitImageBarrier(&image, firstFace, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, GpuImageLayout::ShaderReadOnly, helper);
	helper.Execute();
	const GpuImageSubresourceTrackingState& first = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	const GpuImageSubresourceTrackingState& second = tracker.GetSubresourceTrackingState(&image, 1, 0, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(first.InitialLayout == GpuImageLayout::General)
	B3D_TEST_ASSERT(first.CurrentLayout == GpuImageLayout::ShaderReadOnly)
	B3D_TEST_ASSERT(first.HazardState->LastWriteEpochHazardState.VisibleStages.IsSet(GpuStageFlag::FragmentShaderNonUniform))
	B3D_TEST_ASSERT(!second.HazardState->LastWriteEpochHazardState.VisibleStages.IsSet(GpuStageFlag::FragmentShaderNonUniform))

	// A new write cannot inherit the earlier write's visibility.
	tracker.TrackImageAccess(&image, firstFace, GpuImageLayout::Undefined, GpuStageFlag::LateFragmentTests, GpuAccessFlag::Write, helper);
	helper.Execute();
	B3D_TEST_ASSERT(first.HazardState->GetRequiredBarrier(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read).IsValid())
	B3D_TEST_ASSERT(static_cast<NativeTestState&>(*first.MetadataState).Expanded.GetRequiredBarrier(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read).IsValid())
	tracker.NotifyUnbound();
	tracker.Clear();

	// Abandoned/reset command buffers release data ownership and start with independent empty values.
	tracker.TrackImageAccess(&image, firstFace, GpuImageLayout::ShaderReadOnly, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, helper);
	helper.Execute();
	B3D_TEST_ASSERT(!tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth).HazardState->HasWrite())
	tracker.NotifyUnbound();
	tracker.Clear();

	// Meta-data preparation retains the range but leaves its executed write to the backend.
	tracker.TrackImageAccess(&image, firstFace, GpuImageLayout::General, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, helper, GpuImageBarrierFlag::None, GpuImageTrackingFlag::MetadataOperation);
	helper.Execute();
	const GpuImageSubresourceTrackingState& metadata = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(!metadata.HazardState->HasAccess())
	B3D_TEST_ASSERT(!static_cast<NativeTestState&>(*metadata.MetadataState).Compressed.HasAccess())
	B3D_TEST_ASSERT(tracker.FindImageTrackingState(&image)->UseHandle.Flags == GpuAccessFlag::Read)
	tracker.NotifyUnbound();
	tracker.Clear();

	// Non-shader usage follows ordinary access tracking, including deferred hazard registration.
	B3D_TEST_ASSERT(TrackImageBinding(tracker, &image, firstFace, GpuImageLayout::TransferDestination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, helper))
	const GpuImageSubresourceTrackingState& transfer = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(tracker.LastRequiredStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(transfer.RequiredLayout == GpuImageLayout::TransferDestination)
	B3D_TEST_ASSERT(!transfer.HazardState->HasAccess())
	B3D_TEST_ASSERT(tracker.FindImageTrackingState(&image)->UseHandle.Flags == GpuAccessFlag::Write)
	helper.Execute();
	B3D_TEST_ASSERT(transfer.HazardState->LastWriteEpochHazardState.WriteStages == GpuStageFlag::Transfer)
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

	tracker.TrackImageAccess(&image, firstFace, GpuImageLayout::Undefined, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, helper);
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
	tracker.TrackRenderPassAttachmentAccesses(helper);
	B3D_TEST_ASSERT(cleared.HazardState->LastWriteEpochHazardState.WriteStages == GpuStageFlag::ComputeShaderNonUniform)
	helper.Execute();
	B3D_TEST_ASSERT(cleared.HazardState->LastBarrier.SourceStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(cleared.HazardState->LastBarrier.DestinationStages == depthStages)
	B3D_TEST_ASSERT(cleared.HazardState->LastWriteEpochHazardState.WriteStages == depthStages)
	B3D_TEST_ASSERT(state.Expanded.LastWriteEpochHazardState.WriteStages == depthStages)
	B3D_TEST_ASSERT(state.Compressed.LastWriteEpochHazardState.WriteStages == depthStages)

	const u32 requirementsBeforeDraw = tracker.Requirements;
	u64 attachmentAccessEpoch = tracker.GetEpoch();
	for(u32 drawIndex = 0; drawIndex < 1000; drawIndex++)
	{
		tracker.TrackRenderPassAttachmentAccesses(helper);

		helper.Execute();
		attachmentAccessEpoch = tracker.GetEpoch();
	}
	B3D_TEST_ASSERT(tracker.Requirements == requirementsBeforeDraw)

	tracker.TrackExplicitImageBarrier(&image, firstFace, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, GpuImageLayout::Undefined, helper);
	helper.Execute();
	B3D_TEST_ASSERT(attachmentAccessEpoch != tracker.GetEpoch())
	tracker.TrackRenderPassAttachmentAccesses(helper);
	helper.Execute();

	tracker.EndRenderPass();
	tracker.TrackImageAccess(&image, firstFace, GpuImageLayout::ShaderReadOnly, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, helper);
	helper.Execute();
	const GpuImageSubresourceTrackingState& sampled = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(sampled.HazardState->LastBarrier.SourceStages == depthStages)
	B3D_TEST_ASSERT(sampled.HazardState->LastBarrier.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)

	tracker.NotifyUnbound();
	tracker.Clear();
}

void GpuImageMetadataStateTestSuite::TestShaderBindingAccess()
{
	NativeTestImage image;
	NativeTestTracker tracker;
	NativeTestBarrierHelper helper(&tracker);
	const GpuTextureSubresourceRange range(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
	const GpuResourceUseFlags shaderUsage = GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader;
	GpuRenderPassAttachmentUsage attachment;
	attachment.Image = &image;
	attachment.Range = range;
	attachment.UseFlags = GpuResourceUseFlag::DepthStencilAttachment;
	attachment.Access = GpuAccessFlag::Write;
	attachment.Layout = GpuImageLayout::DepthStencilAttachment;
	GpuRenderPassAttachmentUsageArray attachments;
	attachments.Add(attachment);
	tracker.PrepareRenderPass(attachments);
	{
		LoggingScope logs(*this);
		logs.ExpectError("Framebuffer attachments sampled during a render pass must be marked read-only");
		B3D_TEST_ASSERT(!TrackImageBinding(tracker, &image, range, GpuImageLayout::General, shaderUsage, GpuAccessFlag::Write, helper))
	}

	tracker.BeginRenderPass(helper);
	helper.Execute();
	{
		LoggingScope logs(*this);
		logs.ExpectError("Framebuffer attachments sampled during a render pass must be marked read-only");
		B3D_TEST_ASSERT(!TrackImageBinding(tracker, &image, range, GpuImageLayout::General, shaderUsage, GpuAccessFlag::Write, helper))
	}

	// The same stages and access are legal for an ordered internal operation.
	tracker.TrackImageAccess(&image, range, GpuImageLayout::Undefined, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, helper);
	helper.Execute();
	B3D_TEST_ASSERT(tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth).CurrentLayout == GpuImageLayout::DepthStencilAttachment)
	tracker.EndRenderPass();

	for(u32 dispatchIndex = 0; dispatchIndex < 2; dispatchIndex++)
	{
		B3D_TEST_ASSERT(TrackImageBinding(tracker, &image, range, GpuImageLayout::General, shaderUsage, GpuAccessFlag::Read | GpuAccessFlag::Write, helper))
		helper.Execute();
		const GpuImageSubresourceTrackingState& state = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
		B3D_TEST_ASSERT(state.HazardState->LastWriteEpochHazardState.WriteStages == GpuStageFlag::ComputeShaderNonUniform)
		B3D_TEST_ASSERT(state.HazardState->LastBarrier.DestinationAccess.IsSet(GpuAccessFlag::Write))
	}

	tracker.NotifyUnbound();
	tracker.Clear();
	attachment.Access = GpuAccessFlag::Read;
	attachments.Clear();
	attachments.Add(attachment);
	GpuRenderPassAttachmentUsage stencilAttachment = attachment;
	stencilAttachment.Range.AspectMask = GpuTextureAspectFlag::Stencil;
	stencilAttachment.Surface = RT_STENCIL;
	stencilAttachment.Access = GpuAccessFlag::Write;
	attachments.Add(stencilAttachment);
	tracker.PrepareRenderPass(attachments);
	tracker.BeginRenderPass(helper);
	helper.Execute();
	B3D_TEST_ASSERT(TrackImageBinding(tracker, &image, range, GpuImageLayout::Undefined, shaderUsage, GpuAccessFlag::Read, helper))
	helper.Execute();
	B3D_TEST_ASSERT(tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth).HazardState->AllAccessScope.ReadStages == (GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests | GpuStageFlag::ComputeShaderNonUniform))
	tracker.EndRenderPass();

	tracker.NotifyUnbound();
	tracker.Clear();
}

void GpuImageMetadataStateTestSuite::TestComputeAccessTracking()
{
	NativeTestImage image;
	NativeTestImage otherImage;
	NativeTestTracker tracker;
	NativeTestBarrierHelper helper(&tracker);
	const GpuTextureSubresourceRange range(0, 1, 0, 1, GpuTextureAspectFlag::Depth);

	B3D_TEST_ASSERT(tracker.TrackImageUsage(&image, range, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Read, helper))
	helper.Execute();
	u32 requirements = tracker.Requirements;
	for(u32 dispatchIndex = 0; dispatchIndex < 1000; dispatchIndex++)
	{
		B3D_TEST_ASSERT(tracker.TrackImageUsage(&image, range, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Read, helper))
		B3D_TEST_ASSERT(!helper.HasBarriers())
		helper.Execute();
	}
	B3D_TEST_ASSERT(tracker.Requirements == requirements + 1000)

	tracker.TrackImageAccess(&otherImage, range, GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, helper);
	helper.Execute();
	requirements = tracker.Requirements;
	B3D_TEST_ASSERT(tracker.TrackImageUsage(&image, range, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Read, helper))
	B3D_TEST_ASSERT(tracker.Requirements == requirements + 1)
	helper.Execute();

	tracker.TrackImageAccess(&image, range, GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, helper);
	helper.Execute();
	requirements = tracker.Requirements;
	B3D_TEST_ASSERT(tracker.TrackImageUsage(&image, range, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Read, helper))
	B3D_TEST_ASSERT(tracker.Requirements == requirements + 1)
	helper.Execute();
	B3D_TEST_ASSERT(tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth).CurrentLayout == GpuImageLayout::ShaderReadOnly)

	// Reading a copy source also invalidates the descriptor layout, despite having no write registration.
	tracker.TrackImageAccess(&image, range, GpuImageLayout::TransferSource, GpuStageFlag::Transfer, GpuAccessFlag::Read, helper);
	helper.Execute();
	requirements = tracker.Requirements;
	B3D_TEST_ASSERT(tracker.TrackImageUsage(&image, range, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Read, helper))
	B3D_TEST_ASSERT(tracker.Requirements == requirements + 1)
	helper.Execute();
	B3D_TEST_ASSERT(tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth).CurrentLayout == GpuImageLayout::ShaderReadOnly)

	tracker.TrackExplicitImageBarrier(&image, range, GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource, helper);
	helper.Execute();
	requirements = tracker.Requirements;
	B3D_TEST_ASSERT(tracker.TrackImageUsage(&image, range, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Read, helper))
	B3D_TEST_ASSERT(tracker.Requirements == requirements + 1)
	helper.Execute();
	tracker.NotifyUnbound();
	tracker.Clear();
}
