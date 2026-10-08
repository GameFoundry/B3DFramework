//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuBackendTestSuite.h"
#include "Utility/B3DPushConstantShaderCompilationTest.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuHazards.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuSplitBarrier.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuResourceTracker.h"
#include "GpuBackend/B3DGpuResourceTracker.inl"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShader.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"
#include "String/B3DStringFormat.h"
#include "Utility/B3DResult.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Frame index of test submissions that stay within one frame. */
	constexpr u32 kTestFrameIndex = 0;

	template<class TTracker, class TBarrierHelper>
	bool TrackImageBinding(TTracker& tracker, IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuResourceUseFlags usage, GpuAccessFlags access, TBarrierHelper& helper)
	{
		return tracker.TrackShaderImageAccess(image, range, layout, usage, access, helper);
	}

	Result ValidatePushConstantWrite(u32 maximumPushConstantSize, u32 offsetInBytes, u32 sizeInBytes, const void* data)
	{
		if(sizeInBytes == 0)
			return Result::Success();

		if(maximumPushConstantSize < kMaxPushConstantSizeInBytes)
		{
			return Result::Fail("The active GPU backend does not support the guaranteed push-constant block.",
				ResultStatus::FailedInvalidInput, StringUtility::Format("Backend limit {0} bytes; required {1} bytes.", maximumPushConstantSize, kMaxPushConstantSizeInBytes));
		}

		if((offsetInBytes & 3u) != 0 || (sizeInBytes & 3u) != 0)
			return Result::Fail("Push-constant offsets and sizes must be aligned to four bytes.", ResultStatus::FailedInvalidInput);

		if(data == nullptr)
			return Result::Fail("Push-constant data cannot be null for a non-empty update.", ResultStatus::FailedInvalidInput);

		if(offsetInBytes > kMaxPushConstantSizeInBytes || sizeInBytes > kMaxPushConstantSizeInBytes - offsetInBytes)
		{
			return Result::Fail("Push-constant update is outside the guaranteed block.", ResultStatus::FailedInvalidInput,
				StringUtility::Format("Offset {0}, size {1}, block size {2} bytes.", offsetInBytes, sizeInBytes, kMaxPushConstantSizeInBytes));
		}

		return Result::Success();
	}

	struct SubmissionTestBarrierHelper
	{
		void QueueResolvedBufferBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier, GpuBarrierFlags barrierFlags)
		{
			LastBufferBarrier = barrier;
			LastBufferBarrierFlags = barrierFlags;
			BufferBarrierCount++;
			QueuedResources.Add(buffer);
		}

		void QueueResolvedImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange&, const GpuBarrierScope& barrier, GpuImageLayout oldLayout, GpuImageLayout newLayout, GpuBarrierFlags barrierFlags)
		{
			LastImageBarrier = barrier;
			LastImageBarrierOldLayout = oldLayout;
			LastImageBarrierNewLayout = newLayout;
			LastImageBarrierFlags = barrierFlags;
			ImageBarrierCount++;
			QueuedResources.Add(image);
		}

		bool HasQueuedBarrier(const IGpuResource* resource) const { return std::find(QueuedResources.begin(), QueuedResources.end(), resource) != QueuedResources.end(); }

		/** Ends the batch of queued barriers. The barrier counts and the last barriers remain. */
		void Clear() { QueuedResources.Clear(); }

		TInlineArray<const IGpuResource*, 4> QueuedResources;
		GpuBarrierScope LastBufferBarrier;
		GpuBarrierFlags LastBufferBarrierFlags;
		u32 BufferBarrierCount = 0;
		u32 ImageBarrierCount = 0;
		GpuBarrierScope LastImageBarrier;
		GpuImageLayout LastImageBarrierOldLayout = GpuImageLayout::Undefined;
		GpuImageLayout LastImageBarrierNewLayout = GpuImageLayout::Undefined;
		GpuBarrierFlags LastImageBarrierFlags;
	};

	class SubmissionTestBuffer : public IGpuBufferResource
	{
	public:
		SubmissionTestBuffer() = default;
	};

	class SubmissionTestImage : public IGpuImageResource
	{
	public:
		SubmissionTestImage(u32 faceCount, u32 mipLevelCount, GpuTextureAspectFlags aspects)
			: IGpuImageResource(faceCount, mipLevelCount, aspects)
		{ }

		/** Commits @p layout as the native layout of every subresource, so an access in it needs no transition. */
		void SetNativeLayout(GpuImageLayout layout)
		{
			GpuImageNativeState nativeState;
			nativeState.Layout = (u32)layout;
			InitializeNativeState(mFullRange.AspectMask, nativeState);
		}
	};

	/** Color image whose shader reads in GpuImageLayout::ShaderReadOnly rest. */
	class RestingTestImage : public SubmissionTestImage
	{
	public:
		RestingTestImage(u32 faceCount, u32 mipLevelCount)
			: SubmissionTestImage(faceCount, mipLevelCount, GpuTextureAspectFlag::Color)
		{
			mCanRest = true;
		}
	};

	class SubmissionTestFramebuffer : public GpuFramebuffer
	{
	public:
		SubmissionTestFramebuffer(u32 width, u32 height, u32 layerCount)
			: GpuFramebuffer(width, height, layerCount)
		{ }

		using GpuFramebuffer::AddColorAttachment;
		using GpuFramebuffer::AddDepthStencilAttachment;
	};

	class SubmissionImageTestVisitor : public GpuSubmissionTransitionVisitor
	{
	public:
		void VisitBuffer(const GpuSubmissionBufferTransition&) override { }

		void VisitImage(const GpuSubmissionImageTransition& transition) override
		{
			B3D_ASSERT(transition.ImageRange.HasSingleAspect());
			VisitedAspects |= transition.ImageRange.AspectMask;
			NativeStates.Add(transition.NativeState);
			SubmissionBarrierFlags |= transition.SubmissionBarrierFlags;
		}

		GpuTextureAspectFlags VisitedAspects;
		TInlineArray<GpuImageNativeState*, 2> NativeStates;
		GpuBarrierFlags SubmissionBarrierFlags;
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

	class SubmissionTestTracker : public TGpuResourceTracker<SubmissionTestTracker, SubmissionTestBarrierHelper>
	{
	public:
		/** Test images encode native layouts as GpuImageLayout values. */
		static constexpr u32 kRestingNativeLayout = (u32)GpuImageLayout::ShaderReadOnly;

		/** Synchronizes layout transitions like Vulkan and D3D12. */
		static constexpr bool kLayoutTransitionsAreWrites = true;

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

	SubmissionTestResult ResolveTestSubmission(SubmissionTestBuffer& buffer, GpuQueueId queueId, GpuStageFlags stages, GpuAccessFlags access, u32 frameIndex = kTestFrameIndex)
	{
		GpuResourceHazardState hazardState;
		ResolveTestAccess(hazardState, stages, access);

		SubmissionTestTracker tracker;
		GpuBufferTrackingState trackingState;
		trackingState.HazardState = &hazardState;
		tracker.GetBuffers().insert(std::make_pair(&buffer, trackingState));

		SubmissionTestVisitor visitor;
		tracker.ResolveSubmissionTransitions(queueId, frameIndex, visitor);

		SubmissionTestResult result;
		result.ParallelAccessWaitMask = visitor.ParallelAccessWaitMask;
		result.ExclusiveAccessWaitMask = visitor.ExclusiveAccessWaitMask;
		result.MemoryBarrier = visitor.MemoryBarrier;
		result.ExecutionBarrier = visitor.ExecutionBarrier;
		return result;
	}

	/** Records every image transition, and commits the final layout as native state like a backend would. */
	class SubmissionImageRecordingVisitor : public GpuSubmissionTransitionVisitor
	{
	public:
		void VisitBuffer(const GpuSubmissionBufferTransition&) override { }

		void VisitImage(const GpuSubmissionImageTransition& transition) override
		{
			Ranges.Add(transition.ImageRange);
			NativeStates.Add(transition.NativeState);
			ParallelAccessWaitMask |= transition.ParallelAccessWaitMask;

			transition.NativeState->Layout = (u32)transition.FinalLayout;
		}

		TInlineArray<GpuTextureSubresourceRange, 4> Ranges;
		TInlineArray<GpuImageNativeState*, 4> NativeStates;
		GpuQueueMask ParallelAccessWaitMask = GpuQueueMask::kNone;
	};

	/** Records one access to @p range of @p image and submits it on @p queueId. @p tracker stays in flight until its NotifyDone(). */
	void SubmitTestImageAccess(SubmissionTestTracker& tracker, SubmissionImageRecordingVisitor& visitor, IGpuImageResource& image, const GpuTextureSubresourceRange& range,
		GpuImageLayout layout, GpuAccessFlags access, GpuQueueId queueId, u32 frameIndex = kTestFrameIndex)
	{
		const GpuStageFlags stages = access.IsSet(GpuAccessFlag::Write) ? GpuStageFlags(GpuStageFlag::Transfer) : GpuStageFlags(GpuStageFlag::FragmentShaderNonUniform);

		SubmissionTestBarrierHelper barrierHelper;
		tracker.TrackImageAccess(&image, range, layout, stages, access, barrierHelper);
		tracker.CommitPendingAccesses();
		tracker.ResolveSubmissionTransitions(queueId, frameIndex, visitor);
		tracker.NotifyUsed(queueId);
	}

	/** Submits one access and completes it before returning. */
	SubmissionImageRecordingVisitor ExecuteTestImageAccess(IGpuImageResource& image, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuAccessFlags access, GpuQueueId queueId,
		u32 frameIndex = kTestFrameIndex)
	{
		SubmissionTestTracker tracker;
		SubmissionImageRecordingVisitor visitor;
		SubmitTestImageAccess(tracker, visitor, image, range, layout, access, queueId, frameIndex);
		tracker.NotifyDone(queueId);
		tracker.Clear();

		return visitor;
	}

	/** Copy of one transition received by SubmissionRecordingVisitor. */
	struct RecordedSubmissionTransition
	{
		GpuTextureSubresourceRange ImageRange;
		GpuImageLayout InitialLayout = GpuImageLayout::Undefined;
		GpuImageLayout FinalLayout = GpuImageLayout::Undefined;
		GpuBarrierScope MemoryBarrier;
		GpuBarrierScope ExecutionBarrier;
		GpuQueueMask ParallelAccessWaitMask;
		GpuQueueMask ExclusiveAccessWaitMask;
		GpuResourceSubmissionState PostTransitionSubmissionState;
		GpuBarrierFlags SubmissionBarrierFlags;
	};

	/** Records every buffer and image transition, and commits the final image layout as native state like a backend would. */
	class SubmissionRecordingVisitor : public GpuSubmissionTransitionVisitor
	{
	public:
		void VisitBuffer(const GpuSubmissionBufferTransition& transition) override { Record(transition).SubmissionBarrierFlags = transition.SubmissionBarrierFlags; }

		void VisitImage(const GpuSubmissionImageTransition& transition) override
		{
			RecordedSubmissionTransition& recordedTransition = Record(transition);
			recordedTransition.ImageRange = transition.ImageRange;
			recordedTransition.InitialLayout = transition.InitialLayout;
			recordedTransition.FinalLayout = transition.FinalLayout;
			recordedTransition.SubmissionBarrierFlags = transition.SubmissionBarrierFlags;

			transition.NativeState->Layout = (u32)transition.FinalLayout;
		}

		Vector<RecordedSubmissionTransition> Transitions;

	private:
		RecordedSubmissionTransition& Record(const GpuSubmissionTransition& transition)
		{
			RecordedSubmissionTransition recordedTransition;
			recordedTransition.MemoryBarrier = transition.MemoryBarrier;
			recordedTransition.ExecutionBarrier = transition.ExecutionBarrier;
			recordedTransition.ParallelAccessWaitMask = transition.ParallelAccessWaitMask;
			recordedTransition.ExclusiveAccessWaitMask = transition.ExclusiveAccessWaitMask;
			recordedTransition.PostTransitionSubmissionState = transition.PostTransitionSubmissionState;

			Transitions.push_back(recordedTransition);
			return Transitions.back();
		}
	};

	/** Resolves the tracker's submission transitions on @p queueId and marks its resources as in flight there. */
	void SubmitTestTracker(SubmissionTestTracker& tracker, GpuQueueId queueId, u32 frameIndex, GpuSubmissionTransitionVisitor& visitor)
	{
		tracker.ResolveSubmissionTransitions(queueId, frameIndex, visitor);
		tracker.NotifyUsed(queueId);
	}

	/** Completes a tracker submitted with SubmitTestTracker() and resets it. */
	void CompleteTestTracker(SubmissionTestTracker& tracker, GpuQueueId queueId)
	{
		tracker.NotifyDone(queueId);
		tracker.Clear();
	}

	/** Records one access of @p buffer and submits it on @p queueId. @p tracker stays in flight until CompleteTestTracker(). */
	Vector<RecordedSubmissionTransition> SubmitRecordedBufferAccess(SubmissionTestTracker& tracker, IGpuBufferResource& buffer, GpuStageFlags stages, GpuAccessFlags access, GpuQueueId queueId,
		u32 frameIndex)
	{
		SubmissionTestBarrierHelper barrierHelper;
		tracker.TrackBufferAccess(&buffer, stages, access, barrierHelper);
		tracker.CommitPendingAccesses();

		SubmissionRecordingVisitor visitor;
		SubmitTestTracker(tracker, queueId, frameIndex, visitor);
		return visitor.Transitions;
	}

	/** Records one access of @p range of @p image and submits it on @p queueId. @p tracker stays in flight until CompleteTestTracker(). */
	Vector<RecordedSubmissionTransition> SubmitRecordedImageAccess(SubmissionTestTracker& tracker, IGpuImageResource& image, const GpuTextureSubresourceRange& range, GpuImageLayout layout,
		GpuStageFlags stages, GpuAccessFlags access, GpuQueueId queueId, u32 frameIndex)
	{
		SubmissionTestBarrierHelper barrierHelper;
		tracker.TrackImageAccess(&image, range, layout, stages, access, barrierHelper);
		tracker.CommitPendingAccesses();

		SubmissionRecordingVisitor visitor;
		SubmitTestTracker(tracker, queueId, frameIndex, visitor);
		return visitor.Transitions;
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
#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_ADD_TEST(GpuBackendTestSuite::TestDrawAccessValidation)
#endif
	B3D_ADD_TEST(GpuBackendTestSuite::TestResourceHazardState)
	B3D_ADD_TEST(GpuBackendTestSuite::TestExplicitBarrierWriteOrdering)
	B3D_ADD_TEST(GpuBackendTestSuite::TestResourceTransition)
	B3D_ADD_TEST(GpuBackendTestSuite::TestSubmissionTransitionPlanning)
	B3D_ADD_TEST(GpuBackendTestSuite::TestImageAspectTracking)
	B3D_ADD_TEST(GpuBackendTestSuite::TestImageAccessEpochTracking)
	B3D_ADD_TEST(GpuBackendTestSuite::TestWholeImageRegistration)
	B3D_ADD_TEST(GpuBackendTestSuite::TestWholeImageSubmission)
	B3D_ADD_TEST(GpuBackendTestSuite::TestImageSplit)
	B3D_ADD_TEST(GpuBackendTestSuite::TestImageMerge)
	B3D_ADD_TEST(GpuBackendTestSuite::TestMergedStateWaits)
	B3D_ADD_TEST(GpuBackendTestSuite::TestSubmissionStateMerge)
	B3D_ADD_TEST(GpuBackendTestSuite::TestFrameIndexClear)
	B3D_ADD_TEST(GpuBackendTestSuite::TestRestingReaderStages)
	B3D_ADD_TEST(GpuBackendTestSuite::TestRestingReadRecording)
	B3D_ADD_TEST(GpuBackendTestSuite::TestRestingReadMaterialization)
	B3D_ADD_TEST(GpuBackendTestSuite::TestRestingSubmission)
	B3D_ADD_TEST(GpuBackendTestSuite::TestLayoutTransitionWrites)
	B3D_ADD_TEST(GpuBackendTestSuite::TestAliasAcquire)
	B3D_ADD_TEST(GpuBackendTestSuite::TestAliasAcquireExecution)
	B3D_ADD_TEST(GpuBackendTestSuite::TestFramebufferAttachmentUsage)
	B3D_ADD_TEST(GpuBackendTestSuite::TestRenderPassResourceTracking)
#if B3D_GPU_EXPLICIT_BARRIERS
	B3D_ADD_TEST(GpuBackendTestSuite::TestTrackingWithoutHazards)
	B3D_ADD_TEST(GpuBackendTestSuite::TestExplicitBarriers)
#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_ADD_TEST(GpuBackendTestSuite::TestExplicitBarrierValidation)
#endif
#endif
	B3D_ADD_TEST(GpuBackendTestSuite::TestPushConstantMetadata)
	B3D_ADD_TEST(GpuBackendTestSuite::TestPushConstantWrites)
	B3D_ADD_TEST(GpuBackendTestSuite::TestPushConstantSerialization)
	B3D_ADD_TEST(GpuBackendTestSuite::TestDynamicOffsetUniformBufferLayout)
	B3D_ADD_TEST(GpuBackendTestSuite::TestResourceLocations)
	// Shader compilation is performed on the host; console applications load cooked shaders.
#if !B3D_PLATFORM_PS5
	B3D_ADD_TEST(GpuBackendTestSuite::TestDynamicUniformBufferOffsets)
	B3D_ADD_TEST(GpuBackendTestSuite::TestHostPushConstantShaderCompilation)
	B3D_ADD_TEST(GpuBackendTestSuite::TestVulkanStorageBufferAccessReflection)
	B3D_ADD_TEST(GpuBackendTestSuite::TestHlslShaderModel66Compilation)
#endif
#if B3D_PLATFORM_MACOS
	B3D_ADD_TEST(GpuBackendTestSuite::TestMetalDynamicUniformBufferReflection)
#endif
}

void GpuBackendTestSuite::TestResourceLocations()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);

	constexpr u32 kElementCount = 64;
	const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kElementCount);
	const GpuMemoryRequirements bufferRequirements = device->GetMemoryRequirements(bufferInformation);
	B3D_TEST_ASSERT(bufferRequirements.MemoryType != GpuMemoryRequirements::kUnsupportedMemoryType)
	B3D_TEST_ASSERT(bufferRequirements.Size >= kElementCount * sizeof(u32))
	B3D_TEST_ASSERT(bufferRequirements.Alignment > 0)
	if(bufferRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return;

	IGpuAllocator& bufferAllocator = device->GetPersistentAllocator(bufferRequirements.MemoryType);

	// Pending allocation: memory is allocated from the allocator, and the buffer stays relocatable
	const TShared<render::GpuBuffer> pendingBuffer = device->CreateGpuBuffer(bufferInformation, GpuAllocation::CreatePending(bufferAllocator));
	B3D_TEST_ASSERT(pendingBuffer != nullptr && !pendingBuffer->HasFixedLocation())

	// Owned allocation: the buffer adopts memory allocated by the caller
	GpuAllocation ownedAllocation;
	B3D_TEST_ASSERT(bufferAllocator.TryAllocate(bufferRequirements.Size, (u32)bufferRequirements.Alignment, bufferRequirements.Kind, nullptr, ownedAllocation))
	B3D_TEST_ASSERT(ownedAllocation.IsOwned())
	if(!ownedAllocation.IsOwned())
		return;

	const TShared<render::GpuBuffer> ownedBuffer = device->CreateGpuBuffer(bufferInformation, ownedAllocation);
	B3D_TEST_ASSERT(ownedBuffer != nullptr && ownedBuffer->HasFixedLocation())

	// Non-owning allocation: the buffer binds to memory that stays owned by the caller
	GpuAllocation sharedAllocation;
	B3D_TEST_ASSERT(bufferAllocator.TryAllocate(bufferRequirements.Size, (u32)bufferRequirements.Alignment, bufferRequirements.Kind, nullptr, sharedAllocation))

	GpuAllocation nonOwningAllocation = sharedAllocation;
	nonOwningAllocation.Allocator = nullptr;
	B3D_TEST_ASSERT(nonOwningAllocation.HasMemory() && !nonOwningAllocation.IsOwned() && !nonOwningAllocation.IsPending())

	TShared<render::GpuBuffer> nonOwningBuffer = device->CreateGpuBuffer(bufferInformation, nonOwningAllocation);
	B3D_TEST_ASSERT(nonOwningBuffer != nullptr && nonOwningBuffer->HasFixedLocation())
	if(pendingBuffer == nullptr || ownedBuffer == nullptr || nonOwningBuffer == nullptr)
		return;

	// Round-trip data through the fixed location buffers, to verify the native resources are bound to usable memory
	u32 expected[kElementCount];
	for(u32 elementIndex = 0; elementIndex < kElementCount; elementIndex++)
		expected[elementIndex] = 0xC0DE0000 + elementIndex;

	const TShared<render::GpuBuffer> upload = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(sizeof(expected)));
	const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(sizeof(expected)));
	{
		const render::GpuBufferMappedScope mapping = upload->Map(GpuMapOption::Write);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		memcpy(mapping.GetMappedMemory(), expected, sizeof(expected));
	}

	const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
	const TShared<render::GpuCommandBufferPool> pool = device->CreateGpuCommandBufferPool(render::GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
	const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Resource allocations"));
	commands->CopyBufferToBuffer(upload, ownedBuffer, 0, 0, sizeof(expected));
	commands->CopyBufferToBuffer(ownedBuffer, nonOwningBuffer, 0, 0, sizeof(expected));
	commands->CopyBufferToBuffer(nonOwningBuffer, pendingBuffer, 0, 0, sizeof(expected));
	commands->CopyBufferToBuffer(pendingBuffer, readback, 0, 0, sizeof(expected));
	context->SubmitCommandBuffer(commands);
	device->WaitUntilIdle();
	{
		const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(mapping.IsValid())
			B3D_TEST_ASSERT(memcmp(mapping.GetMappedMemory(), expected, sizeof(expected)) == 0)
	}

	// Memory behind a non-owning allocation outlives the buffer, and is released by its owner
	nonOwningBuffer = nullptr;
	device->WaitUntilIdle();
	bufferAllocator.Free(sharedAllocation);

	// Textures follow the same rules
	TextureCreateInformation textureInformation;
	textureInformation.Name = "Resource allocation test";
	textureInformation.Width = 64;
	textureInformation.Height = 64;
	textureInformation.Format = PF_RGBA8;
	textureInformation.Usage = TextureUsageFlag::Default;

	const GpuMemoryRequirements textureRequirements = device->GetMemoryRequirements(textureInformation);
	B3D_TEST_ASSERT(textureRequirements.MemoryType != GpuMemoryRequirements::kUnsupportedMemoryType)
	B3D_TEST_ASSERT(textureRequirements.Size >= 64 * 64 * 4)
	if(textureRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return;

	IGpuAllocator& textureAllocator = device->GetPersistentAllocator(textureRequirements.MemoryType);

	GpuAllocation textureAllocation;
	B3D_TEST_ASSERT(textureAllocator.TryAllocate(textureRequirements.Size, (u32)textureRequirements.Alignment, textureRequirements.Kind, nullptr, textureAllocation))

	TextureCreateInformation cpuTextureInformation = textureInformation;
	cpuTextureInformation.Usage = TextureUsageFlag::StoreOnCPUWithGPUAccess;

	const GpuMemoryRequirements cpuTextureRequirements = device->GetMemoryRequirements(cpuTextureInformation);
	IGpuAllocator& cpuTextureAllocator = device->GetPersistentAllocator(cpuTextureRequirements.MemoryType);

	GpuAllocation cpuTextureAllocation;
	B3D_TEST_ASSERT(cpuTextureAllocator.TryAllocate(cpuTextureRequirements.Size, (u32)cpuTextureRequirements.Alignment, cpuTextureRequirements.Kind, nullptr, cpuTextureAllocation))

	bool pendingTextureCreated = false;
	bool ownedTextureCreated = false;
	bool cpuTextureCreated = false;
	GetRenderThread().PostCommand([&]()
	{
		const TShared<render::Texture> pendingTexture = device->CreateTexture(textureInformation, GpuAllocation::CreatePending(textureAllocator));
		pendingTextureCreated = pendingTexture != nullptr && !pendingTexture->HasFixedLocation();

		const TShared<render::Texture> ownedTexture = device->CreateTexture(textureInformation, textureAllocation);
		ownedTextureCreated = ownedTexture != nullptr && ownedTexture->HasFixedLocation();

		// CPU accessible textures can be created at a fixed location
		const TShared<render::Texture> cpuTexture = device->CreateTexture(cpuTextureInformation, cpuTextureAllocation);
		cpuTextureCreated = cpuTexture != nullptr && cpuTexture->HasFixedLocation();
	}, "GpuBackendTestSuite::TestResourceLocations", true);

	B3D_TEST_ASSERT(pendingTextureCreated)
	B3D_TEST_ASSERT(ownedTextureCreated)
	B3D_TEST_ASSERT(cpuTextureCreated)

	// Invalid allocations are rejected
	{
		LoggingScope logs(*this);
		logs.ExpectError("Cannot create a GPU resource at an empty allocation.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(bufferInformation, GpuAllocation()) == nullptr)
	}

	{
		LoggingScope logs(*this);
		logs.ExpectError("Only a GPU resource created at a fixed memory location can be aliased.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(bufferInformation, GpuAllocation::CreatePending(bufferAllocator), GpuObjectCreateFlag::Aliased) == nullptr)
	}

	// CPU accessible and suballocated buffers can be created at a fixed location
	{
		const GpuBufferCreateInformation uniformInformation = GpuBufferCreateInformation::CreateUniform(16, GpuBufferFlag::StoreOnCPUWithGPUAccess, 4);
		const GpuMemoryRequirements uniformRequirements = device->GetMemoryRequirements(uniformInformation);
		IGpuAllocator& uniformAllocator = device->GetPersistentAllocator(uniformRequirements.MemoryType);

		GpuAllocation uniformAllocation;
		B3D_TEST_ASSERT(uniformAllocator.TryAllocate(uniformRequirements.Size, (u32)uniformRequirements.Alignment, uniformRequirements.Kind, nullptr, uniformAllocation))

		const TShared<render::GpuBuffer> uniformBuffer = device->CreateGpuBuffer(uniformInformation, uniformAllocation);
		B3D_TEST_ASSERT(uniformBuffer != nullptr && uniformBuffer->HasFixedLocation())
		if(uniformBuffer != nullptr)
		{
			const render::GpuBufferMappedScope mapping = uniformBuffer->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
		}
	}
}

void GpuBackendTestSuite::TestDynamicOffsetUniformBufferLayout()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);

	GpuUniformBufferInformation staticBuffer;
	staticBuffer.Name = "StaticData";
	staticBuffer.Set = 0;
	staticBuffer.Slot = device->GetUniformBufferParameterSlot(0);
	staticBuffer.Size = 4;
	staticBuffer.Stages = GpuProgramStageBit::Vertex;
	staticBuffer.IsShareable = true;

	GpuUniformBufferInformation dynamicBuffer = staticBuffer;
	dynamicBuffer.Name = "PerObject";
	dynamicBuffer.Slot = device->GetUniformBufferParameterSlot(1);
	dynamicBuffer.UsesDynamicOffset = true;

	GpuProgramParameterDescription description;
	description.UniformBuffers[staticBuffer.Name] = staticBuffer;
	description.UniformBuffers[dynamicBuffer.Name] = dynamicBuffer;

	const TShared<GpuPipelineParameterSetLayout> layout = device->CreateGpuPipelineParameterSetLayout(description);
	B3D_TEST_ASSERT(layout != nullptr)
	if(layout == nullptr)
		return;

	B3D_TEST_ASSERT(layout->GetDynamicOffsetCount() == 1)
	B3D_TEST_ASSERT(layout->GetDynamicOffsetIndex("PerObject") == 0)
	B3D_TEST_ASSERT(layout->GetDynamicOffsetIndex("StaticData") == ~0u)

	const UniformInformation* dynamicInformation = layout->TryGetUniformInformation("PerObject");
	B3D_TEST_ASSERT(dynamicInformation != nullptr && dynamicInformation->DynamicOffsetIndex == 0)

	const UniformInformation* staticInformation = layout->TryGetUniformInformation("StaticData");
	B3D_TEST_ASSERT(staticInformation != nullptr && staticInformation->DynamicOffsetIndex == ~0u)

	// Stages sharing a buffer must agree on its dynamic-offset declaration
	GpuProgramParameterDescription vertex;
	vertex.UniformBuffers[dynamicBuffer.Name] = dynamicBuffer;

	GpuProgramParameterDescription fragment;
	fragment.UniformBuffers[dynamicBuffer.Name] = dynamicBuffer;
	fragment.UniformBuffers[dynamicBuffer.Name].UsesDynamicOffset = false;

	GpuProgramParameterDescription combined;
	B3D_TEST_ASSERT(combined.TryCombine(vertex, GpuProgramStageBit::Vertex).IsSuccessful())
	B3D_TEST_ASSERT(!combined.TryCombine(fragment, GpuProgramStageBit::Fragment).IsSuccessful())
}

#if !B3D_PLATFORM_PS5
void GpuBackendTestSuite::TestDynamicUniformBufferOffsets()
{
	GpuBackend& backend = GpuBackend::Instance();
	const String backendName = backend.GetBackendName();
	if(backend.GetDeviceCount() == 0 || (backendName != "bsfD3D12GpuBackend" && backendName != "bsfVulkanGpuBackend"))
		return;

	GpuDevice* const device = backend.GetDevice(0).get();
	const String language = backendName == "bsfD3D12GpuBackend" ? "hlsl" : "vksl";

	const TShared<IShaderCompiler> compiler = ShaderCompilers::Instance().GetCompiler("bsl");
	B3D_TEST_ASSERT(compiler != nullptr)
	if(compiler == nullptr)
		return;

	const String source = R"(
shader DynamicUniformBufferOffsets
{
	code
	{
		cbuffer StaticParameters : register(c0, space0) { uint StaticValue; };
		[dynamicOffset] cbuffer DynamicParameters : register(c3, space0) { uint DynamicValue; };
		[dynamicOffset] cbuffer OtherParameters : register(c1, space1) { uint OtherValue; };
		[pushConstant] cbuffer Constants { uint OutputIndex; };
		RWStructuredBuffer<uint> Output;
		[numthreads(1, 1, 1)]
		void csmain() { Output[OutputIndex] = StaticValue + 100 * DynamicValue + 10000 * OtherValue; }
	};
};
)";
	TShared<b3d::Shader> shader;
	const ShaderCompilerResult result = compiler->Compile("DynamicUniformBufferOffsets", source, {}, { language }, true, shader);
	B3D_TEST_ASSERT_MSG(result.ErrorMessage.empty(), result.ErrorMessage)
	B3D_TEST_ASSERT(shader != nullptr && shader->GetVariations().size() == 1)
	if(!result.ErrorMessage.empty() || shader == nullptr || shader->GetVariations().size() != 1)
		return;

	const TShared<b3d::Variation>& variation = shader->GetVariations().front();
	B3D_TEST_ASSERT(variation->GetPassCount() == 1)
	if(variation->GetPassCount() != 1)
		return;

	GpuComputePipelineStateCreateInformation pipelineInformation;
	pipelineInformation.Program = device->CreateGpuProgram(variation->GetPass(0)->GetGpuProgramCreateInformation(GPT_COMPUTE_PROGRAM));
	const TShared<GpuComputePipelineState> pipeline = device->CreateGpuComputePipelineState(pipelineInformation);
	const TShared<GpuComputePipelineState> alternatePipeline = device->CreateGpuComputePipelineState(pipelineInformation);
	B3D_TEST_ASSERT(pipeline != nullptr && alternatePipeline != nullptr)
	if(pipeline == nullptr || alternatePipeline == nullptr)
		return;

	const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
	const TShared<render::GpuBuffer> values = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(16, GpuBufferFlag::StoreOnCPUWithGPUAccess, 4));
	const u32 stride = values->GetSuballocationSize();
	{
		const render::GpuBufferMappedScope mapping = values->Map(GpuMapOption::Write);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		for(u32 valueIndex = 0; valueIndex < 4; valueIndex++)
			*reinterpret_cast<u32*>(static_cast<u8*>(mapping.GetMappedMemory()) + valueIndex * stride) = 1 + 10 * valueIndex;
	}

	const u32 expected[] = { 12111, 13111, 213111, 210111, 212131, 212111 };
	const TShared<render::GpuBuffer> output = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 6, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
	const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(sizeof(expected)));
	const TShared<GpuPipelineParameterSetLayout> firstLayout = pipeline->GetParameterLayout()->GetSet(0);
	const TShared<GpuPipelineParameterSetLayout> secondLayout = pipeline->GetParameterLayout()->GetSet(1);
	const TShared<render::GpuParameterSet> firstSet = context->GetParameterSetPool().Create(firstLayout, 0);
	const TShared<render::GpuParameterSet> secondSet = context->GetParameterSetPool().Create(secondLayout, 1);
	firstSet->SetUniformBuffer("DynamicParameters", values, 0, 2 * stride);
	firstSet->SetStorageBuffer("Output", output);
	secondSet->SetUniformBuffer("OtherParameters", values);
	const TShared<render::GpuCommandBufferPool> pool = device->CreateGpuCommandBufferPool(render::GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));

	// Reusing parameter sets with a fresh command buffer restores their initial offsets.
	for(u32 iteration = 0; iteration < 2; iteration++)
	{
		firstSet->SetUniformBuffer("StaticParameters", values, 0, stride);
		const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Dynamic uniform offsets"));
		commands->SetGpuComputePipelineState(pipeline);
		commands->SetGpuParameterSet(firstSet);
		commands->SetGpuParameterSet(secondSet);
		for(u32 outputIndex = 0; outputIndex < 6; outputIndex++)
		{
			if(outputIndex == 1)
				commands->SetDynamicBufferOffset(0, firstLayout->GetDynamicOffsetIndex("DynamicParameters"), 3 * stride);
			else if(outputIndex == 2)
				commands->SetDynamicBufferOffset(1, secondLayout->GetDynamicOffsetIndex("OtherParameters"), 2 * stride);
			else if(outputIndex == 3)
			{
				commands->SetGpuComputePipelineState(alternatePipeline);
				commands->SetDynamicBufferOffset(0, firstLayout->GetDynamicOffsetIndex("DynamicParameters"), 0);
			}
			else if(outputIndex >= 4)
			{
				firstSet->SetUniformBuffer("StaticParameters", values, 0, outputIndex == 4 ? 3 * stride : stride);
				commands->SetGpuParameterSet(firstSet);
			}

			commands->SetPushConstants(0, sizeof(outputIndex), &outputIndex);
			commands->DispatchCompute(1);
		}

		commands->CopyBufferToBuffer(output, readback, 0, 0, sizeof(expected));
		context->SubmitCommandBuffer(commands);
		device->WaitUntilIdle();

		const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		const u32* actual = static_cast<const u32*>(mapping.GetMappedMemory());
		for(u32 outputIndex = 0; outputIndex < 6; outputIndex++)
			B3D_TEST_ASSERT(actual[outputIndex] == expected[outputIndex])
	}
}
#endif

