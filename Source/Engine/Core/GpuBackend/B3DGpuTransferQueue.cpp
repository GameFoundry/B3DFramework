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
#include "Image/B3DPixelUtility.h"
#include "Image/B3DTexture.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Staging bytes after which creating a staging buffer ends the epoch, so processes that never end a frame still reuse staging memory. */
	constexpr u64 kEpochStagingBudget = 64ull * 1024 * 1024;

	/**
	 * Returns true if @p length bytes of @p buffer at @p offset, copied to or from the start of @p staging, lie within both buffers. Logs an
	 * error otherwise.
	 */
	bool IsTransferRangeValid(const render::GpuBuffer& buffer, u32 offset, u32 length, const render::GpuBuffer& staging)
	{
		const bool isInBounds = (u64)offset + length <= buffer.GetTotalSize() && length <= staging.GetTotalSize();
		return B3D_ENSURE_LOG(isInBounds, "A transfer of buffer '{0}' exceeds the size of the buffer or of its staging buffer.", buffer.GetName());
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

	Staging.Release();
}

GpuTransferQueue::DroppedTransfers::~DroppedTransfers()
{
	for(const TShared<AsyncRead>& read : Reads)
		read->Complete(false);
}

GpuTransferQueue::GpuTransferQueue(GpuDevice& device)
	: mDevice(device), mEpochTracker(*this), mBatchFence(device.CreateTimelineFence())
{
	mEpochs.emplace_back();
}

GpuTransferQueue::~GpuTransferQueue()
{
	// The device is idle by now. The transfers are dropped before the epochs and the allocators their staging buffers live in.
	{
		DroppedTransfers droppedTransfers;
		Lock lock(mMutex);

		for(auto& [resource, transferList] : mTransferListsByResource)
			DropTransferList(transferList, droppedTransfers);

		for(TransferBatch& batch : mTakenBatches)
		{
			for(TransferList& transferList : batch.TransferLists)
				DropTransferList(transferList, droppedTransfers);
		}

		mTransferListsByResource.clear();
		mTakenBatches.clear();
	}

	for(StagingEpoch& epoch : mEpochs)
	{
		for(IGpuResource* buffer : epoch.RecordedUploadBuffers)
			buffer->NotifyUnpinned();
	}

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

	if(buffer == nullptr || buffer->GetGpuResource() == nullptr)
	{
		NotifyStagingBufferReleased(epoch);
		return {};
	}

	if(endEpoch)
		EndEpoch();

	return GpuStagingBuffer(*this, std::move(buffer), epoch);
}

GpuTransferQueue::TransferResourceInformation GpuTransferQueue::GetTransferResourceInformation(const render::GpuBuffer& buffer, GpuQueueType queueType)
{
	TransferResourceInformation information;
	information.Resource = buffer.GetGpuResource();
	information.Type = TransferResourceType::Buffer;
	information.RestingState = GpuBackendUtility::GetRestingState(buffer, queueType);
	information.IsBindless = buffer.GetInformation().Flags.IsSet(GpuBufferFlag::Bindless);

	return information;
}

GpuTransferQueue::TransferResourceInformation GpuTransferQueue::GetTransferResourceInformation(const render::Texture& texture, GpuQueueType queueType)
{
	const TextureProperties& properties = texture.GetProperties();

	TransferResourceInformation information;
	information.Resource = texture.GetGpuResource();
	information.Type = TransferResourceType::Image;
	information.RestingState = GpuBackendUtility::GetRestingState(texture, queueType);
	information.IsBindless = properties.Usage.IsSet(TextureUsageFlag::Bindless);
	information.Aspects = GpuBackendUtility::GetFormatAspects(properties.Format);
	information.ImageCopyInformation = TextureUtility::GetImageCopyInformation(texture, 0, 0);
	information.MipCount = properties.MipMapCount + 1;

	return information;
}

void GpuTransferQueue::QueueUpload(GpuStagingBuffer&& source, const TShared<render::GpuBuffer>& destination, u32 destinationOffset, u32 length)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr) || !IsTransferRangeValid(*destination, destinationOffset, length, *source.mBuffer))
		return;

	QueuedTransfer upload;
	upload.Offset = destinationOffset;
	upload.Length = length;

	DroppedTransfers droppedTransfers;
	{
		// Captured under the mutex, so a concurrent relocation can't move the transfer list away from the captured native resource
		Lock lock(mMutex);

		const TransferResourceInformation information = GetTransferResourceInformation(*destination, GQT_GRAPHICS);
		if(!B3D_ENSURE(information.Resource != nullptr))
			return;

#if B3D_BUILD_TYPE_DEVELOPMENT
		// Under the mutex the command buffer this queue records into is never recorded but not submitted, see RecordTakenTransfers(). Only the command
		// buffers of other threads count.
		if(destination->GetBoundCount() > destination->GetUseCount() && destination->IsRangeBound(destinationOffset, length))
		{
			B3D_LOG(Warning, LogRenderBackend, "Queuing an upload to a buffer '{0}' that a recorded command buffer which isn't submitted yet uses. The upload runs before that command buffer, so the command buffer sees it instead of the contents it was recorded after. Record the write into a command buffer to order it.", destination->GetName());
		}
#endif

		AppendUpload(information, source, std::move(upload), droppedTransfers);
	}

	// The queue pinned the native staging buffer, so the handle's proxy is released here, outside of the mutex
	source.mBuffer = nullptr;
}

