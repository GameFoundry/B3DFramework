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
	void RecordBufferUpload(GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& source, const TShared<render::GpuBuffer>& destination, u32 destinationOffset, u32 length)
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
	void RecordTextureUpload(GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& source, const TShared<render::Texture>& destination, u32 mipLevel, u32 arrayLayer)
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
		mQueue->NotifyStagingBufferReleased(mEpoch);
		mQueue = nullptr;
	}
}

void GpuTransferQueue::AsyncRead::Complete(bool isExecuted)
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

GpuTransferQueue::ProxiesToRelease& GpuTransferQueue::ProxiesToRelease::operator=(ProxiesToRelease&& other) noexcept
{
	if(this == &other)
		return *this;

	for(const TShared<AsyncRead>& read : Reads)
		read->Complete(false);

	Buffers = std::move(other.Buffers);
	Textures = std::move(other.Textures);
	Reads = std::move(other.Reads);

	return *this;
}

GpuTransferQueue::ProxiesToRelease::~ProxiesToRelease()
{
	for(const TShared<AsyncRead>& read : Reads)
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
		ProxiesToRelease proxiesToRelease;
		{
			Lock lock(mMutex);
			while(!mTransferListsByResource.empty())
				DropTransferList(mTransferListsByResource.begin(), proxiesToRelease);
		}
	}

	mSubmitThreadProxiesToRelease = ProxiesToRelease();
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
		stagingEpoch.UnreleasedBufferCount++;
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
		NotifyStagingBufferReleased(epoch);
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

	ProxiesToRelease proxiesToRelease;
	Lock lock(mMutex);

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Under the mutex the command buffer this queue records into is never recorded but not submitted, see EndRecording(). Only the command
	// buffers of other threads count.
	if(destination->GetBoundCount() > destination->GetUseCount() && destination->IsRangeBound(destinationOffset, length))
	{
		B3D_LOG(Warning, LogRenderBackend, "Queuing an upload to a buffer '{0}' that a recorded command buffer which isn't submitted yet uses. The upload runs before that command buffer, so the command buffer sees it instead of the contents it was recorded after. Record the write into a command buffer to order it.", destination->GetName());
	}
#endif

	TransferList& transferList = FindOrAddTransferList(*destinationResource, destination);

	const u32 destinationEnd = destinationOffset + length;
	AppendUpload(transferList, std::move(upload), [destinationOffset, destinationEnd](const QueuedTransfer& queuedUpload)
	{
		return queuedUpload.Offset >= destinationOffset && queuedUpload.Offset + queuedUpload.Length <= destinationEnd;
	}, proxiesToRelease);
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

	ProxiesToRelease proxiesToRelease;
	Lock lock(mMutex);

#if B3D_BUILD_TYPE_DEVELOPMENT
	// See QueueUpload(GpuStagingBuffer&&, const TShared<render::GpuBuffer>&, u32, u32)
	const u32 subresourceIndex = destination->GetProperties().MapToSubresourceIndex(arrayLayer, mipLevel);
	if(destination->GetBoundCount(subresourceIndex) > destination->GetUseCount(subresourceIndex))
	{
		B3D_LOG(Warning, LogRenderBackend, "Queuing an upload to an image '{0}' that a recorded command buffer which isn't submitted yet uses. The upload runs before that command buffer, so the command buffer sees it instead of the contents it was recorded after. Record the write into a command buffer to order it.", destination->GetName());
	}
#endif

	TransferList& transferList = FindOrAddTransferList(*destinationResource, destination);

	// The copy overwrites the whole subresource
	AppendUpload(transferList, std::move(upload), [mipLevel, arrayLayer](const QueuedTransfer& queuedUpload)
	{
		return queuedUpload.MipLevel == mipLevel && queuedUpload.ArrayLayer == arrayLayer;
	}, proxiesToRelease);
}

