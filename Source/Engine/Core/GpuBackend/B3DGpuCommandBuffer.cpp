//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuBarrierValidator.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

#include "Image/B3DTexture.h"
#include "Image/B3DPixelUtility.h"
#include "Profiling/B3DProfilerGPU.h"

using namespace b3d;

namespace b3d
{
	TConfigVariable<bool> gGpuValidateRestingStates("gpu.ValidateRestingStates",
		"If enabled, development builds check the accesses of resources that a command buffer neither declared nor barriered against the resources' resting state.", false);

	TConfigVariable<bool> gGpuPoisonUnloadedAttachments("gpu.PoisonUnloadedAttachments",
		"If enabled, development builds clear every render target surface that a render pass neither loads nor clears, to a value that stands out, so passes that rely on contents they did not load show them.", B3D_DEBUG != 0);
} // namespace b3d

namespace b3d { namespace render
{
#if B3D_BUILD_TYPE_DEVELOPMENT
void GpuDrawAccessValidator::BeginRenderPass()
{
	mPreviousDrawAccesses.clear();
	mBindingsUsed = false;
}

void GpuDrawAccessValidator::ClearBindings()
{
	mBindings.clear();
	mBindingsUsed = false;
	mBindingsHaveWrites = false;
}

void GpuDrawAccessValidator::AddResource(const void* resource, GpuAccessFlags access)
{
	if(resource == nullptr)
		return;

	mBindings[resource] |= access;
	mBindingsHaveWrites |= access.IsSet(GpuAccessFlag::Write);
	mBindingsUsed = false;
}

void GpuDrawAccessValidator::AddParameterSet(const GpuParameterSet& parameters, const GpuPipelineParameterSetLayout& pipelineSetLayout)
{
	parameters.ForEachBoundResource(&pipelineSetLayout, [this](const GpuParameterSetResource& resource)
	{
		AddResource(resource.Buffer != nullptr ? (const void*)resource.Buffer : (const void*)resource.Texture, resource.Access);
	});
}

bool GpuDrawAccessValidator::ValidateDraw()
{
	if(mBindingsUsed)
		return !mBindingsHaveWrites;

	for(const auto& [resource, access] : mBindings)
	{
		const auto previousAccess = mPreviousDrawAccesses.find(resource);
		if(previousAccess != mPreviousDrawAccesses.end() && (previousAccess->second | access).IsSet(GpuAccessFlag::Write))
			return false;
	}

	for(const auto& [resource, access] : mBindings)
		mPreviousDrawAccesses[resource] |= access;

	mBindingsUsed = true;
	return true;
}

#endif

GpuCommandBufferPool::GpuCommandBufferPool(GpuDevice& gpuDevice, const GpuCommandBufferPoolCreateInformation& createInformation)
	:mGpuDevice(gpuDevice), mInformation(createInformation)
{
	// Process messages related to this command buffer pool on this thread. Mostly these are command buffer resets once they are done executing.
	Scheduler* const scheduler = Scheduler::Get();
	if (B3D_ENSURE(scheduler))
	{
		mMessageQueue.ScheduleRunUntilShutdown(*scheduler, true);
	}
}

void GpuCommandBufferPool::Destroy()
{
	if (mIsDestroyed)
		return;

	mIsDestroyed = true;
}

GpuCommandBuffer::GpuCommandBuffer(GpuDevice& gpuDevice, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation)
	:mGpuDevice(gpuDevice), mQueueType(queueType), mOwnerThread(ownerThread), mInformation(createInformation)
{ }


GpuCommandBuffer::~GpuCommandBuffer()
{
	OnDestroyed(mState == GpuCommandBufferState::Executing);

	ClearBarrierState(mState == GpuCommandBufferState::Executing || mState == GpuCommandBufferState::Done);
}

#if B3D_BUILD_TYPE_DEVELOPMENT
bool GpuCommandBuffer::ValidateDrawAccesses()
{
	if(mDrawAccessValidator.ValidateDraw())
		return true;

	B3D_LOG(Error, LogRenderBackend, "Draw rejected: shader resources written by a draw cannot be accessed by another draw in the same render pass.");
	return false;
}

#endif

void GpuCommandBuffer::SetGpuParameterSet([[maybe_unused]] const TShared<GpuParameterSet>& parameters)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
		mBarrierValidator->SetParameterSet(parameters);
#endif
}

void GpuCommandBuffer::SetGpuGraphicsPipelineState([[maybe_unused]] const TShared<GpuGraphicsPipelineState>& pipelineState)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
		mBarrierValidator->SetGraphicsPipeline(pipelineState);
#endif
}