#if B3D_PLATFORM_MACOS
void GpuBackendTestSuite::TestMetalDynamicUniformBufferReflection()
{
	const TShared<IShaderCompiler> compiler = ShaderCompilers::Instance().GetCompiler("bsl");
	B3D_TEST_ASSERT_MSG(compiler != nullptr, "Metal reflection tests require the BSL compiler.")
	if(compiler == nullptr)
		return;

	const String shaderName = "MetalDynamicUniformBufferReflection";
	const String source = R"(
shader MetalDynamicUniformBufferReflection
{
	code
	{
		[dynamicOffset]
		cbuffer PerObject
		{
			float4x4 gTransform;
		};

		[dynamicOffset]
		cbuffer Tint
		{
			float4 gColor;
		};

		SamplerState gSampler;
		Texture2D gTexture;

		struct VStoFS
		{
			float4 position : SV_Position;
			float2 uv : TEXCOORD0;
		};

		VStoFS vsmain(float3 position : POSITION, float2 uv : TEXCOORD0)
		{
			VStoFS output;
			output.position = mul(gTransform, float4(position, 1.0f));
			output.uv = uv;
			return output;
		}

		float4 fsmain(VStoFS input) : SV_Target0
		{
			return gTexture.Sample(gSampler, input.uv) * gColor;
		}
	};
};
)";

	TShared<Shader> shader;
	const ShaderCompilerResult compileResult = compiler->Compile(shaderName, source, {}, { "msl" }, true, shader);
	B3D_TEST_ASSERT_MSG(compileResult.ErrorMessage.empty(), compileResult.ErrorMessage)
	B3D_TEST_ASSERT(shader != nullptr && shader->GetVariations().size() == 1)
	if(!compileResult.ErrorMessage.empty() || shader == nullptr || shader->GetVariations().size() != 1)
		return;

	const TShared<Variation>& variation = shader->GetVariations().front();
	B3D_TEST_ASSERT(variation != nullptr && variation->GetPassCount() == 1 && variation->GetPass(0) != nullptr)
	if(variation == nullptr || variation->GetPassCount() != 1 || variation->GetPass(0) == nullptr)
		return;

	const GpuProgramType stages[] = { GPT_VERTEX_PROGRAM, GPT_FRAGMENT_PROGRAM };
	for(GpuProgramType stage : stages)
	{
		const GpuProgramCreateInformation& program = variation->GetPass(0)->GetGpuProgramCreateInformation(stage);
		B3D_TEST_ASSERT(program.Bytecode != nullptr)
		if(program.Bytecode == nullptr)
			continue;

		B3D_TEST_ASSERT_MSG(program.Bytecode->Instructions.Data != nullptr, program.Bytecode->Messages)
		B3D_TEST_ASSERT(program.Bytecode->ParameterDescription != nullptr && program.Bytecode->ResourceTableLayout != nullptr)
		if(program.Bytecode->Instructions.Data == nullptr || program.Bytecode->ParameterDescription == nullptr || program.Bytecode->ResourceTableLayout == nullptr)
			continue;

		// Each stage reflects only the resources it reads, but argument-table indices follow the declared (set, slot)
		// order shared by every stage: PerObject is declared first and takes index 8 in the vertex stage, so Tint keeps
		// index 9 in the fragment stage even though PerObject is absent there
		const GpuProgramParameterDescription& parameterDescription = *program.Bytecode->ParameterDescription;
		const GpuResourceTableLayout& tableLayout = *program.Bytecode->ResourceTableLayout;
		const char* expectedBuffer = stage == GPT_VERTEX_PROGRAM ? "PerObject" : "Tint";
		const char* expectedMember = stage == GPT_VERTEX_PROGRAM ? "gTransform" : "gColor";
		const u32 expectedIndex = stage == GPT_VERTEX_PROGRAM ? 8 : 9;
		B3D_TEST_ASSERT(parameterDescription.UniformBuffers.size() == 1)
		B3D_TEST_ASSERT_MSG(parameterDescription.UniformBuffers.find(expectedBuffer) != parameterDescription.UniformBuffers.end(), expectedBuffer)
		B3D_TEST_ASSERT_MSG(parameterDescription.UniformBufferMembers.find(expectedMember) != parameterDescription.UniformBufferMembers.end(), expectedMember)
		B3D_TEST_ASSERT(!tableLayout.IsEmpty())
		if(tableLayout.IsEmpty())
			continue;

		// The uniform buffer is flagged and listed directly in the root table at its argument-table index rather than
		// as an argument-buffer member of the set's table
		for(const auto& [name, uniformBuffer] : parameterDescription.UniformBuffers)
		{
			B3D_TEST_ASSERT_MSG(uniformBuffer.UsesDynamicOffset, name)

			const GpuDescriptorTableEntry* rootEntry = tableLayout.FindRootResourceEntry(GpuParameterType::UniformBuffer, uniformBuffer.Set, uniformBuffer.Slot);

			B3D_TEST_ASSERT_MSG(rootEntry != nullptr && rootEntry->BindingIndex == expectedIndex, name)
		}

		for(u32 tableIndex = 1; tableIndex < (u32)tableLayout.Tables.size(); tableIndex++)
		{
			for(const GpuDescriptorTableEntry& entry : tableLayout.GetEntries(tableLayout.Tables[tableIndex]))
				B3D_TEST_ASSERT(entry.Kind != GpuDescriptorEntryKind::Resource || entry.Type != GpuParameterType::UniformBuffer)
		}

		// The texture still lives in the fragment stage's argument buffer
		bool foundTexture = false;
		for(u32 tableIndex = 1; tableIndex < (u32)tableLayout.Tables.size(); tableIndex++)
		{
			for(const GpuDescriptorTableEntry& entry : tableLayout.GetEntries(tableLayout.Tables[tableIndex]))
				foundTexture |= entry.Kind == GpuDescriptorEntryKind::Resource && entry.Type == GpuParameterType::SampledTexture;
		}

		if(stage == GPT_FRAGMENT_PROGRAM)
			B3D_TEST_ASSERT(foundTexture)
	}
}
#endif

void GpuBackendTestSuite::TestPushConstantMetadata()
{
	GpuProgramParameterDescription vertex;
	vertex.PushConstantBufferSize = 4;
	GpuProgramParameterDescription fragment;
	fragment.PushConstantBufferSize = 12;

	GpuProgramParameterDescription combined;
	B3D_TEST_ASSERT(combined.TryCombine(vertex, GpuProgramStageBit::Vertex).IsSuccessful())
	B3D_TEST_ASSERT(combined.TryCombine(fragment, GpuProgramStageBit::Fragment).IsSuccessful())
	B3D_TEST_ASSERT(combined.PushConstantBufferSize == 12)

	TInlineArray<GpuProgramParameterDescription, 4> perSetDescriptions;
	combined.SplitBySet(perSetDescriptions);
	B3D_TEST_ASSERT(perSetDescriptions.Size() == 0)
}

void GpuBackendTestSuite::TestPushConstantWrites()
{
	const Array<u32, 4> values = { 1, 2, 3, 4 };

	B3D_TEST_ASSERT(ValidatePushConstantWrite(16, 0, 16, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(ValidatePushConstantWrite(16, 4, 8, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(ValidatePushConstantWrite(0, 16, 0, nullptr).IsSuccessful())
	B3D_TEST_ASSERT(!ValidatePushConstantWrite(0, 0, 4, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(!ValidatePushConstantWrite(8, 0, 4, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(!ValidatePushConstantWrite(16, 2, 4, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(!ValidatePushConstantWrite(16, 12, 8, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(!ValidatePushConstantWrite(16, 0, 4, nullptr).IsSuccessful())

	GpuPushConstantPayload payload;
	payload.Write(4, 8, values.data() + 1);
	B3D_TEST_ASSERT(payload.Values[0] == 0)
	B3D_TEST_ASSERT(payload.Values[1] == 2)
	B3D_TEST_ASSERT(payload.Values[2] == 3)
	B3D_TEST_ASSERT(payload.Values[3] == 0)

	const u32 lastValue = 9;
	payload.Write(12, 4, &lastValue);
	B3D_TEST_ASSERT(payload.Values[3] == 9)
	payload.Clear();
	const Array<u32, 4> emptyValues{};
	B3D_TEST_ASSERT(payload.Values == emptyValues)
}

void GpuBackendTestSuite::TestPushConstantSerialization()
{
	GpuProgramCreateInformation createInformation;
	createInformation.Name = "PushConstantSerialization";
	createInformation.Type = GPT_COMPUTE_PROGRAM;
	createInformation.ShaderReflection = B3DMakeShared<ShaderReflection>();
	createInformation.ShaderReflection->EntryPoints[createInformation.EntryPoint].Type = createInformation.Type;
	createInformation.ShaderReflection->EntryPoints[createInformation.EntryPoint].PushConstantBufferSize = 12;
	createInformation.Bytecode = B3DMakeShared<GpuProgramBytecode>();
	createInformation.Bytecode->ParameterDescription = B3DMakeShared<GpuProgramParameterDescription>();
	createInformation.Bytecode->ParameterDescription->PushConstantBufferSize = 12;

	const TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(&createInformation, stream);
	stream->Seek(0);

	const TShared<GpuProgramCreateInformation> decoded = B3DRTTICast<GpuProgramCreateInformation>(
		serializer.Decode(stream, (u32)stream->Size()));
	B3D_TEST_ASSERT(decoded != nullptr)
	if(decoded == nullptr)
		return;

	B3D_TEST_ASSERT(decoded->GetEntryPointReflection().PushConstantBufferSize == 12)
	B3D_TEST_ASSERT(decoded->Bytecode != nullptr)
	if(decoded->Bytecode == nullptr)
		return;

	B3D_TEST_ASSERT(decoded->Bytecode->ParameterDescription != nullptr)
	if(decoded->Bytecode->ParameterDescription == nullptr)
		return;

	B3D_TEST_ASSERT(decoded->Bytecode->ParameterDescription->PushConstantBufferSize == 12)
}

#if !B3D_PLATFORM_PS5
void GpuBackendTestSuite::TestHostPushConstantShaderCompilation()
{
	TestPushConstantShaderCompilation(*this, "vksl");
#if B3D_PLATFORM_MACOS
	TestPushConstantShaderCompilation(*this, "msl");
#endif
#if B3D_PLATFORM_WIN32
	TestPushConstantShaderCompilation(*this, "hlsl");
#endif
}
#endif

void GpuBackendTestSuite::TestVulkanStorageBufferAccessReflection()
{
	const TShared<IGpuBytecodeCompiler> compiler = ShaderCompilers::Instance().GetBytecodeCompiler("vksl");
	B3D_TEST_ASSERT(compiler != nullptr)
	if(compiler == nullptr)
		return;

	GpuProgramCreateInformation createInformation;
	createInformation.Name = "StorageBufferAccessReflection";
	createInformation.Language = "vksl";
	createInformation.Type = GPT_COMPUTE_PROGRAM;
	createInformation.EntryPoint = "main";
	createInformation.Source = R"(
#version 450
layout(local_size_x = 1) in;
layout(set = 0, binding = 0, std430) readonly buffer ReadOnlyBlock { vec4 Values[]; } readOnlyData;
layout(set = 0, binding = 1, std430) buffer WritableBlock { vec4 Values[]; } writableData;
layout(set = 0, binding = 2, std430) writeonly buffer WriteOnlyBlock { vec4 Values[]; } writeOnlyData;
layout(set = 0, binding = 3, std430) buffer ReadOnlyMemberBlock { readonly vec4 Values[]; } readOnlyMemberData;
layout(set = 0, binding = 4, std430) buffer MixedMemberBlock { readonly vec4 Input; vec4 Output; } mixedMemberData;
void main()
{
	writeOnlyData.Values[0] = readOnlyData.Values[0] + writableData.Values[0] + readOnlyMemberData.Values[0];
	mixedMemberData.Output = mixedMemberData.Input;
}
)";

	const TShared<GpuProgramBytecode> bytecode = compiler->CompileBytecode(createInformation);
	B3D_TEST_ASSERT(bytecode != nullptr)
	if(bytecode == nullptr)
		return;

	B3D_TEST_ASSERT_MSG(bytecode->Instructions.Data != nullptr, bytecode->Messages)
	B3D_TEST_ASSERT(bytecode->ParameterDescription != nullptr)
	if(bytecode->ParameterDescription == nullptr)
		return;

	B3D_TEST_ASSERT(bytecode->ParameterDescription->Buffers.size() == 5)
	for(const auto& [name, buffer] : bytecode->ParameterDescription->Buffers)
	{
		const bool isReadOnly = buffer.Slot == 0 || buffer.Slot == 3;
		B3D_TEST_ASSERT_MSG(buffer.Type == (isReadOnly ? GPOT_STRUCTURED_BUFFER : GPOT_RWSTRUCTURED_BUFFER), name)
	}
}

void GpuBackendTestSuite::TestHlslShaderModel66Compilation()
{
#if B3D_PLATFORM_WIN32
	const TShared<IGpuBytecodeCompiler> compiler = ShaderCompilers::Instance().GetBytecodeCompiler("hlsl");
	B3D_TEST_ASSERT(compiler != nullptr)

	auto compile = [&](const String& name, GpuProgramType type, const String& source)
	{
		GpuProgramCreateInformation createInformation;
		createInformation.Name = name;
		createInformation.Source = source;
		createInformation.EntryPoint = "main";
		createInformation.Language = "hlsl";
		createInformation.Type = type;

		const TShared<GpuProgramBytecode> bytecode = compiler->CompileBytecode(createInformation);
		B3D_TEST_ASSERT(bytecode != nullptr)
		B3D_TEST_ASSERT(bytecode->Instructions.Data != nullptr)
		B3D_TEST_ASSERT(bytecode->Instructions.Size != 0)
		B3D_TEST_ASSERT(bytecode->CompilerId == "HLSL_DXC")
		B3D_TEST_ASSERT(compiler->IsUpToDate(*bytecode))

		bool containsDxilPart = false;
		for(u32 offset = 0; offset + 4 <= bytecode->Instructions.Size; offset++)
		{
			const u8* marker = bytecode->Instructions.Data + offset;
			if(marker[0] == 'D' && marker[1] == 'X' && marker[2] == 'I' && marker[3] == 'L')
			{
				containsDxilPart = true;
				break;
			}
		}
		B3D_TEST_ASSERT(containsDxilPart)

		return bytecode;
	};

	const String vertexSource = R"(
cbuffer FrameData : register(b2, space1)
{
	column_major float4x4 Transform;
	float Exponent;
};

struct VertexInput
{
	float3 Position : POSITION0;
	float2 Uv : TEXCOORD0;
};

struct VertexOutput
{
	float4 Position : SV_Position;
	float Value : TEXCOORD0;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.Position = mul(Transform, float4(input.Position, 1.0f));
	output.Value = pow(abs(input.Uv.x + 1.0f), Exponent);
	return output;
}
)";
	const TShared<GpuProgramBytecode> vertexBytecode = compile("HlslVertex", GPT_VERTEX_PROGRAM, vertexSource);
	B3D_TEST_ASSERT(vertexBytecode->VertexInput.size() == 2)
	B3D_TEST_ASSERT(vertexBytecode->ParameterDescription != nullptr)
	B3D_TEST_ASSERT(vertexBytecode->ParameterDescription->UniformBuffers.find("FrameData") != vertexBytecode->ParameterDescription->UniformBuffers.end())
	const auto transformMember = vertexBytecode->ParameterDescription->UniformBufferMembers.find("Transform");
	const auto exponentMember = vertexBytecode->ParameterDescription->UniformBufferMembers.find("Exponent");
	B3D_TEST_ASSERT(transformMember != vertexBytecode->ParameterDescription->UniformBufferMembers.end())
	B3D_TEST_ASSERT(exponentMember != vertexBytecode->ParameterDescription->UniformBufferMembers.end())
	B3D_TEST_ASSERT(transformMember->second.GpuOffset == 0)
	B3D_TEST_ASSERT(exponentMember->second.GpuOffset == 16)

	const String fragmentSource = R"(
Texture2D<float4> InputTextures[3] : register(t2, space1);
SamplerState InputSampler : register(s1, space1);

float4 main(float2 uv : TEXCOORD0) : SV_Target0
{
	return InputTextures[1].Sample(InputSampler, uv);
}
)";
	const TShared<GpuProgramBytecode> fragmentBytecode = compile("HlslFragment", GPT_FRAGMENT_PROGRAM, fragmentSource);
	B3D_TEST_ASSERT(fragmentBytecode->ParameterDescription != nullptr)
	B3D_TEST_ASSERT(fragmentBytecode->ParameterDescription->SampledTextures.find("InputTextures") != fragmentBytecode->ParameterDescription->SampledTextures.end())
	B3D_TEST_ASSERT(fragmentBytecode->ParameterDescription->Samplers.find("InputSampler") != fragmentBytecode->ParameterDescription->Samplers.end())
	B3D_TEST_ASSERT(fragmentBytecode->ResourceTableLayout != nullptr)
	B3D_TEST_ASSERT(fragmentBytecode->ResourceTableLayout->Tables.size() == 2)
	B3D_TEST_ASSERT(fragmentBytecode->ResourceTableLayout->Tables[1].Set == 1)
	const TArrayView<const GpuDescriptorTableEntry> fragmentEntries = fragmentBytecode->ResourceTableLayout->GetEntries(
		fragmentBytecode->ResourceTableLayout->Tables[1]);
	B3D_TEST_ASSERT(fragmentEntries.Size() == 2)
	B3D_TEST_ASSERT(fragmentEntries[1].DescriptorCount == 3)

	const String geometrySource = R"(
struct Vertex
{
	float4 Position : SV_Position;
};

[maxvertexcount(3)]
void main(triangle Vertex input[3], inout TriangleStream<Vertex> outputStream)
{
	outputStream.Append(input[0]);
	outputStream.Append(input[1]);
	outputStream.Append(input[2]);
}
)";
	compile("HlslGeometry", GPT_GEOMETRY_PROGRAM, geometrySource);

	const String hullSource = R"(
struct ControlPoint
{
	float4 Position : POSITION0;
};

struct PatchConstants
{
	float Edges[3] : SV_TessFactor;
	float Inside : SV_InsideTessFactor;
};

PatchConstants GetPatchConstants(InputPatch<ControlPoint, 3> input)
{
	PatchConstants output;
	output.Edges[0] = 1.0f;
	output.Edges[1] = 1.0f;
	output.Edges[2] = 1.0f;
	output.Inside = 1.0f;
	return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("GetPatchConstants")]
ControlPoint main(InputPatch<ControlPoint, 3> input, uint controlPointId : SV_OutputControlPointID)
{
	return input[controlPointId];
}
)";
	compile("HlslHull", GPT_HULL_PROGRAM, hullSource);

	const String domainSource = R"(
struct ControlPoint
{
	float4 Position : POSITION0;
};

struct PatchConstants
{
	float Edges[3] : SV_TessFactor;
	float Inside : SV_InsideTessFactor;
};

[domain("tri")]
float4 main(PatchConstants constants, float3 barycentric : SV_DomainLocation,
	const OutputPatch<ControlPoint, 3> input) : SV_Position
{
	return input[0].Position * barycentric.x + input[1].Position * barycentric.y +
		input[2].Position * barycentric.z + constants.Inside * 0.0f;
}
)";
	compile("HlslDomain", GPT_DOMAIN_PROGRAM, domainSource);

	const String computeSource = R"(
RWStructuredBuffer<uint> OutputData : register(u0, space2);

[numthreads(8, 4, 2)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	OutputData[dispatchThreadId.x] = dispatchThreadId.y;
}
)";
	const TShared<GpuProgramBytecode> computeBytecode = compile("HlslCompute", GPT_COMPUTE_PROGRAM, computeSource);
	B3D_TEST_ASSERT(computeBytecode->ThreadGroupSize[0] == 8)
	B3D_TEST_ASSERT(computeBytecode->ThreadGroupSize[1] == 4)
	B3D_TEST_ASSERT(computeBytecode->ThreadGroupSize[2] == 2)

	const String waveSource = R"(
RWStructuredBuffer<uint> OutputData : register(u0);

[numthreads(32, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	OutputData[dispatchThreadId.x] = WaveActiveSum(dispatchThreadId.x + 1);
}
)";
	compile("HlslWave", GPT_COMPUTE_PROGRAM, waveSource);

	const String bindlessSource = R"(
cbuffer ResourceIndices : register(b0)
{
	uint TextureIndex;
	uint SamplerIndex;
};

float4 main(float2 uv : TEXCOORD0) : SV_Target0
{
	Texture2D<float4> textureResource = ResourceDescriptorHeap[TextureIndex];
	SamplerState samplerResource = SamplerDescriptorHeap[SamplerIndex];
	return textureResource.Sample(samplerResource, uv);
}
)";
	compile("HlslBindless", GPT_FRAGMENT_PROGRAM, bindlessSource);

	GpuProgramBytecode fxcBytecode;
	fxcBytecode.CompilerId = "HLSL_FXC";
	fxcBytecode.CompilerVersion = 6;
	B3D_TEST_ASSERT(!compiler->IsUpToDate(fxcBytecode))
#endif
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

	tracker.TrackImageAccess(&image, depthRange, GpuImageLayout::ShaderReadOnly, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, barrierHelper);
	tracker.TrackImageAccess(&image, stencilRange, GpuImageLayout::DepthStencilAttachment, GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests, GpuAccessFlag::Write, barrierHelper);
	tracker.CommitPendingAccesses();

	const GpuImageSubresourceTrackingState& depthState = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Depth);
	const GpuImageSubresourceTrackingState& stencilState = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Stencil);
	B3D_TEST_ASSERT(depthState.Access == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(depthState.CurrentLayout == GpuImageLayout::ShaderReadOnly)
	B3D_TEST_ASSERT(stencilState.Access == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(stencilState.CurrentLayout == GpuImageLayout::DepthStencilAttachment)
	B3D_TEST_ASSERT(depthState.HazardState != stencilState.HazardState)

	SubmissionImageTestVisitor visitor;
	tracker.ResolveSubmissionTransitions(GpuQueueId(GQT_GRAPHICS, 0), kTestFrameIndex, visitor);
	B3D_TEST_ASSERT(visitor.VisitedAspects == (GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil))
	B3D_TEST_ASSERT(visitor.NativeStates.Size() == 2)
	B3D_TEST_ASSERT(visitor.NativeStates[0] != visitor.NativeStates[1])

	const TArrayView<const GpuImageSubresourceTrackingState> finalPartitions = tracker.GetSubresourceTrackingStatesForImage(&image);
	for(const GpuImageSubresourceTrackingState& partition : finalPartitions)
		B3D_TEST_ASSERT(partition.Range.HasSingleAspect())

	tracker.NotifyUnbound();
	tracker.Clear();

	SubmissionTestImage combinedUseImage(1, 1, GpuTextureAspectFlag::Depth);
	SubmissionTestTracker combinedUseTracker;
	const GpuResourceUseFlags combinedUseFlags = GpuResourceUseFlag::DepthStencilAttachment |
		GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader;
	combinedUseTracker.TrackImageAccess(&combinedUseImage, depthRange, GpuImageLayout::DepthStencilReadOnly, GpuBackendUtility::GetStageFlags(combinedUseFlags), GpuAccessFlag::Read, barrierHelper);
	combinedUseTracker.CommitPendingAccesses();

	const GpuImageSubresourceTrackingState& combinedUseState = combinedUseTracker.GetSubresourceTrackingState(
		&combinedUseImage, 0, 0, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(combinedUseState.CurrentLayout == GpuImageLayout::DepthStencilReadOnly)
	B3D_TEST_ASSERT(combinedUseState.HazardState->AllAccessScope.ReadStages ==
		(GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests | GpuStageFlag::FragmentShaderNonUniform))

	combinedUseTracker.NotifyUnbound();
	combinedUseTracker.Clear();
}

void GpuBackendTestSuite::TestWholeImageRegistration()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	SubmissionTestImage image(2, 3, GpuTextureAspectFlag::Color);
	GpuImageSubresource& fullRangeSubresource = *image.GetFullRangeSubresource();
	SubmissionTestBarrierHelper barrierHelper;

	// A full-range access registers only the full-range subresource
	SubmissionTestTracker fullRangeTracker;
	fullRangeTracker.TrackImageAccess(&image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, barrierHelper);
	B3D_TEST_ASSERT(image.GetBoundCount() == 1)
	B3D_TEST_ASSERT(fullRangeSubresource.GetBoundCount() == 1)
	for(u32 mipLevel = 0; mipLevel < 3; mipLevel++)
	{
		for(u32 face = 0; face < 2; face++)
		{
			B3D_TEST_ASSERT(image.GetSubresource(face, mipLevel, GpuTextureAspectFlag::Color)->GetBoundCount() == 0)
			B3D_TEST_ASSERT(image.GetSubresourceBoundCount(face, mipLevel) == 1)
		}
	}

	// A partial access registers only the subresources it touches
	SubmissionTestTracker partialTracker;
	partialTracker.TrackImageAccess(&image, GpuTextureSubresourceRange(1, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::ShaderReadOnly, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, barrierHelper);
	B3D_TEST_ASSERT(image.GetBoundCount() == 2)
	B3D_TEST_ASSERT(fullRangeSubresource.GetBoundCount() == 1)
	B3D_TEST_ASSERT(image.GetSubresource(0, 1, GpuTextureAspectFlag::Color)->GetBoundCount() == 1)
	B3D_TEST_ASSERT(image.GetSubresourceBoundCount(0, 1) == 2)
	B3D_TEST_ASSERT(image.GetSubresourceBoundCount(1, 1) == 1)

	// Full-range uses count towards every subresource
	SubmissionImageRecordingVisitor visitor;
	fullRangeTracker.CommitPendingAccesses();
	fullRangeTracker.ResolveSubmissionTransitions(graphics, kTestFrameIndex, visitor);
	fullRangeTracker.NotifyUsed(graphics);
	for(u32 mipLevel = 0; mipLevel < 3; mipLevel++)
	{
		for(u32 face = 0; face < 2; face++)
		{
			B3D_TEST_ASSERT(image.GetSubresourceUseInfo(face, mipLevel, GpuAccessFlag::Read).IsSet(graphics))
			B3D_TEST_ASSERT(image.GetSubresourceUseCount(face, mipLevel) == 1)
		}
	}

	fullRangeTracker.NotifyDone(graphics);
	fullRangeTracker.Clear();
	partialTracker.NotifyUnbound();
	partialTracker.Clear();
	B3D_TEST_ASSERT(image.GetBoundCount() == 0 && fullRangeSubresource.GetBoundCount() == 0)
	B3D_TEST_ASSERT(image.GetSubresourceBoundCount(0, 1) == 0 && image.GetSubresourceUseCount(0, 1) == 0)

	// A render-pass style access to both aspects of a depth-stencil image covers its full range
	SubmissionTestImage depthStencilImage(1, 1, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
	SubmissionTestTracker depthStencilTracker;
	depthStencilTracker.TrackImageAccess(&depthStencilImage, depthStencilImage.GetRange(), GpuImageLayout::DepthStencilAttachment, GpuStageFlag::LateFragmentTests, GpuAccessFlag::Write, barrierHelper);
	B3D_TEST_ASSERT(depthStencilImage.GetFullRangeSubresource()->GetBoundCount() == 1)
	B3D_TEST_ASSERT(depthStencilImage.GetSubresource(0, 0, GpuTextureAspectFlag::Depth)->GetBoundCount() == 0)
	B3D_TEST_ASSERT(depthStencilImage.GetSubresourceBoundCount(0, 0) == 1)
	depthStencilTracker.NotifyUnbound();
	depthStencilTracker.Clear();
}

void GpuBackendTestSuite::TestWholeImageSubmission()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	SubmissionTestImage image(1, 10, GpuTextureAspectFlag::Color);
	GpuImageSubresource& fullRangeSubresource = *image.GetFullRangeSubresource();
	B3D_TEST_ASSERT(image.HasUniformSubmissionState())

	const SubmissionImageRecordingVisitor visitor = ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::TransferDestination, GpuAccessFlag::Write, graphics);
	B3D_TEST_ASSERT(visitor.Ranges.Size() == 1)
	B3D_TEST_ASSERT(visitor.Ranges[0].BaseMipLevel == 0 && visitor.Ranges[0].MipLevelCount == 10)
	B3D_TEST_ASSERT(visitor.Ranges[0].BaseArrayLayer == 0 && visitor.Ranges[0].ArrayLayerCount == 1)
	B3D_TEST_ASSERT(visitor.NativeStates[0] == &fullRangeSubresource.NativeState)

	B3D_TEST_ASSERT(image.HasUniformSubmissionState())
	B3D_TEST_ASSERT(fullRangeSubresource.SubmissionState.HasWriter)
	B3D_TEST_ASSERT(fullRangeSubresource.NativeState.Layout == (u32)GpuImageLayout::TransferDestination)
	B3D_TEST_ASSERT(&image.GetSubmissionStateResource(0, 5, GpuTextureAspectFlag::Color) == &fullRangeSubresource)

	// Subresources are not touched while the image is uniform
	B3D_TEST_ASSERT(!image.GetSubresource(0, 5, GpuTextureAspectFlag::Color)->SubmissionState.HasWriter)
}

void GpuBackendTestSuite::TestImageSplit()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	SubmissionTestImage image(1, 4, GpuTextureAspectFlag::Color);
	ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::TransferDestination, GpuAccessFlag::Write, graphics);
	B3D_TEST_ASSERT(image.HasUniformSubmissionState())

	const SubmissionImageRecordingVisitor visitor = ExecuteTestImageAccess(image, GpuTextureSubresourceRange(2, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::ShaderReadOnly, GpuAccessFlag::Read, graphics);
	B3D_TEST_ASSERT(visitor.Ranges.Size() == 1)
	B3D_TEST_ASSERT(visitor.NativeStates[0] == &image.GetSubresource(0, 2, GpuTextureAspectFlag::Color)->NativeState)

	// The read leaves mip 2 in a different layout, so the image stays split
	B3D_TEST_ASSERT(!image.HasUniformSubmissionState())

	// Untouched subresources inherit the uniform state
	const GpuImageSubresource& untouched = image.GetSubmissionStateResource(0, 0, GpuTextureAspectFlag::Color);
	B3D_TEST_ASSERT(&untouched == image.GetSubresource(0, 0, GpuTextureAspectFlag::Color))
	B3D_TEST_ASSERT(untouched.SubmissionState.HasWriter && untouched.SubmissionState.WriterQueueId.Id == graphics.Id)
	B3D_TEST_ASSERT(untouched.SubmissionState.ReaderQueues.IsEmpty())
	B3D_TEST_ASSERT(untouched.NativeState.Layout == (u32)GpuImageLayout::TransferDestination)

	// The read's submission transitions mip 2 to a new layout, which counts as a write on its queue
	const GpuImageSubresource& touched = image.GetSubmissionStateResource(0, 2, GpuTextureAspectFlag::Color);
	B3D_TEST_ASSERT(touched.SubmissionState.HasWriter && touched.SubmissionState.WriterQueueId.Id == graphics.Id)
	B3D_TEST_ASSERT(touched.SubmissionState.ReaderQueues.IsEmpty())
	B3D_TEST_ASSERT(touched.NativeState.Layout == (u32)GpuImageLayout::ShaderReadOnly)
}

void GpuBackendTestSuite::TestImageMerge()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);

	// A full-range command buffer merges a split image, even when its subresources started in different layouts
	{
		SubmissionTestImage image(1, 4, GpuTextureAspectFlag::Color);
		ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::TransferDestination, GpuAccessFlag::Write, graphics);
		ExecuteTestImageAccess(image, GpuTextureSubresourceRange(2, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::ShaderReadOnly, GpuAccessFlag::Read, graphics);
		B3D_TEST_ASSERT(!image.HasUniformSubmissionState())

		const SubmissionImageRecordingVisitor visitor = ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuAccessFlag::Read, graphics);
		B3D_TEST_ASSERT(visitor.Ranges.Size() == 4)
		B3D_TEST_ASSERT(image.HasUniformSubmissionState())

		const GpuImageSubresource& fullRangeSubresource = *image.GetFullRangeSubresource();
		B3D_TEST_ASSERT(fullRangeSubresource.NativeState.Layout == (u32)GpuImageLayout::ShaderReadOnly)
		B3D_TEST_ASSERT(fullRangeSubresource.SubmissionState.HasWriter && fullRangeSubresource.SubmissionState.WriterQueueId.Id == graphics.Id)
		B3D_TEST_ASSERT(fullRangeSubresource.SubmissionState.ReaderQueues.IsSet(graphics))

		// The next full-range command buffer resolves one transition
		const SubmissionImageRecordingVisitor nextVisitor = ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuAccessFlag::Read, graphics);
		B3D_TEST_ASSERT(nextVisitor.Ranges.Size() == 1)
	}

	// A partial command buffer merges only if it leaves every subresource in the same native state
	{
		SubmissionTestImage image(1, 3, GpuTextureAspectFlag::Color);
		ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::TransferDestination, GpuAccessFlag::Write, graphics);
		ExecuteTestImageAccess(image, GpuTextureSubresourceRange(1, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::TransferDestination, GpuAccessFlag::Write, graphics);
		B3D_TEST_ASSERT(image.HasUniformSubmissionState())

		ExecuteTestImageAccess(image, GpuTextureSubresourceRange(1, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::ShaderReadOnly, GpuAccessFlag::Read, graphics);
		B3D_TEST_ASSERT(!image.HasUniformSubmissionState())
	}

	// Writers on different queues cannot share one state
	{
		SubmissionTestImage image(1, 2, GpuTextureAspectFlag::Color);
		ExecuteTestImageAccess(image, GpuTextureSubresourceRange(0, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::TransferDestination, GpuAccessFlag::Write, graphics);
		ExecuteTestImageAccess(image, GpuTextureSubresourceRange(1, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::TransferDestination, GpuAccessFlag::Write, compute);
		B3D_TEST_ASSERT(image.GetSubresource(0, 0, GpuTextureAspectFlag::Color)->NativeState == image.GetSubresource(0, 1, GpuTextureAspectFlag::Color)->NativeState)
		B3D_TEST_ASSERT(!image.HasUniformSubmissionState())
	}

	// Depth-stencil images keep per-aspect state and never merge
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
		B3D_TEST_ASSERT(!image.HasUniformSubmissionState())

		const SubmissionImageRecordingVisitor visitor = ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::DepthStencilAttachment, GpuAccessFlag::Write, graphics);
		B3D_TEST_ASSERT(visitor.Ranges.Size() == 2)
		B3D_TEST_ASSERT(!image.HasUniformSubmissionState())
	}
}

