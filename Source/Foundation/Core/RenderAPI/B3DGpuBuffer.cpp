//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DGpuBuffer.h"
#include "B3DApplication.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuDevice.h"
#include "B3DGpuDeviceCapabilities.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "FileSystem/B3DDataStream.h"

using namespace b3d;

static u32 CalculateUnalignedGpuBufferSize(const GpuBufferInformation& information)
{
	switch(information.Type)
	{
	case GpuBufferType::Vertex:
		return information.Vertex.Count * information.Vertex.ElementSize;
	case GpuBufferType::Index:
		return information.Index.Count * (GpuBuffer::GetIndexSize(information.Index.Type));
	case GpuBufferType::Uniform: 
		return information.Uniform.Size;
	case GpuBufferType::SimpleStorage:
		return information.SimpleStorage.Count * GpuBuffer::GetFormatSize(information.SimpleStorage.Format);
	case GpuBufferType::StructuredStorage: 
		return information.StructuredStorage.Count * information.StructuredStorage.ElementSize;
	case GpuBufferType::StagingRead:
	case GpuBufferType::StagingWrite:
		return information.Staging.Size;
	}

	B3D_ENSURE(false);
	return 128;
}

GpuBuffer::GpuBuffer(const GpuBufferCreateInformation& createInformation)
	: mInformation(createInformation)
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	mSuballocationSize = CalculateSuballocatedBufferSize(createInformation, gpuDevice);
	mTotalSize = CalculateTotalBufferSize(createInformation, gpuDevice);
}

void GpuBuffer::Initialize()
{
	if(mInformation.Flags.IsSet(GpuBufferFlag::AllowWriteCachingOnCPU))
	{
		mCache = (u8*)B3DAllocate(mTotalSize);
	}
}

void GpuBuffer::Destroy()
{
	if(mCache != nullptr)
	{
		B3DFree(mCache);
	}
}

void GpuBuffer::WriteCached(u32 offset, u32 length, const void* source)
{
	if(!B3D_ENSURE(mCache != nullptr))
		return;

	if(!B3D_ENSURE((offset + length) <= mTotalSize))
		return;

	memcpy(mCache + offset, source, length);
	MarkRenderProxyDataDirty();
}

u32 GpuBuffer::WriteCachedType(u32 offset, const GpuDataParameterTypeInformation& typeInformation, const void* source)
{
	const u8* value = (const u8*)source;

	const u32 startOffset = offset;
	for(u32 row = 0; row < typeInformation.NumRows; ++row)
	{
		const u32 rowSize = typeInformation.NumColumns * typeInformation.BaseTypeSize;
		WriteCached(offset, rowSize, value);

		offset += typeInformation.Alignment;
		value += rowSize;
	}

	return offset - startOffset;
}

void GpuBuffer::ZeroOutCached(u32 offset, u32 length)
{
	if(!B3D_ENSURE(mCache != nullptr))
		return;

	if(!B3D_ENSURE((offset + length) <= mTotalSize))
		return;

	memset(mCache + offset, 0, length);
	MarkRenderProxyDataDirty();
}

void GpuBuffer::ReadCached(u32 offset, u32 length, void* destination)
{
	if(!B3D_ENSURE(mCache != nullptr))
		return;

	if(!B3D_ENSURE((offset + length) <= mTotalSize))
		return;

	memcpy(destination, mCache + offset, length);
}

SPtr<render::RenderProxy> GpuBuffer::CreateRenderProxy() const
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	const GpuBufferCreateInformation createInformation = mInformation;
	return gpuDevice->CreateGpuBuffer(createInformation, true);
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(GpuBuffer, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(u32, BufferSize)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(u8*, BufferData)
	B3D_SYNC_BLOCK_END
}

RenderProxySyncPacket* GpuBuffer::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	if(!mInformation.Flags.IsSet(GpuBufferFlag::AllowWriteCachingOnCPU))
		return nullptr;

	SyncPacket* syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	syncPacket->BufferSize = mTotalSize;
	syncPacket->BufferData = allocator.Allocate(mTotalSize);
	ReadCached(0, mTotalSize, syncPacket->BufferData);

	return syncPacket;
}

SPtr<GpuBuffer> GpuBuffer::Create(const GpuBufferCreateInformation& createInformation)
{
	SPtr<GpuBuffer> buffer = B3DMakeSharedFromExisting<GpuBuffer>(new(B3DAllocate<GpuBuffer>()) GpuBuffer(createInformation));
	buffer->SetShared(buffer);
	buffer->Initialize();

	return buffer;
}