void GpuCommandBuffer::SetGpuComputePipelineState([[maybe_unused]] const TShared<GpuComputePipelineState>& pipelineState)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
		mBarrierValidator->SetComputePipeline(pipelineState);
#endif
}

void GpuCommandBuffer::SetVertexBuffers([[maybe_unused]] u32 index, [[maybe_unused]] TShared<GpuBuffer>* buffers, [[maybe_unused]] u32 bufferCount)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
		mBarrierValidator->SetVertexBuffers(index, buffers, bufferCount);
#endif
}

void GpuCommandBuffer::SetIndexBuffer([[maybe_unused]] const TShared<GpuBuffer>& buffer)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
		mBarrierValidator->SetIndexBuffer(buffer);
#endif
}

void GpuCommandBuffer::Draw(u32 /*vertexOffset*/, u32 /*vertexCount*/, u32 /*instanceCount*/, u32 /*firstInstance*/)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
		mBarrierValidator->ValidateDraw(false);
#endif
}

void GpuCommandBuffer::DrawIndexed(u32 /*startIndex*/, u32 /*indexCount*/, u32 /*vertexOffset*/, u32 /*vertexCount*/, u32 /*instanceCount*/, u32 /*firstInstance*/)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
		mBarrierValidator->ValidateDraw(true);
#endif
}

void GpuCommandBuffer::DispatchCompute(u32 /*groupCountX*/, u32 /*groupCountY*/, u32 /*groupCountZ*/)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
		mBarrierValidator->ValidateDispatch();
#endif
}

void GpuCommandBuffer::BeginRenderPass(const RenderPassCreateInformation& createInformation)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
		mBarrierValidator->ValidateRenderPass(createInformation);
#endif

	// The render pass may change the contents of the target unless every surface is read-only
	if(createInformation.Target != nullptr && createInformation.ReadOnlyMask != RT_ALL)
		createInformation.Target->TickUpdateCount();
}

void GpuCommandBuffer::CopyBufferToBuffer([[maybe_unused]] const TShared<GpuBuffer>& source, [[maybe_unused]] const TShared<GpuBuffer>& destination, u32 /*sourceOffset*/, u32 /*destinationOffset*/, u32 /*length*/)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr && source != nullptr && destination != nullptr)
	{
		mBarrierValidator->ValidateBufferAccess(*source, GpuStageFlag::Transfer, GpuAccessFlag::Read);
		mBarrierValidator->ValidateBufferAccess(*destination, GpuStageFlag::Transfer, GpuAccessFlag::Write);
	}
#endif
}

void GpuCommandBuffer::CopyBufferToTexture([[maybe_unused]] const TShared<GpuBuffer>& source, [[maybe_unused]] const TShared<Texture>& destination, u32 /*bufferOffset*/, [[maybe_unused]] u32 mipLevel, [[maybe_unused]] u32 arrayLayer)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr && source != nullptr && destination != nullptr)
	{
		mBarrierValidator->ValidateBufferAccess(*source, GpuStageFlag::Transfer, GpuAccessFlag::Read);
		mBarrierValidator->ValidateTextureAccess(*destination, GpuTextureSubresourceRange(mipLevel, 1, arrayLayer, 1, kAllImageAspects), GpuImageLayout::Undefined, GpuStageFlag::Transfer, GpuAccessFlag::Write);
	}
#endif
}