void GpuBackendTestSuite::TestMergedStateWaits()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);
	const GpuQueueId transfer(GQT_TRANSFER, 0);

	// Every access uses the committed layout, so no submission transitions the layout (a transition counts as a write)
	SubmissionTestImage image(1, 2, GpuTextureAspectFlag::Color);
	image.SetNativeLayout(GpuImageLayout::General);

	// A partial read stays in flight across the merge
	SubmissionTestTracker partialReader;
	SubmissionImageRecordingVisitor partialVisitor;
	SubmitTestImageAccess(partialReader, partialVisitor, image, GpuTextureSubresourceRange(0, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::General, GpuAccessFlag::Read, compute);

	// The read leaves every subresource in one native state, so the image merges right away. The read stays registered on its own subresource.
	B3D_TEST_ASSERT(image.HasUniformSubmissionState())

	// A full-range read leaves every subresource in the same layout, and stays in flight as well
	SubmissionTestTracker fullRangeReader;
	SubmissionImageRecordingVisitor fullRangeVisitor;
	SubmitTestImageAccess(fullRangeReader, fullRangeVisitor, image, image.GetRange(), GpuImageLayout::General, GpuAccessFlag::Read, transfer);
	B3D_TEST_ASSERT(image.HasUniformSubmissionState())

	const GpuResourceSubmissionState& mergedState = image.GetFullRangeSubresource()->SubmissionState;
	B3D_TEST_ASSERT(mergedState.ReaderQueues.IsSet(compute) && mergedState.ReaderQueues.IsSet(transfer))

	// A write must wait for both the merged full-range reader and the partial reader from before the merge
	SubmissionTestTracker writer;
	SubmissionImageRecordingVisitor writeVisitor;
	SubmitTestImageAccess(writer, writeVisitor, image, image.GetRange(), GpuImageLayout::General, GpuAccessFlag::Write, graphics);
	B3D_TEST_ASSERT(writeVisitor.Ranges.Size() == 1)
	B3D_TEST_ASSERT(writeVisitor.ParallelAccessWaitMask.IsSet(compute))
	B3D_TEST_ASSERT(writeVisitor.ParallelAccessWaitMask.IsSet(transfer))

	for(auto [tracker, queue] : { std::make_pair(&partialReader, compute), std::make_pair(&fullRangeReader, transfer), std::make_pair(&writer, graphics) })
	{
		tracker->NotifyDone(queue);
		tracker->Clear();
	}

	// Completed readers need no wait
	const SubmissionImageRecordingVisitor readVisitor = ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::General, GpuAccessFlag::Read, compute);
	B3D_TEST_ASSERT(readVisitor.ParallelAccessWaitMask.IsSet(graphics))
	const SubmissionImageRecordingVisitor rewriteVisitor = ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::General, GpuAccessFlag::Write, graphics);
	B3D_TEST_ASSERT(rewriteVisitor.ParallelAccessWaitMask.IsEmpty())
}

void GpuBackendTestSuite::TestSubmissionStateMerge()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);
	const GpuQueueId transfer(GQT_TRANSFER, 0);

	GpuResourceSubmissionState graphicsWriter;
	graphicsWriter.HasWriter = true;
	graphicsWriter.WriterQueueId = graphics;
	graphicsWriter.WriterHazards.WriteStages = GpuStageFlag::Transfer;
	graphicsWriter.WriterHazards.ReaderStages = GpuStageFlag::FragmentShaderNonUniform;
	graphicsWriter.WriterHazards.VisibleStages = GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform;
	graphicsWriter.AcquiredQueues = GpuQueueMask(graphics) | compute;
	graphicsWriter.ReaderQueues = GpuQueueMask(compute);
	graphicsWriter.ReaderStages = GpuStageFlag::ComputeShaderNonUniform;

	GpuResourceSubmissionState reader;
	reader.ReaderQueues = GpuQueueMask(transfer);
	reader.ReaderStages = GpuStageFlag::Transfer;

	// No writers: readers are combined
	{
		GpuResourceSubmissionState merged = reader;
		GpuResourceSubmissionState otherReader;
		otherReader.ReaderQueues = GpuQueueMask(compute);
		otherReader.ReaderStages = GpuStageFlag::ComputeShaderNonUniform;
		B3D_TEST_ASSERT(merged.TryMerge(otherReader))
		B3D_TEST_ASSERT(!merged.HasWriter)
		B3D_TEST_ASSERT(merged.ReaderQueues.IsSet(transfer) && merged.ReaderQueues.IsSet(compute))
		B3D_TEST_ASSERT(merged.ReaderStages == (GpuStageFlag::Transfer | GpuStageFlag::ComputeShaderNonUniform))
	}

	// One writer: its write epoch is kept, in either order
	for(bool writerFirst : { false, true })
	{
		GpuResourceSubmissionState merged = writerFirst ? graphicsWriter : reader;
		B3D_TEST_ASSERT(merged.TryMerge(writerFirst ? reader : graphicsWriter))
		B3D_TEST_ASSERT(merged.HasWriter && merged.WriterQueueId.Id == graphics.Id)
		B3D_TEST_ASSERT(merged.WriterHazards.WriteStages == GpuStageFlag::Transfer)
		B3D_TEST_ASSERT(merged.WriterHazards.VisibleStages == graphicsWriter.WriterHazards.VisibleStages)
		B3D_TEST_ASSERT(merged.AcquiredQueues == graphicsWriter.AcquiredQueues)
		B3D_TEST_ASSERT(merged.ReaderQueues.IsSet(transfer) && merged.ReaderQueues.IsSet(compute))
		B3D_TEST_ASSERT(merged.ReaderStages == (GpuStageFlag::Transfer | GpuStageFlag::ComputeShaderNonUniform))
	}

	// Writers on the same queue: source stages widen, visibility and acquisition narrow
	{
		GpuResourceSubmissionState otherWriter;
		otherWriter.HasWriter = true;
		otherWriter.WriterQueueId = graphics;
		otherWriter.WriterHazards.WriteStages = GpuStageFlag::ColorAttachment;
		otherWriter.WriterHazards.VisibleStages = GpuStageFlag::FragmentShaderNonUniform;
		otherWriter.AcquiredQueues = GpuQueueMask(graphics);

		GpuResourceSubmissionState merged = graphicsWriter;
		B3D_TEST_ASSERT(merged.TryMerge(otherWriter))
		B3D_TEST_ASSERT(merged.WriterHazards.WriteStages == (GpuStageFlag::Transfer | GpuStageFlag::ColorAttachment))
		B3D_TEST_ASSERT(merged.WriterHazards.ReaderStages == GpuStageFlag::FragmentShaderNonUniform)
		B3D_TEST_ASSERT(merged.WriterHazards.VisibleStages == GpuStageFlag::FragmentShaderNonUniform)
		B3D_TEST_ASSERT(merged.AcquiredQueues == GpuQueueMask(graphics))
	}

	// Writers on different queues: the merge fails and leaves the state unchanged
	{
		GpuResourceSubmissionState computeWriter;
		computeWriter.HasWriter = true;
		computeWriter.WriterQueueId = compute;
		computeWriter.WriterHazards.WriteStages = GpuStageFlag::ComputeShaderNonUniform;
		computeWriter.ReaderQueues = GpuQueueMask(transfer);

		GpuResourceSubmissionState merged = graphicsWriter;
		B3D_TEST_ASSERT(!merged.TryMerge(computeWriter))
		B3D_TEST_ASSERT(merged.WriterQueueId.Id == graphics.Id)
		B3D_TEST_ASSERT(merged.WriterHazards.WriteStages == graphicsWriter.WriterHazards.WriteStages)
		B3D_TEST_ASSERT(merged.WriterHazards.VisibleStages == graphicsWriter.WriterHazards.VisibleStages)
		B3D_TEST_ASSERT(merged.AcquiredQueues == graphicsWriter.AcquiredQueues)
		B3D_TEST_ASSERT(merged.ReaderQueues == graphicsWriter.ReaderQueues)
		B3D_TEST_ASSERT(merged.ReaderStages == graphicsWriter.ReaderStages)
	}
}

void GpuBackendTestSuite::TestFrameIndexClear()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);

	// Clearing drops all hazards
	{
		GpuResourceSubmissionState state;
		state.HasWriter = true;
		state.WriterQueueId = graphics;
		state.WriterHazards.WriteStages = GpuStageFlag::Transfer;
		state.AcquiredQueues = GpuQueueMask(graphics);
		state.ReaderQueues = GpuQueueMask(compute);
		state.ReaderStages = GpuStageFlag::ComputeShaderNonUniform;
		state.FrameIndex = 1;

		state.Clear();
		B3D_TEST_ASSERT(!state.HasWriter)
		B3D_TEST_ASSERT(state.WriterHazards.WriteStages == GpuStageFlag::None)
		B3D_TEST_ASSERT(state.AcquiredQueues.IsEmpty())
		B3D_TEST_ASSERT(state.ReaderQueues.IsEmpty())
		B3D_TEST_ASSERT(state.ReaderStages == GpuStageFlag::None)
		B3D_TEST_ASSERT(state.FrameIndex == 0)
	}

	// Build ignores the hazards of a source state from an earlier frame, and records the frame in the state it commits
	{
		GpuResourceSubmissionState computeWriter;
		computeWriter.HasWriter = true;
		computeWriter.WriterQueueId = compute;
		computeWriter.WriterHazards.WriteStages = GpuStageFlag::ComputeShaderNonUniform;
		computeWriter.AcquiredQueues = GpuQueueMask(compute);
		computeWriter.FrameIndex = 1;

		GpuResourceHazardState readHazardState;
		ResolveTestAccess(readHazardState, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);

		const GpuSubmissionTransition sameFrameRead = GpuSubmissionTransition::Build(computeWriter, 1, GpuQueueMask::kNone, graphics, readHazardState);
		B3D_TEST_ASSERT(sameFrameRead.ParallelAccessWaitMask == GpuQueueMask(compute))
		B3D_TEST_ASSERT(sameFrameRead.PostTransitionSubmissionState.HasWriter)
		B3D_TEST_ASSERT(sameFrameRead.PostTransitionSubmissionState.FrameIndex == 1)

		const GpuSubmissionTransition nextFrameRead = GpuSubmissionTransition::Build(computeWriter, 2, GpuQueueMask::kNone, graphics, readHazardState);
		B3D_TEST_ASSERT(nextFrameRead.ParallelAccessWaitMask.IsEmpty())
		B3D_TEST_ASSERT(nextFrameRead.ExclusiveAccessWaitMask.IsEmpty())
		B3D_TEST_ASSERT(!nextFrameRead.HasSameQueueDependency())
		B3D_TEST_ASSERT(!nextFrameRead.PostTransitionSubmissionState.HasWriter)
		B3D_TEST_ASSERT(nextFrameRead.PostTransitionSubmissionState.ReaderQueues == GpuQueueMask(graphics))
		B3D_TEST_ASSERT(nextFrameRead.PostTransitionSubmissionState.FrameIndex == 2)

		GpuResourceHazardState writeHazardState;
		ResolveTestAccess(writeHazardState, GpuStageFlag::Transfer, GpuAccessFlag::Write);

		const GpuSubmissionTransition laterWrite = GpuSubmissionTransition::Build(computeWriter, 3, GpuQueueMask::kNone, graphics, writeHazardState);
		B3D_TEST_ASSERT(laterWrite.ParallelAccessWaitMask.IsEmpty())
		B3D_TEST_ASSERT(laterWrite.PostTransitionSubmissionState.HasWriter && laterWrite.PostTransitionSubmissionState.WriterQueueId.Id == graphics.Id)
		B3D_TEST_ASSERT(laterWrite.PostTransitionSubmissionState.FrameIndex == 3)
	}

	// Writers from the same frame are kept, writers from an earlier frame are not
	{
		SubmissionTestBuffer buffer;
		ResolveTestSubmission(buffer, compute, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, 4);

		const SubmissionTestResult sameFrameRead = ResolveTestSubmission(buffer, graphics, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, 4);
		B3D_TEST_ASSERT(sameFrameRead.ParallelAccessWaitMask == GpuQueueMask(compute))

		SubmissionTestBuffer otherBuffer;
		ResolveTestSubmission(otherBuffer, compute, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, 4);

		const SubmissionTestResult nextFrameRead = ResolveTestSubmission(otherBuffer, graphics, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, 5);
		B3D_TEST_ASSERT(nextFrameRead.ParallelAccessWaitMask.IsEmpty())
		B3D_TEST_ASSERT(!nextFrameRead.MemoryBarrier.IsValid() && !nextFrameRead.ExecutionBarrier.IsValid())
	}

	// Readers from an earlier frame need no wait, even while in flight
	{
		SubmissionTestBuffer buffer;
		ResolveTestSubmission(buffer, graphics, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, 0);
		BeginTestRead(buffer, graphics);

		const SubmissionTestResult nextFrameWrite = ResolveTestSubmission(buffer, compute, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, 1);
		B3D_TEST_ASSERT(nextFrameWrite.ParallelAccessWaitMask.IsEmpty())
		B3D_TEST_ASSERT(nextFrameWrite.ExclusiveAccessWaitMask.IsEmpty())

		EndTestRead(buffer, graphics);
	}

	// Subresources written on different queues merge in a later frame. Every access uses the committed layout, so no read transitions the layout.
	{
		SubmissionTestImage image(1, 2, GpuTextureAspectFlag::Color);
		image.SetNativeLayout(GpuImageLayout::General);

		const GpuTextureSubresourceRange mip0(0, 1, 0, 1, GpuTextureAspectFlag::Color);
		const GpuTextureSubresourceRange mip1(1, 1, 0, 1, GpuTextureAspectFlag::Color);
		ExecuteTestImageAccess(image, mip0, GpuImageLayout::General, GpuAccessFlag::Write, compute, 0);
		ExecuteTestImageAccess(image, mip1, GpuImageLayout::General, GpuAccessFlag::Write, graphics, 0);

		const SubmissionImageRecordingVisitor sameFrameRead = ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::General, GpuAccessFlag::Read, graphics, 0);
		B3D_TEST_ASSERT(sameFrameRead.Ranges.Size() == 2)
		B3D_TEST_ASSERT(sameFrameRead.ParallelAccessWaitMask == GpuQueueMask(compute))
		B3D_TEST_ASSERT(!image.HasUniformSubmissionState())

		const SubmissionImageRecordingVisitor nextFrameRead = ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::General, GpuAccessFlag::Read, graphics, 1);
		B3D_TEST_ASSERT(nextFrameRead.Ranges.Size() == 2)
		B3D_TEST_ASSERT(nextFrameRead.ParallelAccessWaitMask.IsEmpty())
		B3D_TEST_ASSERT(image.HasUniformSubmissionState())
		B3D_TEST_ASSERT(!image.GetFullRangeSubresource()->SubmissionState.HasWriter)

		const SubmissionImageRecordingVisitor uniformRead = ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::General, GpuAccessFlag::Read, graphics, 1);
		B3D_TEST_ASSERT(uniformRead.Ranges.Size() == 1)
		B3D_TEST_ASSERT(image.IsFullRange(uniformRead.Ranges[0]))
	}

	// Subresources the submission doesn't touch are cleared during the merge
	{
		SubmissionTestImage image(1, 2, GpuTextureAspectFlag::Color);
		const GpuTextureSubresourceRange mip0(0, 1, 0, 1, GpuTextureAspectFlag::Color);
		const GpuTextureSubresourceRange mip1(1, 1, 0, 1, GpuTextureAspectFlag::Color);
		ExecuteTestImageAccess(image, mip0, GpuImageLayout::TransferDestination, GpuAccessFlag::Write, compute, 0);
		ExecuteTestImageAccess(image, mip1, GpuImageLayout::TransferDestination, GpuAccessFlag::Write, graphics, 0);
		B3D_TEST_ASSERT(!image.HasUniformSubmissionState())

		const SubmissionImageRecordingVisitor nextFrameWrite = ExecuteTestImageAccess(image, mip1, GpuImageLayout::TransferDestination, GpuAccessFlag::Write, graphics, 1);
		B3D_TEST_ASSERT(nextFrameWrite.Ranges.Size() == 1)
		B3D_TEST_ASSERT(image.HasUniformSubmissionState())

		const GpuResourceSubmissionState& mergedState = image.GetFullRangeSubresource()->SubmissionState;
		B3D_TEST_ASSERT(mergedState.HasWriter && mergedState.WriterQueueId.Id == graphics.Id)
		B3D_TEST_ASSERT(mergedState.FrameIndex == 1)
	}

	// Subresources written on different queues merge once a resting read transitions all of them, and rest after the next frame boundary
	{
		RestingTestImage image(1, 3);
		const GpuTextureSubresourceRange mip0(0, 1, 0, 1, GpuTextureAspectFlag::Color);
		const GpuTextureSubresourceRange mips1To2(1, 2, 0, 1, GpuTextureAspectFlag::Color);
		const GpuStageFlags readStages = GpuStageFlag::FragmentShaderNonUniform;

		auto fnExecuteImageAccess = [](IGpuImageResource& image, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access,
			GpuQueueId queueId, u32 frameIndex)
		{
			SubmissionTestTracker tracker;
			const Vector<RecordedSubmissionTransition> transitions = SubmitRecordedImageAccess(tracker, image, range, layout, stages, access, queueId, frameIndex);
			CompleteTestTracker(tracker, queueId);
			return transitions;
		};

		fnExecuteImageAccess(image, mip0, GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, compute, 0);
		fnExecuteImageAccess(image, mips1To2, GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, graphics, 0);
		B3D_TEST_ASSERT(!image.HasUniformSubmissionState())

		// Every surface leaves TransferDestination, so the transition makes the reading queue the writer of every surface
		const Vector<RecordedSubmissionTransition> sameFrameRead = fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 0);
		B3D_TEST_ASSERT(sameFrameRead.size() == 3)
		for(const RecordedSubmissionTransition& transition : sameFrameRead)
			B3D_TEST_ASSERT(transition.ParallelAccessWaitMask.IsSet(compute) == (transition.ImageRange.BaseMipLevel == 0))

		B3D_TEST_ASSERT(image.HasUniformSubmissionState())

		const GpuResourceSubmissionState& mergedState = image.GetFullRangeSubresource()->SubmissionState;
		B3D_TEST_ASSERT(mergedState.HasWriter && mergedState.WriterQueueId.Id == graphics.Id)

		// The transition's writer keeps the image from resting in its frame
		const Vector<RecordedSubmissionTransition> uniformRead = fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 0);
		B3D_TEST_ASSERT(uniformRead.size() == 1)
		B3D_TEST_ASSERT(image.IsFullRange(uniformRead[0].ImageRange))

		B3D_TEST_ASSERT(fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 1).empty())
	}
}