void GpuTransferQueue::QueueUpload(GpuStagingBuffer&& source, const TShared<render::Texture>& destination, u32 mipLevel, u32 arrayLayer)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr))
		return;

	QueuedTransfer upload;
	if(!GpuCommandBuffer::TryGetBufferTextureCopyInformation(*source.mBuffer, *destination, 0, mipLevel, arrayLayer, upload.ImageCopy))
		return;

	DroppedTransfers droppedTransfers;
	{
		// See QueueUpload(GpuStagingBuffer&&, const TShared<render::GpuBuffer>&, u32, u32)
		Lock lock(mMutex);

		const TransferResourceInformation information = GetTransferResourceInformation(*destination, GQT_GRAPHICS);
		if(!B3D_ENSURE(information.Resource != nullptr))
			return;

#if B3D_BUILD_TYPE_DEVELOPMENT
		const u32 subresourceIndex = destination->GetProperties().MapToSubresourceIndex(arrayLayer, mipLevel);
		if(destination->GetBoundCount(subresourceIndex) > destination->GetUseCount(subresourceIndex))
		{
			B3D_LOG(Warning, LogRenderBackend, "Queuing an upload to an image '{0}' that a recorded command buffer which isn't submitted yet uses. The upload runs before that command buffer, so the command buffer sees it instead of the contents it was recorded after. Record the write into a command buffer to order it.", destination->GetName());
		}
#endif

		AppendUpload(information, source, std::move(upload), droppedTransfers);
	}

	// See QueueUpload(GpuStagingBuffer&&, const TShared<render::GpuBuffer>&, u32, u32)
	source.mBuffer = nullptr;
}

void GpuTransferQueue::AppendUpload(const TransferResourceInformation& information, GpuStagingBuffer& source, QueuedTransfer&& upload, DroppedTransfers& droppedTransfers)
{
	upload.Type = QueuedTransferType::Upload;
	upload.Source = source.mBuffer->GetGpuResource();
	upload.Destination = information.Resource;
	upload.Epoch = source.mEpoch;

	upload.Source->NotifyPinned();
	upload.Destination->NotifyPinned();

	// The transfer counts the staging buffer in its epoch from now on
	source.mQueue = nullptr;

	TransferList& transferList = FindOrAddTransferList(information);

	// A read or a relocation queued in between needs the earlier uploads
	for(u32 transferIndex = (u32)transferList.Transfers.Size(); transferIndex-- > 0;)
	{
		QueuedTransfer& queuedTransfer = transferList.Transfers[transferIndex];
		if(queuedTransfer.Type != QueuedTransferType::Upload)
			break;

		// An image upload overwrites its whole subresource
		const bool isCovered = information.Type == TransferResourceType::Buffer
			? queuedTransfer.Offset >= upload.Offset && queuedTransfer.Offset + queuedTransfer.Length <= upload.Offset + upload.Length
			: queuedTransfer.ImageCopy.MipLevel == upload.ImageCopy.MipLevel && queuedTransfer.ImageCopy.ArrayLayer == upload.ImageCopy.ArrayLayer;

		if(!isCovered)
			continue;

		DropTransfer(queuedTransfer, droppedTransfers);
		transferList.Transfers.Erase(transferList.Transfers.Begin() + transferIndex);
	}

	transferList.Transfers.Add(std::move(upload));
}

void GpuTransferQueue::RecordUpload(GpuStagingBuffer&& source, const TShared<render::GpuBuffer>& destination, u32 destinationOffset, u32 length, GpuCommandBuffer& commandBuffer)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr) || !IsTransferRangeValid(*destination, destinationOffset, length, *source.mBuffer))
		return;

	QueuedTransfer upload;
	upload.Offset = destinationOffset;
	upload.Length = length;

	RecordUpload(source, GetTransferResourceInformation(*destination, commandBuffer.GetQueueType()), std::move(upload), commandBuffer);
}