void GpuCommandBuffer::CopyTextureToBuffer([[maybe_unused]] const TShared<Texture>& source, [[maybe_unused]] const TShared<GpuBuffer>& destination, [[maybe_unused]] u32 mipLevel, [[maybe_unused]] u32 arrayLayer, u32 /*bufferOffset*/)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr && source != nullptr && destination != nullptr)
	{
		mBarrierValidator->ValidateTextureAccess(*source, GpuTextureSubresourceRange(mipLevel, 1, arrayLayer, 1, kAllImageAspects), GpuImageLayout::Undefined, GpuStageFlag::Transfer, GpuAccessFlag::Read);
		mBarrierValidator->ValidateBufferAccess(*destination, GpuStageFlag::Transfer, GpuAccessFlag::Write);
	}
#endif
}

void GpuCommandBuffer::CopyWindowToBuffer([[maybe_unused]] const TShared<RenderWindow>& source, [[maybe_unused]] const TShared<GpuBuffer>& destination)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr && source != nullptr && destination != nullptr)
	{
		mBarrierValidator->ValidateRenderTargetAccess(*source, RT_COLOR0, GpuImageLayout::TransferSource, GpuStageFlag::Transfer, GpuAccessFlag::Read);
		mBarrierValidator->ValidateBufferAccess(*destination, GpuStageFlag::Transfer, GpuAccessFlag::Write);
	}
#endif
}

void GpuCommandBuffer::SetPushConstants(u32 /*offsetInBytes*/, u32 sizeInBytes, const void* /*data*/)
{
	EnsureValidThread();
	if(sizeInBytes == 0)
		return;

	B3D_LOG(Error, LogRenderBackend, "Push constants are unsupported by this GPU backend.");
}

void GpuCommandBuffer::NotifyWillQueueForSubmit([[maybe_unused]] GpuQueueId queueId, [[maybe_unused]] GpuQueueMask syncMask)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	// The submit thread executes submissions in the order they are queued, so this sees the releases in GPU submission order
	using ReleaseSubmission = GpuSplitBarrier::ReleaseSubmission;

	for(const RecordedSplitBarrier& recorded : mSplitBarriers)
	{
		GpuSplitBarrier& splitBarrier = *recorded.SplitBarrier;
		if(recorded.Phase == GpuBarrierPhase::Release)
		{
			splitBarrier.mReleaseQueue = queueId.Id;
			splitBarrier.mReleaseSubmission = ReleaseSubmission::Submitted;
			continue;
		}

		if(recorded.IsReleasedByRecording)
			continue;

		// Without the release ahead of it, the acquire waits for a signal that never arrives
		if(!B3D_ENSURE_LOG(splitBarrier.mIsReleased, "The acquire of a split barrier was submitted, but its release was never recorded."))
			continue;

		const ReleaseSubmission releaseSubmission = splitBarrier.mReleaseSubmission;
		if(!B3D_ENSURE_LOG(releaseSubmission != ReleaseSubmission::Pending, "The acquire of a split barrier was submitted before its release."))
			continue;

		if(!B3D_ENSURE_LOG(releaseSubmission != ReleaseSubmission::Discarded, "The acquire of a split barrier was submitted, but its release was discarded without being submitted."))
			continue;

		// Halves on the same queue type may be connected by a native object that only works within one queue
		const GpuQueueId releaseQueueId(splitBarrier.mReleaseQueue.load());
		if(!splitBarrier.IsQueueTransfer())
			B3D_ENSURE_LOG(releaseQueueId.Id == queueId.Id, "The acquire of a split barrier was submitted on a different queue than its release. Create the split barrier with the queue type of each half to transfer resources between queues.");
		else
			B3D_ENSURE_LOG(syncMask.IsSet(releaseQueueId), "The acquire of a split barrier was submitted without waiting for the queue of its release.");
	}
#endif
}

#if B3D_BUILD_TYPE_DEVELOPMENT
namespace
{
	/**
	 * Checks that both halves of a split barrier are given the same barriers. The half recorded first stores the hash of its barriers
	 * in @p recordedHash, and the other half compares against it. The acquire derives its synchronization from the barriers alone,
	 * so it misses the work of the release if they differ.
	 */
	void ValidateSplitBarrierHalf(std::atomic<u64>& recordedHash, const GpuBarriers& barriers)
	{
		const u64 hash = barriers.GenerateHash();

		// 0 marks a split barrier with neither half recorded
		const u64 barriersHash = hash != 0 ? hash : 1;

		u64 otherHalfHash = 0;
		if(recordedHash.compare_exchange_strong(otherHalfHash, barriersHash))
			return;

		B3D_ENSURE_LOG(otherHalfHash == barriersHash, "The release and the acquire of a split barrier must be given the same barriers, in the same order.");
	}
}
#endif

