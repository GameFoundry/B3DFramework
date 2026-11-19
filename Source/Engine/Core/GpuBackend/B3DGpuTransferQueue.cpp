//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuTransferQueue.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Image/B3DTexture.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Staging bytes after which CreateStagingBuffer() ends the epoch, so processes that never end a frame still reuse staging memory. */
	constexpr u64 kEpochStagingBudget = 64ull * 1024 * 1024;

	/** Records a copy from @p source into @p destination, taking the destination from its resting state and returning it there. */
	void RecordBufferCopy(GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& source, const TShared<render::GpuBuffer>& destination, u32 destinationOffset, u32 length)
	{
		const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*destination, commandBuffer.GetQueueType());
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);

		commandBuffer.IssueBarriers(GpuBufferBarrier(destination, restingState, transferWrite));
		commandBuffer.CopyBufferToBuffer(source, destination, 0, destinationOffset, length);
		commandBuffer.IssueBarriers(GpuBufferBarrier(destination, transferWrite, restingState));
	}

	/**
	 * Records a copy from @p source into one subresource of @p destination, taking the subresource from its resting state and returning it
	 * there. The copy overwrites the whole subresource, so its old contents are discarded.
	 */
	void RecordTextureCopy(GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& source, const TShared<render::Texture>& destination, u32 mipLevel, u32 arrayLayer)
	{
		const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*destination, commandBuffer.GetQueueType());
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuTextureSubresourceRange range(mipLevel, 1, arrayLayer, 1, GpuBackendUtility::GetFormatAspects(destination->GetProperties().Format));

		GpuAccessState discardedRestingState = restingState;
		discardedRestingState.Layout = GpuImageLayout::Undefined;

		commandBuffer.IssueBarriers(GpuTextureBarrier(destination, discardedRestingState, transferWrite, range));
		commandBuffer.CopyBufferToTexture(source, destination, 0, mipLevel, arrayLayer);
		commandBuffer.IssueBarriers(GpuTextureBarrier(destination, transferWrite, restingState, range));
	}
} // namespace

GpuStagingBuffer::GpuStagingBuffer(GpuTransferQueue& queue, TShared<render::GpuBuffer> buffer, u64 epoch)
	: mQueue(&queue), mBuffer(std::move(buffer)), mEpoch(epoch)
{ }

GpuStagingBuffer::GpuStagingBuffer(GpuStagingBuffer&& other) noexcept
	: mQueue(std::exchange(other.mQueue, nullptr)), mBuffer(std::move(other.mBuffer)), mEpoch(other.mEpoch)
{ }

GpuStagingBuffer& GpuStagingBuffer::operator=(GpuStagingBuffer&& other) noexcept
{
	if(this == &other)
		return *this;

	Release();

	mQueue = std::exchange(other.mQueue, nullptr);
	mBuffer = std::move(other.mBuffer);
	mEpoch = other.mEpoch;

	return *this;
}

GpuStagingBuffer::~GpuStagingBuffer()
{
	Release();
}

void GpuStagingBuffer::Release()
{
	mBuffer = nullptr;

	if(mQueue != nullptr)
	{
		mQueue->ReleaseStagingBuffer(mEpoch);
		mQueue = nullptr;
	}
}

GpuTransferQueue::GpuTransferQueue(GpuDevice& device)
	: mDevice(device), mEpochTracker(*this)
{
	mEpochs.emplace_back();
}

GpuTransferQueue::~GpuTransferQueue()
{
	// The device is idle by now. The staging buffers and destinations are released before the allocators the staging buffers live in.
	mQueuedUploadsByDestination.clear();
	mEpochs.clear();
	mRecordedDestinations = ReleasedProxies();

	mStagingAllocators.clear();
}

