//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuTransferQueue.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "FileSystem/B3DDataStream.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DTexture.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Staging bytes after which creating a staging buffer ends the epoch, so processes that never end a frame still reuse staging memory. */
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

	/** Records the barriers that make the GPU's writes to @p buffer visible to the host, and return the buffer to rest. */
	void RecordBufferHostRead(GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& buffer)
	{
		const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*buffer, commandBuffer.GetQueueType());
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);

		commandBuffer.IssueBarriers(GpuBufferBarrier(buffer, restingState, hostRead));
		commandBuffer.IssueBarriers(GpuBufferBarrier(buffer, hostRead, restingState));
	}

	/**
	 * Records the barriers that make the GPU's writes to one subresource of @p texture visible to the host, and return the subresource to rest.
	 * The subresource keeps its resting layout.
	 */
	void RecordTextureHostRead(GpuCommandBuffer& commandBuffer, const TShared<render::Texture>& texture, u32 mipLevel, u32 arrayLayer)
	{
		const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*texture, commandBuffer.GetQueueType());
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read, restingState.Layout);
		const GpuTextureSubresourceRange range(mipLevel, 1, arrayLayer, 1, GpuBackendUtility::GetFormatAspects(texture->GetProperties().Format));

		commandBuffer.IssueBarriers(GpuTextureBarrier(texture, restingState, hostRead, range));
		commandBuffer.IssueBarriers(GpuTextureBarrier(texture, hostRead, restingState, range));
	}

	/** Records a copy of the whole of @p source into @p destination, a buffer of the same description, which takes the resting state after it. */
	void RecordBufferRelocation(GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& source, const TShared<render::GpuBuffer>& destination)
	{
		const GpuQueueType queueType = commandBuffer.GetQueueType();
		const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*source, queueType);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);

		commandBuffer.IssueBarriers(GpuBarriers({
			GpuBufferBarrier(source, restingState, transferRead),
			GpuBufferBarrier(destination, GpuBackendUtility::GetRestingState(*destination, queueType), transferWrite) }, {}));
		commandBuffer.CopyBufferToBuffer(source, destination, 0, 0, source->GetTotalSize());
		commandBuffer.IssueBarriers(GpuBufferBarrier(destination, transferWrite, restingState));
	}

	/**
	 * Records a copy of every subresource of @p source into @p destination, a texture of the same description, which takes the resting state
	 * after it. The copy overwrites every subresource of the destination, so its old contents are discarded.
	 */
	void RecordTextureRelocation(GpuCommandBuffer& commandBuffer, const TShared<render::Texture>& source, const TShared<render::Texture>& destination)
	{
		const GpuQueueType queueType = commandBuffer.GetQueueType();
		const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*source, queueType);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);

		GpuAccessState discardedState = GpuBackendUtility::GetRestingState(*destination, queueType);
		discardedState.Layout = GpuImageLayout::Undefined;

		commandBuffer.IssueBarriers(GpuBarriers({}, {
			GpuTextureBarrier(source, restingState, transferRead),
			GpuTextureBarrier(destination, discardedState, transferWrite) }));

		const TextureProperties& properties = source->GetProperties();

		TextureCopyInformation copyInformation;
		copyInformation.FaceCount = properties.GetFaceCount();
		for(u32 mipLevel = 0; mipLevel <= properties.MipMapCount; mipLevel++)
		{
			copyInformation.SourceMip = mipLevel;
			copyInformation.DestinationMip = mipLevel;
			commandBuffer.CopyTexture(source, destination, copyInformation);
		}

		commandBuffer.IssueBarriers(GpuTextureBarrier(destination, transferWrite, restingState));
	}

	/** Adds @p proxy to @p proxies, if it isn't null. */
	template<class ProxyType>
	void AddIfNotNull(TArray<TShared<ProxyType>>& proxies, TShared<ProxyType>&& proxy)
	{
		if(proxy != nullptr)
			proxies.Add(std::move(proxy));
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

void GpuTransferQueue::QueuedRead::Complete(bool isExecuted)
{
	// Each read completes once, from whichever of its command buffer's completion and destruction comes first
	if(Staging.GetBuffer() == nullptr)
		return;

	if(TexturePixelData != nullptr)
	{
		if(isExecuted)
		{
			render::GpuBufferMappedScope mapping = Staging.GetBuffer()->Map(GpuMapOption::Read);
			TexturePixelData->AllocateInternalBuffer();
			memcpy(TexturePixelData->GetData(), mapping.GetMappedMemory(), TexturePixelData->GetSize());
		}

		TextureOperation.CompleteOperation(isExecuted ? TexturePixelData : nullptr);
	}
	else
	{
		TShared<MemoryDataStream> dataStream;
		if(isExecuted)
		{
			render::GpuBufferMappedScope mapping = Staging.GetBuffer()->Map(0, Length, GpuMapOption::Read);
			dataStream = B3DMakeShared<MemoryDataStream>(Length);
			memcpy(dataStream->Data(), mapping.GetMappedMemory(), Length);
		}

		BufferOperation.CompleteOperation(dataStream);
	}

	Staging = GpuStagingBuffer();
}

GpuTransferQueue::ReleasedProxies& GpuTransferQueue::ReleasedProxies::operator=(ReleasedProxies&& other) noexcept
{
	if(this == &other)
		return *this;

	for(const TShared<QueuedRead>& read : Reads)
		read->Complete(false);

	Buffers = std::move(other.Buffers);
	Textures = std::move(other.Textures);
	Reads = std::move(other.Reads);

	return *this;
}

GpuTransferQueue::ReleasedProxies::~ReleasedProxies()
{
	for(const TShared<QueuedRead>& read : Reads)
		read->Complete(false);
}

GpuTransferQueue::GpuTransferQueue(GpuDevice& device)
	: mDevice(device), mEpochTracker(*this)
{
	mEpochs.emplace_back();
}

GpuTransferQueue::~GpuTransferQueue()
{
	// The device is idle by now. The staging buffers, resources and reads are released before the allocators the staging buffers live in,
	// and the reads release their staging buffers into the epochs, so those go last.
	{
		ReleasedProxies releasedProxies;
		{
			Lock lock(mMutex);
			while(!mQueuedTransfersByDestination.empty())
				DropEntry(mQueuedTransfersByDestination.begin(), releasedProxies);
		}
	}

	mRecordedDestinations = ReleasedProxies();
	mEpochs.clear();

	mStagingAllocators.clear();
}

GpuStagingBuffer GpuTransferQueue::CreateStagingWriteBuffer(u32 size)
{
	return CreateStagingBuffer(GpuBufferCreateInformation::CreateStagingWrite(size));
}

GpuStagingBuffer GpuTransferQueue::CreateStagingReadBuffer(u32 size)
{
	return CreateStagingBuffer(GpuBufferCreateInformation::CreateStagingRead(size));
}

GpuStagingBuffer GpuTransferQueue::CreateStagingBuffer(const GpuBufferCreateInformation& createInformation)
{
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
		stagingEpoch.StagingByteCount += createInformation.Staging.Size;
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

	QueuedTransfer upload;
	upload.Type = QueuedTransferType::Upload;
	upload.Staging = std::move(source.mBuffer);
	upload.Epoch = source.mEpoch;
	upload.Buffer = destination;
	upload.Offset = destinationOffset;
	upload.Length = length;
	source.mQueue = nullptr;

	ReleasedProxies releasedProxies;
	Lock lock(mMutex);

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Under the mutex the command buffer this queue records into is never recorded but not submitted, see EndRecording(). Only the command
	// buffers of other threads count.
	if(destination->GetBoundCount() > destination->GetUseCount() && destination->IsRangeBound(destinationOffset, length))
	{
		B3D_LOG(Warning, LogRenderBackend, "Queuing an upload to a buffer '{0}' that a recorded command buffer which isn't submitted yet uses. The upload runs before that command buffer, so the command buffer sees it instead of the contents it was recorded after. Record the write into a command buffer to order it.", destination->GetName());
	}
#endif

	QueuedResourceTransfers& entry = GetOrAddEntry(*destinationResource, destination);

	const u32 destinationEnd = destinationOffset + length;
	AddUpload(entry, std::move(upload), [destinationOffset, destinationEnd](const QueuedTransfer& queuedUpload)
	{
		return queuedUpload.Offset >= destinationOffset && queuedUpload.Offset + queuedUpload.Length <= destinationEnd;
	}, releasedProxies);
}

void GpuTransferQueue::QueueUpload(GpuStagingBuffer&& source, const TShared<render::Texture>& destination, u32 mipLevel, u32 arrayLayer)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr))
		return;

	IGpuResource* const destinationResource = destination->GetGpuResource();
	if(!B3D_ENSURE(destinationResource != nullptr))
		return;

	QueuedTransfer upload;
	upload.Type = QueuedTransferType::Upload;
	upload.Staging = std::move(source.mBuffer);
	upload.Epoch = source.mEpoch;
	upload.Texture = destination;
	upload.MipLevel = mipLevel;
	upload.ArrayLayer = arrayLayer;
	source.mQueue = nullptr;

	ReleasedProxies releasedProxies;
	Lock lock(mMutex);