void GpuBackendTestSuite::TestRestingReaderStages()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);

	B3D_TEST_ASSERT(GpuBackendUtility::GetQueueStageFlags(GQT_GRAPHICS) == GpuStageFlag::All)
	B3D_TEST_ASSERT(GpuBackendUtility::GetQueueStageFlags(GQT_COMPUTE).IsSet(GpuStageFlag::ComputeShaderNonUniform))
	B3D_TEST_ASSERT(!GpuBackendUtility::GetQueueStageFlags(GQT_COMPUTE).IsSetAny(GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform))
	B3D_TEST_ASSERT(GpuBackendUtility::GetQueueStageFlags(GQT_TRANSFER) == (GpuStageFlag::Transfer | GpuStageFlag::Host))

	// A write only orders after the reader stages its own queue can execute; readers on other queues are waited on instead
	GpuResourceSubmissionState sharedState;
	sharedState.ReaderQueues = GpuQueueMask(graphics) | compute;
	sharedState.ReaderStages = GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform;

	GpuResourceHazardState writeHazardState;
	ResolveTestAccess(writeHazardState, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);

	const GpuQueueMask inFlightReadQueues = GpuQueueMask(graphics) | compute;
	const GpuSubmissionTransition computeWrite = GpuSubmissionTransition::Build(sharedState, kTestFrameIndex, inFlightReadQueues, compute, writeHazardState);
	B3D_TEST_ASSERT(computeWrite.ExecutionBarrier.SourceStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(computeWrite.ExclusiveAccessWaitMask == GpuQueueMask(graphics))
}

void GpuBackendTestSuite::TestRestingReadRecording()
{
	SubmissionTestBarrierHelper barrierHelper;
	const GpuResourceUseFlags fragmentSample = GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader;
	const GpuStageFlags fragmentSampleStages = GpuBackendUtility::GetStageFlags(fragmentSample);

	// Buffers
	{
		SubmissionTestTracker tracker;

		const GpuStageFlags uniformStages = GpuStageFlag::VertexShaderUniform | GpuStageFlag::FragmentShaderUniform;
		SubmissionTestBuffer uniformBuffer;
		tracker.TrackBufferAccess(&uniformBuffer, uniformStages, GpuAccessFlag::Read, barrierHelper);
		const GpuBufferTrackingState* const uniformState = tracker.FindBufferTrackingState(&uniformBuffer);
		B3D_TEST_ASSERT(uniformState->HasOnlyRestingReads())
		B3D_TEST_ASSERT(uniformState->HazardState == nullptr)
		B3D_TEST_ASSERT(uniformState->UseHandle.Flags == GpuAccessFlag::Read)
		B3D_TEST_ASSERT(uniformState->UseHandle.Stages == uniformStages)
		B3D_TEST_ASSERT(uniformBuffer.GetBoundCount() == 1)

		// Reads in any stage rest
		SubmissionTestBuffer transferBuffer;
		tracker.TrackBufferAccess(&transferBuffer, GpuStageFlag::Transfer, GpuAccessFlag::Read, barrierHelper);
		B3D_TEST_ASSERT(tracker.FindBufferTrackingState(&transferBuffer)->HasOnlyRestingReads())

		// A read after a tracked write stays tracked
		SubmissionTestBuffer writtenBuffer;
		tracker.TrackBufferAccess(&writtenBuffer, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, barrierHelper);
		tracker.CommitPendingAccesses();
		tracker.TrackBufferAccess(&writtenBuffer, GpuStageFlag::FragmentShaderUniform, GpuAccessFlag::Read, barrierHelper);
		tracker.CommitPendingAccesses();

		const GpuBufferTrackingState* const writtenState = tracker.FindBufferTrackingState(&writtenBuffer);
		B3D_TEST_ASSERT(!writtenState->HasOnlyRestingReads())
		B3D_TEST_ASSERT(writtenState->HazardState->AllAccessScope.ReadStages == GpuStageFlag::FragmentShaderUniform)
		B3D_TEST_ASSERT(writtenState->UseHandle.Stages == (GpuStageFlag::ComputeShaderNonUniform | GpuStageFlag::FragmentShaderUniform))

		tracker.NotifyUnbound();
		tracker.Clear();
		B3D_TEST_ASSERT(uniformBuffer.GetBoundCount() == 0)
	}

	// An image sampled over its full range registers only the full-range subresource
	{
		RestingTestImage image(1, 4);
		SubmissionTestTracker tracker;

		B3D_TEST_ASSERT(TrackImageBinding(tracker, &image, image.GetRange(), GpuImageLayout::ShaderReadOnly, fragmentSample, GpuAccessFlag::Read, barrierHelper))

		const GpuImageTrackingState* const imageState = tracker.FindImageTrackingState(&image);
		B3D_TEST_ASSERT(imageState->HasOnlyRestingReads())
		B3D_TEST_ASSERT(tracker.GetSubresourceTrackingStatesForImage(&image).Size() == 0)
		B3D_TEST_ASSERT(tracker.FindSubresourceTrackingState(&image, 0, 1, GpuTextureAspectFlag::Color) == nullptr)
		B3D_TEST_ASSERT(GpuBackendUtility::RangeEquals(imageState->Range, image.GetRange()))
		B3D_TEST_ASSERT(imageState->UseHandle.Flags == GpuAccessFlag::Read)
		B3D_TEST_ASSERT(imageState->UseHandle.Stages == fragmentSampleStages)

		B3D_TEST_ASSERT(image.GetBoundCount() == 1)
		B3D_TEST_ASSERT(image.GetFullRangeSubresource()->GetBoundCount() == 1)
		for(u32 mipLevel = 0; mipLevel < 4; mipLevel++)
		{
			B3D_TEST_ASSERT(image.GetSubresource(0, mipLevel, GpuTextureAspectFlag::Color)->GetBoundCount() == 0)
			B3D_TEST_ASSERT(image.GetSubresourceBoundCount(0, mipLevel) == 1)
		}

		tracker.NotifyUnbound();
		tracker.Clear();
		B3D_TEST_ASSERT(image.GetSubresourceBoundCount(0, 0) == 0)
	}

	// An image sampled on part of its mip chain registers only the sampled mips
	{
		RestingTestImage image(1, 4);
		SubmissionTestTracker tracker;

		const GpuTextureSubresourceRange sampledMips(1, 2, 0, 1, GpuTextureAspectFlag::Color);
		B3D_TEST_ASSERT(TrackImageBinding(tracker, &image, sampledMips, GpuImageLayout::ShaderReadOnly, fragmentSample, GpuAccessFlag::Read, barrierHelper))

		const GpuImageTrackingState* const imageState = tracker.FindImageTrackingState(&image);
		B3D_TEST_ASSERT(imageState->HasOnlyRestingReads())
		B3D_TEST_ASSERT(GpuBackendUtility::RangeEquals(imageState->Range, sampledMips))
		B3D_TEST_ASSERT(image.GetSubresource(0, 1, GpuTextureAspectFlag::Color)->GetBoundCount() == 1)
		B3D_TEST_ASSERT(image.GetSubresource(0, 2, GpuTextureAspectFlag::Color)->GetBoundCount() == 1)
		B3D_TEST_ASSERT(image.GetSubresourceBoundCount(0, 0) == 0)
		B3D_TEST_ASSERT(image.GetSubresourceBoundCount(0, 3) == 0)

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// Further resting reads widen the recorded range, and register the surfaces the bounding range adds
	{
		RestingTestImage image(1, 4);
		SubmissionTestTracker tracker;

		tracker.TrackImageAccess(&image, GpuTextureSubresourceRange(0, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::ShaderReadOnly, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, barrierHelper);
		tracker.TrackImageAccess(&image, GpuTextureSubresourceRange(2, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::ShaderReadOnly, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read, barrierHelper);

		const GpuImageTrackingState* const imageState = tracker.FindImageTrackingState(&image);
		B3D_TEST_ASSERT(imageState->HasOnlyRestingReads())
		B3D_TEST_ASSERT(GpuBackendUtility::RangeEquals(imageState->Range, GpuTextureSubresourceRange(0, 3, 0, 1, GpuTextureAspectFlag::Color)))
		B3D_TEST_ASSERT(imageState->UseHandle.Stages == (GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform))
		for(u32 mipLevel = 0; mipLevel < 3; mipLevel++)
			B3D_TEST_ASSERT(image.GetSubresource(0, mipLevel, GpuTextureAspectFlag::Color)->GetBoundCount() == 1)

		B3D_TEST_ASSERT(image.GetSubresourceBoundCount(0, 3) == 0)

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// Images that can't rest, reads outside the resting layout, and reads after a tracked write are tracked
	{
		SubmissionTestImage trackedImage(1, 1, GpuTextureAspectFlag::Color);
		RestingTestImage copiedImage(1, 1);
		RestingTestImage writtenImage(1, 1);
		SubmissionTestTracker tracker;

		const GpuTextureSubresourceRange range(0, 1, 0, 1, GpuTextureAspectFlag::Color);
		B3D_TEST_ASSERT(TrackImageBinding(tracker, &trackedImage, range, GpuImageLayout::ShaderReadOnly, fragmentSample, GpuAccessFlag::Read, barrierHelper))
		B3D_TEST_ASSERT(tracker.GetSubresourceTrackingStatesForImage(&trackedImage).Size() == 1)
		B3D_TEST_ASSERT(tracker.GetSubresourceTrackingStatesForImage(&trackedImage)[0].HazardState != nullptr)

		tracker.TrackImageAccess(&copiedImage, range, GpuImageLayout::TransferSource, GpuStageFlag::Transfer, GpuAccessFlag::Read, barrierHelper);
		B3D_TEST_ASSERT(tracker.GetSubresourceTrackingStatesForImage(&copiedImage).Size() == 1)

		tracker.TrackImageAccess(&writtenImage, range, GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, barrierHelper);
		tracker.CommitPendingAccesses();
		B3D_TEST_ASSERT(TrackImageBinding(tracker, &writtenImage, range, GpuImageLayout::ShaderReadOnly, fragmentSample, GpuAccessFlag::Read, barrierHelper))
		B3D_TEST_ASSERT(tracker.GetSubresourceTrackingStatesForImage(&writtenImage).Size() == 1)
		B3D_TEST_ASSERT(tracker.FindImageTrackingState(&writtenImage)->UseHandle.Stages == (fragmentSampleStages | GpuStageFlag::Transfer))

		tracker.NotifyUnbound();
		tracker.Clear();
	}
}

void GpuBackendTestSuite::TestRestingReadMaterialization()
{
	// Buffer read, then written
	{
		SubmissionTestBarrierHelper barrierHelper;
		SubmissionTestTracker tracker;
		SubmissionTestBuffer buffer;

		tracker.TrackBufferAccess(&buffer, GpuStageFlag::FragmentShaderUniform, GpuAccessFlag::Read, barrierHelper);
		tracker.CommitPendingAccesses();
		tracker.TrackBufferAccess(&buffer, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, barrierHelper);
		tracker.CommitPendingAccesses();

		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.SourceStages == GpuStageFlag::FragmentShaderUniform)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.SourceAccess == GpuAccessFlag::Read)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.DestinationAccess == GpuAccessFlag::Write)

		// Only the resting reads are recorded as reads, not the stages of the access that converted them
		const GpuBufferTrackingState* const bufferState = tracker.FindBufferTrackingState(&buffer);
		B3D_TEST_ASSERT(!bufferState->HasOnlyRestingReads())
		B3D_TEST_ASSERT(bufferState->HazardState->AllAccessScope.ReadStages == GpuStageFlag::FragmentShaderUniform)
		B3D_TEST_ASSERT(bufferState->HazardState->AllAccessScope.WriteStages == GpuStageFlag::ComputeShaderNonUniform)

		// An explicit barrier after resting reads orders after them, instead of becoming a leading barrier
		SubmissionTestBuffer barrierBuffer;
		tracker.TrackBufferAccess(&barrierBuffer, GpuStageFlag::VertexInputAttributes, GpuAccessFlag::Read, barrierHelper);
		tracker.TrackBufferBarrier(&barrierBuffer, GpuStageFlag::Transfer, GpuAccessFlag::Write, barrierHelper);
		B3D_TEST_ASSERT(!tracker.FindBufferTrackingState(&barrierBuffer)->HazardState->HasLeadingBarrier)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.SourceStages == GpuStageFlag::VertexInputAttributes)

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// Image sampled, then one mip written
	{
		SubmissionTestBarrierHelper barrierHelper;
		SubmissionTestTracker tracker;
		RestingTestImage image(1, 3);

		const GpuResourceUseFlags fragmentSample = GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader;
		B3D_TEST_ASSERT(TrackImageBinding(tracker, &image, image.GetRange(), GpuImageLayout::ShaderReadOnly, fragmentSample, GpuAccessFlag::Read, barrierHelper))
		tracker.CommitPendingAccesses();
		tracker.TrackImageAccess(&image, GpuTextureSubresourceRange(0, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, barrierHelper);
		tracker.CommitPendingAccesses();

		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.SourceStages == GpuBackendUtility::GetStageFlags(fragmentSample))
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierOldLayout == GpuImageLayout::ShaderReadOnly)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierNewLayout == GpuImageLayout::TransferDestination)

		const GpuImageSubresourceTrackingState* const writtenState = tracker.FindSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Color);
		B3D_TEST_ASSERT(writtenState->InitialLayout == GpuImageLayout::ShaderReadOnly)
		B3D_TEST_ASSERT(writtenState->RequiredLayout == GpuImageLayout::TransferDestination)
		B3D_TEST_ASSERT(writtenState->Access == (GpuAccessFlag::Read | GpuAccessFlag::Write))

		const GpuImageSubresourceTrackingState* const untouchedState = tracker.FindSubresourceTrackingState(&image, 0, 2, GpuTextureAspectFlag::Color);
		B3D_TEST_ASSERT(GpuBackendUtility::RangeEquals(untouchedState->Range, GpuTextureSubresourceRange(1, 2, 0, 1, GpuTextureAspectFlag::Color)))
		B3D_TEST_ASSERT(untouchedState->InitialLayout == GpuImageLayout::ShaderReadOnly)
		B3D_TEST_ASSERT(untouchedState->CurrentLayout == GpuImageLayout::ShaderReadOnly)
		B3D_TEST_ASSERT(untouchedState->Access == GpuAccessFlag::Read)
		B3D_TEST_ASSERT(untouchedState->HazardState->AllAccessScope.ReadStages == GpuBackendUtility::GetStageFlags(fragmentSample))
		B3D_TEST_ASSERT(!tracker.FindImageTrackingState(&image)->HasOnlyRestingReads())

		tracker.NotifyUnbound();
		tracker.Clear();
	}
}

void GpuBackendTestSuite::TestRestingSubmission()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);
	const GpuStageFlags readStages = GpuStageFlag::FragmentShaderNonUniform;
	const GpuTextureSubresourceRange mip0(0, 1, 0, 1, GpuTextureAspectFlag::Color);
	const GpuTextureSubresourceRange mip1(1, 1, 0, 1, GpuTextureAspectFlag::Color);
	const GpuTextureSubresourceRange mips1To2(1, 2, 0, 1, GpuTextureAspectFlag::Color);

	auto fnExecuteBufferAccess = [](IGpuBufferResource& buffer, GpuStageFlags stages, GpuAccessFlags access, GpuQueueId queueId, u32 frameIndex)
	{
		SubmissionTestTracker tracker;
		const Vector<RecordedSubmissionTransition> transitions = SubmitRecordedBufferAccess(tracker, buffer, stages, access, queueId, frameIndex);
		CompleteTestTracker(tracker, queueId);
		return transitions;
	};

	auto fnExecuteImageAccess = [](IGpuImageResource& image, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access, GpuQueueId queueId,
		u32 frameIndex)
	{
		SubmissionTestTracker tracker;
		const Vector<RecordedSubmissionTransition> transitions = SubmitRecordedImageAccess(tracker, image, range, layout, stages, access, queueId, frameIndex);
		CompleteTestTracker(tracker, queueId);
		return transitions;
	};

	// Buffer, frame of the write: resting reads take a Build, barrier-free after the first one
	SubmissionTestBuffer buffer;
	{
		fnExecuteBufferAccess(buffer, GpuStageFlag::Transfer, GpuAccessFlag::Write, graphics, 0);

		const Vector<RecordedSubmissionTransition> firstRead = fnExecuteBufferAccess(buffer, readStages, GpuAccessFlag::Read, graphics, 0);
		B3D_TEST_ASSERT(firstRead.size() == 1)
		B3D_TEST_ASSERT(firstRead[0].MemoryBarrier.SourceStages == GpuStageFlag::Transfer)
		B3D_TEST_ASSERT(firstRead[0].MemoryBarrier.SourceAccess == GpuAccessFlag::Write)
		B3D_TEST_ASSERT(firstRead[0].MemoryBarrier.DestinationStages == readStages)
		B3D_TEST_ASSERT(firstRead[0].MemoryBarrier.DestinationAccess == GpuAccessFlag::Read)

		const Vector<RecordedSubmissionTransition> secondRead = fnExecuteBufferAccess(buffer, readStages, GpuAccessFlag::Read, graphics, 0);
		B3D_TEST_ASSERT(secondRead.size() == 1)
		B3D_TEST_ASSERT(!secondRead[0].MemoryBarrier.IsValid() && !secondRead[0].ExecutionBarrier.IsValid())
		B3D_TEST_ASSERT(secondRead[0].ParallelAccessWaitMask.IsEmpty() && secondRead[0].ExclusiveAccessWaitMask.IsEmpty())
	}

	// Buffer, next frame: the read is recorded at rest
	SubmissionTestTracker restingReadTracker;
	{
		B3D_TEST_ASSERT(SubmitRecordedBufferAccess(restingReadTracker, buffer, readStages, GpuAccessFlag::Read, graphics, 1).empty())

		const GpuResourceSubmissionState& state = buffer.GetSubmissionState();
		B3D_TEST_ASSERT(!state.HasWriter)
		B3D_TEST_ASSERT(state.ReaderQueues == GpuQueueMask(graphics))
		B3D_TEST_ASSERT(state.ReaderStages == readStages)
		B3D_TEST_ASSERT(state.FrameIndex == 1)
	}

	// Reads at rest order later writes in the same frame, on another queue and on the same queue
	{
		const Vector<RecordedSubmissionTransition> computeWrite = fnExecuteBufferAccess(buffer, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, compute, 1);
		B3D_TEST_ASSERT(computeWrite.size() == 1)
		B3D_TEST_ASSERT(computeWrite[0].ExclusiveAccessWaitMask.IsSet(graphics))
		CompleteTestTracker(restingReadTracker, graphics);

		SubmissionTestBuffer otherBuffer;
		SubmissionTestTracker otherReadTracker;
		B3D_TEST_ASSERT(SubmitRecordedBufferAccess(otherReadTracker, otherBuffer, readStages, GpuAccessFlag::Read, graphics, 1).empty())

		const Vector<RecordedSubmissionTransition> graphicsWrite = fnExecuteBufferAccess(otherBuffer, GpuStageFlag::Transfer, GpuAccessFlag::Write, graphics, 1);
		B3D_TEST_ASSERT(graphicsWrite.size() == 1)
		B3D_TEST_ASSERT(graphicsWrite[0].ExecutionBarrier.SourceStages == readStages)
		CompleteTestTracker(otherReadTracker, graphics);
	}

	// Reads at rest in an earlier frame need no wait, even while in flight
	{
		SubmissionTestBuffer otherBuffer;
		SubmissionTestTracker earlierReadTracker;
		B3D_TEST_ASSERT(SubmitRecordedBufferAccess(earlierReadTracker, otherBuffer, readStages, GpuAccessFlag::Read, graphics, 1).empty())

		const Vector<RecordedSubmissionTransition> computeWrite = fnExecuteBufferAccess(otherBuffer, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, compute, 2);
		B3D_TEST_ASSERT(computeWrite.size() == 1)
		B3D_TEST_ASSERT(computeWrite[0].ParallelAccessWaitMask.IsEmpty() && computeWrite[0].ExclusiveAccessWaitMask.IsEmpty())
		CompleteTestTracker(earlierReadTracker, graphics);
	}

	// Uniform image: the read in the frame of the write takes one full-range Build, and the next frame's reads rest on every queue
	{
		RestingTestImage image(1, 3);
		fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, graphics, 0);

		const Vector<RecordedSubmissionTransition> sameFrameRead = fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 0);
		B3D_TEST_ASSERT(sameFrameRead.size() == 1)
		B3D_TEST_ASSERT(image.IsFullRange(sameFrameRead[0].ImageRange))
		B3D_TEST_ASSERT(sameFrameRead[0].InitialLayout == GpuImageLayout::ShaderReadOnly && sameFrameRead[0].FinalLayout == GpuImageLayout::ShaderReadOnly)
		B3D_TEST_ASSERT(image.HasUniformSubmissionState())

		B3D_TEST_ASSERT(fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 1).empty())
		B3D_TEST_ASSERT(fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read, compute, 1).empty())

		const GpuResourceSubmissionState& state = image.GetFullRangeSubresource()->SubmissionState;
		B3D_TEST_ASSERT(state.ReaderQueues == (GpuQueueMask(graphics) | compute))
		B3D_TEST_ASSERT(state.ReaderStages == (readStages | GpuStageFlag::ComputeShaderNonUniform))

		// Leaving the resting layout: the next resting read transitions back. Both transitions write, so reads rest again only in the next frame.
		fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::TransferSource, GpuStageFlag::Transfer, GpuAccessFlag::Read, graphics, 1);
		const Vector<RecordedSubmissionTransition> returnRead = fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 1);
		B3D_TEST_ASSERT(returnRead.size() == 1)
		B3D_TEST_ASSERT(returnRead[0].InitialLayout == GpuImageLayout::ShaderReadOnly)
		B3D_TEST_ASSERT(fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 1).size() == 1)
		B3D_TEST_ASSERT(fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 2).empty())
	}

	// Partial read, not at rest: like any partial access it splits the image, and only the surfaces read are synchronized
	{
		RestingTestImage image(1, 3);
		fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, graphics, 0);

		SubmissionTestTracker partialReadTracker;
		const Vector<RecordedSubmissionTransition> partialRead = SubmitRecordedImageAccess(partialReadTracker, image, mips1To2, GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 0);
		B3D_TEST_ASSERT(partialRead.size() == 2)
		for(const RecordedSubmissionTransition& transition : partialRead)
			B3D_TEST_ASSERT(transition.ImageRange.BaseMipLevel != 0 && transition.ImageRange.MipLevelCount == 1)

		B3D_TEST_ASSERT(!image.HasUniformSubmissionState())
		B3D_TEST_ASSERT(image.GetFullRangeSubresource()->GetBoundCount() == 0)
		CompleteTestTracker(partialReadTracker, graphics);
	}

	// Partial read, at rest: the image stays uniform, and later writes wait only on the surfaces it read
	{
		RestingTestImage image(1, 3);
		fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, graphics, 0);
		fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 0);

		SubmissionTestTracker partialReadTracker;
		B3D_TEST_ASSERT(SubmitRecordedImageAccess(partialReadTracker, image, mips1To2, GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 1).empty())
		B3D_TEST_ASSERT(image.HasUniformSubmissionState())

		const Vector<RecordedSubmissionTransition> mip0Write = fnExecuteImageAccess(image, mip0, GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, compute, 1);
		B3D_TEST_ASSERT(mip0Write.size() == 1)
		B3D_TEST_ASSERT(!mip0Write[0].ParallelAccessWaitMask.IsSet(graphics) && !mip0Write[0].ExclusiveAccessWaitMask.IsSet(graphics))

		const Vector<RecordedSubmissionTransition> mip1Write = fnExecuteImageAccess(image, mip1, GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, compute, 1);
		B3D_TEST_ASSERT(mip1Write.size() == 1)
		B3D_TEST_ASSERT(mip1Write[0].ExclusiveAccessWaitMask.IsSet(graphics))
		CompleteTestTracker(partialReadTracker, graphics);
	}

	// Split image: only the surfaces read are synchronized, then the image merges and rests in the next frame
	{
		RestingTestImage image(1, 3);
		SubmissionTestTracker computeWriteTracker;
		SubmitRecordedImageAccess(computeWriteTracker, image, mip0, GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, compute, 0);
		B3D_TEST_ASSERT(!image.HasUniformSubmissionState())

		const Vector<RecordedSubmissionTransition> partialRead = fnExecuteImageAccess(image, mips1To2, GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 0);
		B3D_TEST_ASSERT(partialRead.size() == 2)
		for(const RecordedSubmissionTransition& transition : partialRead)
		{
			B3D_TEST_ASSERT(transition.ImageRange.BaseMipLevel != 0)
			B3D_TEST_ASSERT(!transition.ParallelAccessWaitMask.IsSet(compute) && !transition.ExclusiveAccessWaitMask.IsSet(compute))
		}

		CompleteTestTracker(computeWriteTracker, compute);

		// Mip 0's writer settles away, and every surface ends in the resting layout. Mip 0's transition writes, so the image rests in the next frame.
		const Vector<RecordedSubmissionTransition> fullRead = fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 1);
		B3D_TEST_ASSERT(fullRead.size() == 3)
		B3D_TEST_ASSERT(image.HasUniformSubmissionState())
		B3D_TEST_ASSERT(fnExecuteImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 2).empty())
	}

	// Full-range in-flight readers count as readers of every surface
	{
		RestingTestImage image(1, 2);
		SubmissionTestTracker computeReadTracker;
		SubmitRecordedImageAccess(computeReadTracker, image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read, compute, 0);

		const Vector<RecordedSubmissionTransition> graphicsWrite = fnExecuteImageAccess(image, mip0, GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, graphics, 0);
		B3D_TEST_ASSERT(graphicsWrite.size() == 1)
		B3D_TEST_ASSERT(graphicsWrite[0].ExclusiveAccessWaitMask.IsSet(compute))
		CompleteTestTracker(computeReadTracker, compute);
	}
}

