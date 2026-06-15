//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBuffer.h"
#include "B3DApplication.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuDevice.h"
#include "B3DGpuDeviceCapabilities.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "FileSystem/B3DDataStream.h"
#include "Renderer/B3DRenderer.h"

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

// Explicit template instantiations
namespace b3d
{
	template class TGpuBufferSuballocation<false>;
	template class TGpuBufferSuballocation<true>;
	template class TGpuBufferMappedScope<false>;
	template class TGpuBufferMappedScope<true>;
} // namespace b3d

GpuBuffer::GpuBuffer(const GpuBufferCreateInformation& createInformation)
	: mInformation(createInformation)
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	mSuballocationSize = CalculateSuballocatedBufferSize(createInformation, gpuDevice);
	mTotalSize = CalculateTotalBufferSize(createInformation, gpuDevice);
}

void GpuBuffer::Initialize()
{
	mCache = (u8*)B3DAllocate(mTotalSize);
}

void GpuBuffer::Destroy()
{
	if(mCache != nullptr)
	{
		B3DFree(mCache);
	}
}

void GpuBuffer::Write(u32 offset, u32 length, const void* source)
{
	if(!B3D_ENSURE(mCache != nullptr))
		return;

	if(!B3D_ENSURE((offset + length) <= mTotalSize))
		return;

	memcpy(mCache + offset, source, length);
	MarkRenderProxyDataDirty();
}

u32 GpuBuffer::WriteTyped(u32 offset, const GpuDataParameterTypeInformation& typeInformation, const void* source)
{
	const u8* value = (const u8*)source;

	const u32 startOffset = offset;
	for(u32 row = 0; row < typeInformation.NumRows; ++row)
	{
		const u32 rowSize = typeInformation.NumColumns * typeInformation.BaseTypeSize;
		Write(offset, rowSize, value);

		offset += typeInformation.Alignment;
		value += rowSize;
	}

	return offset - startOffset;
}

void GpuBuffer::ZeroOut(u32 offset, u32 length)
{
	if(!B3D_ENSURE(mCache != nullptr))
		return;

	if(!B3D_ENSURE((offset + length) <= mTotalSize))
		return;

	memset(mCache + offset, 0, length);
	MarkRenderProxyDataDirty();
}

void GpuBuffer::Read(u32 offset, u32 length, void* destination)
{
	if(!B3D_ENSURE(mCache != nullptr))
		return;

	if(!B3D_ENSURE((offset + length) <= mTotalSize))
		return;

	memcpy(destination, mCache + offset, length);
}

TShared<render::RenderProxy> GpuBuffer::CreateRenderProxy() const
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	const GpuBufferCreateInformation createInformation = mInformation;
	return gpuDevice->CreateGpuBuffer(createInformation, GpuObjectCreateFlag::DeferredInitialize | GpuObjectCreateFlag::RenderThreadDestroy);
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
	SyncPacket* syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	syncPacket->BufferSize = mTotalSize;
	syncPacket->BufferData = allocator.Allocate(mTotalSize);
	Read(0, mTotalSize, syncPacket->BufferData);

	return syncPacket;
}