GpuStagingBuffer GpuTransferQueue::CreateStagingBuffer(u32 size)
{
	const GpuBufferCreateInformation createInformation = GpuBufferCreateInformation::CreateStagingWrite(size);
	const GpuMemoryRequirements memoryRequirements = mDevice.GetMemoryRequirements(createInformation);
	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return {};

	// The buffer joins the current epoch before it is allocated. Its memory then lies in a page that retires with that epoch or a later
	// one, never an earlier one, even when the epoch ends concurrently.
	u64 epoch;
	IGpuAllocator* allocator;
	bool endEpoch;
	{
		Lock lock(mMutex);

		epoch = mCurrentEpoch.load(std::memory_order_relaxed);
		StagingEpoch& stagingEpoch = GetEpoch(epoch);
		stagingEpoch.OutstandingCount++;
		stagingEpoch.StagingByteCount += size;
		endEpoch = stagingEpoch.StagingByteCount >= kEpochStagingBudget;

		const auto [iterAllocator, inserted] = mStagingAllocators.try_emplace(memoryRequirements.MemoryType);
		if(inserted)
			iterAllocator->second = mDevice.CreateScratchAllocator(memoryRequirements.MemoryType, mEpochTracker, GpuScratchAllocatorFlag::ThreadSafe);

		allocator = iterAllocator->second.get();
	}

	// Backends without scratch allocators fall back to persistent memory
	TShared<render::GpuBuffer> buffer = allocator != nullptr
		? mDevice.CreateGpuBuffer(createInformation, GpuAllocation::CreatePending(*allocator))
		: mDevice.CreateGpuBuffer(createInformation);

	if(buffer == nullptr)
	{
		ReleaseStagingBuffer(epoch);
		return {};
	}

	if(endEpoch)
		EndEpoch();

	return GpuStagingBuffer(*this, std::move(buffer), epoch);
}

void GpuTransferQueue::QueueUpload(GpuStagingBuffer&& source, const TShared<render::GpuBuffer>& destination, u32 destinationOffset, u32 length)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr))
		return;

	IGpuResource* const destinationResource = destination->GetGpuResource();
	if(!B3D_ENSURE(destinationResource != nullptr))
		return;

	QueuedUpload upload;
	upload.Source = std::move(source.mBuffer);
	upload.Epoch = source.mEpoch;
	upload.DestinationOffset = destinationOffset;
	upload.Length = length;
	source.mQueue = nullptr;

	ReleasedProxies releasedProxies;
	Lock lock(mMutex);

	QueuedResourceUploads& entry = GetOrAddEntry(*destinationResource, destination);

	// The new upload replaces those it fully covers
	const u32 destinationEnd = destinationOffset + length;
	for(u32 uploadIndex = (u32)entry.Uploads.Size(); uploadIndex-- > 0;)
	{
		QueuedUpload& queuedUpload = entry.Uploads[uploadIndex];
		if(queuedUpload.DestinationOffset < destinationOffset || queuedUpload.DestinationOffset + queuedUpload.Length > destinationEnd)
			continue;

		DropUpload(queuedUpload, releasedProxies);
		entry.Uploads.Erase(entry.Uploads.Begin() + uploadIndex);
	}

	entry.Uploads.Add(std::move(upload));
}

void GpuTransferQueue::QueueUpload(GpuStagingBuffer&& source, const TShared<render::Texture>& destination, u32 mipLevel, u32 arrayLayer)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr))
		return;

	IGpuResource* const destinationResource = destination->GetGpuResource();
	if(!B3D_ENSURE(destinationResource != nullptr))
		return;

	QueuedUpload upload;
	upload.Source = std::move(source.mBuffer);
	upload.Epoch = source.mEpoch;
	upload.MipLevel = mipLevel;
	upload.ArrayLayer = arrayLayer;
	source.mQueue = nullptr;

	ReleasedProxies releasedProxies;
	Lock lock(mMutex);

	QueuedResourceUploads& entry = GetOrAddEntry(*destinationResource, destination);

	// The new upload replaces an earlier one to the same subresource, as it overwrites the whole subresource
	for(u32 uploadIndex = (u32)entry.Uploads.Size(); uploadIndex-- > 0;)
	{
		QueuedUpload& queuedUpload = entry.Uploads[uploadIndex];
		if(queuedUpload.MipLevel != mipLevel || queuedUpload.ArrayLayer != arrayLayer)
			continue;

		DropUpload(queuedUpload, releasedProxies);
		entry.Uploads.Erase(entry.Uploads.Begin() + uploadIndex);
	}

	entry.Uploads.Add(std::move(upload));
}

void GpuTransferQueue::RecordUpload(GpuStagingBuffer&& source, const TShared<render::GpuBuffer>& destination, u32 destinationOffset, u32 length, GpuCommandBuffer& commandBuffer)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr))
		return;

	RecordBufferCopy(commandBuffer, source.mBuffer, destination, destinationOffset, length);

	Lock lock(mMutex);
	AddRecordedBuffer(std::move(source.mBuffer), source.mEpoch);
	source.mQueue = nullptr;
}