void GpuTransferQueue::RecordUpload(GpuStagingBuffer&& source, const TShared<render::Texture>& destination, u32 mipLevel, u32 arrayLayer, GpuCommandBuffer& commandBuffer)
{
	if(!B3D_ENSURE(source.mQueue == this && destination != nullptr))
		return;

	QueuedTransfer upload;
	if(!GpuCommandBuffer::TryGetBufferTextureCopyInformation(*source.mBuffer, *destination, 0, mipLevel, arrayLayer, upload.ImageCopy))
		return;

	RecordUpload(source, GetTransferResourceInformation(*destination, commandBuffer.GetQueueType()), std::move(upload), commandBuffer);
}

void GpuTransferQueue::RecordUpload(GpuStagingBuffer& source, const TransferResourceInformation& information, QueuedTransfer&& upload, GpuCommandBuffer& commandBuffer)
{
	if(!B3D_ENSURE(information.Resource != nullptr))
		return;

	upload.Type = QueuedTransferType::Upload;
	upload.Source = source.mBuffer->GetGpuResource();
	upload.Destination = information.Resource;
	RecordTransfer(commandBuffer, information, upload);

	// The staging buffer stays pinned until the copy has executed, so its epoch knows when to reuse its memory
	upload.Source->NotifyPinned();
	{
		Lock lock(mMutex);
		RetainRecordedStagingBuffer(*upload.Source, source.mEpoch);
	}

	source.mQueue = nullptr;
	source.mBuffer = nullptr;
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

	const render::GpuBuffer& staging = *read->Staging.GetBuffer();
	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::CopyToStaging;
	transfer.Destination = staging.GetGpuResource();
	transfer.StagingRestingState = GpuBackendUtility::GetRestingState(staging, GQT_GRAPHICS);
	transfer.Offset = offset;
	transfer.Length = length;
	transfer.AsyncRead = read;

	const u64 batch = AppendReadAndTake(*source, std::move(transfer));
	if(batch == 0)
	{
		read->Complete(false);
		return read->BufferOperation;
	}

	mDevice.GetSubmitThread().QueueTransferSubmit(batch);
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

	const render::GpuBuffer& staging = *read->Staging.GetBuffer();
	QueuedTransfer transfer;
	transfer.Type = QueuedTransferType::CopyToStaging;
	transfer.Destination = staging.GetGpuResource();
	transfer.StagingRestingState = GpuBackendUtility::GetRestingState(staging, GQT_GRAPHICS);
	transfer.AsyncRead = read;

	u64 batch = 0;
	if(GpuCommandBuffer::TryGetBufferTextureCopyInformation(staging, *source, 0, mipLevel, arrayLayer, transfer.ImageCopy))
		batch = AppendReadAndTake(*source, std::move(transfer));

	if(batch == 0)
	{
		read->Complete(false);
		return read->TextureOperation;
	}

	mDevice.GetSubmitThread().QueueTransferSubmit(batch);
	return read->TextureOperation;
}

void GpuTransferQueue::CopyToStagingAndWait(const TShared<render::GpuBuffer>& source, u32 offset, u32 length, const GpuStagingBuffer& destination)
{
	if(!B3D_ENSURE(source != nullptr && destination.mQueue == this) || !IsTransferRangeValid(*source, offset, length, *destination.GetBuffer()))
		return;

	const render::GpuBuffer& staging = *destination.GetBuffer();
	QueuedTransfer read;
	read.Type = QueuedTransferType::CopyToStaging;
	read.Destination = staging.GetGpuResource();
	read.StagingRestingState = GpuBackendUtility::GetRestingState(staging, GQT_GRAPHICS);
	read.Offset = offset;
	read.Length = length;

	SubmitAndWait(AppendReadAndTake(*source, std::move(read)));
}

void GpuTransferQueue::CopyToStagingAndWait(const TShared<render::Texture>& source, u32 mipLevel, u32 arrayLayer, const GpuStagingBuffer& destination)
{
	if(!B3D_ENSURE(source != nullptr && destination.mQueue == this))
		return;

	const render::GpuBuffer& staging = *destination.GetBuffer();
	QueuedTransfer read;
	read.Type = QueuedTransferType::CopyToStaging;
	read.Destination = staging.GetGpuResource();
	read.StagingRestingState = GpuBackendUtility::GetRestingState(staging, GQT_GRAPHICS);
	if(!GpuCommandBuffer::TryGetBufferTextureCopyInformation(staging, *source, 0, mipLevel, arrayLayer, read.ImageCopy))
		return;

	SubmitAndWait(AppendReadAndTake(*source, std::move(read)));
}

void GpuTransferQueue::WaitUntilHostReadable(const TShared<render::GpuBuffer>& source)
{
	if(!B3D_ENSURE(source != nullptr))
		return;

	QueuedTransfer read;
	read.Type = QueuedTransferType::MakeHostReadable;

	SubmitAndWait(AppendReadAndTake(*source, std::move(read)));
}