TShared<GpuBuffer> GpuBuffer::Create(const GpuBufferCreateInformation& createInformation)
{
	TShared<GpuBuffer> buffer = B3DMakeSharedFromExisting<GpuBuffer>(new(B3DAllocate<GpuBuffer>()) GpuBuffer(createInformation));
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

u32 GpuBuffer::CalculateSuballocatedBufferSize(const GpuBufferInformation& information, const TShared<GpuDevice>& gpuDevice)
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

u32 GpuBuffer::CalculateTotalBufferSize(const GpuBufferInformation& information, const TShared<GpuDevice>& gpuDevice)
{
	const u32 stride = CalculateSuballocatedBufferSize(information, gpuDevice);
	return stride * Math::Max(1u, information.SuballocationCount);
}

namespace b3d::render
{
	GpuBuffer::GpuBuffer(GpuDevice& device, const GpuBufferCreateInformation& createInformation, u32 suballocationSize)
		: mInformation(createInformation), mDevice(device), mSuballocationSize(suballocationSize), mTotalSize(createInformation.SuballocationCount * mSuballocationSize)
	{ }

	GpuBuffer::~GpuBuffer()
	{ }

#if B3D_BUILD_TYPE_DEVELOPMENT
	void GpuBuffer::ValidateMap(u32 offset, u32 size, GpuMapOptions options)
	{
		if(options.IsSet(GpuMapOption::Write))
		{
			// We currently don't track exact bound ranges for buffers that aren't suballocated, so we cannot warn in that case
			const bool ignoreWarning = mInformation.SuballocationCount <= 1 && (offset != 0 || size != mTotalSize);

			if(!ignoreWarning)
			{
				if(IsRangeInUse(offset, size))
					B3D_LOG(Warning, LogRenderBackend, "Writing to a buffer that is currently used on the GPU. This will result in undefined behaviour. Buffer: {0}", mName);
				else if(!options.IsSet(GpuMapOption::NoOverwrite))
				{
					if(IsRangeBound(offset, size))
						B3D_LOG(Warning, LogRenderBackend, "Writing to a buffer that is currently bound on a command buffer. Previous usages of the buffer will be affected. Buffer: {0}", mName);
				}
			}
		}
		else
		{
			// TODO - When reading buffers writable by the GPU we should also check if they are currently used by the GPU. But it's not a common case to have a CPU visible buffers writable by the GPU, so skipping that for now.
		}	
	}
#endif

	void GpuBuffer::Write(u32 offset, u32 length, const void* source)
	{
		if(!B3D_ENSURE((offset + length) <= mTotalSize))
			return;

		GpuBufferMappedScope mapping = Map(offset, length, GpuMapOption::Write);
		memcpy(mapping.GetMappedMemory(), source, length);
	}

	u32 GpuBuffer::WriteTyped(u32 offset, const GpuDataParameterTypeInformation& typeInformation, const void* source)
	{
		const u32 length = typeInformation.NumRows * typeInformation.Alignment;
		GpuBufferMappedScope mapping = Map(offset, length, GpuMapOption::Write);
		void* destination = mapping.GetMappedMemory();

		const u8* value = (const u8*)source;
		for(u32 row = 0; row < typeInformation.NumRows; ++row)
		{
			const u32 rowSize = typeInformation.NumColumns * typeInformation.BaseTypeSize;
			memcpy(destination, value, rowSize);

			offset += typeInformation.Alignment;
			value += rowSize;
		}

		return length;
	}

	void GpuBuffer::ZeroOut(u32 offset, u32 length)
	{
		if(!B3D_ENSURE((offset + length) <= mTotalSize))
			return;

		GpuBufferMappedScope mapping = Map(offset, length, GpuMapOption::Write);
		memset(mapping.GetMappedMemory(), 0, length);
	}

	void GpuBuffer::Read(u32 offset, u32 length, void* destination)
	{
		if(!B3D_ENSURE((offset + length) <= mTotalSize))
			return;

		GpuBufferMappedScope mapping = Map(offset, length, GpuMapOption::Read);
		memcpy(destination, mapping.GetMappedMemory(), length);
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
			GpuWorkContext& gpuContext = GetRenderer()->GetGpuContext();
			GpuBufferUtility::Write(gpuContext, std::static_pointer_cast<GpuBuffer>(GetShared()), 0, syncPacket->BufferSize, syncPacket->BufferData);
		}

		allocator.Free(syncPacket->BufferData);
	}

	TShared<GpuBuffer> GpuBufferUtility::CreateStaging(GpuWorkContext& gpuContext, const TShared<GpuBuffer>& buffer, bool readable)
	{
		if(!B3D_ENSURE(buffer != nullptr))
			return nullptr;

		GpuBufferCreateInformation createInformation;
		createInformation.Type = readable ? GpuBufferType::StagingRead : GpuBufferType::StagingWrite;
		createInformation.Staging.Size = buffer->GetTotalSize();

		return gpuContext.CreateTransientGpuBuffer(createInformation);
	}

	void GpuBufferUtility::Write(GpuWorkContext& gpuContext, const TShared<GpuBuffer>& buffer, u32 offset, u32 length, const void* source, GpuBufferWriteFlags flags, TShared<GpuCommandBuffer> commandBuffer)
	{
		B3D_ASSERT(buffer != nullptr);

		if((offset + length) > buffer->GetTotalSize())
			return;

		if(length == 0)
			return;

		const GpuBufferInformation& gpuBufferInformation = buffer->GetInformation();
		const bool supportsGPUWrites = gpuBufferInformation.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		GpuMapOptions mapOptions = GpuMapOption::Write;
		if(flags.IsSet(GpuBufferWriteFlag::NoOverwrite))
			mapOptions |= GpuMapOption::NoOverwrite;

		const bool canDiscardBuffer = flags.IsSet(GpuBufferWriteFlag::Discard) || (offset == 0 && length == buffer->GetTotalSize());

		// Check is the GPU currently reading or writing from the buffer
		const GpuQueueMask useMask = buffer->GetUseMask(GpuAccessFlag::Read | GpuAccessFlag::Write);
		const u32 useCount = buffer->GetUseCount();
		const u32 boundCount = buffer->GetBoundCount();

		void* mappedMemory = buffer->GetMappedMemory();
		if(mappedMemory != nullptr)
		{
			// Note: Even if GPU isn't currently using the buffer, but the buffer supports GPU writes, we consider it as
			// being used because the write could have completed yet still not visible, so we need to issue a pipeline
			// barrier below.
			const bool isUsedOnGPU = !useMask.IsEmpty() || supportsGPUWrites;

			// Recreate the internal buffer if it is bound to a command buffer, to avoid overwriting the old data. But only if the user
			// allows discard via a flag. In case the user provided an explicit command buffer, perfer a staging buffer over discard
			// (it still costs us creation of a new buffer, and the original buffer bindings remain valid). Finally, if no-overwrite is
			// specified, we never recreate the buffer as the user guarantees he won't touch the previously bound region.
			const bool recreateInternalBuffer = boundCount > 0 && commandBuffer == nullptr && canDiscardBuffer && !flags.IsSet(GpuBufferWriteFlag::NoOverwrite);

			// Even if the buffer is directly mappable we might wish to avoid mapping it directly in these situations:
			const bool shouldMapDirectly =
				(!isUsedOnGPU // GPU is currently using the buffer
				&& (boundCount == 0 || recreateInternalBuffer)) // Buffer is bound to a command buffer already, and we are not creating a new one. Cannot map without affecting the previous binding.
				|| flags.IsSet(GpuBufferWriteFlag::NoOverwrite); // If no-overwrite flag is set, user guarantees he won't touch the memory the GPU is using

			if(shouldMapDirectly)
			{
				if(recreateInternalBuffer)
					buffer->RecreateInternalBuffer();

				GpuBufferMappedScope mapping = buffer->Map(offset, length, mapOptions);
				memcpy(mapping.GetMappedMemory(), source, length);

				return;
			}
		}

		// Note: Not supporting staging memory. Not sure if there's a benefit.

		// Create a staging buffer
		TShared<GpuBuffer> stagingBuffer = CreateStaging(gpuContext, buffer, false);

		// Copy the source into the staging buffer
		if(stagingBuffer != nullptr)
		{
			GpuBufferMappedScope mapping = stagingBuffer->Map(0, length, GpuMapOption::Write);
			memcpy(mapping.GetMappedMemory(), source, length);
		}

		// If the buffer is used in any way on the GPU, we need to wait for that use to finish before we issue our copy
		GpuQueueMask syncMask;
		if(!useMask.IsEmpty() && mapOptions.IsSet(GpuMapOption::NoOverwrite)) // Buffer is currently used on the GPU
			syncMask = useMask;

		// Check if the buffer will still be bound somewhere after the command buffers using it finish. If it is, we have to recreate the internal buffer otherwise the copy
		// operation might just get overwritten by those command buffers when the execute. This is because the transfer command buffers are always submitted before regular
		// command buffers. If user provided an explicit command buffer, then it's up to him to ensure the correct ordering.
		const bool isBoundWithoutUse = boundCount > useCount;
		if(isBoundWithoutUse && commandBuffer == nullptr)
		{
			if(!canDiscardBuffer)
			{
#if B3D_BUILD_TYPE_DEVELOPMENT
				if(buffer->IsRangeBound(offset, length))
				{
					B3D_LOG(Warning, LogRenderBackend, "Writing to a buffer '{0}' that is currently bound on a command buffer, without providing an explicit command buffer. Such writes will be queued on the transfer buffer which is submitted before any user command buffers. This means  writes will overwrite it each other if not careful.", buffer->GetName());
				}
#endif
			}
			else
				buffer->RecreateInternalBuffer();
		}

		// Queue copy/update command for the actual write
		if(commandBuffer == nullptr)
			commandBuffer = gpuContext.GetTransferCommandBuffer();

		commandBuffer->CopyBufferToBuffer(stagingBuffer, buffer, 0, offset, length);
		commandBuffer->AddQueueSyncMask(syncMask);

		// We don't actually flush the transfer buffer here since it's an expensive operation, but it's instead
		// done automatically before next "normal" command buffer submission.
	}

	void GpuBufferUtility::Read(GpuWorkContext& gpuContext, const TShared<GpuBuffer>& buffer, u32 offset, u32 length, void* destination, const TShared<GpuQueue>& gpuQueue)
	{
		B3D_ASSERT(buffer != nullptr);

		if((offset + length) > buffer->GetTotalSize())
			return;

		if(length == 0)
			return;

		const GpuBufferInformation& gpuBufferInformation = buffer->GetInformation();
		const bool supportsGPUWrites = gpuBufferInformation.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		GpuQueue& transferGpuQueue = gpuQueue != nullptr ? *gpuQueue : *buffer->GetDevice().GetQueue(GQT_GRAPHICS, 0);

		// Check is the GPU currently writing to the buffer
		const GpuQueueMask writeUseMask = buffer->GetUseMask(GpuAccessFlag::Write);

	// If memory is host visible try mapping it directly
		void* mappedMemory = buffer->GetMappedMemory();
		if(mappedMemory != nullptr)
		{
			// Note: Even if GPU isn't currently using the buffer, but the buffer supports GPU writes, we consider it as
			// being used because the write could have completed yet still not visible, so we need to wait for any
			// GPU operations to complete.
			const bool isUsedOnGPU = !writeUseMask.IsEmpty() || supportsGPUWrites;

			// If used on the GPU, we need to wait until all write operations complete before mapping it
			if(isUsedOnGPU)
			{
				TShared<GpuCommandBuffer> commandBuffer = gpuContext.GetTransferCommandBuffer();

				// Make any writes visible before mapping
				if(supportsGPUWrites)
				{
					// Issue a barrier so the device makes the written memory available for read (read-after-write hazard)
					commandBuffer->IssueBarriers({{ GpuBufferBarrier(buffer, GpuResourceUseFlag::Host, GpuAccessFlag::Read)}});
				}

				// Submit the command buffer and wait until it finishes
				commandBuffer->AddQueueSyncMask(writeUseMask);
				gpuContext.SubmitTransferCommandBuffers(true);
			}

			GpuBufferMappedScope mapping = buffer->Map(offset, length, GpuMapOption::Read);
			memcpy(destination, mapping.GetMappedMemory(), length);

			return;
		}

		// Not directly mappable, will need a staging buffer to copy into
		TShared<GpuBuffer> stagingBuffer = CreateStaging(gpuContext, buffer, true);

		// If buffer supports GPU writes or is currently being written to, we need to wait on any potential writes to complete
		GpuQueueMask syncMask;
		if(supportsGPUWrites || !writeUseMask.IsEmpty())
		{
			// Ensure flush will wait for all queues currently writing to the buffer (if any) to finish
			syncMask = writeUseMask;
		}

		TShared<GpuCommandBuffer> commandBuffer = gpuContext.GetTransferCommandBuffer();

		// Queue copy command
		commandBuffer->CopyBufferToBuffer(buffer, stagingBuffer, offset, 0, length);

		// Submit the command buffer and wait until it finishes
		commandBuffer->AddQueueSyncMask(syncMask);
		gpuContext.SubmitTransferCommandBuffers(true);

		{
			GpuBufferMappedScope mapping = stagingBuffer->Map(0, length, GpuMapOption::Read);
			memcpy(destination, mapping.GetMappedMemory(), length);
		}

		stagingBuffer->Destroy();
	}

	TAsyncOp<TShared<MemoryDataStream>> GpuBufferUtility::ReadAsync(GpuWorkContext& gpuContext, const TShared<GpuBuffer>& buffer, u32 offset, u32 length, GpuCommandBuffer& commandBuffer)
	{
		if(buffer == nullptr)
			return {};

		// TODO - Staging buffer might not be necessary if he texture is directly mappable
		TShared<GpuBuffer> stagingBuffer = CreateStaging(gpuContext, buffer, true);
		commandBuffer.CopyBufferToBuffer(buffer, stagingBuffer, offset, 0, length);

		TAsyncOp<TShared<MemoryDataStream>> op;
		auto fnOnCommandBufferCompleted = [stagingBuffer, offset, length, op]() mutable
		{
			GpuBufferMappedScope mapping = stagingBuffer->Map(GpuMapOption::Read);
			const TShared<MemoryDataStream> dataStream = B3DMakeShared<MemoryDataStream>(stagingBuffer->GetTotalSize());
			memcpy(dataStream->Data(), mapping.GetMappedMemory(), length);

			op.CompleteOperation(dataStream);
		};

		auto fnOnCommandBufferDestroyed = [op](bool isSubmitted) mutable
		{
			// In this case the completion callback will trigger.
			if(isSubmitted)
				return;

			op.CompleteOperation(nullptr);
		};

		commandBuffer.OnDidComplete.Connect(fnOnCommandBufferCompleted);
		commandBuffer.OnDestroyed.Connect(fnOnCommandBufferDestroyed);

		return op;
	}
}