void GpuTransferQueue::RecordUpload(GpuStagingBuffer&& source, const TShared<render::Texture>& destination, u32 mipLevel, u32 arrayLayer, GpuCommandBuffer& commandBuffer)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr))
		return;

	RecordTextureCopy(commandBuffer, source.mBuffer, destination, mipLevel, arrayLayer);

	Lock lock(mMutex);
	AddRecordedBuffer(std::move(source.mBuffer), source.mEpoch);
	source.mQueue = nullptr;
}

bool GpuTransferQueue::TryRecreateInternalBuffer(render::GpuBuffer& destination)
{
	ReleasedProxies releasedProxies;
	Lock lock(mMutex);

	IGpuResource* const destinationResource = destination.GetGpuResource();
	if(destinationResource != nullptr && destinationResource->HasQueuedUploads())
	{
		// BoundCount > UseCount means a command buffer that uses the old native buffer was recorded, but not yet submitted. That
		// command buffer tracks the old native buffer, and pulls in the uploads queued to it when it is submitted. Recreating the
		// buffer would drop those uploads, so the command buffer wouldn't see them.
		if(destinationResource->GetBoundCount() > destinationResource->GetUseCount())
			return false;

		// Any unsubmitted uploads are dropped if the buffer is being recreated
		destinationResource->mHasQueuedUploads.store(false, std::memory_order_relaxed);
		DropEntry(mQueuedUploadsByDestination.find(destinationResource), releasedProxies);
	}

	destination.RecreateInternalBuffer();
	return true;
}

bool GpuTransferQueue::TryRecreateInternalTexture(render::Texture& destination)
{
	ReleasedProxies releasedProxies;
	Lock lock(mMutex);

	IGpuResource* const destinationResource = destination.GetGpuResource();
	if(destinationResource != nullptr && destinationResource->HasQueuedUploads())
	{
		// BoundCount > UseCount means a command buffer that uses the old native texture was recorded, but not yet submitted. That
		// command buffer tracks the old native texture, and pulls in the uploads queued to it when it is submitted. Recreating the
		// texture would drop those uploads, so the command buffer wouldn't see them.
		if(destinationResource->GetBoundCount() > destinationResource->GetUseCount())
			return false;

		// Any unsubmitted uploads are dropped if the texture is being recreated
		destinationResource->mHasQueuedUploads.store(false, std::memory_order_relaxed);
		DropEntry(mQueuedUploadsByDestination.find(destinationResource), releasedProxies);
	}

	destination.RecreateInternalTexture();
	return true;
}

void GpuTransferQueue::SubmitQueuedUploads(const render::GpuBuffer& destination)
{
	IGpuResource* const destinationResource = destination.GetGpuResource();
	if(destinationResource == nullptr || !destinationResource->HasQueuedUploads())
		return;

	mDevice.GetSubmitThread().QueueUploadSubmit(*destinationResource);
}

void GpuTransferQueue::SubmitQueuedUploads(const render::Texture& destination)
{
	IGpuResource* const destinationResource = destination.GetGpuResource();
	if(destinationResource == nullptr || !destinationResource->HasQueuedUploads())
		return;

	mDevice.GetSubmitThread().QueueUploadSubmit(*destinationResource);
}

void GpuTransferQueue::SubmitQueuedUploads()
{
	ReleasedProxies releasedProxies;
	bool hasQueuedUploads;
	{
		Lock lock(mMutex);
		hasQueuedUploads = !mQueuedUploadsByDestination.empty();

		// Recorded on the submit thread, released here so their destruction doesn't run there
		releasedProxies = std::move(mRecordedDestinations);
		mRecordedDestinations = ReleasedProxies();
	}

	if(hasQueuedUploads)
		mDevice.GetSubmitThread().QueueAllUploadsSubmit();

	EndEpoch();
}