void GpuCommandBuffer::IssueBarriers(const GpuBarriers& barriers)
{
	EnsureValidThread();

	if(!B3D_ENSURE_LOG(!IsInRenderPass(), "Explicit barriers can only be recorded outside of a render pass."))
		return;

	if(!RecordBarriers(barriers, GpuBarrierPhase::Full, nullptr))
		return;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
		mBarrierValidator->ValidateBarriers(barriers, GpuBarrierPhase::Full, nullptr);
#endif
}

void GpuCommandBuffer::ReleaseBarriers(const GpuBarriers& barriers, const TShared<GpuSplitBarrier>& split)
{
	EnsureValidThread();

	if(!B3D_ENSURE_LOG(split != nullptr, "A split barrier must be created by GpuDevice::CreateSplitBarrier()."))
		return;

	if(!B3D_ENSURE_LOG(!IsInRenderPass(), "Explicit barriers can only be recorded outside of a render pass."))
		return;

	if(!split->ValidateQueueType(true, mQueueType))
		return;

#if B3D_BUILD_TYPE_DEVELOPMENT
	// An acquire recorded earlier would wait for a release that follows it, so the GPU would never get past it
	for(const RecordedSplitBarrier& recorded : mSplitBarriers)
	{
		if(!B3D_ENSURE_LOG(recorded.SplitBarrier != split || recorded.Phase != GpuBarrierPhase::Acquire, "The release of a split barrier must be recorded before its acquire."))
			return;
	}
#endif

	if(!B3D_ENSURE_LOG(!split->mIsReleased.exchange(true), "The split barrier was already released."))
		return;

	if(!RecordBarriers(barriers, GpuBarrierPhase::Release, split.get()))
	{
		split->mIsReleased = false;
		return;
	}

	RecordedSplitBarrier recorded;
	recorded.SplitBarrier = split;
#if B3D_BUILD_TYPE_DEVELOPMENT
	recorded.Phase = GpuBarrierPhase::Release;
	ValidateSplitBarrierHalf(split->mBarrierHash, barriers);
	if(mBarrierValidator != nullptr)
		mBarrierValidator->ValidateBarriers(barriers, GpuBarrierPhase::Release, split.get());
#endif
	mSplitBarriers.Add(std::move(recorded));
}

void GpuCommandBuffer::AcquireBarriers(const GpuBarriers& barriers, const TShared<GpuSplitBarrier>& split)
{
	EnsureValidThread();

	if(!B3D_ENSURE_LOG(split != nullptr, "A split barrier must be created by GpuDevice::CreateSplitBarrier()."))
		return;

	if(!B3D_ENSURE_LOG(!IsInRenderPass(), "Explicit barriers can only be recorded outside of a render pass."))
		return;

	if(!split->ValidateQueueType(false, mQueueType))
		return;

	if(!B3D_ENSURE_LOG(!split->mIsAcquired.exchange(true), "The split barrier was already acquired."))
		return;

	if(!RecordBarriers(barriers, GpuBarrierPhase::Acquire, split.get()))
	{
		split->mIsAcquired = false;
		return;
	}

	RecordedSplitBarrier recorded;
	recorded.SplitBarrier = split;
#if B3D_BUILD_TYPE_DEVELOPMENT
	recorded.Phase = GpuBarrierPhase::Acquire;
	recorded.IsReleasedByRecording = std::any_of(mSplitBarriers.begin(), mSplitBarriers.end(), [&split](const RecordedSplitBarrier& entry) { return entry.SplitBarrier == split; });
	ValidateSplitBarrierHalf(split->mBarrierHash, barriers);
	if(mBarrierValidator != nullptr)
		mBarrierValidator->ValidateBarriers(barriers, GpuBarrierPhase::Acquire, split.get());
#endif
	mSplitBarriers.Add(std::move(recorded));
}