void GpuBackendTestSuite::TestImageAccessEpochTracking()
{
	SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
	SubmissionTestBarrierHelper barrierHelper;
	SubmissionTestTracker tracker;
	const GpuTextureSubresourceRange range(0, 1, 0, 1, GpuTextureAspectFlag::Color);
	const GpuResourceUseFlags shaderUse = GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader;

	TrackImageBinding(tracker, &image, range, GpuImageLayout::ShaderReadOnly, shaderUse, GpuAccessFlag::Read, barrierHelper);
	TrackImageBinding(tracker, &image, range, GpuImageLayout::TransferSource, shaderUse, GpuAccessFlag::Read, barrierHelper);
	B3D_TEST_ASSERT(tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Color).RequiredLayout == GpuImageLayout::General)

	tracker.CommitPendingAccesses();
	TrackImageBinding(tracker, &image, range, GpuImageLayout::ShaderReadOnly, shaderUse, GpuAccessFlag::Read, barrierHelper);
	B3D_TEST_ASSERT(tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Color).RequiredLayout == GpuImageLayout::ShaderReadOnly)

	tracker.CommitPendingAccesses();
	tracker.NotifyUnbound();
	tracker.Clear();
}

void GpuBackendTestSuite::TestLayoutTransitionWrites()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);
	const GpuStageFlags readStages = GpuStageFlag::FragmentShaderNonUniform;

	// Records a read in ShaderReadOnly followed by a copy from the image, which transitions it inside the command buffer
	auto fnTrackReadThenCopy = [readStages](SubmissionTestTracker& tracker, IGpuImageResource& image)
	{
		SubmissionTestBarrierHelper barrierHelper;
		tracker.TrackImageAccess(&image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, barrierHelper);
		tracker.CommitPendingAccesses();
		tracker.TrackImageAccess(&image, image.GetRange(), GpuImageLayout::TransferSource, GpuStageFlag::Transfer, GpuAccessFlag::Read, barrierHelper);
		tracker.CommitPendingAccesses();
	};

	// A submission transition orders later reads on other queues and on its own queue after it, and stops the image from resting
	{
		RestingTestImage image(1, 1);
		image.SetNativeLayout(GpuImageLayout::ShaderReadOnly);

		SubmissionTestTracker graphicsCopyTracker;
		SubmitRecordedImageAccess(graphicsCopyTracker, image, image.GetRange(), GpuImageLayout::TransferSource, GpuStageFlag::Transfer, GpuAccessFlag::Read, graphics, 0);

		const GpuResourceSubmissionState& state = image.GetFullRangeSubresource()->SubmissionState;
		B3D_TEST_ASSERT(state.HasWriter && state.WriterQueueId.Id == graphics.Id)

		SubmissionTestTracker computeReadTracker;
		const Vector<RecordedSubmissionTransition> computeRead = SubmitRecordedImageAccess(computeReadTracker, image, image.GetRange(), GpuImageLayout::ShaderReadOnly,
			GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read, compute, 0);
		B3D_TEST_ASSERT(computeRead.size() == 1)
		B3D_TEST_ASSERT(computeRead[0].ExclusiveAccessWaitMask.IsSet(graphics))
		B3D_TEST_ASSERT(state.HasWriter && state.WriterQueueId.Id == compute.Id)

		// The layout matches and the image could rest, but the read must still wait for compute's transition
		SubmissionTestTracker graphicsReadTracker;
		const Vector<RecordedSubmissionTransition> graphicsRead = SubmitRecordedImageAccess(graphicsReadTracker, image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages,
			GpuAccessFlag::Read, graphics, 0);
		B3D_TEST_ASSERT(graphicsRead.size() == 1)
		B3D_TEST_ASSERT(graphicsRead[0].ParallelAccessWaitMask.IsSet(compute))

		// Reads that don't transition add a reader and keep the writer
		B3D_TEST_ASSERT(state.HasWriter && state.WriterQueueId.Id == compute.Id)
		B3D_TEST_ASSERT(state.ReaderQueues.IsSet(graphics))

		CompleteTestTracker(graphicsCopyTracker, graphics);
		CompleteTestTracker(computeReadTracker, compute);
		CompleteTestTracker(graphicsReadTracker, graphics);

		// The transition writer settles away in the next frame, and reads rest again
		SubmissionTestTracker nextFrameTracker;
		B3D_TEST_ASSERT(SubmitRecordedImageAccess(nextFrameTracker, image, image.GetRange(), GpuImageLayout::ShaderReadOnly, readStages, GpuAccessFlag::Read, graphics, 1).empty())
		B3D_TEST_ASSERT(!state.HasWriter)
		CompleteTestTracker(nextFrameTracker, graphics);
	}

	// A transition inside the command buffer waits for every in-flight reader, and is published as a write
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
		image.SetNativeLayout(GpuImageLayout::ShaderReadOnly);

		SubmissionTestTracker computeReadTracker;
		SubmitRecordedImageAccess(computeReadTracker, image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read, compute, 0);

		SubmissionTestTracker copyTracker;
		fnTrackReadThenCopy(copyTracker, image);

		SubmissionRecordingVisitor visitor;
		SubmitTestTracker(copyTracker, graphics, 0, visitor);
		B3D_TEST_ASSERT(visitor.Transitions.size() == 1)
		B3D_TEST_ASSERT(visitor.Transitions[0].InitialLayout == GpuImageLayout::ShaderReadOnly)
		B3D_TEST_ASSERT(visitor.Transitions[0].ParallelAccessWaitMask.IsSet(compute))

		const GpuResourceSubmissionState& state = image.GetFullRangeSubresource()->SubmissionState;
		B3D_TEST_ASSERT(state.HasWriter && state.WriterQueueId.Id == graphics.Id)
		B3D_TEST_ASSERT(state.ReaderQueues.IsEmpty())

		// The transition's stages are not tracked, so the next access on the same queue orders after every stage
		B3D_TEST_ASSERT(state.WriterHazards.WriteStages == (GpuStageFlags(GpuStageFlag::All) & GpuBackendUtility::GetQueueStageFlags(GQT_GRAPHICS)))
		B3D_TEST_ASSERT(state.WriterHazards.VisibleStages == GpuStageFlag::None)

		CompleteTestTracker(computeReadTracker, compute);
		CompleteTestTracker(copyTracker, graphics);
	}

	// A transition inside the command buffer orders after earlier reads on its own queue
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
		image.SetNativeLayout(GpuImageLayout::ShaderReadOnly);

		SubmissionTestTracker earlierReadTracker;
		SubmitRecordedImageAccess(earlierReadTracker, image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read, graphics, 0);

		SubmissionTestTracker copyTracker;
		fnTrackReadThenCopy(copyTracker, image);

		SubmissionRecordingVisitor visitor;
		SubmitTestTracker(copyTracker, graphics, 0, visitor);
		B3D_TEST_ASSERT(visitor.Transitions.size() == 1)
		B3D_TEST_ASSERT(visitor.Transitions[0].ExecutionBarrier.SourceStages.IsSet(GpuStageFlag::ComputeShaderNonUniform))
		B3D_TEST_ASSERT(visitor.Transitions[0].ExecutionBarrier.DestinationStages.IsSet(GpuStageFlag::FragmentShaderNonUniform))

		CompleteTestTracker(earlierReadTracker, graphics);
		CompleteTestTracker(copyTracker, graphics);
	}

	// A read in the committed layout transitions nothing and leaves no writer
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
		image.SetNativeLayout(GpuImageLayout::ShaderReadOnly);

		const SubmissionImageRecordingVisitor visitor = ExecuteTestImageAccess(image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuAccessFlag::Read, graphics);
		B3D_TEST_ASSERT(visitor.Ranges.Size() == 1)

		const GpuResourceSubmissionState& state = image.GetFullRangeSubresource()->SubmissionState;
		B3D_TEST_ASSERT(!state.HasWriter)
		B3D_TEST_ASSERT(state.ReaderQueues == GpuQueueMask(graphics))
	}
}