void GpuTransferQueue::WaitUntilHostReadable(const TShared<render::Texture>& source, u32 mipLevel, u32 arrayLayer)
{
	if(!B3D_ENSURE(source != nullptr))
		return;

	QueuedTransfer read;
	read.Type = QueuedTransferType::MakeHostReadable;
	read.ImageCopy.MipLevel = mipLevel;
	read.ImageCopy.ArrayLayer = arrayLayer;

	SubmitAndWait(AppendReadAndTake(*source, std::move(read)));
}

template<class ProxyType>
u64 GpuTransferQueue::AppendReadAndTake(const ProxyType& resource, QueuedTransfer&& read)
{
	// Captured under the mutex, so a concurrent relocation can't move the transfer list away from the captured native resource
	Lock lock(mMutex);

	const TransferResourceInformation information = GetTransferResourceInformation(resource, GQT_GRAPHICS);
	if(!B3D_ENSURE(information.Resource != nullptr))
		return 0;

	read.Source = information.Resource;
	read.Source->NotifyPinned();

	if(read.Destination != nullptr)
		read.Destination->NotifyPinned();

	FindOrAddTransferList(information).Transfers.Add(std::move(read));
	return TakeTransferLists(TArrayView<IGpuResource* const>(&information.Resource, 1));
}

void GpuTransferQueue::SubmitAndWait(u64 batch)
{
	if(batch == 0)
		return;

	// The fence reaches the batch number once the batch has executed, whichever submission recorded it
	GpuSubmitThread& submitThread = mDevice.GetSubmitThread();
	submitThread.QueueTransferSubmit(batch);
	submitThread.WaitUntilFence(*mDevice.GetQueue(GQT_GRAPHICS, 0), mBatchFence, batch);
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

	const TransferResourceInformation information = GetTransferResourceInformation(*resource, GQT_GRAPHICS);
	if(B3D_ENSURE(oldResource != nullptr && information.Resource != nullptr))
		AppendRelocation(*oldResource, information, resource->GetTotalSize());
}

void GpuTransferQueue::QueueRelocation(const TShared<render::Texture>& resource, const TShared<render::Texture>& oldResourceHolder)
{
	if(!B3D_ENSURE(resource != nullptr && oldResourceHolder != nullptr))
		return;

	// See QueueRelocation(const TShared<render::GpuBuffer>&, const TShared<render::GpuBuffer>&)
	Lock lock(mMutex);

	IGpuResource* const oldResource = resource->GetGpuResource();
	resource->Relocate(*oldResourceHolder);

	const TransferResourceInformation information = GetTransferResourceInformation(*resource, GQT_GRAPHICS);
	if(B3D_ENSURE(oldResource != nullptr && information.Resource != nullptr))
		AppendRelocation(*oldResource, information, 0);
}

void GpuTransferQueue::AppendRelocation(IGpuResource& oldResource, const TransferResourceInformation& information, u32 length)
{
	B3D_ASSERT(mTransferListsByResource.find(information.Resource) == mTransferListsByResource.end());

	QueuedTransfer relocation;
	relocation.Type = QueuedTransferType::Relocation;
	relocation.Source = &oldResource;
	relocation.Destination = information.Resource;
	relocation.Length = length;

	relocation.Source->NotifyPinned();
	relocation.Destination->NotifyPinned();

	// The transfers queued so far keep writing and reading the old native resource, before the copy. The list moves to the new native
	// resource, so the transfers queued from now on follow the copy.
	TInlineArray<QueuedTransfer, 1> transfers;
	const auto iterOldTransferList = mTransferListsByResource.find(&oldResource);
	if(iterOldTransferList != mTransferListsByResource.end())
	{
		TransferList& oldTransferList = iterOldTransferList->second;
		transfers = std::move(oldTransferList.Transfers);

		if(oldTransferList.Information.IsBindless)
			mBindlessTransferListCount.fetch_sub(1, std::memory_order_relaxed);

		oldResource.NotifyTransfersDequeued();
		oldResource.NotifyUnpinned();
		mTransferListsByResource.erase(iterOldTransferList);
	}

	transfers.Add(std::move(relocation));

	TransferList& transferList = FindOrAddTransferList(information);
	transferList.Transfers = std::move(transfers);
}

bool GpuTransferQueue::TryRecreateNativeResource(render::GpuBuffer& resource)
{
	DroppedTransfers droppedTransfers;
	Lock lock(mMutex);

	if(!TryDropTransferListForRecreate(resource.GetGpuResource(), droppedTransfers))
		return false;

	resource.RecreateInternalBuffer();
	return true;
}