template<class CoversFunction>
void GpuTransferQueue::AppendUpload(TransferList& transferList, QueuedTransfer&& upload, CoversFunction fnCovers, ProxiesToRelease& proxiesToRelease)
{
	// A read or a relocation queued in between needs the earlier uploads
	for(u32 transferIndex = (u32)transferList.Transfers.Size(); transferIndex-- > 0;)
	{
		QueuedTransfer& queuedTransfer = transferList.Transfers[transferIndex];
		if(queuedTransfer.Type != QueuedTransferType::Upload)
			break;

		if(!fnCovers(queuedTransfer))
			continue;

		DropTransfer(queuedTransfer, proxiesToRelease);
		transferList.Transfers.Erase(transferList.Transfers.Begin() + transferIndex);
	}

	transferList.Transfers.Add(std::move(upload));
}

void GpuTransferQueue::RecordUpload(GpuStagingBuffer&& source, const TShared<render::GpuBuffer>& destination, u32 destinationOffset, u32 length, GpuCommandBuffer& commandBuffer)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr))
		return;

	RecordBufferUpload(commandBuffer, source.mBuffer, destination, destinationOffset, length);

	Lock lock(mMutex);
	RetainRecordedStagingBuffer(std::move(source.mBuffer), source.mEpoch);
	source.mQueue = nullptr;
}

void GpuTransferQueue::RecordUpload(GpuStagingBuffer&& source, const TShared<render::Texture>& destination, u32 mipLevel, u32 arrayLayer, GpuCommandBuffer& commandBuffer)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr))
		return;

	RecordTextureUpload(commandBuffer, source.mBuffer, destination, mipLevel, arrayLayer);

	Lock lock(mMutex);
	RetainRecordedStagingBuffer(std::move(source.mBuffer), source.mEpoch);
	source.mQueue = nullptr;
}

TAsyncOp<TShared<MemoryDataStream>> GpuTransferQueue::QueueRead(const TShared<render::GpuBuffer>& source, u32 offset, u32 length)
{
	const TShared<AsyncRead> read = B3DMakeShared<AsyncRead>();
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
	transfer.Type = QueuedTransferType::CopyToStaging;
	transfer.Staging = read->Staging.GetBuffer();
	transfer.Offset = offset;
	transfer.Length = length;
	transfer.AsyncRead = read;

	IGpuResource* const sourceResource = AppendRead(source, std::move(transfer));
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
	const TShared<AsyncRead> read = B3DMakeShared<AsyncRead>();
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
	transfer.Type = QueuedTransferType::CopyToStaging;
	transfer.Staging = read->Staging.GetBuffer();
	transfer.MipLevel = mipLevel;
	transfer.ArrayLayer = arrayLayer;
	transfer.AsyncRead = read;

	IGpuResource* const sourceResource = AppendRead(source, std::move(transfer));
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
	transfer.Type = QueuedTransferType::CopyToStaging;
	transfer.Staging = destination.GetBuffer();
	transfer.Offset = offset;
	transfer.Length = length;

	if(AppendRead(source, std::move(transfer)) != nullptr)
		mDevice.GetSubmitThread().QueueReadSubmitAndWait(source);
}

void GpuTransferQueue::CopyToStagingAndWait(const TShared<render::Texture>& source, u32 mipLevel, u32 arrayLayer, const GpuStagingBuffer& destination)
{
	if(!B3D_ENSURE(source != nullptr && destination.mQueue == this))
		return;

	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::CopyToStaging;
	transfer.Staging = destination.GetBuffer();
	transfer.MipLevel = mipLevel;
	transfer.ArrayLayer = arrayLayer;

	if(AppendRead(source, std::move(transfer)) != nullptr)
		mDevice.GetSubmitThread().QueueReadSubmitAndWait(source);
}

void GpuTransferQueue::WaitUntilHostReadable(const TShared<render::GpuBuffer>& source)
{
	if(!B3D_ENSURE(source != nullptr))
		return;

	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::MakeHostReadable;

	if(AppendRead(source, std::move(transfer)) != nullptr)
		mDevice.GetSubmitThread().QueueReadSubmitAndWait(source);
}

void GpuTransferQueue::WaitUntilHostReadable(const TShared<render::Texture>& source, u32 mipLevel, u32 arrayLayer)
{
	if(!B3D_ENSURE(source != nullptr))
		return;

	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::MakeHostReadable;
	transfer.MipLevel = mipLevel;
	transfer.ArrayLayer = arrayLayer;

	if(AppendRead(source, std::move(transfer)) != nullptr)
		mDevice.GetSubmitThread().QueueReadSubmitAndWait(source);
}