#if B3D_BUILD_TYPE_DEVELOPMENT
void GpuCommandBuffer::SetValidator(GpuBarrierValidator* validator)
{
	EnsureValidThread();

	mBarrierValidator = validator;
	if(mBarrierValidator != nullptr)
		mBarrierValidator->BeginCommandBuffer(mQueueType);
}
#endif

bool GpuCommandBuffer::RecordBarriers(const GpuBarriers& /*barriers*/, GpuBarrierPhase /*phase*/, GpuSplitBarrier* /*split*/)
{
	B3D_LOG(Error, LogRenderBackend, "Explicit barriers are unsupported by this GPU backend.");
	return false;
}

void GpuCommandBuffer::ClearBarrierState([[maybe_unused]] bool wasSubmitted)
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	using ReleaseSubmission = GpuSplitBarrier::ReleaseSubmission;

	for(const RecordedSplitBarrier& recorded : mSplitBarriers)
	{
		if(recorded.Phase == GpuBarrierPhase::Release && !wasSubmitted)
			recorded.SplitBarrier->mReleaseSubmission = ReleaseSubmission::Discarded;
	}

	mBarrierValidator = nullptr;
#endif

	mSplitBarriers.Clear();
	mImageLayouts.Clear();
}

void GpuCommandBuffer::AddImageLayout(IGpuImageResource& image, const GpuTextureSubresourceRange& range, u32 nativeLayout)
{
	ImageLayout imageLayout;
	imageLayout.Image = &image;
	imageLayout.Range = range;
	imageLayout.Layout = nativeLayout;
	mImageLayouts.Add(imageLayout);
}

void GpuCommandBuffer::PublishImageLayouts()
{
	AssertIfNotSubmitThread();

	for(const ImageLayout& imageLayout : mImageLayouts)
		imageLayout.Image->SetNativeLayout(imageLayout.Range, imageLayout.Layout);

	mImageLayouts.Clear();
}

#if B3D_PROFILING_ENABLED
TShared<GpuCommandBufferProfiler> GpuCommandBuffer::BeginProfiling(const ProfilerString& profilingScopeName)
{
	if(!B3D_ENSURE(mProfiler == nullptr))
		return nullptr;

	if(!mGpuDevice.GetCapabilities().HasCapability(RSC_TIMER_QUERIES))
		return nullptr;

	mProfiler = GetGpuProfiler().CreateCommandBufferProfiler(*this);
	mProfilingScopeName = profilingScopeName;

	return mProfiler;
}

void GpuCommandBuffer::EndProfiling()
{
	if(mProfiler == nullptr && !mGpuDevice.GetCapabilities().HasCapability(RSC_TIMER_QUERIES))
		return;

	if(!B3D_ENSURE(mProfiler != nullptr))
		return;

	GetGpuProfiler().ResolveProfileWhenReady(mProfilingScopeName, mProfiler);

	mProfiler = nullptr;
	mProfilingScopeName.clear();
}
#endif