bool GpuTransferQueue::TryRecreateNativeResource(render::Texture& resource)
{
	DroppedTransfers droppedTransfers;
	Lock lock(mMutex);

	if(!TryDropTransferListForRecreate(resource.GetGpuResource(), droppedTransfers))
		return false;

	resource.RecreateInternalTexture();
	return true;
}

bool GpuTransferQueue::TryDropTransferListForRecreate(IGpuResource* resource, DroppedTransfers& droppedTransfers)
{
	if(resource == nullptr || !resource->HasQueuedTransfers())
		return true;

	// Transfers taken into a batch keep the old native resource, so they don't hold back the recreate
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
	// tracks the old native resource, and takes the uploads queued to it when it is submitted. Recreating the resource would drop those
	// uploads, so the command buffer wouldn't see them.
	if(resource->GetBoundCount() > resource->GetUseCount())
		return false;

	// The uploads would only write the old native resource, which nothing reads any more
	DropTransferList(iterTransferList->second, droppedTransfers);
	mTransferListsByResource.erase(iterTransferList);

	return true;
}

void GpuTransferQueue::SubmitQueuedTransfers(const render::GpuBuffer& destination)
{
	SubmitQueuedTransfers(destination.GetGpuResource());
}

void GpuTransferQueue::SubmitQueuedTransfers(const render::Texture& destination)
{
	SubmitQueuedTransfers(destination.GetGpuResource());
}

void GpuTransferQueue::SubmitQueuedTransfers(IGpuResource* resource)
{
	if(resource == nullptr || !resource->HasQueuedTransfers())
		return;

	const u64 batch = TakeQueuedTransfers(TArrayView<IGpuResource* const>(&resource, 1));
	if(batch != 0)
		mDevice.GetSubmitThread().QueueTransferSubmit(batch);
}

void GpuTransferQueue::SubmitAllQueuedTransfers()
{
	const u64 batch = TakeAllQueuedTransfers();
	if(batch != 0)
		mDevice.GetSubmitThread().QueueTransferSubmit(batch);

	EndEpoch();
}

u64 GpuTransferQueue::TakeQueuedTransfers(TArrayView<IGpuResource* const> resources)
{
	Lock lock(mMutex);
	return TakeTransferLists(resources);
}

u64 GpuTransferQueue::TakeAllQueuedTransfers()
{
	Lock lock(mMutex);

	TArray<IGpuResource*> resources;
	for(const auto& [resource, transferList] : mTransferListsByResource)
		resources.Add(resource);

	return TakeTransferLists(resources);
}

u64 GpuTransferQueue::TakeTransferLists(TArrayView<IGpuResource* const> resources)
{
	// A taken list's transfers stay queued on its resource, and the list stays in the count of bindless lists, until it is recorded
	TransferBatch batch;

	// Resources whose transfers were taken earlier, or dropped, have no transfer list any more
	for(IGpuResource* resource : resources)
	{
		const auto iterTransferList = mTransferListsByResource.find(resource);
		if(iterTransferList == mTransferListsByResource.end() || iterTransferList->second.Information.IsBindless)
			continue;

		batch.TransferLists.Add(std::move(iterTransferList->second));
		mTransferListsByResource.erase(iterTransferList);
	}

	// TODO - We must refactor upload queue so that it does not need to flush all bindless resources. We will likely have a separate async upload
	// path for initial data uploads only, in which case we could just assert here that a bindless resource is not being transferred
	if(mBindlessTransferListCount.load(std::memory_order_relaxed) > 0)
	{
		for(auto iterTransferList = mTransferListsByResource.begin(); iterTransferList != mTransferListsByResource.end();)
		{
			if(!iterTransferList->second.Information.IsBindless)
			{
				++iterTransferList;
				continue;
			}

			batch.TransferLists.Add(std::move(iterTransferList->second));
			iterTransferList = mTransferListsByResource.erase(iterTransferList);
		}
	}

	// A batch taken earlier that isn't recorded yet may hold transfers to the resources, and must run first
	if(batch.TransferLists.Empty())
		return mLastTakenBatch > mLastRecordedBatch ? mLastTakenBatch : 0;

	batch.Number = ++mLastTakenBatch;
	mTakenBatches.push_back(std::move(batch));

	return mLastTakenBatch;
}