#if B3D_BUILD_TYPE_DEVELOPMENT
	// See QueueUpload(GpuStagingBuffer&&, const TShared<render::GpuBuffer>&, u32, u32)
	const u32 subresourceIndex = destination->GetProperties().MapToSubresourceIndex(arrayLayer, mipLevel);
	if(destination->GetBoundCount(subresourceIndex) > destination->GetUseCount(subresourceIndex))
	{
		B3D_LOG(Warning, LogRenderBackend, "Queuing an upload to an image '{0}' that a recorded command buffer which isn't submitted yet uses. The upload runs before that command buffer, so the command buffer sees it instead of the contents it was recorded after. Record the write into a command buffer to order it.", destination->GetName());
	}
#endif

	QueuedResourceTransfers& entry = GetOrAddEntry(*destinationResource, destination);

	// The copy overwrites the whole subresource
	AddUpload(entry, std::move(upload), [mipLevel, arrayLayer](const QueuedTransfer& queuedUpload)
	{
		return queuedUpload.MipLevel == mipLevel && queuedUpload.ArrayLayer == arrayLayer;
	}, releasedProxies);
}

template<class CoversFunction>
void GpuTransferQueue::AddUpload(QueuedResourceTransfers& entry, QueuedTransfer&& upload, CoversFunction fnCovers, ReleasedProxies& releasedProxies)
{
	// A read or a relocation queued in between needs the earlier uploads
	for(u32 transferIndex = (u32)entry.Transfers.Size(); transferIndex-- > 0;)
	{
		QueuedTransfer& queuedTransfer = entry.Transfers[transferIndex];
		if(queuedTransfer.Type != QueuedTransferType::Upload)
			break;

		if(!fnCovers(queuedTransfer))
			continue;

		DropTransfer(queuedTransfer, releasedProxies);
		entry.Transfers.Erase(entry.Transfers.Begin() + transferIndex);
	}

	entry.Transfers.Add(std::move(upload));
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

TAsyncOp<TShared<MemoryDataStream>> GpuTransferQueue::QueueRead(const TShared<render::GpuBuffer>& source, u32 offset, u32 length)
{
	const TShared<QueuedRead> read = B3DMakeShared<QueuedRead>();
	read->Length = length;

	if(!B3D_ENSURE(source != nullptr && length > 0 && offset + length <= source->GetTotalSize()))
	{
		read->BufferOperation.CompleteOperation(nullptr);
		return read->BufferOperation;
	}

	read->Staging = CreateStagingReadBuffer(length);
	if(!B3D_ENSURE(read->Staging.GetBuffer() != nullptr))
	{
		read->BufferOperation.CompleteOperation(nullptr);
		return read->BufferOperation;
	}

	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::Read;
	transfer.Staging = read->Staging.GetBuffer();
	transfer.Offset = offset;
	transfer.Length = length;
	transfer.AsyncRead = read;

	IGpuResource* const sourceResource = QueueReadTransfer(source, std::move(transfer));
	if(sourceResource == nullptr)
	{
		read->Complete(false);
		return read->BufferOperation;
	}

	mDevice.GetSubmitThread().QueueTransferSubmit(*sourceResource);
	return read->BufferOperation;
}

TAsyncOp<TShared<PixelData>> GpuTransferQueue::QueueRead(const TShared<render::Texture>& source, u32 mipLevel, u32 arrayLayer)
{
	const TShared<QueuedRead> read = B3DMakeShared<QueuedRead>();
	if(!B3D_ENSURE(source != nullptr))
	{
		read->TextureOperation.CompleteOperation(nullptr);
		return read->TextureOperation;
	}

	read->TexturePixelData = TextureUtility::CreateStagingPixelData(source, mipLevel, arrayLayer);
	read->Staging = CreateStagingReadBuffer(read->TexturePixelData->GetSize());
	if(!B3D_ENSURE(read->Staging.GetBuffer() != nullptr))
	{
		read->TextureOperation.CompleteOperation(nullptr);
		return read->TextureOperation;
	}

	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::Read;
	transfer.Staging = read->Staging.GetBuffer();
	transfer.MipLevel = mipLevel;
	transfer.ArrayLayer = arrayLayer;
	transfer.AsyncRead = read;

	IGpuResource* const sourceResource = QueueReadTransfer(source, std::move(transfer));
	if(sourceResource == nullptr)
	{
		read->Complete(false);
		return read->TextureOperation;
	}

	mDevice.GetSubmitThread().QueueTransferSubmit(*sourceResource);
	return read->TextureOperation;
}

void GpuTransferQueue::CopyToStagingAndWait(const TShared<render::GpuBuffer>& source, u32 offset, u32 length, const GpuStagingBuffer& destination)
{
	if(!B3D_ENSURE(source != nullptr && destination.mQueue == this))
		return;

	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::Read;
	transfer.Staging = destination.GetBuffer();
	transfer.Offset = offset;
	transfer.Length = length;

	if(QueueReadTransfer(source, std::move(transfer)) != nullptr)
		mDevice.GetSubmitThread().QueueReadSubmitAndWait(source);
}

void GpuTransferQueue::CopyToStagingAndWait(const TShared<render::Texture>& source, u32 mipLevel, u32 arrayLayer, const GpuStagingBuffer& destination)
{
	if(!B3D_ENSURE(source != nullptr && destination.mQueue == this))
		return;

	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::Read;
	transfer.Staging = destination.GetBuffer();
	transfer.MipLevel = mipLevel;
	transfer.ArrayLayer = arrayLayer;

	if(QueueReadTransfer(source, std::move(transfer)) != nullptr)
		mDevice.GetSubmitThread().QueueReadSubmitAndWait(source);
}

void GpuTransferQueue::WaitUntilHostReadable(const TShared<render::GpuBuffer>& source)
{
	if(!B3D_ENSURE(source != nullptr))
		return;

	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::Read;

	if(QueueReadTransfer(source, std::move(transfer)) != nullptr)
		mDevice.GetSubmitThread().QueueReadSubmitAndWait(source);
}

void GpuTransferQueue::WaitUntilHostReadable(const TShared<render::Texture>& source, u32 mipLevel, u32 arrayLayer)
{
	if(!B3D_ENSURE(source != nullptr))
		return;

	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::Read;
	transfer.MipLevel = mipLevel;
	transfer.ArrayLayer = arrayLayer;

	if(QueueReadTransfer(source, std::move(transfer)) != nullptr)
		mDevice.GetSubmitThread().QueueReadSubmitAndWait(source);
}

IGpuResource* GpuTransferQueue::QueueReadTransfer(const TShared<render::GpuBuffer>& buffer, QueuedTransfer&& transfer)
{
	Lock lock(mMutex);

	IGpuResource* const resource = buffer->GetGpuResource();
	if(!B3D_ENSURE(resource != nullptr))
		return nullptr;

	transfer.Buffer = buffer;
	GetOrAddEntry(*resource, buffer).Transfers.Add(std::move(transfer));

	return resource;
}

IGpuResource* GpuTransferQueue::QueueReadTransfer(const TShared<render::Texture>& texture, QueuedTransfer&& transfer)
{
	Lock lock(mMutex);

	IGpuResource* const resource = texture->GetGpuResource();
	if(!B3D_ENSURE(resource != nullptr))
		return nullptr;

	transfer.Texture = texture;
	GetOrAddEntry(*resource, texture).Transfers.Add(std::move(transfer));

	return resource;
}

void GpuTransferQueue::QueueRelocation(const TShared<render::GpuBuffer>& source, const TShared<render::GpuBuffer>& target)
{
	if(!B3D_ENSURE(source != nullptr && target != nullptr))
		return;

	// The exchange and the move of the entry happen together, so a transfer queued concurrently goes either before the relocation, to the
	// old native buffer, or after it, to the new one
	Lock lock(mMutex);

	IGpuResource* const oldResource = source->GetGpuResource();
	source->Relocate(*target);

	IGpuResource* const newResource = source->GetGpuResource();
	if(!B3D_ENSURE(newResource != nullptr))
		return;

	QueuedTransfer relocation;
	relocation.Type = QueuedTransferType::Relocation;
	relocation.Buffer = source;
	relocation.RelocationSourceBuffer = target;

	MoveEntryForRelocation(oldResource, *newResource, source, target).Transfers.Add(std::move(relocation));
}

void GpuTransferQueue::QueueRelocation(const TShared<render::Texture>& source, const TShared<render::Texture>& target)
{
	if(!B3D_ENSURE(source != nullptr && target != nullptr))
		return;

	// See QueueRelocation(const TShared<render::GpuBuffer>&, const TShared<render::GpuBuffer>&)
	Lock lock(mMutex);

	IGpuResource* const oldResource = source->GetGpuResource();
	source->Relocate(*target);

	IGpuResource* const newResource = source->GetGpuResource();
	if(!B3D_ENSURE(newResource != nullptr))
		return;

	QueuedTransfer relocation;
	relocation.Type = QueuedTransferType::Relocation;
	relocation.Texture = source;
	relocation.RelocationSourceTexture = target;

	MoveEntryForRelocation(oldResource, *newResource, source, target).Transfers.Add(std::move(relocation));
}

template<class ProxyType>
GpuTransferQueue::QueuedResourceTransfers& GpuTransferQueue::MoveEntryForRelocation(IGpuResource* oldResource, IGpuResource& newResource,
	const TShared<ProxyType>& resource, const TShared<ProxyType>& oldResourceHolder)
{
	B3D_ASSERT(mQueuedTransfersByDestination.find(&newResource) == mQueuedTransfersByDestination.end());

	const auto iterOldEntry = mQueuedTransfersByDestination.find(oldResource);
	if(iterOldEntry == mQueuedTransfersByDestination.end())
		return GetOrAddEntry(newResource, resource);

	QueuedResourceTransfers entry = std::move(iterOldEntry->second);
	mQueuedTransfersByDestination.erase(iterOldEntry);
	oldResource->mHasQueuedTransfers.store(false, std::memory_order_relaxed);

	// The transfers queued so far mean the old native resource, which the holder has now
	for(QueuedTransfer& transfer : entry.Transfers)
	{
		if constexpr(std::is_same_v<ProxyType, render::GpuBuffer>)
		{
			if(transfer.Buffer == resource)
				transfer.Buffer = oldResourceHolder;

			if(transfer.RelocationSourceBuffer == resource)
				transfer.RelocationSourceBuffer = oldResourceHolder;
		}
		else
		{
			if(transfer.Texture == resource)
				transfer.Texture = oldResourceHolder;

			if(transfer.RelocationSourceTexture == resource)
				transfer.RelocationSourceTexture = oldResourceHolder;
		}
	}

	QueuedResourceTransfers& newEntry = AddEntry(newResource);
	newEntry = std::move(entry);

	return newEntry;
}

bool GpuTransferQueue::TryRecreateInternalBuffer(render::GpuBuffer& destination)
{
	ReleasedProxies releasedProxies;
	Lock lock(mMutex);

	if(!TryDropEntryForRecreate(destination.GetGpuResource(), releasedProxies))
		return false;

	destination.RecreateInternalBuffer();
	return true;
}

bool GpuTransferQueue::TryRecreateInternalTexture(render::Texture& destination)
{
	ReleasedProxies releasedProxies;
	Lock lock(mMutex);

	if(!TryDropEntryForRecreate(destination.GetGpuResource(), releasedProxies))
		return false;

	destination.RecreateInternalTexture();
	return true;
}

bool GpuTransferQueue::TryDropEntryForRecreate(IGpuResource* resource, ReleasedProxies& releasedProxies)
{
	if(resource == nullptr || !resource->HasQueuedTransfers())
		return true;

	const auto iterEntry = mQueuedTransfersByDestination.find(resource);
	if(iterEntry == mQueuedTransfersByDestination.end())
		return true;

	// A read or a relocation needs the contents of the old native resource
	for(const QueuedTransfer& transfer : iterEntry->second.Transfers)
	{
		if(transfer.Type != QueuedTransferType::Upload)
			return false;
	}

	// BoundCount > UseCount means a command buffer that uses the old native resource was recorded, but not yet submitted. That command buffer
	// tracks the old native resource, and pulls in the uploads queued to it when it is submitted. Recreating the resource would drop those
	// uploads, so the command buffer wouldn't see them.
	if(resource->GetBoundCount() > resource->GetUseCount())
		return false;

	// Any unsubmitted uploads are dropped if the resource is being recreated
	resource->mHasQueuedTransfers.store(false, std::memory_order_relaxed);
	DropEntry(iterEntry, releasedProxies);

	return true;
}

void GpuTransferQueue::SubmitQueuedTransfers(const render::GpuBuffer& destination)
{
	IGpuResource* const destinationResource = destination.GetGpuResource();
	if(destinationResource == nullptr || !destinationResource->HasQueuedTransfers())
		return;

	mDevice.GetSubmitThread().QueueTransferSubmit(*destinationResource);
}

void GpuTransferQueue::SubmitQueuedTransfers(const render::Texture& destination)
{
	IGpuResource* const destinationResource = destination.GetGpuResource();
	if(destinationResource == nullptr || !destinationResource->HasQueuedTransfers())
		return;

	mDevice.GetSubmitThread().QueueTransferSubmit(*destinationResource);
}

void GpuTransferQueue::SubmitQueuedTransfers()
{
	ReleasedProxies releasedProxies;
	bool hasQueuedTransfers;
	{
		Lock lock(mMutex);
		hasQueuedTransfers = !mQueuedTransfersByDestination.empty();

		// Recorded on the submit thread, released here so their destruction doesn't run there
		releasedProxies = std::move(mRecordedDestinations);
		mRecordedDestinations = ReleasedProxies();
	}

	if(hasQueuedTransfers)
		mDevice.GetSubmitThread().QueueAllTransfersSubmit();

	EndEpoch();
}

GpuTransferQueue::RecordedTransfers GpuTransferQueue::RecordQueuedTransfers(TArrayView<IGpuResource* const> resources)
{
	AssertIfNotSubmitThread();

	RecordedTransfers recorded;
	TArray<TShared<QueuedRead>> asyncReads;
	{
		Lock lock(mMutex);

		// Resources whose transfers were recorded earlier, or dropped, have no entry any more
		for(IGpuResource* resource : resources)
		{
			if(mQueuedTransfersByDestination.find(resource) != mQueuedTransfersByDestination.end())
				RecordEntry(*resource, recorded, asyncReads);
		}

		RecordUntrackedEntries(recorded, asyncReads);
		EndRecording(recorded);
	}

	CompleteReadsOnCompletion(recorded, std::move(asyncReads));
	return recorded;
}

GpuTransferQueue::RecordedTransfers GpuTransferQueue::RecordQueuedTransfers(const render::GpuBuffer& resource)
{
	AssertIfNotSubmitThread();

	RecordedTransfers recorded;
	TArray<TShared<QueuedRead>> asyncReads;
	{
		// The native resource is looked up under the mutex, so a concurrent relocation can't move the entry away in between
		Lock lock(mMutex);

		IGpuResource* const gpuResource = resource.GetGpuResource();
		if(gpuResource != nullptr && mQueuedTransfersByDestination.find(gpuResource) != mQueuedTransfersByDestination.end())
			RecordEntry(*gpuResource, recorded, asyncReads);

		RecordUntrackedEntries(recorded, asyncReads);
		EndRecording(recorded);
	}

	CompleteReadsOnCompletion(recorded, std::move(asyncReads));
	return recorded;
}

GpuTransferQueue::RecordedTransfers GpuTransferQueue::RecordQueuedTransfers(const render::Texture& resource)
{
	AssertIfNotSubmitThread();

	RecordedTransfers recorded;
	TArray<TShared<QueuedRead>> asyncReads;
	{
		// See RecordQueuedTransfers(const render::GpuBuffer&)
		Lock lock(mMutex);

		IGpuResource* const gpuResource = resource.GetGpuResource();
		if(gpuResource != nullptr && mQueuedTransfersByDestination.find(gpuResource) != mQueuedTransfersByDestination.end())
			RecordEntry(*gpuResource, recorded, asyncReads);

		RecordUntrackedEntries(recorded, asyncReads);
		EndRecording(recorded);
	}

	CompleteReadsOnCompletion(recorded, std::move(asyncReads));
	return recorded;
}

GpuTransferQueue::RecordedTransfers GpuTransferQueue::RecordAllQueuedTransfers()
{
	AssertIfNotSubmitThread();

	RecordedTransfers recorded;
	TArray<TShared<QueuedRead>> asyncReads;
	{
		Lock lock(mMutex);

		while(!mQueuedTransfersByDestination.empty())
			RecordEntry(*mQueuedTransfersByDestination.begin()->first, recorded, asyncReads);

		EndRecording(recorded);
	}

	CompleteReadsOnCompletion(recorded, std::move(asyncReads));
	return recorded;
}

void GpuTransferQueue::RecordUntrackedEntries(RecordedTransfers& recorded, TArray<TShared<QueuedRead>>& asyncReads)
{
	if(mUntrackedDestinationCount.load(std::memory_order_relaxed) == 0)
		return;

	TInlineArray<IGpuResource*, 8> untrackedResources;
	for(const auto& [resource, entry] : mQueuedTransfersByDestination)
	{
		if(entry.IsUntracked)
			untrackedResources.Add(resource);
	}

	for(IGpuResource* resource : untrackedResources)
		RecordEntry(*resource, recorded, asyncReads);
}

void GpuTransferQueue::EndRecording(RecordedTransfers& recorded)
{
	// Recording bound the resources. Marking them used under the same mutex means QueueUpload(), which also holds it, never sees this command
	// buffer as recorded but not submitted. A write queued now lands in a later command buffer, so this one never sees it.
	if(recorded.CommandBuffer != nullptr)
	{
		recorded.CommandBuffer->End();
		recorded.CommandBuffer->NotifyWillQueueForSubmit(mDevice.GetQueue(GQT_GRAPHICS, 0)->GetId(), GpuQueueMask::kAll);
	}

	// Cleared only now, so a writer that sees no queued transfers also sees the resource as used, and doesn't map it while the copy is pending
	for(IGpuResource* resource : mRecordedResources)
		resource->mHasQueuedTransfers.store(false, std::memory_order_release);

	mRecordedResources.Clear();
}

void GpuTransferQueue::CompleteReadsOnCompletion(const RecordedTransfers& recorded, TArray<TShared<QueuedRead>>&& asyncReads)
{
	if(asyncReads.Empty())
		return;

	B3D_ASSERT(recorded.CommandBuffer != nullptr);

	auto fnOnCommandBufferCompleted = [asyncReads]()
	{
		for(const TShared<QueuedRead>& read : asyncReads)
			read->Complete(true);
	};

	auto fnOnCommandBufferDestroyed = [asyncReads](bool isSubmitted)
	{
		// A submitted command buffer completes the reads on completion instead
		if(isSubmitted)
			return;

		for(const TShared<QueuedRead>& read : asyncReads)
			read->Complete(false);
	};

	recorded.CommandBuffer->OnDidComplete.Connect(fnOnCommandBufferCompleted);
	recorded.CommandBuffer->OnDestroyed.Connect(fnOnCommandBufferDestroyed);
}

GpuTransferQueue::QueuedResourceTransfers& GpuTransferQueue::GetOrAddEntry(IGpuResource& resource, const TShared<render::GpuBuffer>& buffer)
{
	const auto iterEntry = mQueuedTransfersByDestination.find(&resource);
	if(iterEntry != mQueuedTransfersByDestination.end())
		return iterEntry->second;

	QueuedResourceTransfers& entry = AddEntry(resource);
	entry.Buffer = buffer;
	entry.IsUntracked = buffer->GetInformation().Flags.IsSet(GpuBufferFlag::Bindless);

	if(entry.IsUntracked)
		mUntrackedDestinationCount.fetch_add(1, std::memory_order_relaxed);

	return entry;
}

GpuTransferQueue::QueuedResourceTransfers& GpuTransferQueue::GetOrAddEntry(IGpuResource& resource, const TShared<render::Texture>& texture)
{
	const auto iterEntry = mQueuedTransfersByDestination.find(&resource);
	if(iterEntry != mQueuedTransfersByDestination.end())
		return iterEntry->second;

	QueuedResourceTransfers& entry = AddEntry(resource);
	entry.Texture = texture;
	entry.IsUntracked = texture->GetProperties().Usage.IsSet(TextureUsageFlag::Bindless);

	if(entry.IsUntracked)
		mUntrackedDestinationCount.fetch_add(1, std::memory_order_relaxed);

	return entry;
}

GpuTransferQueue::QueuedResourceTransfers& GpuTransferQueue::AddEntry(IGpuResource& resource)
{
	resource.mHasQueuedTransfers.store(true, std::memory_order_relaxed);
	return mQueuedTransfersByDestination[&resource];
}

void GpuTransferQueue::DropTransfer(QueuedTransfer& transfer, ReleasedProxies& releasedProxies)
{
	// The staging buffer of a read is counted by the handle the reader holds
	if(transfer.Type == QueuedTransferType::Upload)
		GetEpoch(transfer.Epoch).OutstandingCount--;

	AddIfNotNull(releasedProxies.Buffers, std::move(transfer.Staging));
	AddIfNotNull(releasedProxies.Buffers, std::move(transfer.Buffer));
	AddIfNotNull(releasedProxies.Textures, std::move(transfer.Texture));
	AddIfNotNull(releasedProxies.Buffers, std::move(transfer.RelocationSourceBuffer));
	AddIfNotNull(releasedProxies.Textures, std::move(transfer.RelocationSourceTexture));

	if(transfer.AsyncRead != nullptr)
		releasedProxies.Reads.Add(std::move(transfer.AsyncRead));
}

void GpuTransferQueue::DropEntry(UnorderedMap<IGpuResource*, QueuedResourceTransfers>::iterator iterEntry, ReleasedProxies& releasedProxies)
{
	QueuedResourceTransfers& entry = iterEntry->second;
	for(QueuedTransfer& transfer : entry.Transfers)
		DropTransfer(transfer, releasedProxies);

	if(entry.IsUntracked)
		mUntrackedDestinationCount.fetch_sub(1, std::memory_order_relaxed);

	AddIfNotNull(releasedProxies.Buffers, std::move(entry.Buffer));
	AddIfNotNull(releasedProxies.Textures, std::move(entry.Texture));

	mQueuedTransfersByDestination.erase(iterEntry);
}

void GpuTransferQueue::RecordEntry(IGpuResource& resource, RecordedTransfers& recorded, TArray<TShared<QueuedRead>>& asyncReads)
{
	const auto iterEntry = mQueuedTransfersByDestination.find(&resource);
	B3D_ASSERT(iterEntry != mQueuedTransfersByDestination.end());

	QueuedResourceTransfers& entry = iterEntry->second;

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Every native resource change that goes through Write() or a relocation drops, keeps or moves the transfers queued to the old resource
	// first
	const IGpuResource* const currentResource = entry.Buffer != nullptr ? entry.Buffer->GetGpuResource() : entry.Texture->GetGpuResource();
	if(currentResource != &resource)
	{
		B3D_LOG(Warning, LogRenderBackend, "The native resource of '{0}' changed while transfers to it were queued. The transfers are dropped.",
			entry.Buffer != nullptr ? entry.Buffer->GetName() : entry.Texture->GetName());

		// The old native resource may be destroyed by now, so it isn't accessed. Its proxies are released on the render thread.
		DropEntry(iterEntry, mRecordedDestinations);
		return;
	}
#endif

	// The flag stays set until the command buffer is queued for submission, see EndRecording()
	mRecordedResources.Add(&resource);

	if(recorded.CommandBuffer == nullptr)
	{
		GpuCommandBufferPool& commandBufferPool = mDevice.GetSubmitThread().GetCommandBufferPool(GQT_GRAPHICS);
		recorded.CommandBuffer = commandBufferPool.FindOrCreate(GpuCommandBufferCreateInformation::Create("Transfers"));
	}

	GpuCommandBuffer& commandBuffer = *recorded.CommandBuffer;
	for(QueuedTransfer& transfer : entry.Transfers)
	{
		switch(transfer.Type)
		{
		case QueuedTransferType::Upload:
			if(transfer.Buffer != nullptr)
				RecordBufferCopy(commandBuffer, transfer.Staging, transfer.Buffer, transfer.Offset, transfer.Length);
			else
				RecordTextureCopy(commandBuffer, transfer.Staging, transfer.Texture, transfer.MipLevel, transfer.ArrayLayer);

			AddRecordedBuffer(std::move(transfer.Staging), transfer.Epoch);
			break;
		case QueuedTransferType::Read:
			if(transfer.Buffer != nullptr)
			{
				if(transfer.Staging != nullptr)
					GpuBackendUtility::CopyToStagingBuffer(commandBuffer, transfer.Buffer, transfer.Offset, transfer.Length, transfer.Staging);
				else
					RecordBufferHostRead(commandBuffer, transfer.Buffer);
			}
			else
			{
				if(transfer.Staging != nullptr)
					GpuBackendUtility::CopyToStagingBuffer(commandBuffer, transfer.Texture, transfer.MipLevel, transfer.ArrayLayer, transfer.Staging);
				else
					RecordTextureHostRead(commandBuffer, transfer.Texture, transfer.MipLevel, transfer.ArrayLayer);
			}

			recorded.HasReads = true;

			if(transfer.AsyncRead != nullptr)
				asyncReads.Add(std::move(transfer.AsyncRead));

			break;
		case QueuedTransferType::Relocation:
			if(transfer.Buffer != nullptr)
				RecordBufferRelocation(commandBuffer, transfer.RelocationSourceBuffer, transfer.Buffer);
			else
				RecordTextureRelocation(commandBuffer, transfer.RelocationSourceTexture, transfer.Texture);

			break;
		}

		AddIfNotNull(mRecordedDestinations.Buffers, std::move(transfer.Staging));
		AddIfNotNull(mRecordedDestinations.Buffers, std::move(transfer.Buffer));
		AddIfNotNull(mRecordedDestinations.Textures, std::move(transfer.Texture));
		AddIfNotNull(mRecordedDestinations.Buffers, std::move(transfer.RelocationSourceBuffer));
		AddIfNotNull(mRecordedDestinations.Textures, std::move(transfer.RelocationSourceTexture));
	}

	if(entry.IsUntracked)
		mUntrackedDestinationCount.fetch_sub(1, std::memory_order_relaxed);

	AddIfNotNull(mRecordedDestinations.Buffers, std::move(entry.Buffer));
	AddIfNotNull(mRecordedDestinations.Textures, std::move(entry.Texture));
	mQueuedTransfersByDestination.erase(iterEntry);
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