u32 GpuBuffer::GetFormatSize(GpuBufferFormat format)
{
	static bool lookupInitialized = false;

	static u32 lookup[BF_COUNT];
	if(!lookupInitialized)
	{
		lookup[BF_16X1F] = 2;
		lookup[BF_16X2F] = 4;
		lookup[BF_16X4F] = 8;
		lookup[BF_32X1F] = 4;
		lookup[BF_32X2F] = 8;
		lookup[BF_32X3F] = 12;
		lookup[BF_32X4F] = 16;
		lookup[BF_8X1] = 1;
		lookup[BF_8X2] = 2;
		lookup[BF_8X4] = 4;
		lookup[BF_16X1] = 2;
		lookup[BF_16X2] = 4;
		lookup[BF_16X4] = 8;
		lookup[BF_8X1S] = 1;
		lookup[BF_8X2S] = 2;
		lookup[BF_8X4S] = 4;
		lookup[BF_16X1S] = 2;
		lookup[BF_16X2S] = 4;
		lookup[BF_16X4S] = 8;
		lookup[BF_32X1S] = 4;
		lookup[BF_32X2S] = 8;
		lookup[BF_32X3S] = 12;
		lookup[BF_32X4S] = 16;
		lookup[BF_8X1U] = 1;
		lookup[BF_8X2U] = 2;
		lookup[BF_8X4U] = 4;
		lookup[BF_16X1U] = 2;
		lookup[BF_16X2U] = 4;
		lookup[BF_16X4U] = 8;
		lookup[BF_32X1U] = 4;
		lookup[BF_32X2U] = 8;
		lookup[BF_32X3U] = 12;
		lookup[BF_32X4U] = 16;
		lookup[BF_64X1F] = 8;
		lookup[BF_64X2F] = 16;
		lookup[BF_64X3F] = 24;
		lookup[BF_64X4F] = 32;
		lookup[BF_64X1S] = 8;
		lookup[BF_64X2S] = 16;
		lookup[BF_64X3S] = 24;
		lookup[BF_64X4S] = 32;
		lookup[BF_64X1U] = 8;
		lookup[BF_64X2U] = 16;
		lookup[BF_64X3U] = 24;
		lookup[BF_64X4U] = 32;

		lookupInitialized = true;
	}

	if(format >= BF_COUNT)
		return 0;

	return lookup[(u32)format];
}

u32 GpuBuffer::CalculateSuballocatedBufferSize(const GpuBufferInformation& information, const SPtr<GpuDevice>& gpuDevice)
{
	const u32 unalignedBufferSize = CalculateUnalignedGpuBufferSize(information);

	if(information.SuballocationCount > 1 && gpuDevice)
		return CalculateSuballocatedBufferSize(information, *gpuDevice);
	
	return unalignedBufferSize;
}

u32 GpuBuffer::CalculateSuballocatedBufferSize(const GpuBufferInformation& information, const GpuDevice& gpuDevice)
{
	const u32 unalignedBufferSize = CalculateUnalignedGpuBufferSize(information);

	if(information.SuballocationCount > 1)
	{
		if(information.Type == GpuBufferType::Uniform)
			return Math::CeilToMultiple(unalignedBufferSize, gpuDevice.GetCapabilities().MinimumUniformBufferOffsetAlignment);
		else if(information.Type == GpuBufferType::StagingWrite || information.Type == GpuBufferType::StagingRead)
			return Math::CeilToMultiple(unalignedBufferSize, gpuDevice.GetCapabilities().OptimalBufferToBufferCopyOffsetAlignment); // Note: Not handling buffers used for image copies here, presumably we don't suballocate those
		else
			B3D_ENSURE(false);
	}
	
	return unalignedBufferSize;
}

u32 GpuBuffer::CalculateTotalBufferSize(const GpuBufferInformation& information, const SPtr<GpuDevice>& gpuDevice)
{
	const u32 stride = CalculateSuballocatedBufferSize(information, gpuDevice);
	return stride * Math::Max(1u, information.SuballocationCount);
}

namespace b3d::render
{
	GpuBuffer::GpuBuffer(GpuDevice& device, const GpuBufferCreateInformation& createInformation, u32 suballocationSize)
		: mInformation(createInformation), mDevice(device), mSuballocationSize(suballocationSize), mTotalSize(createInformation.SuballocationCount * mSuballocationSize)
	{
		if(mInformation.Flags.IsSet(GpuBufferFlag::AllowWriteCachingOnCPU))
		{
			mCache = (u8*)B3DAllocate(mTotalSize);
		}
	}