GpuTransferQueue::RecordedTransfers GpuTransferQueue::RecordTakenTransfers(u64 lastBatch)
{
	AssertIfNotSubmitThread();

	RecordedTransfers recorded;
	TArray<TShared<AsyncRead>> asyncReads;
	{
		Lock lock(mMutex);

		// Resources of the recorded transfer lists, whose transfers are dequeued and pins released once the command buffer is queued for
		// submission, and the other resources the recorded transfers pinned
		TArray<IGpuResource*> recordedResources;
		TArray<IGpuResource*> pinnedResources;

		// Record the batches in the order they were taken
		while(!mTakenBatches.empty() && mTakenBatches.front().Number <= lastBatch)
		{
			TransferBatch& batch = mTakenBatches.front();
			for(TransferList& transferList : batch.TransferLists)
			{
				if(recorded.CommandBuffer == nullptr)
				{
					GpuCommandBufferPool& commandBufferPool = mDevice.GetSubmitThread().GetCommandBufferPool(GQT_GRAPHICS);
					recorded.CommandBuffer = commandBufferPool.FindOrCreate(GpuCommandBufferCreateInformation::Create("Transfers"));
				}

				const TransferResourceInformation& information = transferList.Information;
				for(QueuedTransfer& transfer : transferList.Transfers)
				{
					RecordTransfer(*recorded.CommandBuffer, information, transfer);

					// The command buffer references the resources from now on, so their pins are released once it is queued for submission.
					// An upload's staging buffer stays pinned until the copy has executed, so its epoch knows when to reuse its memory.
					if(transfer.Type == QueuedTransferType::Upload)
						RetainRecordedStagingBuffer(*transfer.Source, transfer.Epoch);
					else
						pinnedResources.Add(transfer.Source);

					if(transfer.Destination != nullptr)
						pinnedResources.Add(transfer.Destination);

					if(transfer.AsyncRead != nullptr)
						asyncReads.Add(std::move(transfer.AsyncRead));
				}

				if(information.IsBindless)
				{
					mBindlessTransferListCount.fetch_sub(1, std::memory_order_relaxed);

					// Command buffers don't track what they access through the bindless heap, so the resource's submission stamps can't
					// tell which queues still use it. Waits for the latest work of every queue instead.
					// TODO - Remove once bindless resources only get initial uploads, on their own path
					recorded.CommandBuffer->AddQueueSyncMask(GpuQueueMask::kAll);
				}

				recordedResources.Add(information.Resource);
			}

			mLastRecordedBatch = batch.Number;
			mTakenBatches.pop_front();
		}

		if(recorded.CommandBuffer == nullptr)
			return recorded;

		recorded.SignalFence.Fence = mBatchFence;
		recorded.SignalFence.Value = mLastRecordedBatch;

		// End the command buffer and queue it for submission. Recording bound the resources. Marking them used under the same mutex means
		// QueueUpload(), which also holds it, never sees this command buffer as recorded but not submitted. A write queued now lands in a
		// later command buffer, so this one never sees it.
		recorded.CommandBuffer->End();
		recorded.CommandBuffer->NotifyWillQueueForSubmit(mDevice.GetQueue(GQT_GRAPHICS, 0)->GetId(), recorded.CommandBuffer->GetQueueSyncMask());

		// Dequeue the transfers only now, so a writer that sees no queued transfers also sees the resource as used, and doesn't map it while
		// the copy is pending. The command buffer keeps the resources alive from now on.
		for(IGpuResource* resource : recordedResources)
		{
			resource->NotifyTransfersDequeued();
			resource->NotifyUnpinned();
		}

		for(IGpuResource* resource : pinnedResources)
			resource->NotifyUnpinned();
	}

	// Complete the reads queued by QueueRead() once the command buffer has executed, or with null if it is destroyed without being submitted
	if(!asyncReads.Empty())
	{
		auto fnOnCommandBufferCompleted = [asyncReads]()
		{
			for(const TShared<AsyncRead>& read : asyncReads)
				read->Complete(true);
		};

		auto fnOnCommandBufferDestroyed = [asyncReads](bool isSubmitted)
		{
			if(isSubmitted)
				return;

			for(const TShared<AsyncRead>& read : asyncReads)
				read->Complete(false);
		};

		recorded.CommandBuffer->OnDidComplete.Connect(fnOnCommandBufferCompleted);
		recorded.CommandBuffer->OnDestroyed.Connect(fnOnCommandBufferDestroyed);
	}

	return recorded;
}

GpuTransferQueue::TransferList& GpuTransferQueue::FindOrAddTransferList(const TransferResourceInformation& information)
{
	const auto [iterTransferList, inserted] = mTransferListsByResource.try_emplace(information.Resource);
	TransferList& transferList = iterTransferList->second;
	if(!inserted)
		return transferList;

	transferList.Information = information;
	information.Resource->NotifyPinned();
	information.Resource->NotifyTransfersQueued();

	if(information.IsBindless)
		mBindlessTransferListCount.fetch_add(1, std::memory_order_relaxed);

	return transferList;
}