TShared<GpuCommandBuffer> GpuTransferQueue::RecordQueuedUploads(TArrayView<IGpuResource* const> destinations)
{
	AssertIfNotSubmitThread();

	TShared<GpuCommandBuffer> commandBuffer;
	Lock lock(mMutex);

	// Destinations whose uploads were recorded earlier, or dropped, have no entry any more
	for(IGpuResource* destination : destinations)
	{
		if(mQueuedUploadsByDestination.find(destination) != mQueuedUploadsByDestination.end())
			RecordEntry(*destination, commandBuffer);
	}

	if(mUntrackedDestinationCount.load(std::memory_order_relaxed) > 0)
	{
		TInlineArray<IGpuResource*, 8> untrackedDestinations;
		for(const auto& [destination, entry] : mQueuedUploadsByDestination)
		{
			if(entry.IsUntracked)
				untrackedDestinations.Add(destination);
		}

		for(IGpuResource* destination : untrackedDestinations)
			RecordEntry(*destination, commandBuffer);
	}

	return commandBuffer;
}

TShared<GpuCommandBuffer> GpuTransferQueue::RecordAllQueuedUploads()
{
	AssertIfNotSubmitThread();

	TShared<GpuCommandBuffer> commandBuffer;
	Lock lock(mMutex);

	while(!mQueuedUploadsByDestination.empty())
		RecordEntry(*mQueuedUploadsByDestination.begin()->first, commandBuffer);

	return commandBuffer;
}

GpuTransferQueue::QueuedResourceUploads& GpuTransferQueue::GetOrAddEntry(IGpuResource& destination, const TShared<render::GpuBuffer>& buffer)
{
	const auto iterEntry = mQueuedUploadsByDestination.find(&destination);
	if(iterEntry != mQueuedUploadsByDestination.end())
		return iterEntry->second;

	QueuedResourceUploads& entry = AddEntry(destination);
	entry.Buffer = buffer;
	entry.IsUntracked = buffer->GetInformation().Flags.IsSet(GpuBufferFlag::Bindless);

	if(entry.IsUntracked)
		mUntrackedDestinationCount.fetch_add(1, std::memory_order_relaxed);

	return entry;
}

GpuTransferQueue::QueuedResourceUploads& GpuTransferQueue::GetOrAddEntry(IGpuResource& destination, const TShared<render::Texture>& texture)
{
	const auto iterEntry = mQueuedUploadsByDestination.find(&destination);
	if(iterEntry != mQueuedUploadsByDestination.end())
		return iterEntry->second;

	QueuedResourceUploads& entry = AddEntry(destination);
	entry.Texture = texture;
	entry.IsUntracked = texture->GetProperties().Usage.IsSet(TextureUsageFlag::Bindless);

	if(entry.IsUntracked)
		mUntrackedDestinationCount.fetch_add(1, std::memory_order_relaxed);

	return entry;
}

GpuTransferQueue::QueuedResourceUploads& GpuTransferQueue::AddEntry(IGpuResource& destination)
{
	destination.mHasQueuedUploads.store(true, std::memory_order_relaxed);
	return mQueuedUploadsByDestination[&destination];
}

void GpuTransferQueue::DropUpload(QueuedUpload& upload, ReleasedProxies& releasedProxies)
{
	GetEpoch(upload.Epoch).OutstandingCount--;
	releasedProxies.Buffers.Add(std::move(upload.Source));
}

void GpuTransferQueue::DropEntry(UnorderedMap<IGpuResource*, QueuedResourceUploads>::iterator iterEntry, ReleasedProxies& releasedProxies)
{
	QueuedResourceUploads& entry = iterEntry->second;
	for(QueuedUpload& upload : entry.Uploads)
		DropUpload(upload, releasedProxies);

	if(entry.IsUntracked)
		mUntrackedDestinationCount.fetch_sub(1, std::memory_order_relaxed);

	releasedProxies.Buffers.Add(std::move(entry.Buffer));
	releasedProxies.Textures.Add(std::move(entry.Texture));

	mQueuedUploadsByDestination.erase(iterEntry);
}