	GpuBuffer::~GpuBuffer()
	{
		if(mCache != nullptr)
		{
			B3DFree(mCache);
		}
	}

	void GpuBuffer::WriteCached(u32 offset, u32 length, const void* source)
	{
		if(!B3D_ENSURE(mCache != nullptr))
			return;

		if(!B3D_ENSURE((offset + length) <= mTotalSize))
			return;

		memcpy(mCache + offset, source, length);
		mIsCacheDirty = true;
	}

	u32 GpuBuffer::WriteCachedType(u32 offset, const GpuDataParameterTypeInformation& typeInformation, const void* source)
	{
		const u8* value = (const u8*)source;

		const u32 startOffset = offset;
		for(u32 row = 0; row < typeInformation.NumRows; ++row)
		{
			const u32 rowSize = typeInformation.NumColumns * typeInformation.BaseTypeSize;
			WriteCached(offset, rowSize, value);

			offset += typeInformation.Alignment;
			value += rowSize;
		}

		return offset - startOffset;
	}

	void GpuBuffer::ZeroOutCached(u32 offset, u32 length)
	{
		if(!B3D_ENSURE(mCache != nullptr))
			return;

		if(!B3D_ENSURE((offset + length) <= mTotalSize))
			return;

		memset(mCache + offset, 0, length);
		mIsCacheDirty = true;
	}

	void GpuBuffer::FlushCache(u32 suballocationIndex)
	{
		if(!B3D_ENSURE(mCache != nullptr))
			return;

		if(!mIsCacheDirty)
			return;

		u32 offset = 0;
		u32 length = mTotalSize;

		if(suballocationIndex != ~0u)
		{
			if(!B3D_ENSURE(suballocationIndex < mInformation.SuballocationCount))
				return;

			offset = suballocationIndex * mSuballocationSize;
			length = mSuballocationSize;
		}

		// TODO - This should write to CPU cached buffer directly via map/unmap. But we need a ring buffer to handle usage over multiple frames
		GpuBufferUtility::Write(std::static_pointer_cast<GpuBuffer>(GetShared()), offset, length, mCache + offset);

		mIsCacheDirty = false;
	}

	void GpuBuffer::ReadCached(u32 offset, u32 length, void* destination)
	{
		if(!B3D_ENSURE(mCache != nullptr))
			return;

		if(!B3D_ENSURE((offset + length) <= mTotalSize))
			return;

		memcpy(destination, mCache + offset, length);
	}

	void GpuBuffer::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
	{
		auto* const syncPacket = data.GetSyncPacket<b3d::GpuBuffer::SyncPacket>();
		if(!syncPacket)
			return;

		if(syncPacket->BufferData == nullptr)
			return;

		if(B3D_ENSURE(mTotalSize == syncPacket->BufferSize))
		{
			// TODO - This should write to CPU cached buffer directly via map/unmap. But we need a ring buffer to handle usage over multiple frames
			GpuBufferUtility::Write(std::static_pointer_cast<GpuBuffer>(GetShared()), 0, syncPacket->BufferSize, syncPacket->BufferData);
		}

		allocator.Free(syncPacket->BufferData);
	}

	void GpuBufferSuballocation::Write(const void* data, u32 size) const
	{
		B3D_ASSERT(IsValid());
		B3D_ASSERT(size <= GetSize());

		void* const bufferMemory = mBuffer->Lock(mSuballocationOffset, size, GBL_WRITE_ONLY);
		memcpy(bufferMemory, data, size);
		mBuffer->Unlock();
	}

	SPtr<GpuBuffer> GpuBufferUtility::CreateStaging(const SPtr<GpuBuffer>& buffer, bool readable)
	{
		if(!B3D_ENSURE(buffer != nullptr))
			return nullptr;

		GpuBufferCreateInformation createInformation;
		createInformation.Type = readable ? GpuBufferType::StagingRead : GpuBufferType::StagingWrite;
		createInformation.Staging.Size = buffer->GetTotalSize();

		GpuDevice& device = buffer->GetDevice();
		return device.CreateGpuBuffer(createInformation);
	}