void GpuTransferQueue::DropTransfer(QueuedTransfer& transfer, DroppedTransfers& droppedTransfers)
{
	// The staging buffer of a read is counted by the handle the reader holds
	if(transfer.Type == QueuedTransferType::Upload)
		GetEpoch(transfer.Epoch).UnreleasedBufferCount--;

	transfer.Source->NotifyUnpinned();
	if(transfer.Destination != nullptr)
		transfer.Destination->NotifyUnpinned();

	if(transfer.AsyncRead != nullptr)
		droppedTransfers.Reads.Add(std::move(transfer.AsyncRead));
}

void GpuTransferQueue::DropTransferList(TransferList& transferList, DroppedTransfers& droppedTransfers)
{
	for(QueuedTransfer& transfer : transferList.Transfers)
		DropTransfer(transfer, droppedTransfers);

	const TransferResourceInformation& information = transferList.Information;
	if(information.IsBindless)
		mBindlessTransferListCount.fetch_sub(1, std::memory_order_relaxed);

	information.Resource->NotifyTransfersDequeued();
	information.Resource->NotifyUnpinned();
}

void GpuTransferQueue::RecordTransfer(GpuCommandBuffer& commandBuffer, const TransferResourceInformation& information, const QueuedTransfer& transfer)
{
	const bool isImage = information.Type == TransferResourceType::Image;
	const GpuAccessState& restingState = information.RestingState;

	// Layouts only apply to the barriers of images
	const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
	const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
	const GpuTextureSubresourceRange subresourceRange(transfer.ImageCopy.MipLevel, 1, transfer.ImageCopy.ArrayLayer, 1, information.Aspects);

	TInlineArray<GpuBufferResourceBarrier, 2> bufferBarriers;
	TInlineArray<GpuImageResourceBarrier, 2> imageBarriers;

	// Adds a barrier of @p resource, the resource the transfer list is for or its old native resource
	const auto fnAddBarrier = [&](IGpuResource& resource, const GpuAccessState& source, const GpuAccessState& destination, const GpuTextureSubresourceRange& range)
	{
		if(isImage)
			imageBarriers.Add(GpuImageResourceBarrier(static_cast<IGpuImageResource&>(resource), source, destination, range));
		else
			bufferBarriers.Add(GpuBufferResourceBarrier(static_cast<IGpuBufferResource&>(resource), source, destination));
	};

	const auto fnIssueBarriers = [&]()
	{
		commandBuffer.IssueBarriers(GpuResourceBarriers(bufferBarriers, imageBarriers));
		bufferBarriers.Clear();
		imageBarriers.Clear();
	};

	switch(transfer.Type)
	{
	case QueuedTransferType::Upload:
	{
		// An image upload overwrites its whole subresource, so the old contents are discarded
		IGpuResource& destination = *transfer.Destination;
		GpuAccessState discardedRestingState = restingState;
		discardedRestingState.Layout = GpuImageLayout::Undefined;

		fnAddBarrier(destination, discardedRestingState, transferWrite, subresourceRange);
		fnIssueBarriers();

		IGpuBufferResource& source = static_cast<IGpuBufferResource&>(*transfer.Source);
		if(isImage)
			commandBuffer.CopyBufferToTexture(source, static_cast<IGpuImageResource&>(destination), transfer.ImageCopy);
		else
		{
			const GpuBufferCopyRange range{ 0, transfer.Offset, transfer.Length };
			commandBuffer.CopyBufferToBuffer(source, static_cast<IGpuBufferResource&>(destination), TArrayView<const GpuBufferCopyRange>(&range, 1));
		}

		fnAddBarrier(destination, transferWrite, restingState, subresourceRange);
		fnIssueBarriers();
		break;
	}
	case QueuedTransferType::CopyToStaging:
	{
		IGpuResource& source = *transfer.Source;
		IGpuBufferResource& staging = static_cast<IGpuBufferResource&>(*transfer.Destination);

		fnAddBarrier(source, restingState, transferRead, subresourceRange);
		bufferBarriers.Add(GpuBufferResourceBarrier(staging, transfer.StagingRestingState, transferWrite));
		fnIssueBarriers();

		if(isImage)
			commandBuffer.CopyTextureToBuffer(static_cast<IGpuImageResource&>(source), staging, transfer.ImageCopy);
		else
		{
			const GpuBufferCopyRange range{ transfer.Offset, 0, transfer.Length };
			commandBuffer.CopyBufferToBuffer(static_cast<IGpuBufferResource&>(source), staging, TArrayView<const GpuBufferCopyRange>(&range, 1));
		}

		fnAddBarrier(source, transferRead, restingState, subresourceRange);
		bufferBarriers.Add(GpuBufferResourceBarrier(staging, transferWrite, transfer.StagingRestingState));
		fnIssueBarriers();
		break;
	}
	case QueuedTransferType::MakeHostReadable:
	{
		// An image keeps its resting layout
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read, restingState.Layout);

		fnAddBarrier(*transfer.Source, restingState, hostRead, subresourceRange);
		fnIssueBarriers();
		fnAddBarrier(*transfer.Source, hostRead, restingState, subresourceRange);
		fnIssueBarriers();
		break;
	}
	case QueuedTransferType::Relocation:
	{
		// The copy overwrites every subresource of the new native resource, so its old contents are discarded
		IGpuResource& source = *transfer.Source;
		IGpuResource& destination = *transfer.Destination;
		const GpuTextureSubresourceRange allSubresources = GpuTextureSubresourceRange::AllSubresources();

		GpuAccessState discardedRestingState = restingState;
		discardedRestingState.Layout = GpuImageLayout::Undefined;

		fnAddBarrier(source, restingState, transferRead, allSubresources);
		fnAddBarrier(destination, discardedRestingState, transferWrite, allSubresources);
		fnIssueBarriers();

		if(isImage)
		{
			// Each copy covers one mip level of every array layer
			GpuImageCopyInformation mipInformation = information.ImageCopyInformation;
			for(u32 mipLevel = 0; mipLevel < information.MipCount; mipLevel++)
			{
				mipInformation.MipLevel = mipLevel;
				const GpuImageCopyInformation& baseInformation = information.ImageCopyInformation;
				PixelUtility::GetSizeForMipLevel(baseInformation.Width, baseInformation.Height, baseInformation.Depth, mipLevel, mipInformation.Width,
					mipInformation.Height, mipInformation.Depth);

				commandBuffer.CopyTexture(static_cast<IGpuImageResource&>(source), static_cast<IGpuImageResource&>(destination), mipInformation);
			}
		}
		else
		{
			const GpuBufferCopyRange range{ 0, 0, transfer.Length };
			commandBuffer.CopyBufferToBuffer(static_cast<IGpuBufferResource&>(source), static_cast<IGpuBufferResource&>(destination),
				TArrayView<const GpuBufferCopyRange>(&range, 1));
		}

		fnAddBarrier(destination, transferWrite, restingState, allSubresources);
		fnIssueBarriers();
		break;
	}
	}
}