void GpuBackendTestSuite::TestAliasAcquire()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);
	const GpuStageFlags computeStages = GpuStageFlag::ComputeShaderNonUniform;
	const GpuStageFlags attachmentStages = GpuStageFlag::ColorAttachment;
	const GpuStageFlags depthStages = GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;

	GpuAliasAcquire attachmentWriteAcquire;
	attachmentWriteAcquire.Source.Add(attachmentStages, GpuAccessFlag::Write);

	GpuAliasAcquire transferWriteAcquire;
	transferWriteAcquire.Source.Add(GpuStageFlag::Transfer, GpuAccessFlag::Write);

	// Applies the barriers queued since the last batch like a backend barrier helper would, then ends the batch. The image barriers of a batch
	// share one scope and cover the full image, so applying the last one to the full range applies them all.
	auto fnExecuteBarriers = [](SubmissionTestTracker& tracker, SubmissionTestBarrierHelper& barrierHelper, IGpuImageResource* image, IGpuBufferResource* buffer)
	{
		if(image != nullptr && barrierHelper.HasQueuedBarrier(image))
		{
			tracker.UpdateImageLayoutTrackingAfterBarrier(image, image->GetRange(), barrierHelper.LastImageBarrierOldLayout, barrierHelper.LastImageBarrierNewLayout);
			tracker.UpdateHazardStateAfterBarrier(image, image->GetRange(), barrierHelper.LastImageBarrier);
		}

		if(buffer != nullptr && barrierHelper.HasQueuedBarrier(buffer))
			tracker.UpdateHazardStateAfterBarrier(buffer, barrierHelper.LastBufferBarrier);

		tracker.CommitPendingAccesses();
		barrierHelper.Clear();
	};

	// Tracks a full-range image access, then applies its barriers
	auto fnExecuteImageAccess = [&fnExecuteBarriers](SubmissionTestTracker& tracker, SubmissionTestBarrierHelper& barrierHelper, IGpuImageResource& image, GpuImageLayout layout, GpuStageFlags stages,
		GpuAccessFlags access, GpuBarrierFlags barrierFlags = GpuBarrierFlag::None)
	{
		tracker.TrackImageAccess(&image, image.GetRange(), layout, stages, access, barrierHelper, barrierFlags);
		fnExecuteBarriers(tracker, barrierHelper, &image, nullptr);
	};

	auto fnExecuteBufferAccess = [&fnExecuteBarriers](SubmissionTestTracker& tracker, SubmissionTestBarrierHelper& barrierHelper, IGpuBufferResource& buffer, GpuStageFlags stages, GpuAccessFlags access)
	{
		tracker.TrackBufferAccess(&buffer, stages, access, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);
	};

	// Issues the barrier of an alias acquire of the whole image, then applies it
	auto fnAcquireImage = [&fnExecuteBarriers](SubmissionTestTracker& tracker, SubmissionTestBarrierHelper& barrierHelper, IGpuImageResource& image, const GpuAliasAcquire& acquire,
		GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access)
	{
		tracker.TrackImageBarrier(&image, image.GetRange(), stages, access, layout, barrierHelper, &acquire);
		fnExecuteBarriers(tracker, barrierHelper, &image, nullptr);
	};

	auto fnAcquireBuffer = [&fnExecuteBarriers](SubmissionTestTracker& tracker, SubmissionTestBarrierHelper& barrierHelper, IGpuBufferResource& buffer, const GpuAliasAcquire& acquire,
		GpuStageFlags stages, GpuAccessFlags access)
	{
		tracker.TrackBufferBarrier(&buffer, stages, access, barrierHelper, &acquire);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);
	};

	// The acquire records one barrier from the source. It applies to the memory, and transitions from Undefined. The first access in its
	// destination needs no barrier of its own.
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		fnAcquireImage(tracker, barrierHelper, image, attachmentWriteAcquire, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 1)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.SourceStages == attachmentStages)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.SourceAccess == GpuAccessFlag::Write)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.DestinationStages == computeStages)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.DestinationAccess == GpuAccessFlag::Write)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire))
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierFlags.IsSet(GpuBarrierFlag::DiscardContents))
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierOldLayout == GpuImageLayout::Undefined)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierNewLayout == GpuImageLayout::General)

		fnExecuteImageAccess(tracker, barrierHelper, image, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 1)

		// Neither the seeded source nor the acquire's transition is an access of the command buffer
		const GpuImageSubresourceTrackingState& trackingState = tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Color);
		B3D_TEST_ASSERT(trackingState.HazardState->AllAccessScope.WriteStages == computeStages)
		B3D_TEST_ASSERT(trackingState.HazardState->AllAccessScope.ReadStages == GpuStageFlag::None)
		B3D_TEST_ASSERT(!trackingState.HazardState->AccessScopeBeforeFirstBarrier.IsValid())
		B3D_TEST_ASSERT(trackingState.InitialLayout == GpuImageLayout::Undefined)
		B3D_TEST_ASSERT(!trackingState.TransitionsLayout)

		// Later accesses are tracked normally
		fnExecuteImageAccess(tracker, barrierHelper, image, GpuImageLayout::General, computeStages, GpuAccessFlag::Read);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 2)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.SourceStages == computeStages)
		B3D_TEST_ASSERT(!barrierHelper.LastImageBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire))

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// A first access outside the destination gets a barrier from the acquire's destination, which chains it after the acquire's barrier
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		fnAcquireImage(tracker, barrierHelper, image, transferWriteAcquire, GpuImageLayout::ColorAttachment, attachmentStages, GpuAccessFlag::Write);
		fnExecuteImageAccess(tracker, barrierHelper, image, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);

		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 2)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.SourceStages == attachmentStages)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.SourceAccess == GpuAccessFlag::Write)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.DestinationStages == computeStages)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierFlags == GpuBarrierFlag::None)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierOldLayout == GpuImageLayout::ColorAttachment)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierNewLayout == GpuImageLayout::General)

		// The first access writes, so submission synchronizes the command buffer as a writer already
		B3D_TEST_ASSERT(!tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Color).TransitionsLayout)

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// Every aspect of the image is acquired
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		fnAcquireImage(tracker, barrierHelper, image, attachmentWriteAcquire, GpuImageLayout::DepthStencilAttachment, depthStages, GpuAccessFlag::Read | GpuAccessFlag::Write);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 2)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire))

		tracker.TrackImageAccess(&image, image.GetRange(), GpuImageLayout::DepthStencilAttachment, depthStages, GpuAccessFlag::Read | GpuAccessFlag::Write, barrierHelper, GpuBarrierFlag::DiscardContents);
		fnExecuteBarriers(tracker, barrierHelper, &image, nullptr);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 2)

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// An empty source only transitions an image's layout, and gives a buffer no barrier
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
		SubmissionTestBuffer buffer;
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		fnAcquireImage(tracker, barrierHelper, image, GpuAliasAcquire(), GpuImageLayout::General, computeStages, GpuAccessFlag::Write);
		fnAcquireBuffer(tracker, barrierHelper, buffer, GpuAliasAcquire(), computeStages, GpuAccessFlag::Write);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 1)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.SourceStages == GpuStageFlag::None)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.DestinationStages == computeStages)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire))
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierOldLayout == GpuImageLayout::Undefined)
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 0)

		fnExecuteImageAccess(tracker, barrierHelper, image, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);
		fnExecuteBufferAccess(tracker, barrierHelper, buffer, computeStages, GpuAccessFlag::Write);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 1)
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 0)

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// A buffer's acquire records the barrier from the source, flagged as an alias acquire
	{
		GpuAliasAcquire readAcquire;
		readAcquire.Source.Add(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);

		SubmissionTestBuffer buffer;
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		fnAcquireBuffer(tracker, barrierHelper, buffer, readAcquire, computeStages, GpuAccessFlag::Write);
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 1)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrierFlags == GpuBarrierFlag::AliasAcquire)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.SourceStages == GpuStageFlag::FragmentShaderNonUniform)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.SourceAccess == GpuAccessFlag::Read)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.DestinationStages == computeStages)

		fnExecuteBufferAccess(tracker, barrierHelper, buffer, computeStages, GpuAccessFlag::Write);
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 1)

		const GpuBufferTrackingState* const trackingState = tracker.FindBufferTrackingState(&buffer);
		B3D_TEST_ASSERT(trackingState->HazardState->AllAccessScope.ReadStages == GpuStageFlag::None)
		B3D_TEST_ASSERT(trackingState->HazardState->AllAccessScope.WriteStages == computeStages)

		// Later accesses are tracked normally
		fnExecuteBufferAccess(tracker, barrierHelper, buffer, computeStages, GpuAccessFlag::Write);
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 2)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrierFlags == GpuBarrierFlag::None)

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// A predecessor accessed earlier in the same command buffer is ordered by the successor's acquire
	{
		SubmissionTestImage predecessor(1, 1, GpuTextureAspectFlag::Color);
		SubmissionTestImage successor(1, 1, GpuTextureAspectFlag::Color);
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		fnExecuteImageAccess(tracker, barrierHelper, predecessor, GpuImageLayout::ColorAttachment, attachmentStages, GpuAccessFlag::Write);
		const u32 barrierCountBeforeAcquire = barrierHelper.ImageBarrierCount;

		fnAcquireImage(tracker, barrierHelper, successor, attachmentWriteAcquire, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == barrierCountBeforeAcquire + 1)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.SourceStages == attachmentStages)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire))

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// An explicit barrier between the acquire and the first access is recorded inline, chained after the acquire's barrier
	{
		SubmissionTestBuffer buffer;
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		fnAcquireBuffer(tracker, barrierHelper, buffer, attachmentWriteAcquire, computeStages, GpuAccessFlag::Write);
		tracker.TrackBufferBarrier(&buffer, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);

		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 2)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.SourceStages == computeStages)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrierFlags == GpuBarrierFlag::None)
		B3D_TEST_ASSERT(!tracker.FindBufferTrackingState(&buffer)->HazardState->HasLeadingBarrier)

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// The first submission ignores the previous lifetime's submission state, and publishes only the new lifetime
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
		image.SetNativeLayout(GpuImageLayout::ShaderReadOnly);

		// Previous lifetime: written on graphics, then read on compute, still in flight. A graphics write would normally need a barrier and
		// a wait on compute.
		SubmissionTestTracker previousWriteTracker;
		SubmitRecordedImageAccess(previousWriteTracker, image, image.GetRange(), GpuImageLayout::ColorAttachment, attachmentStages, GpuAccessFlag::Write, graphics, 0);
		SubmissionTestTracker previousReadTracker;
		SubmitRecordedImageAccess(previousReadTracker, image, image.GetRange(), GpuImageLayout::ShaderReadOnly, computeStages, GpuAccessFlag::Read, compute, 0);

		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;
		fnAcquireImage(tracker, barrierHelper, image, attachmentWriteAcquire, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);
		fnExecuteImageAccess(tracker, barrierHelper, image, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);

		SubmissionRecordingVisitor visitor;
		SubmitTestTracker(tracker, graphics, 0, visitor);
		B3D_TEST_ASSERT(visitor.Transitions.size() == 1)

		const RecordedSubmissionTransition& transition = visitor.Transitions[0];
		B3D_TEST_ASSERT(transition.SubmissionBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire))
		B3D_TEST_ASSERT(transition.InitialLayout == GpuImageLayout::Undefined)
		B3D_TEST_ASSERT(transition.FinalLayout == GpuImageLayout::General)
		B3D_TEST_ASSERT(transition.ParallelAccessWaitMask.IsEmpty())
		B3D_TEST_ASSERT(transition.ExclusiveAccessWaitMask.IsEmpty())
		B3D_TEST_ASSERT(!transition.MemoryBarrier.IsValid())
		B3D_TEST_ASSERT(!transition.ExecutionBarrier.IsValid())

		const GpuResourceSubmissionState& state = image.GetFullRangeSubresource()->SubmissionState;
		B3D_TEST_ASSERT(state.HasWriter && state.WriterQueueId.Id == graphics.Id)
		B3D_TEST_ASSERT(state.WriterHazards.WriteStages == computeStages)
		B3D_TEST_ASSERT(state.WriterHazards.ReaderStages == GpuStageFlag::None)
		B3D_TEST_ASSERT(state.ReaderQueues.IsEmpty())

		CompleteTestTracker(previousWriteTracker, graphics);
		CompleteTestTracker(previousReadTracker, compute);
		CompleteTestTracker(tracker, graphics);

		// The next command buffer resolves against the new lifetime
		SubmissionTestTracker nextTracker;
		const Vector<RecordedSubmissionTransition> nextRead = SubmitRecordedImageAccess(nextTracker, image, image.GetRange(), GpuImageLayout::General, computeStages, GpuAccessFlag::Read, compute, 0);
		B3D_TEST_ASSERT(nextRead.size() == 1)
		B3D_TEST_ASSERT(!nextRead[0].SubmissionBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire))
		B3D_TEST_ASSERT(nextRead[0].ParallelAccessWaitMask.IsSet(graphics))
		CompleteTestTracker(nextTracker, compute);
	}

	// The same holds for buffers
	{
		SubmissionTestBuffer buffer;

		SubmissionTestTracker previousWriteTracker;
		SubmitRecordedBufferAccess(previousWriteTracker, buffer, computeStages, GpuAccessFlag::Write, compute, 0);

		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;
		fnAcquireBuffer(tracker, barrierHelper, buffer, GpuAliasAcquire(), GpuStageFlag::Transfer, GpuAccessFlag::Write);
		fnExecuteBufferAccess(tracker, barrierHelper, buffer, GpuStageFlag::Transfer, GpuAccessFlag::Write);

		SubmissionRecordingVisitor visitor;
		SubmitTestTracker(tracker, graphics, 0, visitor);
		B3D_TEST_ASSERT(visitor.Transitions.size() == 1)
		B3D_TEST_ASSERT(visitor.Transitions[0].SubmissionBarrierFlags == GpuBarrierFlag::AliasAcquire)
		B3D_TEST_ASSERT(visitor.Transitions[0].ParallelAccessWaitMask.IsEmpty())
		B3D_TEST_ASSERT(visitor.Transitions[0].ExclusiveAccessWaitMask.IsEmpty())
		B3D_TEST_ASSERT(!visitor.Transitions[0].MemoryBarrier.IsValid())

		const GpuResourceSubmissionState& state = buffer.GetSubmissionState();
		B3D_TEST_ASSERT(state.HasWriter && state.WriterQueueId.Id == graphics.Id)
		B3D_TEST_ASSERT(state.WriterHazards.WriteStages == GpuStageFlag::Transfer)

		CompleteTestTracker(previousWriteTracker, compute);
		CompleteTestTracker(tracker, graphics);
	}

	// The acquire starts the new lifetime also where the command buffer doesn't access the resource. It publishes the acquire's transition as
	// a write, so accesses on other queues wait for it.
	{
		SubmissionTestBuffer buffer;
		SubmissionTestImage image(1, 2, GpuTextureAspectFlag::Color);
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;
		fnAcquireBuffer(tracker, barrierHelper, buffer, attachmentWriteAcquire, computeStages, GpuAccessFlag::Write);
		fnAcquireImage(tracker, barrierHelper, image, attachmentWriteAcquire, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);

		// The buffer is not accessed, and the image only on its first mip level
		tracker.TrackImageAccess(&image, GpuTextureSubresourceRange(0, 1, 0, 1, GpuTextureAspectFlag::Color), GpuImageLayout::General, computeStages, GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, &image, nullptr);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 1)

		SubmissionRecordingVisitor visitor;
		SubmitTestTracker(tracker, graphics, 0, visitor);

		// The buffer, then each mip level of the image
		B3D_TEST_ASSERT(visitor.Transitions.size() == 3)
		for(const RecordedSubmissionTransition& transition : visitor.Transitions)
		{
			B3D_TEST_ASSERT(transition.SubmissionBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire))
			B3D_TEST_ASSERT(transition.ParallelAccessWaitMask.IsEmpty())
			B3D_TEST_ASSERT(!transition.MemoryBarrier.IsValid())
			B3D_TEST_ASSERT(transition.PostTransitionSubmissionState.HasWriter && transition.PostTransitionSubmissionState.WriterQueueId.Id == graphics.Id)
		}

		const RecordedSubmissionTransition& unaccessedMip = visitor.Transitions[1].ImageRange.BaseMipLevel == 1 ? visitor.Transitions[1] : visitor.Transitions[2];
		B3D_TEST_ASSERT(unaccessedMip.ImageRange.BaseMipLevel == 1)
		B3D_TEST_ASSERT(unaccessedMip.InitialLayout == GpuImageLayout::Undefined)
		B3D_TEST_ASSERT(unaccessedMip.FinalLayout == GpuImageLayout::General)
		CompleteTestTracker(tracker, graphics);

		SubmissionTestTracker nextImageTracker;
		const Vector<RecordedSubmissionTransition> nextImageRead = SubmitRecordedImageAccess(nextImageTracker, image, GpuTextureSubresourceRange(1, 1, 0, 1, GpuTextureAspectFlag::Color),
			GpuImageLayout::General, computeStages, GpuAccessFlag::Read, compute, 0);
		B3D_TEST_ASSERT(nextImageRead.size() == 1)
		B3D_TEST_ASSERT(nextImageRead[0].ParallelAccessWaitMask.IsSet(graphics))
		CompleteTestTracker(nextImageTracker, compute);

		SubmissionTestTracker nextBufferTracker;
		const Vector<RecordedSubmissionTransition> nextBufferRead = SubmitRecordedBufferAccess(nextBufferTracker, buffer, computeStages, GpuAccessFlag::Read, compute, 0);
		B3D_TEST_ASSERT(nextBufferRead.size() == 1)
		B3D_TEST_ASSERT(nextBufferRead[0].ParallelAccessWaitMask.IsSet(graphics))
		CompleteTestTracker(nextBufferTracker, compute);
	}

	// Validation: the acquire must precede every other use on the command buffer
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;
		fnExecuteImageAccess(tracker, barrierHelper, image, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);

		{
			LoggingScope logs(*this);
			logs.ExpectError("An alias acquire must precede every other use of the GPU image on the command buffer.");
			tracker.TrackImageBarrier(&image, image.GetRange(), computeStages, GpuAccessFlag::Write, GpuImageLayout::General, barrierHelper, &attachmentWriteAcquire);
		}

		B3D_TEST_ASSERT(!tracker.GetSubresourceTrackingState(&image, 0, 0, GpuTextureAspectFlag::Color).IsAliasAcquired())

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// Validation: the acquire covers the whole image, and provides the layout to transition it to
	{
		SubmissionTestImage image(1, 2, GpuTextureAspectFlag::Color);
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		{
			LoggingScope logs(*this);
			logs.ExpectError("An alias acquire must cover the whole GPU image.");
			logs.ExpectError("An alias acquire of a GPU image must provide a destination layout.");
			tracker.TrackImageBarrier(&image, GpuTextureSubresourceRange(0, 1, 0, 1, GpuTextureAspectFlag::Color), computeStages, GpuAccessFlag::Write, GpuImageLayout::General, barrierHelper,
				&attachmentWriteAcquire);
			tracker.TrackImageBarrier(&image, image.GetRange(), computeStages, GpuAccessFlag::Write, GpuImageLayout::Undefined, barrierHelper, &attachmentWriteAcquire);
		}

		B3D_TEST_ASSERT(tracker.FindImageTrackingState(&image) == nullptr)
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 0)
	}

	// Validation: the first access must write without reading the previous contents
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		auto fnExpectInvalidFirstAccess = [this, &image, &tracker, &barrierHelper, &attachmentWriteAcquire, &fnAcquireImage, &attachmentStages](GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access)
		{
			fnAcquireImage(tracker, barrierHelper, image, attachmentWriteAcquire, GpuImageLayout::ColorAttachment, attachmentStages, GpuAccessFlag::Write);
			{
				LoggingScope logs(*this);
				logs.ExpectError("The first access of an alias acquired image must write without reading the previous contents.");
				tracker.TrackImageAccess(&image, image.GetRange(), layout, stages, access, barrierHelper);
			}

			tracker.CommitPendingAccesses();
			barrierHelper.Clear();
			tracker.NotifyUnbound();
			tracker.Clear();
		};

		fnExpectInvalidFirstAccess(GpuImageLayout::ShaderReadOnly, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);

		// A writable attachment that is loaded reads the previous contents
		fnExpectInvalidFirstAccess(GpuImageLayout::ColorAttachment, attachmentStages, GpuAccessFlag::Read | GpuAccessFlag::Write);

		// One that discards them does not. Its read needs the acquire made visible, and the barrier does not discard again, as the acquire's
		// barrier already discarded the contents.
		fnAcquireImage(tracker, barrierHelper, image, attachmentWriteAcquire, GpuImageLayout::ColorAttachment, attachmentStages, GpuAccessFlag::Write);
		const u32 barrierCountBeforeAccess = barrierHelper.ImageBarrierCount;
		tracker.TrackImageAccess(&image, image.GetRange(), GpuImageLayout::ColorAttachment, attachmentStages, GpuAccessFlag::Read | GpuAccessFlag::Write, barrierHelper, GpuBarrierFlag::DiscardContents);
		fnExecuteBarriers(tracker, barrierHelper, &image, nullptr);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == barrierCountBeforeAccess + 1)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.DestinationAccess == GpuAccessFlag::Read)
		B3D_TEST_ASSERT(!barrierHelper.LastImageBarrierFlags.IsSet(GpuBarrierFlag::DiscardContents))
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierOldLayout == GpuImageLayout::ColorAttachment)
		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// Validation: a buffer's first access must write. A resting read must not hide the acquire.
	{
		SubmissionTestBuffer buffer;
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;
		fnAcquireBuffer(tracker, barrierHelper, buffer, attachmentWriteAcquire, computeStages, GpuAccessFlag::Write);

		{
			LoggingScope logs(*this);
			logs.ExpectError("The first access of an alias acquired buffer must write.");
			tracker.TrackBufferAccess(&buffer, computeStages, GpuAccessFlag::Read, barrierHelper);
		}

		B3D_TEST_ASSERT(tracker.FindBufferTrackingState(&buffer)->HazardState != nullptr)

		tracker.CommitPendingAccesses();
		tracker.NotifyUnbound();
		tracker.Clear();
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Validation: earlier resources on the memory are invalid after the acquire, until they are acquired again
	{
		SubmissionTestImage predecessorImage(1, 1, GpuTextureAspectFlag::Color);
		SubmissionTestBuffer predecessorBuffer;
		SubmissionTestImage successor(1, 1, GpuTextureAspectFlag::Color);
		IGpuResource* const predecessors[] = { &predecessorImage, &predecessorBuffer };

		GpuAliasAcquire acquire = attachmentWriteAcquire;
		acquire.Predecessors = TArrayView<IGpuResource* const>(predecessors, 2);

		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;
		fnAcquireImage(tracker, barrierHelper, successor, acquire, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);
		B3D_TEST_ASSERT(predecessorImage.IsSupersededByAlias())
		B3D_TEST_ASSERT(predecessorBuffer.IsSupersededByAlias())
		B3D_TEST_ASSERT(!successor.IsSupersededByAlias())

		{
			LoggingScope logs(*this);
			logs.ExpectError("A GPU image is used after another resource took over its memory with an alias acquire.");
			logs.ExpectError("A GPU buffer is used after another resource took over its memory with an alias acquire.");
			tracker.TrackImageAccess(&predecessorImage, predecessorImage.GetRange(), GpuImageLayout::General, computeStages, GpuAccessFlag::Write, barrierHelper);
			tracker.TrackBufferAccess(&predecessorBuffer, computeStages, GpuAccessFlag::Write, barrierHelper);
		}

		tracker.CommitPendingAccesses();
		barrierHelper.Clear();
		tracker.NotifyUnbound();
		tracker.Clear();

		// Acquiring the predecessor again starts its own new lifetime, which supersedes the successor
		GpuAliasAcquire predecessorAcquire;
		IGpuResource* const successors[] = { &successor };
		predecessorAcquire.Predecessors = TArrayView<IGpuResource* const>(successors, 1);
		fnAcquireImage(tracker, barrierHelper, predecessorImage, predecessorAcquire, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);
		B3D_TEST_ASSERT(!predecessorImage.IsSupersededByAlias())
		B3D_TEST_ASSERT(successor.IsSupersededByAlias())

		fnExecuteImageAccess(tracker, barrierHelper, predecessorImage, GpuImageLayout::General, computeStages, GpuAccessFlag::Write);
		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// Validation: a barrier of an earlier resource is not issued together with the acquire, as barriers of one batch are not ordered
	{
		SubmissionTestImage predecessor(1, 1, GpuTextureAspectFlag::Color);
		SubmissionTestImage successor(1, 1, GpuTextureAspectFlag::Color);
		IGpuResource* const predecessors[] = { &predecessor };

		GpuAliasAcquire acquire = attachmentWriteAcquire;
		acquire.Predecessors = TArrayView<IGpuResource* const>(predecessors, 1);

		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;
		fnExecuteImageAccess(tracker, barrierHelper, predecessor, GpuImageLayout::ColorAttachment, attachmentStages, GpuAccessFlag::Write);

		tracker.TrackImageBarrier(&predecessor, predecessor.GetRange(), computeStages, GpuAccessFlag::Read, GpuImageLayout::General, barrierHelper);
		{
			LoggingScope logs(*this);
			logs.ExpectError("A barrier of an earlier resource on the memory is issued together with the alias acquire that supersedes it.");
			tracker.TrackImageBarrier(&successor, successor.GetRange(), computeStages, GpuAccessFlag::Write, GpuImageLayout::General, barrierHelper, &acquire);
		}

		fnExecuteBarriers(tracker, barrierHelper, &successor, nullptr);
		tracker.NotifyUnbound();
		tracker.Clear();
	}
#endif
}

void GpuBackendTestSuite::TestAliasAcquireExecution()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	GetRenderThread().PostCommand([this, &device]()
	{
		static constexpr u32 kSize = 64;
		static constexpr u32 kClearValue = 0xFF00FF00; // Color(0, 1, 0, 1) in PF_RGBA8
		const GpuQueueId compute(GQT_COMPUTE, 0);

		// Transfers on the same queue, recorded before the acquiring command buffer
		GpuAliasAcquire transferAcquire;
		transferAcquire.Source.Add(GpuStageFlag::Transfer, GpuAccessFlag::Read | GpuAccessFlag::Write);

		TextureCreateInformation targetInformation;
		targetInformation.Name = "Alias acquire target";
		targetInformation.Width = kSize;
		targetInformation.Height = kSize;
		targetInformation.Format = PF_RGBA8;
		targetInformation.Usage = TextureUsageFlag::RenderTarget;
		targetInformation.ClearColor = Color(0, 1, 0, 1);

		TextureCreateInformation storageInformation = targetInformation;
		storageInformation.Name = "Alias acquire storage";
		storageInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

		const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize);

		// Staging buffers hold one pattern per resource. Their size covers a texture's staging pitch.
		const GpuMemoryRequirements targetRequirements = device->GetMemoryRequirements(targetInformation);
		const GpuMemoryRequirements storageRequirements = device->GetMemoryRequirements(storageInformation);
		const GpuMemoryRequirements bufferRequirements = device->GetMemoryRequirements(bufferInformation);
		B3D_TEST_ASSERT(targetRequirements.MemoryType != GpuMemoryRequirements::kUnsupportedMemoryType)
		B3D_TEST_ASSERT(storageRequirements.MemoryType != GpuMemoryRequirements::kUnsupportedMemoryType)
		B3D_TEST_ASSERT(bufferRequirements.MemoryType != GpuMemoryRequirements::kUnsupportedMemoryType)
		if(targetRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType || storageRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType ||
			bufferRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
			return;

		const TShared<render::Texture> pitchTexture = device->CreateTexture(storageInformation);
		const ImageSubresourcePitch pitch = pitchTexture->GetStagingBufferPitchForSubresource(0, 0);
		const u32 stagingElementCount = pitch.RowPitch * pitch.SliceHeight;

		// Each pattern stores the pattern index in the high bits, and the element index in the low bits
		auto fnPatternValue = [](u32 pattern, u32 elementIndex) { return (pattern << 24) | elementIndex; };

		constexpr u32 kPatternCount = 3;
		TShared<render::GpuBuffer> uploads[kPatternCount];
		for(u32 pattern = 0; pattern < kPatternCount; pattern++)
		{
			uploads[pattern] = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(stagingElementCount * sizeof(u32)));
			const render::GpuBufferMappedScope mapping = uploads[pattern]->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			u32* elements = static_cast<u32*>(mapping.GetMappedMemory());
			for(u32 elementIndex = 0; elementIndex < stagingElementCount; elementIndex++)
				elements[elementIndex] = fnPatternValue(pattern + 1, elementIndex);
		}

		constexpr u32 kReadbackCount = 3;
		TShared<render::GpuBuffer> readbacks[kReadbackCount];
		for(TShared<render::GpuBuffer>& readback : readbacks)
			readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(stagingElementCount * sizeof(u32)));

		// Checks that a readback holds a buffer pattern, a texture pattern, or the clear value in every texel
		enum class ReadbackLayout { Buffer, Texture };
		auto fnCheckReadback = [this, &readbacks, &pitch, &fnPatternValue](u32 readbackIndex, ReadbackLayout layout, u32 pattern)
		{
			const render::GpuBufferMappedScope mapping = readbacks[readbackIndex]->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			const u32* elements = static_cast<const u32*>(mapping.GetMappedMemory());
			bool matches = true;
			for(u32 row = 0; row < kSize; row++)
			{
				for(u32 column = 0; column < kSize; column++)
				{
					const u32 elementIndex = layout == ReadbackLayout::Buffer ? row * kSize + column : row * pitch.RowPitch + column;
					const u32 expected = pattern == 0 ? kClearValue : fnPatternValue(pattern, elementIndex);
					matches &= elements[elementIndex] == expected;
				}
			}

			B3D_TEST_ASSERT(matches)
		};

		// Allocates memory that fits every resource in the list, and checks that they can share it
		// Memory behind the non-owning allocations is released once the resources placed on it are destroyed
		struct SharedMemory
		{
			IGpuAllocator* Allocator = nullptr;
			GpuAllocation Allocation;
		};

		TInlineArray<SharedMemory, 4> sharedMemories;
		auto fnAllocateShared = [this, &device, &sharedMemories](std::initializer_list<const GpuMemoryRequirements*> requirements, SharedMemory& output)
		{
			u64 size = 0;
			u64 alignment = 1;
			for(const GpuMemoryRequirements* entry : requirements)
			{
				if(entry->MemoryType != (*requirements.begin())->MemoryType)
					return false;

				size = std::max(size, entry->Size);
				alignment = std::max(alignment, entry->Alignment);
			}

			const GpuMemoryRequirements& first = **requirements.begin();
			output.Allocator = &device->GetPersistentAllocator(first.MemoryType);
			const bool allocated = output.Allocator->TryAllocate(size, (u32)alignment, first.Kind, nullptr, output.Allocation);
			B3D_TEST_ASSERT(allocated)
			if(allocated)
				sharedMemories.Add(output);

			return allocated;
		};

		auto fnCreateTexture = [&device](const TextureCreateInformation& information, const SharedMemory& memory)
		{
			GpuAllocation allocation = memory.Allocation;
			allocation.Allocator = nullptr;
			return device->CreateTexture(information, allocation, GpuObjectCreateFlag::Aliased);
		};

		auto fnCreateBuffer = [&device, &bufferInformation](const SharedMemory& memory)
		{
			GpuAllocation allocation = memory.Allocation;
			allocation.Allocator = nullptr;
			return device->CreateGpuBuffer(bufferInformation, allocation, GpuObjectCreateFlag::Aliased);
		};

		auto fnClearTarget = [](render::GpuCommandBuffer& commands, const TShared<render::Texture>& texture)
		{
			render::RenderTextureCreateInformation renderTextureInformation;
			renderTextureInformation.ColorSurfaces[0].Texture = texture;

			RenderPassCreateInformation pass(render::RenderTexture::Create(renderTextureInformation));
			pass.ClearMask = RT_COLOR0;
			commands.BeginRenderPass(pass);
			commands.EndRenderPass();
		};

		// Issue the barriers that start a new lifetime of a resource, with the resource's first access as the destination
		auto fnAcquireBuffer = [](render::GpuCommandBuffer& commands, const TShared<render::GpuBuffer>& buffer, const GpuAliasAcquire& acquire)
		{
			GpuBufferBarrier barrier(buffer, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write);
			barrier.AliasAcquire = &acquire;
			commands.IssueBarriers(barrier);
		};

		auto fnAcquireTexture = [](render::GpuCommandBuffer& commands, const TShared<render::Texture>& texture, const GpuAliasAcquire& acquire, GpuResourceUseFlags usage, GpuImageLayout layout)
		{
			GpuTextureBarrier barrier(texture, usage, GpuAccessFlag::Write, layout);
			barrier.AliasAcquire = &acquire;
			commands.IssueBarriers(barrier);
		};

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuCommandBufferPool> graphicsPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
		const u32 bufferBytes = kSize * kSize * sizeof(u32);

		// Two buffers hand the memory over within one command buffer
		{
			SharedMemory memory;
			if(!fnAllocateShared({ &bufferRequirements }, memory))
				return;

			const TShared<render::GpuBuffer> first = fnCreateBuffer(memory);
			const TShared<render::GpuBuffer> second = fnCreateBuffer(memory);

			const TShared<render::GpuCommandBuffer> commands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Alias acquire within a command buffer"));
			fnAcquireBuffer(*commands, first, GpuAliasAcquire());
			commands->CopyBufferToBuffer(uploads[0], first, 0, 0, bufferBytes);
			commands->CopyBufferToBuffer(first, readbacks[0], 0, 0, bufferBytes);
			fnAcquireBuffer(*commands, second, transferAcquire);
			commands->CopyBufferToBuffer(uploads[1], second, 0, 0, bufferBytes);
			commands->CopyBufferToBuffer(second, readbacks[1], 0, 0, bufferBytes);
			context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
			device->WaitUntilIdle();

			fnCheckReadback(0, ReadbackLayout::Buffer, 1);
			fnCheckReadback(1, ReadbackLayout::Buffer, 2);
		}

		// Two render targets hand the memory over between command buffers on one queue. The second one is cleared as its first access.
		{
			SharedMemory memory;
			if(!fnAllocateShared({ &targetRequirements }, memory))
				return;

			const TShared<render::Texture> first = fnCreateTexture(targetInformation, memory);
			const TShared<render::Texture> second = fnCreateTexture(targetInformation, memory);

			const TShared<render::GpuCommandBuffer> firstCommands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Alias acquire first target"));
			fnAcquireTexture(*firstCommands, first, GpuAliasAcquire(), GpuResourceUseFlag::Transfer, GpuImageLayout::TransferDestination);
			firstCommands->CopyBufferToTexture(uploads[0], first, 0, 0, 0);
			firstCommands->CopyTextureToBuffer(first, readbacks[0], 0, 0);
			context->SubmitCommandBuffer(firstCommands, GpuQueueMask::kNone);

			const TShared<render::GpuCommandBuffer> secondCommands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Alias acquire second target"));
			fnAcquireTexture(*secondCommands, second, transferAcquire, GpuResourceUseFlag::ColorAttachment, GpuImageLayout::ColorAttachment);
			fnClearTarget(*secondCommands, second);
			secondCommands->CopyTextureToBuffer(second, readbacks[1], 0, 0);
			context->SubmitCommandBuffer(secondCommands, GpuQueueMask::kNone);
			device->WaitUntilIdle();

			fnCheckReadback(0, ReadbackLayout::Texture, 1);
			fnCheckReadback(1, ReadbackLayout::Texture, 0);
		}

		// Storage textures hand the memory over from the compute queue to the graphics queue, behind a queue wait. The first texture then
		// acquires the memory again on the graphics queue, while its last use was on the compute queue.
		if(device->GetQueueCount(GQT_COMPUTE) > 0)
		{
			SharedMemory memory;
			if(!fnAllocateShared({ &storageRequirements }, memory))
				return;

			const TShared<render::Texture> first = fnCreateTexture(storageInformation, memory);
			const TShared<render::Texture> second = fnCreateTexture(storageInformation, memory);
			const TShared<render::GpuCommandBufferPool> computePool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_COMPUTE));

			const TShared<render::GpuCommandBuffer> computeCommands = computePool->Create(GpuCommandBufferCreateInformation::Create("Alias acquire on compute"));
			fnAcquireTexture(*computeCommands, first, GpuAliasAcquire(), GpuResourceUseFlag::Transfer, GpuImageLayout::TransferDestination);
			computeCommands->CopyBufferToTexture(uploads[0], first, 0, 0, 0);
			computeCommands->CopyTextureToBuffer(first, readbacks[0], 0, 0);
			context->SubmitCommandBuffer(computeCommands, GpuQueueMask::kNone);

			// Accesses on the compute queue are ordered by the queue wait, so they are not part of the source
			const TShared<render::GpuCommandBuffer> graphicsCommands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Alias acquire after compute"));
			fnAcquireTexture(*graphicsCommands, second, GpuAliasAcquire(), GpuResourceUseFlag::Transfer, GpuImageLayout::TransferDestination);
			graphicsCommands->CopyBufferToTexture(uploads[1], second, 0, 0, 0);
			graphicsCommands->CopyTextureToBuffer(second, readbacks[1], 0, 0);
			context->SubmitCommandBuffer(graphicsCommands, GpuQueueMask(compute));

			const TShared<render::GpuCommandBuffer> reacquireCommands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Alias acquire again"));
			fnAcquireTexture(*reacquireCommands, first, transferAcquire, GpuResourceUseFlag::Transfer, GpuImageLayout::TransferDestination);
			reacquireCommands->CopyBufferToTexture(uploads[2], first, 0, 0, 0);
			reacquireCommands->CopyTextureToBuffer(first, readbacks[2], 0, 0);
			context->SubmitCommandBuffer(reacquireCommands, GpuQueueMask::kNone);
			device->WaitUntilIdle();

			fnCheckReadback(0, ReadbackLayout::Texture, 1);
			fnCheckReadback(1, ReadbackLayout::Texture, 2);
			fnCheckReadback(2, ReadbackLayout::Texture, 3);
		}

		// Resources on separate memory start their lifetimes with one barrier batch
		{
			SharedMemory bufferMemory;
			SharedMemory textureMemory;
			if(!fnAllocateShared({ &bufferRequirements }, bufferMemory) || !fnAllocateShared({ &storageRequirements }, textureMemory))
				return;

			const TShared<render::GpuBuffer> buffer = fnCreateBuffer(bufferMemory);
			const TShared<render::Texture> texture = fnCreateTexture(storageInformation, textureMemory);

			GpuBufferBarrier bufferBarrier(buffer, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write);
			bufferBarrier.AliasAcquire = &transferAcquire;
			GpuTextureBarrier textureBarrier(texture, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
			textureBarrier.AliasAcquire = &transferAcquire;

			// A transfer earlier in the command buffer gives the acquires' source an access to order after
			const TShared<render::GpuCommandBuffer> commands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Alias acquire in one batch"));
			commands->CopyBufferToBuffer(uploads[2], readbacks[2], 0, 0, bufferBytes);
			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&bufferBarrier, 1), TArrayView<const GpuTextureBarrier>(&textureBarrier, 1)));
			commands->CopyBufferToBuffer(uploads[0], buffer, 0, 0, bufferBytes);
			commands->CopyBufferToBuffer(buffer, readbacks[0], 0, 0, bufferBytes);
			commands->CopyBufferToTexture(uploads[1], texture, 0, 0, 0);
			commands->CopyTextureToBuffer(texture, readbacks[1], 0, 0);
			context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
			device->WaitUntilIdle();

			fnCheckReadback(0, ReadbackLayout::Buffer, 1);
			fnCheckReadback(1, ReadbackLayout::Texture, 2);
		}

		// A buffer hands the memory over to a render target, on devices that place both in the same memory type
		SharedMemory mixedMemory;
		if(fnAllocateShared({ &targetRequirements, &bufferRequirements }, mixedMemory))
		{
			const TShared<render::GpuBuffer> buffer = fnCreateBuffer(mixedMemory);
			const TShared<render::Texture> texture = fnCreateTexture(targetInformation, mixedMemory);

			const TShared<render::GpuCommandBuffer> commands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Alias acquire buffer to texture"));
			fnAcquireBuffer(*commands, buffer, GpuAliasAcquire());
			commands->CopyBufferToBuffer(uploads[0], buffer, 0, 0, bufferBytes);
			commands->CopyBufferToBuffer(buffer, readbacks[0], 0, 0, bufferBytes);
			fnAcquireTexture(*commands, texture, transferAcquire, GpuResourceUseFlag::ColorAttachment, GpuImageLayout::ColorAttachment);
			fnClearTarget(*commands, texture);
			commands->CopyTextureToBuffer(texture, readbacks[1], 0, 0);
			context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
			device->WaitUntilIdle();

			fnCheckReadback(0, ReadbackLayout::Buffer, 1);
			fnCheckReadback(1, ReadbackLayout::Texture, 0);
		}

		device->WaitUntilIdle();
		for(SharedMemory& memory : sharedMemories)
			memory.Allocator->Free(memory.Allocation);
	}, "GpuBackendTestSuite::TestAliasAcquireExecution", true);
}