	void GpuBufferUtility::Write(const SPtr<GpuBuffer>& buffer, u32 offset, u32 length, const void* source, GpuBufferWriteFlags flags, SPtr<GpuCommandBuffer> commandBuffer)
	{
		if(!B3D_ENSURE(buffer != nullptr))
			return;

		if((offset + length) > buffer->GetTotalSize())
			return;

		if(length == 0)
			return;

		const GpuBufferInformation& gpuBufferInformation = buffer->GetInformation();
		const bool isCPUAccessible = gpuBufferInformation.Type == GpuBufferType::StagingRead || gpuBufferInformation.Type == GpuBufferType::StagingWrite || gpuBufferInformation.Flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess);
		const bool supportsGPUWrites = gpuBufferInformation.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		GpuLockOptions options = GBL_WRITE_ONLY_DISCARD_RANGE;
		if(flags.IsSet(GpuBufferWriteFlag::NoOverwrite))
			options = GBL_WRITE_ONLY_NO_OVERWRITE;
		else if(flags.IsSet(GpuBufferWriteFlag::Discard))
			options = GBL_WRITE_ONLY_DISCARD;

		const bool canDiscardBuffer = flags.IsSet(GpuBufferWriteFlag::Discard) ||
			(offset == 0 && length == buffer->GetTotalSize());

		// Check is the GPU currently reading or writing from the buffer
		const GpuQueueMask useMask = buffer->GetUseMask(GpuAccessFlag::Read | GpuAccessFlag::Write);

		if(isCPUAccessible) // TODO - Need to check if this is memory on an integrated GPU, in which case it might be directly mappable always
		{
			// Note: Even if GPU isn't currently using the buffer, but the buffer supports GPU writes, we consider it as
			// being used because the write could have completed yet still not visible, so we need to issue a pipeline
			// barrier below.
			const bool isUsedOnGPU = !useMask.IsEmpty() || supportsGPUWrites;

			// Even if the buffer is directly mappable we might wish to avoid mapping it directly in these situations:
			const bool shouldMapDirectly =
				(!isUsedOnGPU || flags.IsSet(GpuBufferWriteFlag::NoOverwrite)) && // GPU is currently using the buffer and we cannot map it safely (unless user specifically requested the no-overwrite flag)
				(buffer->GetBoundCount() == 0 || (commandBuffer == nullptr && canDiscardBuffer) || flags.IsSet(GpuBufferWriteFlag::NoOverwrite)); // Buffer is bound to a command buffer already. If user provided a command buffer queue a write operation there instead of mapping directly. If not, discard the original buffer and lock a new copy of the buffer.

			if(shouldMapDirectly)
			{
				void* lockedData = buffer->Lock(offset, length, options);
				memcpy(lockedData, source, length);
				buffer->Unlock();

				return;
			}
		}

		// Note: Not supporting staging memory. Not sure if there's a benefit.

		// Create a staging buffer
		SPtr<GpuBuffer> stagingBuffer = CreateStaging(buffer, false);

		// Copy the source into the staging buffer
		if(stagingBuffer != nullptr)
		{
			void* lockedStagingData = stagingBuffer->Lock(0, length, GBL_WRITE_ONLY);
			memcpy(lockedStagingData, source, length);

			stagingBuffer->Unlock();
		}

		// If the buffer is used in any way on the GPU, we need to wait for that use to finish before we issue our copy
		GpuQueueMask syncMask;
		if(!useMask.IsEmpty() && options != GBL_WRITE_ONLY_NO_OVERWRITE) // Buffer is currently used on the GPU
			syncMask = useMask;

		// Check if the buffer will still be bound somewhere after the CBs using it finish
		const u32 useCount = buffer->GetUseCount();
		const u32 boundCount = buffer->GetBoundCount();

		const bool isBoundWithoutUse = boundCount > useCount;

		if(isBoundWithoutUse && commandBuffer == nullptr)
		{
			if(!canDiscardBuffer)
			{
#if B3D_BUILD_TYPE_DEVELOPMENT
				if(buffer->IsRangeBound(offset, length))
				{
					B3D_LOG(Warning, RenderBackend, "Writing to a buffer '{0}' that is currently bound on a command buffer, without providing an explicit command buffer. Such writes will be queued on the transfer buffer which is submitted before any user command buffers. This means multiple writes will overwrite it each other if not careful.", buffer->GetName());
				}
#endif
			}
			else
				buffer->RecreateInternalBuffer();
		}

		// Queue copy/update command for the actual write
		if(commandBuffer == nullptr)
			commandBuffer = buffer->GetDevice().GetQueue(GQT_GRAPHICS, 0)->GetOrCreateTransferCommandBuffer();

		commandBuffer->CopyBufferToBuffer(stagingBuffer, buffer, 0, offset, length);
		commandBuffer->AddQueueSyncMask(syncMask);