void GpuTransferQueue::RetainRecordedStagingBuffer(IGpuResource& source, u64 epoch)
{
	GetEpoch(epoch).RecordedUploadBuffers.Add(&source);
}

void GpuTransferQueue::NotifyStagingBufferReleased(u64 epoch)
{
	Lock lock(mMutex);
	GetEpoch(epoch).UnreleasedBufferCount--;
}

void GpuTransferQueue::EndEpoch()
{
	Lock lock(mMutex);

	// Retire the pages the ending epoch allocated from, before the next epoch starts
	for(const auto& [memoryType, allocator] : mStagingAllocators)
	{
		if(allocator != nullptr)
			allocator->FreeAll();
	}

	mEpochs.emplace_back();
	mCurrentEpoch.fetch_add(1, std::memory_order_acq_rel);

	// Unpin the recorded staging buffers no command buffer references any more. A buffer is unbound once the command buffers that copy from
	// it complete, or are reset or destroyed unsubmitted. Its proxy is gone, so unpinning it destroys it.
	for(StagingEpoch& epoch : mEpochs)
	{
		for(u32 bufferIndex = (u32)epoch.RecordedUploadBuffers.Size(); bufferIndex-- > 0;)
		{
			IGpuResource* const buffer = epoch.RecordedUploadBuffers[bufferIndex];
			if(buffer->GetBoundCount() > 0)
				continue;

			buffer->NotifyUnpinned();
			epoch.RecordedUploadBuffers.Erase(epoch.RecordedUploadBuffers.Begin() + bufferIndex);
			epoch.UnreleasedBufferCount--;
		}
	}

	// Forget complete epochs. The last epoch is the current one, which hasn't ended.
	u64 firstIncompleteEpoch = mFirstIncompleteEpoch.load(std::memory_order_relaxed);
	while(mEpochs.size() > 1 && mEpochs.front().UnreleasedBufferCount == 0)
	{
		mEpochs.pop_front();
		firstIncompleteEpoch++;
	}

	mFirstIncompleteEpoch.store(firstIncompleteEpoch, std::memory_order_release);

	for(const auto& [memoryType, allocator] : mStagingAllocators)
	{
		if(allocator != nullptr)
			allocator->ReclaimUnused();
	}
}

GpuTransferQueue::StagingEpoch& GpuTransferQueue::GetEpoch(u64 epoch)
{
	const u64 epochIndex = epoch - mFirstIncompleteEpoch.load(std::memory_order_relaxed);
	B3D_ASSERT(epoch >= mFirstIncompleteEpoch.load(std::memory_order_relaxed) && epochIndex < mEpochs.size());

	return mEpochs[epochIndex];
}