void GpuBackendTestSuite::TestFramebufferAttachmentUsage()
{
	SubmissionTestImage colorImage(4, 2, GpuTextureAspectFlag::Color);
	SubmissionTestImage depthStencilImage(2, 1, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
	SubmissionTestFramebuffer framebuffer(640, 480, 2);

	const GpuTextureSubresourceRange colorRange(1, 1, 2, 2, GpuTextureAspectFlag::Color);
	const GpuTextureSubresourceRange depthStencilRange(0, 1, 1, 1, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
	framebuffer.AddColorAttachment(colorImage, colorRange, 3, GpuImageLayout::TransferSource);
	framebuffer.AddDepthStencilAttachment(depthStencilImage, depthStencilRange, GpuImageLayout::General);

	B3D_TEST_ASSERT(framebuffer.GetWidth() == 640)
	B3D_TEST_ASSERT(framebuffer.GetHeight() == 480)
	B3D_TEST_ASSERT(framebuffer.GetLayerCount() == 2)
	B3D_TEST_ASSERT(framebuffer.GetAttachmentCount() == 3)
	B3D_TEST_ASSERT(framebuffer.GetColorAttachmentCount() == 1)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_COLOR0) == nullptr)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_COLOR3)->GetIndex() == 3)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_DEPTH)->GetIndex() == 0)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_STENCIL)->GetIndex() == 0)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_DEPTH)->Range.AspectMask == GpuTextureAspectFlag::Depth)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_STENCIL)->Range.AspectMask == GpuTextureAspectFlag::Stencil)

	const GpuFramebufferLayoutPolicy layoutPolicy(
		GpuRenderPassAttachmentLayout(GpuImageLayout::ColorAttachment),
		GpuRenderPassAttachmentLayout(GpuImageLayout::General, GpuImageLayout::ShaderReadOnly),
		GpuRenderPassAttachmentLayout(GpuImageLayout::DepthStencilAttachment),
		GpuRenderPassAttachmentLayout(GpuImageLayout::DepthReadOnlyStencilAttachment, GpuImageLayout::DepthStencilReadOnly),
		GpuRenderPassAttachmentLayout(GpuImageLayout::DepthAttachmentStencilReadOnly, GpuImageLayout::DepthStencilReadOnly),
		GpuRenderPassAttachmentLayout(GpuImageLayout::DepthStencilReadOnly, GpuImageLayout::DepthStencilReadOnly),
		GpuImageLayout::Undefined);
	const RenderSurfaceMask readOnlyMask = RT_COLOR3 | RT_DEPTH;
	const RenderSurfaceMask loadMask = RT_COLOR3 | RT_STENCIL;
	const GpuRenderPassAttachmentUsageArray attachmentUsages = framebuffer.BuildRenderPassAttachmentUsages(readOnlyMask, loadMask, layoutPolicy);

	B3D_TEST_ASSERT(attachmentUsages.Size() == 3)
	B3D_TEST_ASSERT(attachmentUsages[0].Surface == RT_COLOR3)
	B3D_TEST_ASSERT(attachmentUsages[0].Access == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(attachmentUsages[0].BarrierFlags == GpuBarrierFlag::None)
	B3D_TEST_ASSERT(attachmentUsages[0].Layout == GpuImageLayout::General)
	B3D_TEST_ASSERT(attachmentUsages[0].ShaderReadLayout == GpuImageLayout::ShaderReadOnly)
	B3D_TEST_ASSERT(attachmentUsages[0].FinalLayout == GpuImageLayout::TransferSource)
	B3D_TEST_ASSERT(attachmentUsages[1].Surface == RT_DEPTH)
	B3D_TEST_ASSERT(attachmentUsages[1].Access == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(attachmentUsages[1].BarrierFlags == GpuBarrierFlag::None)
	B3D_TEST_ASSERT(attachmentUsages[1].Layout == GpuImageLayout::Undefined)
	B3D_TEST_ASSERT(attachmentUsages[1].ShaderReadLayout == GpuImageLayout::DepthStencilReadOnly)
	B3D_TEST_ASSERT(attachmentUsages[2].Surface == RT_STENCIL)
	B3D_TEST_ASSERT(attachmentUsages[2].Access == (GpuAccessFlag::Read | GpuAccessFlag::Write))
	B3D_TEST_ASSERT(attachmentUsages[2].BarrierFlags == GpuBarrierFlag::None)
	B3D_TEST_ASSERT(attachmentUsages[2].Layout == GpuImageLayout::DepthReadOnlyStencilAttachment)
	B3D_TEST_ASSERT(!attachmentUsages[2].ShaderReadLayout.has_value())
	B3D_TEST_ASSERT(attachmentUsages[2].FinalLayout == GpuImageLayout::General)

	const GpuRenderPassAttachmentUsageArray discardAttachmentUsages = framebuffer.BuildRenderPassAttachmentUsages(RT_NONE, RT_NONE, layoutPolicy);
	for(const GpuRenderPassAttachmentUsage& attachmentUsage : discardAttachmentUsages)
		B3D_TEST_ASSERT(attachmentUsage.BarrierFlags == GpuBarrierFlag::DiscardContents)
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
	attachmentUsage.Surface = RT_COLOR2;
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
	B3D_TEST_ASSERT(tracker.ResolveShaderImageLayout(&image, attachmentRange, GpuImageLayout::General) == GpuImageLayout::ShaderReadOnly)
	B3D_TEST_ASSERT(tracker.ResolveShaderImageLayout(&image, GpuTextureSubresourceRange(1, 1, 1, 1, GpuTextureAspectFlag::Color), GpuImageLayout::General) == GpuImageLayout::General)
	TrackImageBinding(tracker, &image, fullRange, GpuImageLayout::ShaderReadOnly, shaderUse, GpuAccessFlag::Read, barrierHelper);

	const TArrayView<const GpuResolvedRenderPassAttachmentUsage> resolvedAttachments = tracker.BeginRenderPass(barrierHelper);
	GpuResourceUseFlags combinedUse = shaderUse;
	combinedUse |= GpuResourceUseFlag::ColorAttachment;
	B3D_TEST_ASSERT(resolvedAttachments.Size() == 1)
	B3D_TEST_ASSERT(resolvedAttachments[0].Surface == RT_COLOR2)
	B3D_TEST_ASSERT(resolvedAttachments[0].UseFlags == combinedUse)
	B3D_TEST_ASSERT(resolvedAttachments[0].Access == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(resolvedAttachments[0].Layout == GpuImageLayout::ShaderReadOnly)
	B3D_TEST_ASSERT(resolvedAttachments[0].FinalLayout == GpuImageLayout::TransferSource)
	B3D_TEST_ASSERT(tracker.ResolveShaderImageLayout(&image, attachmentRange, GpuImageLayout::General) == GpuImageLayout::ShaderReadOnly)

	tracker.CommitPendingAccesses();
	const GpuImageSubresourceTrackingState& attachmentStateBeforeEnd = tracker.GetSubresourceTrackingState(
		&image, 0, 0, GpuTextureAspectFlag::Color);
	const GpuImageSubresourceTrackingState& sampledOnlyStateBeforeEnd = tracker.GetSubresourceTrackingState(
		&image, 1, 1, GpuTextureAspectFlag::Color);
	B3D_TEST_ASSERT(attachmentStateBeforeEnd.RequiredLayout == GpuImageLayout::ShaderReadOnly)
	B3D_TEST_ASSERT(sampledOnlyStateBeforeEnd.RequiredLayout == GpuImageLayout::ShaderReadOnly)

	tracker.EndRenderPass();
	const GpuImageSubresourceTrackingState& attachmentStateAfterEnd = tracker.GetSubresourceTrackingState(
		&image, 0, 0, GpuTextureAspectFlag::Color);
	const GpuImageSubresourceTrackingState& sampledOnlyStateAfterEnd = tracker.GetSubresourceTrackingState(
		&image, 1, 1, GpuTextureAspectFlag::Color);
	B3D_TEST_ASSERT(attachmentStateAfterEnd.CurrentLayout == GpuImageLayout::TransferSource)
	B3D_TEST_ASSERT(sampledOnlyStateAfterEnd.CurrentLayout == GpuImageLayout::ShaderReadOnly)

	tracker.NotifyUnbound();
	tracker.Clear();

	SubmissionTestImage depthStencilImage(1, 1, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
	SubmissionTestBarrierHelper depthStencilBarrierHelper;
	SubmissionTestTracker depthStencilTracker;
	const GpuTextureSubresourceRange depthRange(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
	const GpuTextureSubresourceRange stencilRange(0, 1, 0, 1, GpuTextureAspectFlag::Stencil);

	GpuRenderPassAttachmentUsage depthAttachmentUsage;
	depthAttachmentUsage.Image = &depthStencilImage;
	depthAttachmentUsage.Range = depthRange;
	depthAttachmentUsage.Surface = RT_DEPTH;
	depthAttachmentUsage.UseFlags = GpuResourceUseFlag::DepthStencilAttachment;
	depthAttachmentUsage.Access = GpuAccessFlag::Read;
	depthAttachmentUsage.Layout = GpuImageLayout::DepthReadOnlyStencilAttachment;
	depthAttachmentUsage.ShaderReadLayout = GpuImageLayout::DepthStencilReadOnly;

	GpuRenderPassAttachmentUsage stencilAttachmentUsage;
	stencilAttachmentUsage.Image = &depthStencilImage;
	stencilAttachmentUsage.Range = stencilRange;
	stencilAttachmentUsage.Surface = RT_STENCIL;
	stencilAttachmentUsage.UseFlags = GpuResourceUseFlag::DepthStencilAttachment;
	stencilAttachmentUsage.Access = GpuAccessFlag::Write;
	stencilAttachmentUsage.Layout = GpuImageLayout::DepthReadOnlyStencilAttachment;

	TInlineArray<GpuRenderPassAttachmentUsage, 2> depthStencilAttachments;
	depthStencilAttachments.Add(depthAttachmentUsage);
	depthStencilAttachments.Add(stencilAttachmentUsage);
	depthStencilTracker.PrepareRenderPass(depthStencilAttachments);
	TrackImageBinding(depthStencilTracker, &depthStencilImage, depthRange, GpuImageLayout::ShaderReadOnly, shaderUse, GpuAccessFlag::Read, depthStencilBarrierHelper);

	const TArrayView<const GpuResolvedRenderPassAttachmentUsage> resolvedDepthStencilAttachments = depthStencilTracker.BeginRenderPass(depthStencilBarrierHelper);
	B3D_TEST_ASSERT(resolvedDepthStencilAttachments.Size() == 2)
	B3D_TEST_ASSERT(resolvedDepthStencilAttachments[0].Surface == RT_DEPTH)
	B3D_TEST_ASSERT(resolvedDepthStencilAttachments[0].UseFlags == (shaderUse | GpuResourceUseFlag::DepthStencilAttachment))
	B3D_TEST_ASSERT(resolvedDepthStencilAttachments[0].Access == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(resolvedDepthStencilAttachments[0].Layout == GpuImageLayout::DepthStencilReadOnly)
	B3D_TEST_ASSERT(resolvedDepthStencilAttachments[1].Surface == RT_STENCIL)
	B3D_TEST_ASSERT(resolvedDepthStencilAttachments[1].UseFlags == GpuResourceUseFlag::DepthStencilAttachment)
	B3D_TEST_ASSERT(resolvedDepthStencilAttachments[1].Access == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(resolvedDepthStencilAttachments[1].Layout == GpuImageLayout::DepthReadOnlyStencilAttachment)

	depthStencilTracker.CommitPendingAccesses();
	depthStencilTracker.EndRenderPass();
	depthStencilTracker.NotifyUnbound();
	depthStencilTracker.Clear();

	SubmissionTestImage discardImage(1, 1, GpuTextureAspectFlag::Color);
	SubmissionTestBarrierHelper discardBarrierHelper;
	SubmissionTestTracker discardTracker;
	GpuRenderPassAttachmentUsage discardAttachmentUsage;
	discardAttachmentUsage.Image = &discardImage;
	discardAttachmentUsage.Range = attachmentRange;
	discardAttachmentUsage.Surface = RT_COLOR0;
	discardAttachmentUsage.UseFlags = GpuResourceUseFlag::ColorAttachment;
	discardAttachmentUsage.Access = GpuAccessFlag::Write;
	discardAttachmentUsage.BarrierFlags = GpuBarrierFlag::DiscardContents;
	discardAttachmentUsage.Layout = GpuImageLayout::ColorAttachment;

	TInlineArray<GpuRenderPassAttachmentUsage, 1> discardAttachments;
	discardAttachments.Add(discardAttachmentUsage);
	discardTracker.PrepareRenderPass(discardAttachments);
	const TArrayView<const GpuResolvedRenderPassAttachmentUsage> resolvedDiscardAttachments = discardTracker.BeginRenderPass(discardBarrierHelper);
	B3D_TEST_ASSERT(resolvedDiscardAttachments[0].BarrierFlags == GpuBarrierFlag::DiscardContents)

	discardTracker.CommitPendingAccesses();
	const GpuImageSubresourceTrackingState& discardTrackingState = discardTracker.GetSubresourceTrackingState(
		&discardImage, 0, 0, GpuTextureAspectFlag::Color);
	B3D_TEST_ASSERT(discardTrackingState.SubmissionBarrierFlags == GpuBarrierFlag::DiscardContents)

	discardTracker.EndRenderPass();
	SubmissionImageTestVisitor discardVisitor;
	discardTracker.ResolveSubmissionTransitions(GpuQueueId(GQT_GRAPHICS, 0), kTestFrameIndex, discardVisitor);
	B3D_TEST_ASSERT(discardVisitor.SubmissionBarrierFlags == GpuBarrierFlag::DiscardContents)

	discardTracker.PrepareRenderPass(discardAttachments);
	discardTracker.BeginRenderPass(discardBarrierHelper);
	B3D_TEST_ASSERT(discardBarrierHelper.LastImageBarrierFlags == GpuBarrierFlag::DiscardContents)
	discardTracker.CommitPendingAccesses();
	discardTracker.EndRenderPass();
	discardTracker.NotifyUnbound();
	discardTracker.Clear();
}

#if B3D_GPU_EXPLICIT_BARRIERS
void GpuBackendTestSuite::TestTrackingWithoutHazards()
{
	SubmissionTestBarrierHelper barrierHelper;
	SubmissionTestTracker tracker;
	tracker.SetHazardTracking(false);

	// Buffer hazards queue no barriers and create no hazard state
	SubmissionTestBuffer buffer;
	tracker.TrackBufferAccess(&buffer, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, barrierHelper);
	tracker.CommitPendingAccesses();
	tracker.TrackBufferAccess(&buffer, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write, barrierHelper);
	tracker.CommitPendingAccesses();
	tracker.TrackBufferAccess(&buffer, GpuStageFlag::FragmentShaderUniform, GpuAccessFlag::Read, barrierHelper);
	tracker.CommitPendingAccesses();

	const GpuBufferTrackingState* const bufferState = tracker.FindBufferTrackingState(&buffer);
	B3D_TEST_ASSERT(bufferState->HazardState == nullptr)
	B3D_TEST_ASSERT(bufferState->UseHandle.Flags == (GpuAccessFlag::Read | GpuAccessFlag::Write))
	B3D_TEST_ASSERT(bufferState->UseHandle.Stages == (GpuStageFlag::ComputeShaderNonUniform | GpuStageFlag::FragmentShaderUniform))
	B3D_TEST_ASSERT(buffer.GetBoundCount() == 1)
	B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 0)

	// Image hazards and layout changes queue no barriers and create no subresource tracking states
	SubmissionTestImage image(2, 2, GpuTextureAspectFlag::Color);
	tracker.TrackImageAccess(&image, GpuTextureSubresourceRange(1, 1, 0, 2, GpuTextureAspectFlag::Color), GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, barrierHelper);
	tracker.CommitPendingAccesses();
	tracker.TrackImageAccess(&image, image.GetRange(), GpuImageLayout::ShaderReadOnly, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, barrierHelper);
	tracker.CommitPendingAccesses();

	const GpuImageTrackingState* const imageState = tracker.FindImageTrackingState(&image);
	B3D_TEST_ASSERT(imageState->UseHandle.Flags == (GpuAccessFlag::Read | GpuAccessFlag::Write))
	B3D_TEST_ASSERT(tracker.GetSubresourceTrackingStatesForImage(&image).Size() == 0)
	B3D_TEST_ASSERT(image.GetBoundCount() == 1)
	B3D_TEST_ASSERT(image.GetFullRangeSubresource()->GetBoundCount() == 1)
	B3D_TEST_ASSERT(image.GetSubresource(0, 1, GpuTextureAspectFlag::Color)->GetBoundCount() == 1)
	B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 0)

	// A sampled read-only attachment still makes the render pass use its shader read layout
	SubmissionTestImage attachmentImage(1, 1, GpuTextureAspectFlag::Color);
	GpuRenderPassAttachmentUsage attachmentUsage;
	attachmentUsage.Image = &attachmentImage;
	attachmentUsage.Range = attachmentImage.GetRange();
	attachmentUsage.Surface = RT_COLOR0;
	attachmentUsage.UseFlags = GpuResourceUseFlag::ColorAttachment;
	attachmentUsage.Access = GpuAccessFlag::Read;
	attachmentUsage.Layout = GpuImageLayout::ColorAttachment;
	attachmentUsage.ShaderReadLayout = GpuImageLayout::ShaderReadOnly;

	TInlineArray<GpuRenderPassAttachmentUsage, 1> attachments;
	attachments.Add(attachmentUsage);
	tracker.PrepareRenderPass(attachments);

	const GpuResourceUseFlags shaderUse = GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader;
	B3D_TEST_ASSERT(TrackImageBinding(tracker, &attachmentImage, attachmentImage.GetRange(), GpuImageLayout::ShaderReadOnly, shaderUse, GpuAccessFlag::Read, barrierHelper))

	const TArrayView<const GpuResolvedRenderPassAttachmentUsage> resolvedAttachments = tracker.BeginRenderPass(barrierHelper);
	B3D_TEST_ASSERT(resolvedAttachments.Size() == 1)
	B3D_TEST_ASSERT(resolvedAttachments[0].Layout == GpuImageLayout::ShaderReadOnly)
	tracker.CommitPendingAccesses();
	tracker.EndRenderPass();

	B3D_TEST_ASSERT(tracker.GetSubresourceTrackingStatesForImage(&attachmentImage).Size() == 0)
	B3D_TEST_ASSERT(attachmentImage.GetBoundCount() == 1)
	B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 0)

	// Submission resolves no transitions and leaves the submission states as they were
	SubmissionImageTestVisitor visitor;
	tracker.ResolveSubmissionTransitions(GpuQueueId(GQT_GRAPHICS, 0), kTestFrameIndex, visitor);
	B3D_TEST_ASSERT(visitor.NativeStates.Empty())
	B3D_TEST_ASSERT(!buffer.GetSubmissionState().HasWriter)
	B3D_TEST_ASSERT(!image.GetFullRangeSubresource()->SubmissionState.HasWriter)

	tracker.NotifyUnbound();
	tracker.Clear();
	B3D_TEST_ASSERT(buffer.GetBoundCount() == 0)
	B3D_TEST_ASSERT(image.GetBoundCount() == 0)
	B3D_TEST_ASSERT(attachmentImage.GetBoundCount() == 0)
	B3D_TEST_ASSERT(!tracker.TracksHazards())
}

namespace
{
	/**
	 * Creates a compute pipeline whose thread (x, y) of a 16 x 16 dispatch writes 0x5A000000 | (y * 16 + x) into element y * 16 + x of
	 * the storage buffer OutputBuffer, and into texel (x, y) of the R32U storage texture OutputTexture. Returns null on backends that
	 * cannot compile shaders on the host.
	 */
	TShared<GpuComputePipelineState> CreateExplicitBarrierDispatchPipeline(GpuDevice& device)
	{
#if B3D_PLATFORM_PS5
		(void)device;
		return nullptr;
#else
		const String backendName = GpuBackend::Instance().GetBackendName();
		if(backendName != "bsfD3D12GpuBackend" && backendName != "bsfVulkanGpuBackend")
			return nullptr;

		const TShared<IShaderCompiler> compiler = ShaderCompilers::Instance().GetCompiler("bsl");
		if(compiler == nullptr)
			return nullptr;

		const String source = R"(
shader ExplicitBarrierDispatch
{
	code
	{
		RWStructuredBuffer<uint> OutputBuffer;
		RWTexture2D<uint> OutputTexture;
		[numthreads(16, 16, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			const uint value = 0x5A000000 | (threadId.y * 16 + threadId.x);
			OutputBuffer[threadId.y * 16 + threadId.x] = value;
			OutputTexture[threadId.xy] = value;
		}
	};
};
)";
		const String language = backendName == "bsfD3D12GpuBackend" ? "hlsl" : "vksl";

		TShared<b3d::Shader> shader;
		const ShaderCompilerResult result = compiler->Compile("ExplicitBarrierDispatch", source, {}, { language }, true, shader);
		if(!result.ErrorMessage.empty() || shader == nullptr || shader->GetVariations().size() != 1 || shader->GetVariations().front()->GetPassCount() != 1)
			return nullptr;

		GpuComputePipelineStateCreateInformation pipelineInformation;
		pipelineInformation.Program = device.CreateGpuProgram(shader->GetVariations().front()->GetPass(0)->GetGpuProgramCreateInformation(GPT_COMPUTE_PROGRAM));
		return device.CreateGpuComputePipelineState(pipelineInformation);
#endif
	}
}

void GpuBackendTestSuite::TestExplicitBarriers()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0 || !backend.GetDevice(0)->GetCapabilities().HasCapability(RSC_EXPLICIT_BARRIERS))
		return;

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuDevice> device = backend.GetDevice(0);
	const TShared<GpuComputePipelineState> pipeline = CreateExplicitBarrierDispatchPipeline(*device);
	GetRenderThread().PostCommand([this, &device, &pipeline]()
	{
		static constexpr u32 kSize = 16;
		static constexpr u32 kPattern = 0x5A000000;
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);
		static constexpr u32 bufferBytes = kSize * kSize * sizeof(u32);

		// Render targets are exclusive to one queue family on Vulkan, while sampleable-only textures are shared between them
		TextureCreateInformation exclusiveInformation;
		exclusiveInformation.Name = "Explicit barrier exclusive texture";
		exclusiveInformation.Width = kSize;
		exclusiveInformation.Height = kSize;
		exclusiveInformation.Format = PF_RGBA8;
		exclusiveInformation.Usage = TextureUsageFlag::RenderTarget;

		TextureCreateInformation sharedInformation = exclusiveInformation;
		sharedInformation.Name = "Explicit barrier shared texture";
		sharedInformation.Usage = TextureUsageFlag::Default;

		const ImageSubresourcePitch pitch = device->CreateTexture(exclusiveInformation)->GetStagingBufferPitchForSubresource(0, 0);
		const u32 stagingElementCount = pitch.RowPitch * pitch.SliceHeight;

		const TShared<render::GpuBuffer> upload = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(stagingElementCount * sizeof(u32)));
		{
			const render::GpuBufferMappedScope mapping = upload->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			u32* elements = static_cast<u32*>(mapping.GetMappedMemory());
			for(u32 elementIndex = 0; elementIndex < stagingElementCount; elementIndex++)
				elements[elementIndex] = kPattern | elementIndex;
		}

		// A buffer and two textures written by copies, and a readback for each
		struct CaseResources
		{
			TShared<render::GpuBuffer> Buffer;
			TShared<render::Texture> Textures[2];
			TShared<render::GpuBuffer> Readbacks[3];
		};

		auto fnCreateResources = [&device, &exclusiveInformation, &sharedInformation, stagingElementCount]()
		{
			CaseResources resources;
			resources.Buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize));
			resources.Textures[0] = device->CreateTexture(exclusiveInformation);
			resources.Textures[1] = device->CreateTexture(sharedInformation);
			for(TShared<render::GpuBuffer>& readback : resources.Readbacks)
				readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(stagingElementCount * sizeof(u32)));

			return resources;
		};

		// The barriers between the writes of a case and its reads
		struct CaseBarriers
		{
			CaseBarriers(const CaseResources& resources, const GpuAccessState& source, const GpuAccessState& destination)
				: Buffer(resources.Buffer, source, destination),
				Textures{ GpuExplicitTextureBarrier(resources.Textures[0], source, destination), GpuExplicitTextureBarrier(resources.Textures[1], source, destination) }
			{ }

			GpuExplicitBarriers Get() const { return GpuExplicitBarriers(TArrayView<const GpuExplicitBufferBarrier>(&Buffer, 1), TArrayView<const GpuExplicitTextureBarrier>(Textures, 2)); }

			GpuExplicitBufferBarrier Buffer;
			GpuExplicitTextureBarrier Textures[2];
		};

		// Writes the buffer and the textures with copies, from undefined contents
		auto fnWrite = [&upload, &transferWrite](render::GpuCommandBuffer& commands, const CaseResources& resources)
		{
			commands.IssueExplicitBarriers(CaseBarriers(resources, GpuAccessState(), transferWrite).Get());
			commands.CopyBufferToBuffer(upload, resources.Buffer, 0, 0, bufferBytes);
			for(const TShared<render::Texture>& texture : resources.Textures)
				commands.CopyBufferToTexture(upload, texture, 0, 0, 0);
		};

		// Copies the buffer and the textures into the readbacks, and makes the copies visible to the host
		auto fnRead = [&transferWrite, &hostRead](render::GpuCommandBuffer& commands, const CaseResources& resources)
		{
			const auto fnIssueReadbackBarriers = [&commands, &resources](const GpuAccessState& source, const GpuAccessState& destination)
			{
				const GpuExplicitBufferBarrier barriers[] = { GpuExplicitBufferBarrier(resources.Readbacks[0], source, destination),
					GpuExplicitBufferBarrier(resources.Readbacks[1], source, destination), GpuExplicitBufferBarrier(resources.Readbacks[2], source, destination) };
				commands.IssueExplicitBarriers(GpuExplicitBarriers(TArrayView<const GpuExplicitBufferBarrier>(barriers, 3)));
			};

			fnIssueReadbackBarriers(GpuAccessState(), transferWrite);
			commands.CopyBufferToBuffer(resources.Buffer, resources.Readbacks[0], 0, 0, bufferBytes);
			commands.CopyTextureToBuffer(resources.Textures[0], resources.Readbacks[1], 0, 0, 0);
			commands.CopyTextureToBuffer(resources.Textures[1], resources.Readbacks[2], 0, 0, 0);
			fnIssueReadbackBarriers(transferWrite, hostRead);
		};

		// Checks that a readback holds the pattern, in the buffer layout or in the texture staging layout
		auto fnCheckReadback = [this, &pitch](const TShared<render::GpuBuffer>& readback, bool isTexture, const String& caseName)
		{
			const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			const u32* elements = static_cast<const u32*>(mapping.GetMappedMemory());
			bool matches = true;
			for(u32 row = 0; row < kSize; row++)
			{
				for(u32 column = 0; column < kSize; column++)
				{
					const u32 elementIndex = isTexture ? row * pitch.RowPitch + column : row * kSize + column;
					matches &= elements[elementIndex] == (kPattern | elementIndex);
				}
			}

			B3D_TEST_ASSERT_MSG(matches, caseName + ": the readback does not hold the written values.")
		};

		auto fnCheck = [&fnCheckReadback](const CaseResources& resources, const String& caseName)
		{
			fnCheckReadback(resources.Readbacks[0], false, caseName);
			fnCheckReadback(resources.Readbacks[1], true, caseName);
			fnCheckReadback(resources.Readbacks[2], true, caseName);
		};

		auto fnCreatePool = [&device](GpuQueueType queueType, bool explicitBarriers)
		{
			GpuCommandBufferPoolCreateInformation information = GpuCommandBufferPoolCreateInformation::CreateForThisThread(queueType);
			information.ExplicitBarriers = explicitBarriers;
			return device->CreateGpuCommandBufferPool(information);
		};

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<GpuCommandBufferPool> graphicsPool = fnCreatePool(GQT_GRAPHICS, true);

		// Full barriers
		{
			const CaseResources resources = fnCreateResources();
			const TShared<GpuCommandBuffer> commands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Explicit full barriers"));
			fnWrite(*commands, resources);
			commands->IssueExplicitBarriers(CaseBarriers(resources, transferWrite, transferRead).Get());
			fnRead(*commands, resources);
			context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
			device->WaitUntilIdle();
			fnCheck(resources, "Full barriers");

			// A command buffer that derives its barriers from tracked state continues from the layouts the explicit barriers left
			const TShared<GpuCommandBufferPool> automaticPool = fnCreatePool(GQT_GRAPHICS, false);
			const TShared<GpuCommandBuffer> automaticCommands = automaticPool->Create(GpuCommandBufferCreateInformation::Create("Automatic barriers after explicit barriers"));
			automaticCommands->CopyTextureToBuffer(resources.Textures[0], resources.Readbacks[1], 0, 0, 0);
			automaticCommands->CopyTextureToBuffer(resources.Textures[1], resources.Readbacks[2], 0, 0, 0);
			context->SubmitCommandBuffer(automaticCommands, GpuQueueMask::kNone);
			device->WaitUntilIdle();
			fnCheck(resources, "Automatic barriers after explicit barriers");
		}

		// Split barrier within one command buffer, with unrelated work between the halves
		{
			const CaseResources resources = fnCreateResources();
			const CaseBarriers barriers(resources, transferWrite, transferRead);
			const TShared<GpuSplitBarrier> split = device->CreateSplitBarrier();
			const TShared<render::GpuBuffer> unrelated = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize));

			const TShared<GpuCommandBuffer> commands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Explicit split barriers"));
			fnWrite(*commands, resources);
			commands->ReleaseBarriers(barriers.Get(), split);
			commands->CopyBufferToBuffer(upload, unrelated, 0, 0, bufferBytes);
			commands->AcquireBarriers(barriers.Get(), split);
			fnRead(*commands, resources);
			context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
			device->WaitUntilIdle();
			fnCheck(resources, "Split barriers within a command buffer");
		}

		// Split barriers whose halves are on different command buffers, recorded in either order. With @p readQueue on a queue of
		// another type than @p writeQueue, the split barrier transfers the resources between the queues.
		auto fnTestSplitAcrossCommandBuffers = [&](const TShared<GpuCommandBufferPool>& writePool, GpuQueueType writeQueue, GpuQueueType readQueue, bool acquireFirst, const String& caseName)
		{
			const CaseResources resources = fnCreateResources();
			const CaseBarriers barriers(resources, transferWrite, transferRead);
			const TShared<GpuSplitBarrier> split = writeQueue == readQueue ? device->CreateSplitBarrier() : device->CreateSplitBarrier(writeQueue, readQueue);

			const TShared<GpuCommandBuffer> readCommands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Explicit split barrier acquire"));
			if(acquireFirst)
				readCommands->AcquireBarriers(barriers.Get(), split);

			const TShared<GpuCommandBuffer> writeCommands = writePool->Create(GpuCommandBufferCreateInformation::Create("Explicit split barrier release"));
			fnWrite(*writeCommands, resources);
			writeCommands->ReleaseBarriers(barriers.Get(), split);

			if(!acquireFirst)
				readCommands->AcquireBarriers(barriers.Get(), split);

			fnRead(*readCommands, resources);

			context->SubmitCommandBuffer(writeCommands, GpuQueueMask::kNone);
			context->SubmitCommandBuffer(readCommands, writeQueue == readQueue ? GpuQueueMask::kNone : GpuQueueMask(GpuQueueId(writeQueue, 0)));
			device->WaitUntilIdle();
			fnCheck(resources, caseName);
		};

		// Both command buffers record at once, which some backends only allow for command buffers from different pools
		const TShared<GpuCommandBufferPool> writePool = fnCreatePool(GQT_GRAPHICS, true);
		fnTestSplitAcrossCommandBuffers(writePool, GQT_GRAPHICS, GQT_GRAPHICS, false, "Split barriers across command buffers");
		fnTestSplitAcrossCommandBuffers(writePool, GQT_GRAPHICS, GQT_GRAPHICS, true, "Split barriers across command buffers, acquire recorded first");

		if(device->GetQueueCount(GQT_COMPUTE) > 0)
		{
			const TShared<GpuCommandBufferPool> computePool = fnCreatePool(GQT_COMPUTE, true);
			fnTestSplitAcrossCommandBuffers(computePool, GQT_COMPUTE, GQT_GRAPHICS, false, "Split barriers across queues");
			fnTestSplitAcrossCommandBuffers(computePool, GQT_COMPUTE, GQT_GRAPHICS, true, "Split barriers across queues, acquire recorded first");
		}

		// Compute shader writes to a storage buffer and a storage texture, on backends that compile shaders on the host
		if(pipeline != nullptr)
		{
			TextureCreateInformation storageInformation = exclusiveInformation;
			storageInformation.Name = "Explicit barrier storage texture";
			storageInformation.Format = PF_R32U;
			storageInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

			const TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
			const TShared<render::Texture> texture = device->CreateTexture(storageInformation);
			const ImageSubresourcePitch storagePitch = texture->GetStagingBufferPitchForSubresource(0, 0);
			const TShared<render::GpuBuffer> bufferReadback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(bufferBytes));
			const TShared<render::GpuBuffer> textureReadback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(storagePitch.RowPitch * storagePitch.SliceHeight * sizeof(u32)));

			const TShared<render::GpuParameterSet> parameters = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			parameters->SetStorageBuffer("OutputBuffer", buffer);
			parameters->SetStorageTexture("OutputTexture", texture, TextureSurface());

			const TShared<GpuCommandBuffer> commands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Explicit barriers around a dispatch"));
			const auto fnIssueBarriers = [&](const GpuAccessState& source, const GpuAccessState& destination, const GpuAccessState& readbackSource, const GpuAccessState& readbackDestination)
			{
				const GpuExplicitBufferBarrier bufferBarriers[] = { GpuExplicitBufferBarrier(buffer, source, destination),
					GpuExplicitBufferBarrier(bufferReadback, readbackSource, readbackDestination), GpuExplicitBufferBarrier(textureReadback, readbackSource, readbackDestination) };
				const GpuExplicitTextureBarrier textureBarrier(texture, source, destination);
				commands->IssueExplicitBarriers(GpuExplicitBarriers(TArrayView<const GpuExplicitBufferBarrier>(bufferBarriers, 3), TArrayView<const GpuExplicitTextureBarrier>(&textureBarrier, 1)));
			};

			const GpuAccessState computeReadWrite(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::General);
			fnIssueBarriers(GpuAccessState(), computeReadWrite, GpuAccessState(), transferWrite);
			commands->SetGpuComputePipelineState(pipeline);
			commands->SetGpuParameterSet(parameters);
			commands->DispatchCompute(1, 1, 1);
			fnIssueBarriers(computeReadWrite, transferRead, transferWrite, transferWrite);
			commands->CopyBufferToBuffer(buffer, bufferReadback, 0, 0, bufferBytes);
			commands->CopyTextureToBuffer(texture, textureReadback, 0, 0, 0);
			fnIssueBarriers(transferRead, transferRead, transferWrite, hostRead);
			context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
			device->WaitUntilIdle();

			fnCheckReadback(bufferReadback, false, "Dispatch");

			const render::GpuBufferMappedScope mapping = textureReadback->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(mapping.IsValid())
			{
				const u32* elements = static_cast<const u32*>(mapping.GetMappedMemory());
				bool matches = true;
				for(u32 row = 0; row < kSize; row++)
				{
					for(u32 column = 0; column < kSize; column++)
						matches &= elements[row * storagePitch.RowPitch + column] == (kPattern | (row * kSize + column));
				}

				B3D_TEST_ASSERT_MSG(matches, "Dispatch: the storage texture readback does not hold the written values.")
			}
		}
		else
			B3D_TEST_ASSERT(String(GpuBackend::Instance().GetBackendName()) != "bsfVulkanGpuBackend")
	}, "GpuBackendTestSuite::TestExplicitBarriers", true);
}