IGpuResource* GpuTransferQueue::AppendRead(const TShared<render::GpuBuffer>& buffer, QueuedTransfer&& transfer)
{
	Lock lock(mMutex);

	IGpuResource* const resource = buffer->GetGpuResource();
	if(!B3D_ENSURE(resource != nullptr))
		return nullptr;

	transfer.Buffer = buffer;
	FindOrAddTransferList(*resource, buffer).Transfers.Add(std::move(transfer));

	return resource;
}

IGpuResource* GpuTransferQueue::AppendRead(const TShared<render::Texture>& texture, QueuedTransfer&& transfer)
{
	Lock lock(mMutex);

	IGpuResource* const resource = texture->GetGpuResource();
	if(!B3D_ENSURE(resource != nullptr))
		return nullptr;

	transfer.Texture = texture;
	FindOrAddTransferList(*resource, texture).Transfers.Add(std::move(transfer));

	return resource;
}

void GpuTransferQueue::QueueRelocation(const TShared<render::GpuBuffer>& resource, const TShared<render::GpuBuffer>& oldResourceHolder)
{
	if(!B3D_ENSURE(resource != nullptr && oldResourceHolder != nullptr))
		return;

	// The exchange and the move of the transfer list happen together, so a transfer queued concurrently goes either before the relocation,
	// to the old native buffer, or after it, to the new one
	Lock lock(mMutex);

	IGpuResource* const oldResource = resource->GetGpuResource();
	resource->Relocate(*oldResourceHolder);

	IGpuResource* const newResource = resource->GetGpuResource();
	if(!B3D_ENSURE(newResource != nullptr))
		return;

	QueuedTransfer relocation;
	relocation.Type = QueuedTransferType::Relocation;
	relocation.Buffer = resource;
	relocation.RelocationSourceBuffer = oldResourceHolder;

	MoveTransferListForRelocation(oldResource, *newResource, resource, oldResourceHolder).Transfers.Add(std::move(relocation));
}

void GpuTransferQueue::QueueRelocation(const TShared<render::Texture>& resource, const TShared<render::Texture>& oldResourceHolder)
{
	if(!B3D_ENSURE(resource != nullptr && oldResourceHolder != nullptr))
		return;

	// See QueueRelocation(const TShared<render::GpuBuffer>&, const TShared<render::GpuBuffer>&)
	Lock lock(mMutex);

	IGpuResource* const oldResource = resource->GetGpuResource();
	resource->Relocate(*oldResourceHolder);

	IGpuResource* const newResource = resource->GetGpuResource();
	if(!B3D_ENSURE(newResource != nullptr))
		return;

	QueuedTransfer relocation;
	relocation.Type = QueuedTransferType::Relocation;
	relocation.Texture = resource;
	relocation.RelocationSourceTexture = oldResourceHolder;

	MoveTransferListForRelocation(oldResource, *newResource, resource, oldResourceHolder).Transfers.Add(std::move(relocation));
}

template<class ProxyType>
GpuTransferQueue::TransferList& GpuTransferQueue::MoveTransferListForRelocation(IGpuResource* oldResource, IGpuResource& newResource,
	const TShared<ProxyType>& resource, const TShared<ProxyType>& oldResourceHolder)
{
	B3D_ASSERT(mTransferListsByResource.find(&newResource) == mTransferListsByResource.end());

	const auto iterOldTransferList = mTransferListsByResource.find(oldResource);
	if(iterOldTransferList == mTransferListsByResource.end())
		return FindOrAddTransferList(newResource, resource);

	TransferList transferList = std::move(iterOldTransferList->second);
	mTransferListsByResource.erase(iterOldTransferList);
	oldResource->mHasQueuedTransfers.store(false, std::memory_order_relaxed);

	// The transfers queued so far mean the old native resource, which the holder has now
	for(QueuedTransfer& transfer : transferList.Transfers)
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

	TransferList& newTransferList = AddTransferList(newResource);
	newTransferList = std::move(transferList);

	return newTransferList;
}