bool GpuCommandBuffer::CopyTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureCopyInformation& copyInformation)
{
	EnsureValidThread();

	if(source == nullptr || destination == nullptr)
	{
		B3D_LOG(Error, LogTexture, "Copy operation failed. Source or destination texture is null.");
		return false;
	}

	const TextureProperties& sourceProperties = source->GetProperties();
	const TextureProperties& destinationProperties = destination->GetProperties();

	if(copyInformation.FaceCount == 0)
	{
		B3D_LOG(Warning, LogTexture, "Copy operation failed. Face count is zero.");
		return false;
	}

	if(destinationProperties.Type != sourceProperties.Type)
	{
		B3D_LOG(Error, LogTexture, "Source and destination textures must be of same type.");
		return false;
	}

	if(sourceProperties.Format != destinationProperties.Format)
	{
		B3D_LOG(Error, LogTexture, "Source and destination texture formats must match.");
		return false;
	}

	if(destinationProperties.SampleCount > 1 && sourceProperties.SampleCount != destinationProperties.SampleCount)
	{
		B3D_LOG(Error, LogTexture, "When copying to a multisampled texture, source texture must have the same number of samples.");
		return false;
	}

	if((copyInformation.SourceFace + copyInformation.FaceCount) > sourceProperties.GetFaceCount())
	{
		B3D_LOG(Error, LogTexture, "Invalid source face index.");
		return false;
	}

	if((copyInformation.DestinationFace + copyInformation.FaceCount) > destinationProperties.GetFaceCount())
	{
		B3D_LOG(Error, LogTexture, "Invalid destination face index.");
		return false;
	}

	if(copyInformation.SourceMip > sourceProperties.MipMapCount)
	{
		B3D_LOG(Error, LogTexture, "Source mip level out of range. Valid range is [0, {0}].", sourceProperties.MipMapCount);
		return false;
	}

	if(copyInformation.DestinationMip > destinationProperties.MipMapCount)
	{
		B3D_LOG(Error, LogTexture, "Destination mip level out of range. Valid range is [0, {0}].", destinationProperties.MipMapCount);
		return false;
	}

	u32 sourceWidth, sourceHeight, sourceDepth;
	PixelUtility::GetSizeForMipLevel(sourceProperties.Width, sourceProperties.Height, sourceProperties.Depth, copyInformation.SourceMip, sourceWidth, sourceHeight, sourceDepth);

	u32 destinationWidth, destinationHeight, destinationDepth;
	PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth, copyInformation.DestinationMip, destinationWidth, destinationHeight, destinationDepth);

	if(copyInformation.DestinationPosition.X < 0 || copyInformation.DestinationPosition.X >= (i32)destinationWidth ||
	   copyInformation.DestinationPosition.Y < 0 || copyInformation.DestinationPosition.Y >= (i32)destinationHeight ||
	   copyInformation.DestinationPosition.Z < 0 || copyInformation.DestinationPosition.Z >= (i32)destinationDepth)
	{
		B3D_LOG(Error, LogTexture, "Destination position falls outside the destination texture.");
		return false;
	}

	bool copyEntireSurface = copyInformation.SourceVolume.GetWidth() == 0 ||
		copyInformation.SourceVolume.GetHeight() == 0 ||
		copyInformation.SourceVolume.GetDepth() == 0;

	u32 destinationRight = (u32)copyInformation.DestinationPosition.X;
	u32 destinationBottom = (u32)copyInformation.DestinationPosition.Y;
	u32 destinationBack = (u32)copyInformation.DestinationPosition.Z;
	if(!copyEntireSurface)
	{
		if(copyInformation.SourceVolume.Left >= sourceWidth || copyInformation.SourceVolume.Right > sourceWidth ||
		   copyInformation.SourceVolume.Top >= sourceHeight || copyInformation.SourceVolume.Bottom > sourceHeight ||
		   copyInformation.SourceVolume.Front >= sourceDepth || copyInformation.SourceVolume.Back > sourceDepth)
		{
			B3D_LOG(Error, LogTexture, "Source volume falls outside the source texture.");
			return false;
		}

		destinationRight += copyInformation.SourceVolume.GetWidth();
		destinationBottom += copyInformation.SourceVolume.GetHeight();
		destinationBack += copyInformation.SourceVolume.GetDepth();
	}
	else
	{
		destinationRight += sourceWidth;
		destinationBottom += sourceHeight;
		destinationBack += sourceDepth;
	}

	if(destinationRight > destinationWidth || destinationBottom > destinationHeight || destinationBack > destinationDepth)
	{
		B3D_LOG(Error, LogTexture, "Destination volume falls outside the destination texture.");
		return false;
	}

	const bool sourceHasMultipleSamples = sourceProperties.SampleCount > 1;
	const bool destinationHasMultipleSamples = destinationProperties.SampleCount > 1;

	if(sourceProperties.Usage.IsSet(TextureUsageFlag::DepthStencil) || destinationProperties.Usage.IsSet(TextureUsageFlag::DepthStencil))
	{
		B3D_LOG(Error, LogRenderBackend, "Texture copy/resolve isn't supported for depth-stencil textures.");
		return false;
	}

	bool needsResolve = sourceHasMultipleSamples && !destinationHasMultipleSamples;
	bool isMSCopy = sourceHasMultipleSamples || destinationHasMultipleSamples;
	if(!needsResolve && isMSCopy)
	{
		if(sourceProperties.SampleCount != destinationProperties.SampleCount)
		{
			B3D_LOG(Error, LogRenderBackend, "When copying textures their multisample counts must match. Ignoring copy.");
			return false;
		}
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
	{
		const GpuStageFlags stages = needsResolve ? GpuStageFlag::Resolve : GpuStageFlag::Transfer;
		const GpuTextureSubresourceRange sourceRange(copyInformation.SourceMip, 1, copyInformation.SourceFace, copyInformation.FaceCount, kAllImageAspects);
		const GpuTextureSubresourceRange destinationRange(copyInformation.DestinationMip, 1, copyInformation.DestinationFace, copyInformation.FaceCount, kAllImageAspects);
		mBarrierValidator->ValidateTextureAccess(*source, sourceRange, GpuImageLayout::Undefined, stages, GpuAccessFlag::Read);
		mBarrierValidator->ValidateTextureAccess(*destination, destinationRange, GpuImageLayout::Undefined, stages, GpuAccessFlag::Write);
	}
#endif

	return true;
}