#if B3D_BUILD_TYPE_DEVELOPMENT
void GpuBackendTestSuite::TestExplicitBarrierValidation()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0 || !backend.GetDevice(0)->GetCapabilities().HasCapability(RSC_EXPLICIT_BARRIERS))
		return;

	// Each case uses its own resource, named after the case, so the errors do not cascade and each one identifies its case
	const String undeclaredStages = "The resource was accessed in stages that its last explicit barrier does not declare.";
	// The D3D12 debug layer tracks layouts natively and fails to close a command list with a misdeclared layout
	const bool recordsMisdeclaredLayouts = String(backend.GetBackendName()) != "bsfD3D12GpuBackend";

	LoggingScope logs(*this);
	if(recordsMisdeclaredLayouts)
		logs.ExpectError("The source layout of an explicit barrier differs from the destination layout of the previous barrier of the resource. Image: 'SourceLayout'.");

	logs.ExpectError("The source of an explicit barrier is missing stages that accessed the resource since the previous barrier. Buffer: 'MissingStages'.");
	logs.ExpectError("The source of an explicit barrier is missing the writes of the resource since the previous barrier. Buffer: 'MissingWrites'.");
	logs.ExpectError("The resource was accessed with reads or writes that its last explicit barrier does not declare. Buffer: 'UndeclaredWrite'.");
	logs.ExpectError("The resource was accessed between the release and the acquire of a split barrier. Buffer: 'AccessBetweenHalves'.");
	logs.ExpectError("An explicit barrier was recorded between the release and the acquire of a split barrier of the same resource. Buffer: 'BarrierBetweenHalves'.");
	logs.ExpectError("An alias acquire must precede every other use of the resource on the command buffer. Buffer: 'LateAliasAcquire'.");
	logs.ExpectError("The release and the acquire of a split barrier must be given the same barriers, in the same order.");
	logs.ExpectError("The image was accessed in a different layout than its last explicit barrier declares. Image: 'RenderPassLayout'.");
	logs.ExpectError("The release of a split barrier was recorded on a queue type other than the one the split barrier was created for.");
	for(const char* name : { "Buffer: 'CopyBufferToBuffer'.", "Image: 'CopyBufferToTexture'.", "Image: 'CopyTextureToBuffer'.", "Image: 'CopyTexture'.", "Image: 'BlitTexture'.", "Image: 'RenderPass'." })
		logs.ExpectError(undeclaredStages + " " + name);

	// Backends that track layouts natively, such as the Vulkan validation layers, also report the misdeclared layouts
	logs.IgnoreError("VUID-VkImageMemoryBarrier-oldLayout-01197");
	logs.IgnoreError("VUID-vkCmdBeginRenderPass-initialLayout-00900");

	const TShared<GpuDevice> device = backend.GetDevice(0);
	const TShared<GpuComputePipelineState> pipeline = CreateExplicitBarrierDispatchPipeline(*device);
	if(pipeline != nullptr)
		logs.ExpectError(undeclaredStages + " Buffer: 'DispatchCompute'.");

	GetRenderThread().PostCommand([&device, &pipeline, recordsMisdeclaredLayouts]()
	{
		static constexpr u32 kSize = 16;
		const GpuAccessFlags readWrite = GpuAccessFlag::Read | GpuAccessFlag::Write;
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		const GpuAccessState colorAttachment(GpuStageFlag::ColorAttachment, readWrite, GpuImageLayout::ColorAttachment);

		// Declares accesses in other stages than the ones the commands access in
		const GpuAccessState host(GpuStageFlag::Host, readWrite);

		const auto fnCreateBuffer = [&device](const char* name)
		{
			const TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize));
			buffer->SetName(name);
			return buffer;
		};

		const auto fnCreateTexture = [&device](const char* name)
		{
			TextureCreateInformation information;
			information.Name = name;
			information.Width = kSize;
			information.Height = kSize;
			information.Format = PF_RGBA8;
			information.Usage = TextureUsageFlag::RenderTarget;
			return device->CreateTexture(information);
		};

		const auto fnCreateTarget = [](const TShared<render::Texture>& texture)
		{
			render::RenderTextureCreateInformation information;
			information.ColorSurfaces[0].Texture = texture;
			return render::RenderTexture::Create(information);
		};

		// Parameter sets come from the context, which outlives the command buffer that keeps them bound
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		GpuCommandBufferPoolCreateInformation poolInformation = GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS);
		poolInformation.ExplicitBarriers = true;
		const TShared<GpuCommandBufferPool> pool = device->CreateGpuCommandBufferPool(poolInformation);
		const TShared<GpuCommandBuffer> commandBuffer = pool->Create(GpuCommandBufferCreateInformation::Create("Explicit barrier validation"));

		const TShared<render::GpuBuffer> source = fnCreateBuffer("Source");
		const TShared<render::Texture> sourceTexture = fnCreateTexture("SourceTexture");
		const ImageSubresourcePitch pitch = sourceTexture->GetStagingBufferPitchForSubresource(0, 0);
		const u32 stagingSize = pitch.RowPitch * pitch.SliceHeight * 4;
		const TShared<render::GpuBuffer> upload = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(stagingSize));
		const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(stagingSize));
		commandBuffer->IssueExplicitBarriers(GpuExplicitBufferBarrier(source, GpuAccessState(), transferRead));
		commandBuffer->IssueExplicitBarriers(GpuExplicitBufferBarrier(upload, GpuAccessState(), transferRead));
		commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(sourceTexture, GpuAccessState(), transferRead));
		const auto fnCopy = [&commandBuffer, &source](const TShared<render::GpuBuffer>& destination) { commandBuffer->CopyBufferToBuffer(source, destination, 0, 0, sizeof(u32)); };

		// The source layout must be the declared layout
		if(recordsMisdeclaredLayouts)
		{
			const TShared<render::Texture> sourceLayout = fnCreateTexture("SourceLayout");
			commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(sourceLayout, GpuAccessState(), transferWrite));
			commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(sourceLayout, transferRead, colorAttachment));
		}

		// The source must include the stages and the writes since the previous barrier
		const TShared<render::GpuBuffer> missingStages = fnCreateBuffer("MissingStages");
		fnCopy(missingStages);
		commandBuffer->IssueExplicitBarriers(GpuExplicitBufferBarrier(missingStages, GpuAccessState(GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Write), transferRead));

		const TShared<render::GpuBuffer> missingWrites = fnCreateBuffer("MissingWrites");
		fnCopy(missingWrites);
		commandBuffer->IssueExplicitBarriers(GpuExplicitBufferBarrier(missingWrites, transferRead, transferRead));

		// Accesses must be declared by the last barrier
		const TShared<render::GpuBuffer> undeclaredWrite = fnCreateBuffer("UndeclaredWrite");
		commandBuffer->IssueExplicitBarriers(GpuExplicitBufferBarrier(undeclaredWrite, GpuAccessState(), transferRead));
		fnCopy(undeclaredWrite);

		// The resource must not be accessed, or given another barrier, between the halves of a split barrier
		const TShared<render::GpuBuffer> accessBetweenHalves = fnCreateBuffer("AccessBetweenHalves");
		const TShared<GpuSplitBarrier> accessedSplit = device->CreateSplitBarrier();
		const GpuExplicitBufferBarrier accessedBarrier(accessBetweenHalves, transferWrite, transferRead);
		commandBuffer->ReleaseBarriers(accessedBarrier, accessedSplit);
		commandBuffer->CopyBufferToBuffer(accessBetweenHalves, readback, 0, 0, sizeof(u32));
		commandBuffer->AcquireBarriers(accessedBarrier, accessedSplit);

		const TShared<render::GpuBuffer> barrierBetweenHalves = fnCreateBuffer("BarrierBetweenHalves");
		const TShared<GpuSplitBarrier> interruptedSplit = device->CreateSplitBarrier();
		const GpuExplicitBufferBarrier interruptedBarrier(barrierBetweenHalves, transferWrite, transferRead);
		commandBuffer->ReleaseBarriers(interruptedBarrier, interruptedSplit);
		commandBuffer->IssueExplicitBarriers(GpuExplicitBufferBarrier(barrierBetweenHalves, transferWrite, transferRead));
		commandBuffer->AcquireBarriers(interruptedBarrier, interruptedSplit);

		// An alias acquire must be the first use of the resource
		const TShared<render::GpuBuffer> lateAliasAcquire = fnCreateBuffer("LateAliasAcquire");
		fnCopy(lateAliasAcquire);
		commandBuffer->IssueExplicitBarriers(GpuExplicitBufferBarrier(lateAliasAcquire, GpuAccessState(), transferWrite, GpuBarrierFlag::AliasAcquire));

		// Both halves of a split barrier must be given the same barriers
		const TShared<render::GpuBuffer> mismatchedHalves = fnCreateBuffer("MismatchedHalves");
		const TShared<GpuSplitBarrier> mismatchedSplit = device->CreateSplitBarrier();
		commandBuffer->ReleaseBarriers(GpuExplicitBufferBarrier(mismatchedHalves, transferWrite, transferRead), mismatchedSplit);
		commandBuffer->AcquireBarriers(GpuExplicitBufferBarrier(mismatchedHalves, transferWrite, GpuAccessState(GpuStageFlag::Host, GpuAccessFlag::Read)), mismatchedSplit);

		// Each half must be recorded on the queue type the split barrier was created for. Nothing is recorded otherwise.
		const TShared<GpuSplitBarrier> otherQueueSplit = device->CreateSplitBarrier(GQT_COMPUTE, GQT_GRAPHICS);
		commandBuffer->ReleaseBarriers(GpuExplicitBufferBarrier(fnCreateBuffer("OtherQueue"), transferWrite, transferRead), otherQueueSplit);

		// Each command validates its accesses
		const TShared<render::GpuBuffer> copyBufferToBuffer = fnCreateBuffer("CopyBufferToBuffer");
		commandBuffer->IssueExplicitBarriers(GpuExplicitBufferBarrier(copyBufferToBuffer, GpuAccessState(), host));
		commandBuffer->CopyBufferToBuffer(copyBufferToBuffer, readback, 0, 0, sizeof(u32));

		const TShared<render::Texture> copyBufferToTexture = fnCreateTexture("CopyBufferToTexture");
		commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(copyBufferToTexture, GpuAccessState(), host));
		commandBuffer->CopyBufferToTexture(upload, copyBufferToTexture, 0, 0, 0);

		const TShared<render::Texture> copyTextureToBuffer = fnCreateTexture("CopyTextureToBuffer");
		commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(copyTextureToBuffer, GpuAccessState(), host));
		commandBuffer->CopyTextureToBuffer(copyTextureToBuffer, readback, 0, 0);

		const TShared<render::Texture> copyTexture = fnCreateTexture("CopyTexture");
		commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(copyTexture, GpuAccessState(), host));
		commandBuffer->CopyTexture(sourceTexture, copyTexture);

		const TShared<render::Texture> blitTexture = fnCreateTexture("BlitTexture");
		commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(blitTexture, GpuAccessState(), host));
		commandBuffer->BlitTexture(sourceTexture, blitTexture);

		// Attachments must be declared for the attachment stages, in the attachment layout
		const TShared<render::Texture> renderPass = fnCreateTexture("RenderPass");
		commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(renderPass, GpuAccessState(), GpuAccessState(GpuStageFlag::FragmentShaderNonUniform, readWrite, GpuImageLayout::ColorAttachment)));
		commandBuffer->BeginRenderPass(RenderPassCreateInformation(fnCreateTarget(renderPass)));
		commandBuffer->EndRenderPass();

		const TShared<render::Texture> renderPassLayout = fnCreateTexture("RenderPassLayout");
		commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(renderPassLayout, GpuAccessState(), GpuAccessState(GpuStageFlag::ColorAttachment, readWrite, GpuImageLayout::TransferDestination)));
		commandBuffer->BeginRenderPass(RenderPassCreateInformation(fnCreateTarget(renderPassLayout)));
		commandBuffer->EndRenderPass();

		// Dispatches validate the resources of their bound parameter sets
		if(pipeline != nullptr)
		{
			const GpuAccessState computeReadWrite(GpuStageFlag::ComputeShaderNonUniform, readWrite, GpuImageLayout::General);

			const TShared<render::GpuBuffer> dispatchCompute = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
			dispatchCompute->SetName("DispatchCompute");
			commandBuffer->IssueExplicitBarriers(GpuExplicitBufferBarrier(dispatchCompute, GpuAccessState(), host));

			TextureCreateInformation storageInformation;
			storageInformation.Name = "DispatchComputeTexture";
			storageInformation.Width = kSize;
			storageInformation.Height = kSize;
			storageInformation.Format = PF_R32U;
			storageInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;
			const TShared<render::Texture> storageTexture = device->CreateTexture(storageInformation);
			commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(storageTexture, GpuAccessState(), computeReadWrite));

			const TShared<render::GpuParameterSet> parameters = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			parameters->SetStorageBuffer("OutputBuffer", dispatchCompute);
			parameters->SetStorageTexture("OutputTexture", storageTexture, TextureSurface());

			commandBuffer->SetGpuComputePipelineState(pipeline);
			commandBuffer->SetGpuParameterSet(parameters);
			commandBuffer->DispatchCompute(1, 1, 1);
		}

		// Correctly declared accesses report nothing
		const TShared<render::Texture> declared = fnCreateTexture("Declared");
		commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(declared, GpuAccessState(), transferWrite));
		commandBuffer->CopyTexture(sourceTexture, declared);
		commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(declared, transferWrite, colorAttachment));
		commandBuffer->BeginRenderPass(RenderPassCreateInformation(fnCreateTarget(declared)));
		commandBuffer->EndRenderPass();
		commandBuffer->IssueExplicitBarriers(GpuExplicitTextureBarrier(declared, colorAttachment, transferRead));
		commandBuffer->CopyTextureToBuffer(declared, readback, 0, 0);

		// The command buffer is never submitted
		commandBuffer->End();
		pool->Reset();
	}, "GpuBackendTestSuite::TestExplicitBarrierValidation", true);
}
#endif
#endif

#if B3D_BUILD_TYPE_DEVELOPMENT
void GpuBackendTestSuite::TestDrawAccessValidation()
{
	GpuDrawAccessValidator validator;
	int firstResource = 0;
	int secondResource = 0;
	const GpuAccessFlags accesses[] = { GpuAccessFlag::Read, GpuAccessFlag::Write, GpuAccessFlag::Read | GpuAccessFlag::Write };
	for(GpuAccessFlags firstAccess : accesses)
	{
		for(GpuAccessFlags secondAccess : accesses)
		{
			validator.BeginRenderPass();
			validator.ClearBindings();
			validator.AddResource(&firstResource, firstAccess);
			B3D_TEST_ASSERT(validator.ValidateDraw());
			validator.ClearBindings();
			validator.AddResource(&firstResource, secondAccess);
			B3D_TEST_ASSERT(validator.ValidateDraw() == !(firstAccess | secondAccess).IsSet(GpuAccessFlag::Write));
		}
	}

	validator.BeginRenderPass();
	validator.ClearBindings();
	validator.AddResource(&firstResource, GpuAccessFlag::Read);
	for(u32 drawIndex = 0; drawIndex < 100; drawIndex++)
		B3D_TEST_ASSERT(validator.ValidateDraw());

	// Binding without a draw does not contribute an access.
	validator.ClearBindings();
	validator.AddResource(&secondResource, GpuAccessFlag::Write);
	validator.ClearBindings();
	validator.AddResource(&secondResource, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(validator.ValidateDraw());

	validator.BeginRenderPass();
	validator.ClearBindings();
	validator.AddResource(&firstResource, GpuAccessFlag::Read);
	validator.AddResource(&firstResource, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(validator.ValidateDraw());
	B3D_TEST_ASSERT(!validator.ValidateDraw());

	// Rejected draws must not publish accesses to otherwise independent resources.
	validator.ClearBindings();
	validator.AddResource(&firstResource, GpuAccessFlag::Read);
	validator.AddResource(&secondResource, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(!validator.ValidateDraw());
	validator.ClearBindings();
	validator.AddResource(&secondResource, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(validator.ValidateDraw());
	validator.BeginRenderPass();
	B3D_TEST_ASSERT(validator.ValidateDraw());
}

#endif

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

	// A barrier that waits on every access of the write epoch orders later writes in its destination
	GpuResourceHazardState orderedWriteHazardState;
	orderedWriteHazardState.RecordAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write);
	orderedWriteHazardState.RecordAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	orderedWriteHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::Transfer | GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read | GpuAccessFlag::Write,
		GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write));
	B3D_TEST_ASSERT(orderedWriteHazardState.OrderedWriteStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(!orderedWriteHazardState.GetRequiredBarrier(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write).IsValid())

	// The write-epoch state alone does not know about the ordering
	B3D_TEST_ASSERT(orderedWriteHazardState.LastWriteEpochHazardState.GetRequiredBarrier(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write).IsValid())

	// A write outside the destination still waits on the write epoch
	const GpuBarrierScope unorderedWriteBarrier = orderedWriteHazardState.GetRequiredBarrier(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(unorderedWriteBarrier.SourceStages == (GpuStageFlag::Transfer | GpuStageFlag::FragmentShaderNonUniform))
	B3D_TEST_ASSERT(unorderedWriteBarrier.DestinationStages == GpuStageFlag::ColorAttachment)

	// A read in the same access still needs the earlier write made visible
	const GpuBarrierScope orderedReadWriteBarrier = orderedWriteHazardState.GetRequiredBarrier(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read | GpuAccessFlag::Write);
	B3D_TEST_ASSERT(orderedReadWriteBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(orderedReadWriteBarrier.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(orderedReadWriteBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(orderedReadWriteBarrier.DestinationAccess == GpuAccessFlag::Read)

	// A write that is only partially ordered waits on the write epoch in the remaining stages
	const GpuBarrierScope partiallyOrderedWriteBarrier = orderedWriteHazardState.GetRequiredBarrier(GpuStageFlag::ComputeShaderNonUniform | GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(partiallyOrderedWriteBarrier.SourceStages == (GpuStageFlag::Transfer | GpuStageFlag::FragmentShaderNonUniform))
	B3D_TEST_ASSERT(partiallyOrderedWriteBarrier.SourceAccess == (GpuAccessFlag::Read | GpuAccessFlag::Write))
	B3D_TEST_ASSERT(partiallyOrderedWriteBarrier.DestinationStages == GpuStageFlag::ColorAttachment)
	B3D_TEST_ASSERT(partiallyOrderedWriteBarrier.DestinationAccess == GpuAccessFlag::Write)

	// Its read also needs the earlier write made visible in the ordered stages
	const GpuBarrierScope partiallyOrderedReadWriteBarrier = orderedWriteHazardState.GetRequiredBarrier(GpuStageFlag::ComputeShaderNonUniform | GpuStageFlag::ColorAttachment,
		GpuAccessFlag::Read | GpuAccessFlag::Write);
	B3D_TEST_ASSERT(partiallyOrderedReadWriteBarrier.SourceStages == (GpuStageFlag::Transfer | GpuStageFlag::FragmentShaderNonUniform))
	B3D_TEST_ASSERT(partiallyOrderedReadWriteBarrier.DestinationStages == (GpuStageFlag::ComputeShaderNonUniform | GpuStageFlag::ColorAttachment))
	B3D_TEST_ASSERT(partiallyOrderedReadWriteBarrier.DestinationAccess == (GpuAccessFlag::Read | GpuAccessFlag::Write))

	// An access after the barrier must be ordered before a later write, even in the same stage
	orderedWriteHazardState.RecordAccess(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(orderedWriteHazardState.OrderedWriteStages == GpuStageFlag::None)
	B3D_TEST_ASSERT(orderedWriteHazardState.GetRequiredBarrier(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write).IsValid())

	// A barrier that misses an access of the write epoch, or does not make its writes available, orders no writes
	GpuResourceHazardState partialBarrierHazardState;
	partialBarrierHazardState.RecordAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write);
	partialBarrierHazardState.RecordAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	partialBarrierHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write));
	B3D_TEST_ASSERT(partialBarrierHazardState.OrderedWriteStages == GpuStageFlag::None)

	GpuResourceHazardState executionBarrierHazardState;
	executionBarrierHazardState.RecordAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write);
	executionBarrierHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::Transfer, GpuAccessFlag::None, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write));
	B3D_TEST_ASSERT(executionBarrierHazardState.OrderedWriteStages == GpuStageFlag::None)

	// A barrier towards reads orders no writes
	GpuResourceHazardState readBarrierHazardState;
	readBarrierHazardState.RecordAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write);
	readBarrierHazardState.RecordBarrier(GpuBarrierScope(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read));
	B3D_TEST_ASSERT(readBarrierHazardState.OrderedWriteStages == GpuStageFlag::None)
}

void GpuBackendTestSuite::TestExplicitBarrierWriteOrdering()
{
	const GpuStageFlags computeStages = GpuStageFlag::ComputeShaderNonUniform;

	// Applies the queued barriers like a backend barrier helper would, then ends the batch
	auto fnExecuteBarriers = [](SubmissionTestTracker& tracker, SubmissionTestBarrierHelper& barrierHelper, IGpuImageResource* image, IGpuBufferResource* buffer)
	{
		if(image != nullptr && barrierHelper.HasQueuedBarrier(image))
		{
			tracker.UpdateImageLayoutTrackingAfterBarrier(image, image->GetRange(), barrierHelper.LastImageBarrierOldLayout, barrierHelper.LastImageBarrierNewLayout);
			tracker.UpdateHazardStateAfterBarrier(image, image->GetRange(), barrierHelper.LastImageBarrier);
		}

		if(buffer != nullptr && barrierHelper.HasQueuedBarrier(buffer))
			tracker.UpdateHazardStateAfterBarrier(buffer, barrierHelper.LastBufferBarrier);

		tracker.CommitPendingAccesses();
		barrierHelper.Clear();
	};

	// A write in the destination of an explicit barrier needs no barrier of its own
	{
		SubmissionTestBuffer buffer;
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		tracker.TrackBufferAccess(&buffer, computeStages, GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 0)

		tracker.TrackBufferBarrier(&buffer, GpuStageFlag::Transfer, GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 1)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.SourceStages == computeStages)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.DestinationStages == GpuStageFlag::Transfer)

		tracker.TrackBufferAccess(&buffer, GpuStageFlag::Transfer, GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 1)

		// The write ends the ordering, so the next write waits on it
		tracker.TrackBufferAccess(&buffer, GpuStageFlag::Transfer, GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 2)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.SourceStages == GpuStageFlag::Transfer)

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// A read between the explicit barrier and the write must be ordered before the write
	{
		SubmissionTestBuffer buffer;
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		tracker.TrackBufferAccess(&buffer, computeStages, GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);
		tracker.TrackBufferBarrier(&buffer, GpuStageFlag::Transfer, GpuAccessFlag::Read | GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 1)

		tracker.TrackBufferAccess(&buffer, GpuStageFlag::Transfer, GpuAccessFlag::Read, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 1)

		tracker.TrackBufferAccess(&buffer, GpuStageFlag::Transfer, GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, nullptr, &buffer);
		B3D_TEST_ASSERT(barrierHelper.BufferBarrierCount == 2)
		B3D_TEST_ASSERT(barrierHelper.LastBufferBarrier.SourceStages == (computeStages | GpuStageFlag::Transfer))

		tracker.NotifyUnbound();
		tracker.Clear();
	}

	// A layout transition after an explicit barrier is not ordered by it, so it waits on the write epoch
	{
		SubmissionTestImage image(1, 1, GpuTextureAspectFlag::Color);
		SubmissionTestTracker tracker;
		SubmissionTestBarrierHelper barrierHelper;

		tracker.TrackImageAccess(&image, image.GetRange(), GpuImageLayout::General, computeStages, GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, &image, nullptr);
		tracker.TrackImageBarrier(&image, image.GetRange(), GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::General, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, &image, nullptr);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 1)

		// Same layout: ordered by the explicit barrier
		tracker.TrackImageAccess(&image, image.GetRange(), GpuImageLayout::General, GpuStageFlag::Transfer, GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, &image, nullptr);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 1)

		tracker.TrackImageBarrier(&image, image.GetRange(), computeStages, GpuAccessFlag::Write, GpuImageLayout::General, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, &image, nullptr);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 2)

		// Another layout: the transition waits on the last write
		tracker.TrackImageAccess(&image, image.GetRange(), GpuImageLayout::TransferDestination, computeStages, GpuAccessFlag::Write, barrierHelper);
		fnExecuteBarriers(tracker, barrierHelper, &image, nullptr);
		B3D_TEST_ASSERT(barrierHelper.ImageBarrierCount == 3)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrier.SourceStages == GpuStageFlag::Transfer)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierOldLayout == GpuImageLayout::General)
		B3D_TEST_ASSERT(barrierHelper.LastImageBarrierNewLayout == GpuImageLayout::TransferDestination)

		tracker.NotifyUnbound();
		tracker.Clear();
	}
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
		GpuSubmissionTransition::Build(buffer.GetSubmissionState(), kTestFrameIndex, GpuQueueMask::kNone, sourceQueueId, fragmentReadHazardState);
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
		GpuSubmissionTransition::Build(buffer.GetSubmissionState(), kTestFrameIndex, GpuQueueMask::kNone, sourceQueueId, computeReadHazardState);
	B3D_TEST_ASSERT(computeReadTransition.MemoryBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(computeReadTransition.MemoryBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)

	fnSetWriterState(sourceWriteEpochHazardState);
	GpuResourceHazardState readThenWriteHazardState;
	ResolveTestAccess(readThenWriteHazardState, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	ResolveTestAccess(readThenWriteHazardState, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Write);

	const GpuSubmissionTransition readThenWriteTransition =
		GpuSubmissionTransition::Build(buffer.GetSubmissionState(), kTestFrameIndex, GpuQueueMask::kNone, sourceQueueId, readThenWriteHazardState);
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
		GpuSubmissionTransition::Build(buffer.GetSubmissionState(), kTestFrameIndex, GpuQueueMask::kNone, sourceQueueId, chainedWriteHazardState);
	B3D_TEST_ASSERT(chainedWriteTransition.ExecutionBarrier.SourceStages == GpuStageFlag::VertexShaderNonUniform)
	B3D_TEST_ASSERT(chainedWriteTransition.ExecutionBarrier.SourceAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(chainedWriteTransition.ExecutionBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(chainedWriteTransition.ExecutionBarrier.DestinationAccess == GpuAccessFlag::Read)

	fnSetWriterState(sourceWithReader);
	GpuResourceHazardState writeHazardState;
	ResolveTestAccess(writeHazardState, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
	const GpuSubmissionTransition writeTransition =
		GpuSubmissionTransition::Build(buffer.GetSubmissionState(), kTestFrameIndex, GpuQueueMask::kNone, sourceQueueId, writeHazardState);
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
		GpuSubmissionTransition::Build(buffer.GetSubmissionState(), kTestFrameIndex, GpuQueueMask::kNone, sourceQueueId, chainedReadHazardState);
	B3D_TEST_ASSERT(chainedReadTransition.MemoryBarrier.DestinationStages == GpuStageFlag::VertexShaderNonUniform)
	B3D_TEST_ASSERT(chainedReadTransition.PostTransitionSubmissionState.WriterHazards.ReaderStages ==
		(GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform))

	fnSetWriterState(sourceWriteEpochHazardState);
	GpuResourceHazardState exactLeadingBarrierHazardState;
	exactLeadingBarrierHazardState.HasLeadingBarrier = true;
	ResolveTestAccess(exactLeadingBarrierHazardState, GpuStageFlag::FragmentShaderNonUniform,
		GpuAccessFlag::Read);
	const GpuSubmissionTransition exactLeadingBarrierTransition =
		GpuSubmissionTransition::Build(buffer.GetSubmissionState(), kTestFrameIndex, GpuQueueMask::kNone, sourceQueueId, exactLeadingBarrierHazardState);
	B3D_TEST_ASSERT(exactLeadingBarrierTransition.MemoryBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(exactLeadingBarrierTransition.MemoryBarrier.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)

	fnSetWriterState(sourceWriteEpochHazardState);
	GpuResourceHazardState leadingBarrierOnlyHazardState;
	leadingBarrierOnlyHazardState.HasLeadingBarrier = true;
	const GpuSubmissionTransition leadingBarrierOnlyTransition =
		GpuSubmissionTransition::Build(buffer.GetSubmissionState(), kTestFrameIndex, GpuQueueMask::kNone, sourceQueueId, leadingBarrierOnlyHazardState);
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