bool GpuTransferQueue::TryRecreateNativeResource(render::GpuBuffer& resource)
{
	ProxiesToRelease proxiesToRelease;
	Lock lock(mMutex);

	if(!TryDropTransferListForRecreate(resource.GetGpuResource(), proxiesToRelease))
		return false;

	resource.RecreateInternalBuffer();
	return true;
}

bool GpuTransferQueue::TryRecreateNativeResource(render::Texture& resource)
{
	ProxiesToRelease proxiesToRelease;
	Lock lock(mMutex);

	if(!TryDropTransferListForRecreate(resource.GetGpuResource(), proxiesToRelease))
		return false;

	resource.RecreateInternalTexture();
	return true;
}

bool GpuTransferQueue::TryDropTransferListForRecreate(IGpuResource* resource, ProxiesToRelease& proxiesToRelease)
{
	if(resource == nullptr || !resource->HasQueuedTransfers())
		return true;

	const auto iterTransferList = mTransferListsByResource.find(resource);
	if(iterTransferList == mTransferListsByResource.end())
		return true;

	// A read or a relocation needs the contents of the old native resource
	for(const QueuedTransfer& transfer : iterTransferList->second.Transfers)
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
	DropTransferList(iterTransferList, proxiesToRelease);

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

void GpuTransferQueue::SubmitAllQueuedTransfers()
{
	ProxiesToRelease proxiesToRelease;
	bool hasQueuedTransfers;
	{
		Lock lock(mMutex);
		hasQueuedTransfers = !mTransferListsByResource.empty();

		// Recorded on the submit thread, released here so their destruction doesn't run there
		proxiesToRelease = std::move(mSubmitThreadProxiesToRelease);
		mSubmitThreadProxiesToRelease = ProxiesToRelease();
	}

	if(hasQueuedTransfers)
		mDevice.GetSubmitThread().QueueAllTransfersSubmit();

	EndEpoch();
}

GpuTransferQueue::RecordedTransfers GpuTransferQueue::RecordQueuedTransfers(TArrayView<IGpuResource* const> resources)
{
	AssertIfNotSubmitThread();

	RecordedTransfers recorded;
	TArray<TShared<AsyncRead>> asyncReads;
	{
		Lock lock(mMutex);

		// Resources whose transfers were recorded earlier, or dropped, have no transfer list any more
		for(IGpuResource* resource : resources)
		{
			if(mTransferListsByResource.find(resource) != mTransferListsByResource.end())
				RecordTransferList(*resource, recorded, asyncReads);
		}

		RecordBindlessTransferLists(recorded, asyncReads);
		EndRecording(recorded);
	}

	CompleteAsyncReadsAfterExecution(recorded, std::move(asyncReads));
	return recorded;
}

GpuTransferQueue::RecordedTransfers GpuTransferQueue::RecordQueuedTransfers(const render::GpuBuffer& resource)
{
	AssertIfNotSubmitThread();

	RecordedTransfers recorded;
	TArray<TShared<AsyncRead>> asyncReads;
	{
		// The native resource is looked up under the mutex, so a concurrent relocation can't move the transfer list away in between
		Lock lock(mMutex);

		IGpuResource* const gpuResource = resource.GetGpuResource();
		if(gpuResource != nullptr && mTransferListsByResource.find(gpuResource) != mTransferListsByResource.end())
			RecordTransferList(*gpuResource, recorded, asyncReads);

		RecordBindlessTransferLists(recorded, asyncReads);
		EndRecording(recorded);
	}

	CompleteAsyncReadsAfterExecution(recorded, std::move(asyncReads));
	return recorded;
}

GpuTransferQueue::RecordedTransfers GpuTransferQueue::RecordQueuedTransfers(const render::Texture& resource)
{
	AssertIfNotSubmitThread();

	RecordedTransfers recorded;
	TArray<TShared<AsyncRead>> asyncReads;
	{
		// See RecordQueuedTransfers(const render::GpuBuffer&)
		Lock lock(mMutex);

		IGpuResource* const gpuResource = resource.GetGpuResource();
		if(gpuResource != nullptr && mTransferListsByResource.find(gpuResource) != mTransferListsByResource.end())
			RecordTransferList(*gpuResource, recorded, asyncReads);

		RecordBindlessTransferLists(recorded, asyncReads);
		EndRecording(recorded);
	}

	CompleteAsyncReadsAfterExecution(recorded, std::move(asyncReads));
	return recorded;
}

GpuTransferQueue::RecordedTransfers GpuTransferQueue::RecordAllQueuedTransfers()
{
	AssertIfNotSubmitThread();

	RecordedTransfers recorded;
	TArray<TShared<AsyncRead>> asyncReads;
	{
		Lock lock(mMutex);

		while(!mTransferListsByResource.empty())
			RecordTransferList(*mTransferListsByResource.begin()->first, recorded, asyncReads);

		EndRecording(recorded);
	}

	CompleteAsyncReadsAfterExecution(recorded, std::move(asyncReads));
	return recorded;
}

void GpuTransferQueue::RecordBindlessTransferLists(RecordedTransfers& recorded, TArray<TShared<AsyncRead>>& asyncReads)
{
	if(mBindlessTransferListCount.load(std::memory_order_relaxed) == 0)
		return;

	TInlineArray<IGpuResource*, 8> bindlessResources;
	for(const auto& [resource, transferList] : mTransferListsByResource)
	{
		if(transferList.IsBindless)
			bindlessResources.Add(resource);
	}

	for(IGpuResource* resource : bindlessResources)
		RecordTransferList(*resource, recorded, asyncReads);
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
	for(IGpuResource* resource : mResourcesInRecording)
		resource->mHasQueuedTransfers.store(false, std::memory_order_release);

	mResourcesInRecording.Clear();
}

void GpuTransferQueue::CompleteAsyncReadsAfterExecution(const RecordedTransfers& recorded, TArray<TShared<AsyncRead>>&& asyncReads)
{
	if(asyncReads.Empty())
		return;

	B3D_ASSERT(recorded.CommandBuffer != nullptr);

	auto fnOnCommandBufferCompleted = [asyncReads]()
	{
		for(const TShared<AsyncRead>& read : asyncReads)
			read->Complete(true);
	};

	auto fnOnCommandBufferDestroyed = [asyncReads](bool isSubmitted)
	{
		// A submitted command buffer completes the reads on completion instead
		if(isSubmitted)
			return;

		for(const TShared<AsyncRead>& read : asyncReads)
			read->Complete(false);
	};

	recorded.CommandBuffer->OnDidComplete.Connect(fnOnCommandBufferCompleted);
	recorded.CommandBuffer->OnDestroyed.Connect(fnOnCommandBufferDestroyed);
}

GpuTransferQueue::TransferList& GpuTransferQueue::FindOrAddTransferList(IGpuResource& resource, const TShared<render::GpuBuffer>& buffer)
{
	const auto iterTransferList = mTransferListsByResource.find(&resource);
	if(iterTransferList != mTransferListsByResource.end())
		return iterTransferList->second;

	TransferList& transferList = AddTransferList(resource);
	transferList.Buffer = buffer;
	transferList.IsBindless = buffer->GetInformation().Flags.IsSet(GpuBufferFlag::Bindless);

	if(transferList.IsBindless)
		mBindlessTransferListCount.fetch_add(1, std::memory_order_relaxed);

	return transferList;
}

GpuTransferQueue::TransferList& GpuTransferQueue::FindOrAddTransferList(IGpuResource& resource, const TShared<render::Texture>& texture)
{
	const auto iterTransferList = mTransferListsByResource.find(&resource);
	if(iterTransferList != mTransferListsByResource.end())
		return iterTransferList->second;

	TransferList& transferList = AddTransferList(resource);
	transferList.Texture = texture;
	transferList.IsBindless = texture->GetProperties().Usage.IsSet(TextureUsageFlag::Bindless);

	if(transferList.IsBindless)
		mBindlessTransferListCount.fetch_add(1, std::memory_order_relaxed);

	return transferList;
}

GpuTransferQueue::TransferList& GpuTransferQueue::AddTransferList(IGpuResource& resource)
{
	resource.mHasQueuedTransfers.store(true, std::memory_order_relaxed);
	return mTransferListsByResource[&resource];
}

void GpuTransferQueue::DropTransfer(QueuedTransfer& transfer, ProxiesToRelease& proxiesToRelease)
{
	// The staging buffer of a read is counted by the handle the reader holds
	if(transfer.Type == QueuedTransferType::Upload)
		GetEpoch(transfer.Epoch).UnreleasedBufferCount--;

	AddIfNotNull(proxiesToRelease.Buffers, std::move(transfer.Staging));
	AddIfNotNull(proxiesToRelease.Buffers, std::move(transfer.Buffer));
	AddIfNotNull(proxiesToRelease.Textures, std::move(transfer.Texture));
	AddIfNotNull(proxiesToRelease.Buffers, std::move(transfer.RelocationSourceBuffer));
	AddIfNotNull(proxiesToRelease.Textures, std::move(transfer.RelocationSourceTexture));

	if(transfer.AsyncRead != nullptr)
		proxiesToRelease.Reads.Add(std::move(transfer.AsyncRead));
}

void GpuTransferQueue::DropTransferList(UnorderedMap<IGpuResource*, TransferList>::iterator iterTransferList, ProxiesToRelease& proxiesToRelease)
{
	TransferList& transferList = iterTransferList->second;
	for(QueuedTransfer& transfer : transferList.Transfers)
		DropTransfer(transfer, proxiesToRelease);

	if(transferList.IsBindless)
		mBindlessTransferListCount.fetch_sub(1, std::memory_order_relaxed);

	AddIfNotNull(proxiesToRelease.Buffers, std::move(transferList.Buffer));
	AddIfNotNull(proxiesToRelease.Textures, std::move(transferList.Texture));

	mTransferListsByResource.erase(iterTransferList);
}

void GpuTransferQueue::RecordTransferList(IGpuResource& resource, RecordedTransfers& recorded, TArray<TShared<AsyncRead>>& asyncReads)
{
	const auto iterTransferList = mTransferListsByResource.find(&resource);
	B3D_ASSERT(iterTransferList != mTransferListsByResource.end());

	TransferList& transferList = iterTransferList->second;

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Every native resource change that goes through Write() or a relocation drops, keeps or moves the transfers queued to the old resource
	// first
	const IGpuResource* const currentResource = transferList.Buffer != nullptr ? transferList.Buffer->GetGpuResource() : transferList.Texture->GetGpuResource();
	if(currentResource != &resource)
	{
		B3D_LOG(Warning, LogRenderBackend, "The native resource of '{0}' changed while transfers to it were queued. The transfers are dropped.",
			transferList.Buffer != nullptr ? transferList.Buffer->GetName() : transferList.Texture->GetName());

		// The old native resource may be destroyed by now, so it isn't accessed. Its proxies are released on the render thread.
		DropTransferList(iterTransferList, mSubmitThreadProxiesToRelease);
		return;
	}
#endif

	// The flag stays set until the command buffer is queued for submission, see EndRecording()
	mResourcesInRecording.Add(&resource);

	if(recorded.CommandBuffer == nullptr)
	{
		GpuCommandBufferPool& commandBufferPool = mDevice.GetSubmitThread().GetCommandBufferPool(GQT_GRAPHICS);
		recorded.CommandBuffer = commandBufferPool.FindOrCreate(GpuCommandBufferCreateInformation::Create("Transfers"));
	}

	GpuCommandBuffer& commandBuffer = *recorded.CommandBuffer;
	for(QueuedTransfer& transfer : transferList.Transfers)
	{
		switch(transfer.Type)
		{
		case QueuedTransferType::Upload:
			if(transfer.Buffer != nullptr)
				RecordBufferUpload(commandBuffer, transfer.Staging, transfer.Buffer, transfer.Offset, transfer.Length);
			else
				RecordTextureUpload(commandBuffer, transfer.Staging, transfer.Texture, transfer.MipLevel, transfer.ArrayLayer);

			RetainRecordedStagingBuffer(std::move(transfer.Staging), transfer.Epoch);
			break;
		case QueuedTransferType::CopyToStaging:
			if(transfer.Buffer != nullptr)
				GpuBackendUtility::CopyToStagingBuffer(commandBuffer, transfer.Buffer, transfer.Offset, transfer.Length, transfer.Staging);
			else
				GpuBackendUtility::CopyToStagingBuffer(commandBuffer, transfer.Texture, transfer.MipLevel, transfer.ArrayLayer, transfer.Staging);

			recorded.HasReads = true;

			if(transfer.AsyncRead != nullptr)
				asyncReads.Add(std::move(transfer.AsyncRead));

			break;
		case QueuedTransferType::MakeHostReadable:
			if(transfer.Buffer != nullptr)
				RecordBufferHostRead(commandBuffer, transfer.Buffer);
			else
				RecordTextureHostRead(commandBuffer, transfer.Texture, transfer.MipLevel, transfer.ArrayLayer);

			recorded.HasReads = true;
			break;
		case QueuedTransferType::Relocation:
			if(transfer.Buffer != nullptr)
				RecordBufferRelocation(commandBuffer, transfer.RelocationSourceBuffer, transfer.Buffer);
			else
				RecordTextureRelocation(commandBuffer, transfer.RelocationSourceTexture, transfer.Texture);

			break;
		}

		AddIfNotNull(mSubmitThreadProxiesToRelease.Buffers, std::move(transfer.Staging));
		AddIfNotNull(mSubmitThreadProxiesToRelease.Buffers, std::move(transfer.Buffer));
		AddIfNotNull(mSubmitThreadProxiesToRelease.Textures, std::move(transfer.Texture));
		AddIfNotNull(mSubmitThreadProxiesToRelease.Buffers, std::move(transfer.RelocationSourceBuffer));
		AddIfNotNull(mSubmitThreadProxiesToRelease.Textures, std::move(transfer.RelocationSourceTexture));
	}

	if(transferList.IsBindless)
		mBindlessTransferListCount.fetch_sub(1, std::memory_order_relaxed);

	AddIfNotNull(mSubmitThreadProxiesToRelease.Buffers, std::move(transferList.Buffer));
	AddIfNotNull(mSubmitThreadProxiesToRelease.Textures, std::move(transferList.Texture));
	mTransferListsByResource.erase(iterTransferList);
}

void GpuTransferQueue::RetainRecordedStagingBuffer(TShared<render::GpuBuffer> source, u64 epoch)
{
	GetEpoch(epoch).RecordedUploadBuffers.Add(std::move(source));
}

void GpuTransferQueue::NotifyStagingBufferReleased(u64 epoch)
{
	Lock lock(mMutex);
	GetEpoch(epoch).UnreleasedBufferCount--;
}

void GpuTransferQueue::EndEpoch()
{
	ProxiesToRelease proxiesToRelease;
	Lock lock(mMutex);

	// Retire the pages the ending epoch allocated from, before the next epoch starts
	for(const auto& [memoryType, allocator] : mStagingAllocators)
	{
		if(allocator != nullptr)
			allocator->FreeAll();
	}

	mEpochs.emplace_back();
	mCurrentEpoch.fetch_add(1, std::memory_order_acq_rel);

	ReleaseCompleteEpochs(proxiesToRelease);

	for(const auto& [memoryType, allocator] : mStagingAllocators)
	{
		if(allocator != nullptr)
			allocator->ReclaimUnused();
	}
}

void GpuTransferQueue::ReleaseCompleteEpochs(ProxiesToRelease& proxiesToRelease)
{
	// A recorded staging buffer is unbound once the command buffers that copy from it complete, or are reset or destroyed unsubmitted
	for(StagingEpoch& epoch : mEpochs)
	{
		for(u32 bufferIndex = (u32)epoch.RecordedUploadBuffers.Size(); bufferIndex-- > 0;)
		{
			if(epoch.RecordedUploadBuffers[bufferIndex]->GetBoundCount() > 0)
				continue;

			proxiesToRelease.Buffers.Add(std::move(epoch.RecordedUploadBuffers[bufferIndex]));
			epoch.RecordedUploadBuffers.Erase(epoch.RecordedUploadBuffers.Begin() + bufferIndex);
			epoch.UnreleasedBufferCount--;
		}
	}

	// The last epoch is the current one, which hasn't ended
	u64 firstIncompleteEpoch = mFirstIncompleteEpoch.load(std::memory_order_relaxed);
	while(mEpochs.size() > 1 && mEpochs.front().UnreleasedBufferCount == 0)
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