		// We don't actually flush the transfer buffer here since it's an expensive operation, but it's instead
		// done automatically before next "normal" command buffer submission.
	}

	void GpuBufferUtility::Read(const SPtr<GpuBuffer>& buffer, u32 offset, u32 length, void* destination, SPtr<GpuCommandBuffer> commandBuffer)
	{
		if(!B3D_ENSURE(buffer != nullptr))
			return;

		if((offset + length) > buffer->GetTotalSize())
			return;

		if(length == 0)
			return;

		const GpuBufferInformation& gpuBufferInformation = buffer->GetInformation();
		const bool isCPUAccessible = gpuBufferInformation.Type == GpuBufferType::StagingRead || gpuBufferInformation.Type == GpuBufferType::StagingWrite || gpuBufferInformation.Flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess);
		const bool supportsGPUWrites = gpuBufferInformation.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		GpuQueue& transferGpuQueue = *buffer->GetDevice().GetQueue(GQT_GRAPHICS, 0);

		// Check is the GPU currently writing to the buffer
		const GpuQueueMask writeUseMask = buffer->GetUseMask(GpuAccessFlag::Write);

		if(isCPUAccessible) // TODO - Need to check if this is memory on an integrated GPU, in which case it might be directly mappable always
		{
			// Note: Even if GPU isn't currently using the buffer, but the buffer supports GPU writes, we consider it as
			// being used because the write could have completed yet still not visible, so we need to wait for any
			// GPU operations to complete.
			const bool isUsedOnGPU = !writeUseMask.IsEmpty() || supportsGPUWrites;

			// If used on the GPU, we need to wait until all write operations complete before mapping it
			if(isUsedOnGPU)
			{
				if(commandBuffer == nullptr)
					commandBuffer = transferGpuQueue.GetOrCreateTransferCommandBuffer();

				// Make any writes visible before mapping
				if(supportsGPUWrites)
				{
					// Issue a barrier so the device makes the written memory available for read (read-after-write hazard)
					commandBuffer->IssueBarriers({{ GpuBufferBarrier(buffer, GpuResourceUseFlag::Host, GpuAccessFlag::Read)}});
				}

				// Submit the command buffer and wait until it finishes
				commandBuffer->AddQueueSyncMask(writeUseMask);
				transferGpuQueue.SubmitTransferCommandBuffer(true);
			}

			void* lockedData = buffer->Lock(offset, length, GBL_READ_ONLY);
			memcpy(destination, lockedData, length);
			buffer->Unlock();

			return;
		}

		// Not directly mappable, will need a staging buffer to copy into
		SPtr<GpuBuffer> stagingBuffer = CreateStaging(buffer, true);

		// If buffer supports GPU writes or is currently being written to, we need to wait on any potential writes to complete
		GpuQueueMask syncMask;
		if(supportsGPUWrites || !writeUseMask.IsEmpty())
		{
			// Ensure flush will wait for all queues currently writing to the buffer (if any) to finish
			syncMask = writeUseMask;
		}

		if(commandBuffer == nullptr)
			commandBuffer = transferGpuQueue.GetOrCreateTransferCommandBuffer();

		// Queue copy command
		commandBuffer->CopyBufferToBuffer(buffer, stagingBuffer, offset, 0, length);

		// Submit the command buffer and wait until it finishes
		commandBuffer->AddQueueSyncMask(syncMask);
		transferGpuQueue.SubmitTransferCommandBuffer(true);

		void* lockedStagingData = stagingBuffer->Lock(0, length, GBL_READ_ONLY);
		memcpy(destination, lockedStagingData, length);

		stagingBuffer->Unlock();
		stagingBuffer->Destroy();
	}

	TAsyncOp<SPtr<MemoryDataStream>> GpuBufferUtility::ReadAsync(const SPtr<GpuBuffer>& buffer, u32 offset, u32 length, GpuCommandBuffer& commandBuffer)
	{
		if(buffer == nullptr)
			return {};

		TAsyncOp<SPtr<MemoryDataStream>> op;
		auto fnOnCommandBufferCompleted = [buffer, offset, length, op]() mutable
		{
			const SPtr<MemoryDataStream> dataStream = B3DMakeShared<MemoryDataStream>(buffer->GetTotalSize());
			Read(buffer, offset, length, dataStream->Data());

			op.CompleteOperation(dataStream);
		};

		auto fnOnCommandBufferDestroyed = [buffer, op](bool isSubmitted) mutable
		{
			// In this case the completion callback will trigger.
			if(isSubmitted)
				return;

			buffer->Destroy();
			op.CompleteOperation(nullptr);
		};

		commandBuffer.OnDidComplete.Connect(fnOnCommandBufferCompleted);
		commandBuffer.OnDestroyed.Connect(fnOnCommandBufferDestroyed);

		return op;
	}
}
