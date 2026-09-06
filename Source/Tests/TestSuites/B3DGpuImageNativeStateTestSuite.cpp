//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuImageNativeStateTestSuite.h"
#include "GpuBackend/B3DGpuResourceTracker.h"
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
	class NativeTestState : public GpuImageNativeState
	{
	public:
		NativeTestState() = default;
		TShared<GpuImageNativeState> Clone() const override { return B3DMakeShared<NativeTestState>(*this); }
		bool AccumulateTransitionRequirement(GpuImageLayout, GpuStageFlags, GpuAccessFlags) override { return false; }
		void RecordAccess(GpuStageFlags stages, GpuAccessFlags access) override
		{
			Expanded.RecordAccess(stages, access);
			Compressed.RecordAccess(stages, access);
		}
		void RecordBarrier(const GpuBarrierScope& barrier) override
		{
			Expanded.RecordBarrier(barrier);
			Compressed.RecordBarrier(barrier);
		}
		const GpuResourceHazardState& ResolveSubmission(IGpuResource& resource) override
		{
			NativeTestSubresource& subresource = static_cast<NativeTestSubresource&>(resource);
			const bool wasCompressed = subresource.Compressed;
			subresource.Compressed = false;
			return wasCompressed ? Compressed : Expanded;
		}

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
		TShared<GpuImageNativeState> CreateNativeTrackingState(const GpuTextureSubresourceRange& range) const override
		{
			return range.AspectMask.IsSet(GpuTextureAspectFlag::Depth) ? B3DMakeShared<NativeTestState>() : nullptr;
		}
	};

	/** No native GPU commands are needed for tracker contract tests. */
	struct NativeTestBarrierHelper
	{
		NativeTestBarrierHelper() = default;
		void QueueResolvedBufferBarrier(IGpuBufferResource*, const GpuBarrierScope&) { }
		void QueueResolvedImageBarrier(IGpuImageResource*, const GpuTextureSubresourceRange&, const GpuBarrierScope&, GpuImageLayout, GpuImageLayout, GpuImageBarrierFlags) { }
	};

	/** Exposes mutable state for synthetic native command recording. */
	class NativeTestTracker : public TGpuResourceTracker<NativeTestBarrierHelper>
	{
	public:
		NativeTestTracker() = default;
		using TGpuResourceTracker::GetSubresourceTrackingState;
	};

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

GpuImageNativeStateTestSuite::GpuImageNativeStateTestSuite() : TestSuite("GpuImageNativeStateTestSuite")
{
	B3D_ADD_TEST(GpuImageNativeStateTestSuite::TestRangeSplits)
	B3D_ADD_TEST(GpuImageNativeStateTestSuite::TestSubmissionSelection)
}

void GpuImageNativeStateTestSuite::TestRangeSplits()
{
	NativeTestImage image;
	NativeTestTracker tracker;
	NativeTestBarrierHelper barrierHelper;
	tracker.TrackImageUsage(&image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader, GpuAccessFlag::Read, barrierHelper);
	const GpuTextureSubresourceRange firstFace(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
	tracker.IterateAndCreateOverlappingImageSubresourceTrackingState(&image, firstFace, [](u32, void*) { });
	tracker.CommitPendingHazardRegistrations();

	const GpuImageSubresourceTrackingState& first = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	const GpuImageSubresourceTrackingState& second = tracker.GetSubresourceTrackingState(&image, 1, 0, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(first.NativeState != second.NativeState)
	B3D_TEST_ASSERT(tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Stencil).NativeState == nullptr)
	NativeTestState& firstNative = static_cast<NativeTestState&>(*first.NativeState);
	NativeTestState& secondNative = static_cast<NativeTestState&>(*second.NativeState);
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

void GpuImageNativeStateTestSuite::TestSubmissionSelection()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);
	for(bool enableRewrite : { false, true })
	{
		NativeTestImage image;
		const GpuTextureSubresourceRange range(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
		NativeTestSubresource& subresource = static_cast<NativeTestSubresource&>(*image.GetSubresource(0, 0, GpuTextureAspectFlag::Depth));
		NativeTestTracker tracker;
		NativeTestBarrierHelper barrierHelper;
		tracker.TrackImageUsage(&image, range, GpuImageLayout::ShaderReadOnly, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Read, barrierHelper);
		const GpuImageSubresourceTrackingState& trackingState = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
		NativeTestState& nativeState = static_cast<NativeTestState&>(*trackingState.NativeState);
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
		tracker.NotifyDone(compute);
		subresource.NotifyDone(graphics, GpuAccessFlag::Read);
		tracker.Clear();
	}
}