void GpuTransferQueue::RecordEntry(IGpuResource& destination, TShared<GpuCommandBuffer>& commandBuffer)
{
	const auto iterEntry = mQueuedUploadsByDestination.find(&destination);
	B3D_ASSERT(iterEntry != mQueuedUploadsByDestination.end());

	QueuedResourceUploads& entry = iterEntry->second;

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Every native resource change that goes through Write() drops or keeps the uploads queued to the old resource first
	const IGpuResource* const currentResource = entry.Buffer != nullptr ? entry.Buffer->GetGpuResource() : entry.Texture->GetGpuResource();
	if(currentResource != &destination)
	{
		B3D_LOG(Warning, LogRenderBackend, "The native resource of '{0}' changed while uploads to it were queued. The uploads are dropped.",
			entry.Buffer != nullptr ? entry.Buffer->GetName() : entry.Texture->GetName());

		// The old native resource may be destroyed by now, so it isn't accessed. Its proxies are released on the render thread.
		DropEntry(iterEntry, mRecordedDestinations);
		return;
	}
#endif

	destination.mHasQueuedUploads.store(false, std::memory_order_relaxed);

	if(commandBuffer == nullptr)
	{
		GpuCommandBufferPool& commandBufferPool = mDevice.GetSubmitThread().GetCommandBufferPool(GQT_GRAPHICS);
		commandBuffer = commandBufferPool.FindOrCreate(GpuCommandBufferCreateInformation::Create("Uploads"));
	}

	for(QueuedUpload& upload : entry.Uploads)
	{
		if(entry.Buffer != nullptr)
			RecordBufferCopy(*commandBuffer, upload.Source, entry.Buffer, upload.DestinationOffset, upload.Length);
		else
			RecordTextureCopy(*commandBuffer, upload.Source, entry.Texture, upload.MipLevel, upload.ArrayLayer);

		AddRecordedBuffer(std::move(upload.Source), upload.Epoch);
	}

	if(entry.IsUntracked)
		mUntrackedDestinationCount.fetch_sub(1, std::memory_order_relaxed);

	mRecordedDestinations.Buffers.Add(std::move(entry.Buffer));
	mRecordedDestinations.Textures.Add(std::move(entry.Texture));
	mQueuedUploadsByDestination.erase(iterEntry);
}

void GpuTransferQueue::AddRecordedBuffer(TShared<render::GpuBuffer> source, u64 epoch)
{
	GetEpoch(epoch).RecordedBuffers.Add(std::move(source));
}

void GpuTransferQueue::ReleaseStagingBuffer(u64 epoch)
{
	Lock lock(mMutex);
	GetEpoch(epoch).OutstandingCount--;
}

void GpuTransferQueue::EndEpoch()
{
	ReleasedProxies releasedProxies;
	Lock lock(mMutex);

	// Retire the pages the ending epoch allocated from, before the next epoch starts
	for(const auto& [memoryType, allocator] : mStagingAllocators)
	{
		if(allocator != nullptr)
			allocator->FreeAll();
	}

	mEpochs.emplace_back();
	mCurrentEpoch.fetch_add(1, std::memory_order_acq_rel);

	UpdateEpochs(releasedProxies);

	for(const auto& [memoryType, allocator] : mStagingAllocators)
	{
		if(allocator != nullptr)
			allocator->ReclaimUnused();
	}
}

void GpuTransferQueue::UpdateEpochs(ReleasedProxies& releasedProxies)
{
	// A recorded staging buffer is unbound once the command buffers that copy from it complete, or are reset or destroyed unsubmitted
	for(StagingEpoch& epoch : mEpochs)
	{
		for(u32 bufferIndex = (u32)epoch.RecordedBuffers.Size(); bufferIndex-- > 0;)
		{
			if(epoch.RecordedBuffers[bufferIndex]->GetBoundCount() > 0)
				continue;

			releasedProxies.Buffers.Add(std::move(epoch.RecordedBuffers[bufferIndex]));
			epoch.RecordedBuffers.Erase(epoch.RecordedBuffers.Begin() + bufferIndex);
			epoch.OutstandingCount--;
		}
	}

	// The last epoch is the current one, which hasn't ended
	u64 firstIncompleteEpoch = mFirstIncompleteEpoch.load(std::memory_order_relaxed);
	while(mEpochs.size() > 1 && mEpochs.front().OutstandingCount == 0)
	{
		mEpochs.pop_front();
		firstIncompleteEpoch++;
	}

	mFirstIncompleteEpoch.store(firstIncompleteEpoch, std::memory_order_release);
}

GpuTransferQueue::StagingEpoch& GpuTransferQueue::GetEpoch(u64 epoch)
{
	const u64 epochIndex = epoch - mFirstIncompleteEpoch.load(std::memory_order_relaxed);
	B3D_ASSERT(epoch >= mFirstIncompleteEpoch.load(std::memory_order_relaxed) && epochIndex < mEpochs.size());

	return mEpochs[epochIndex];
}