bool GpuCommandBuffer::BlitTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureBlitInformation& blitInformation)
{
	EnsureValidThread();

	if(source == nullptr || destination == nullptr)
	{
		B3D_LOG(Error, LogTexture, "Blit operation failed. Source or destination texture is null.");
		return false;
	}

	const TextureProperties& sourceProperties = source->GetProperties();
	const TextureProperties& destinationProperties = destination->GetProperties();

	if(blitInformation.FaceCount == 0)
	{
		B3D_LOG(Warning, LogTexture, "Blit operation failed. Face count is zero.");
		return false;
	}

	if((blitInformation.SourceFace + blitInformation.FaceCount) > sourceProperties.GetFaceCount())
	{
		B3D_LOG(Error, LogTexture, "Blit operation failed. Source face out of valid range.");
		return false;
	}

	if((blitInformation.DestinationFace + blitInformation.FaceCount) > destinationProperties.GetFaceCount())
	{
		B3D_LOG(Error, LogTexture, "Blit operation failed. Destination face out of valid range.");
		return false;
	}

	if(blitInformation.SourceMip > sourceProperties.MipMapCount)
	{
		B3D_LOG(Error, LogTexture, "Blit operation failed. Source mip level out of valid range. Valid range is [0, {0}].", sourceProperties.MipMapCount);
		return false;
	}

	if(blitInformation.DestinationMip > destinationProperties.MipMapCount)
	{
		B3D_LOG(Error, LogTexture, "Blit operation failed. Destination mip level out of range. Valid range is [0, {0}].", destinationProperties.MipMapCount);
		return false;
	}

	if(sourceProperties.Usage.IsSet(TextureUsageFlag::DepthStencil) || destinationProperties.Usage.IsSet(TextureUsageFlag::DepthStencil))
	{
		B3D_LOG(Error, LogRenderBackend, "Texture blit isn't supported for depth-stencil textures.");
		return false;
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBarrierValidator != nullptr)
	{
		const GpuTextureSubresourceRange sourceRange(blitInformation.SourceMip, 1, blitInformation.SourceFace, blitInformation.FaceCount, kAllImageAspects);
		const GpuTextureSubresourceRange destinationRange(blitInformation.DestinationMip, 1, blitInformation.DestinationFace, blitInformation.FaceCount, kAllImageAspects);
		mBarrierValidator->ValidateTextureAccess(*source, sourceRange, GpuImageLayout::Undefined, GpuStageFlag::Transfer, GpuAccessFlag::Read);
		mBarrierValidator->ValidateTextureAccess(*destination, destinationRange, GpuImageLayout::Undefined, GpuStageFlag::Transfer, GpuAccessFlag::Write);
	}
#endif

	return true;
}
}}
