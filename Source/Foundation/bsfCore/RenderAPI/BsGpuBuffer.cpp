//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuBuffer.h"
#include "BsCoreApplication.h"
#include "BsGpuDevice.h"
#include "BsGpuDeviceCapabilities.h"
#include "CoreThread/BsCoreObjectSync.h"

using namespace bs;

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
	}

	B3D_ENSURE(false);
	return 128;
}

GpuBuffer::GpuBuffer(const GpuBufferCreateInformation& createInformation)
	: mInformation(createInformation)
{
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
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
	MarkCoreDirty();
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
	MarkCoreDirty();
}

void GpuBuffer::ReadCached(u32 offset, u32 length, void* destination)
{
	if(!B3D_ENSURE(mCache != nullptr))
		return;

	if(!B3D_ENSURE((offset + length) <= mTotalSize))
		return;

	memcpy(destination, mCache + offset, length);
}

SPtr<ct::GpuBuffer> GpuBuffer::GetCore() const
{
	return std::static_pointer_cast<ct::GpuBuffer>(mCoreSpecific);
}

SPtr<ct::CoreObject> GpuBuffer::CreateCore() const
{
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	const GpuBufferCreateInformation createInformation = mInformation;
	return gpuDevice->CreateGpuBuffer(createInformation, true);
}

namespace bs
{
	B3D_SYNC_BLOCK_BEGIN(GpuBuffer, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(u32, BufferSize)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(u8*, BufferData)
	B3D_SYNC_BLOCK_END
}

CoreSyncPacket* GpuBuffer::CreateSyncPacket(FrameAlloc& allocator, u32 flags)
{
	if(!mInformation.Flags.IsSet(GpuBufferFlag::AllowWriteCachingOnCPU))
		return nullptr;

	SyncPacket* syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	syncPacket->BufferSize = mTotalSize;
	syncPacket->BufferData = allocator.Alloc(mTotalSize);
	ReadCached(0, mTotalSize, syncPacket->BufferData);

	return syncPacket;
}

SPtr<GpuBuffer> GpuBuffer::Create(const GpuBufferCreateInformation& createInformation)
{
	SPtr<GpuBuffer> buffer = B3DMakeCoreFromExisting<GpuBuffer>(new(B3DAllocate<GpuBuffer>()) GpuBuffer(createInformation));
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
	{
		B3D_ENSURE(information.Type == GpuBufferType::Uniform); // Currently only supported for uniform buffers
		return Math::CeilToMultiple(unalignedBufferSize, gpuDevice->GetCapabilities().MinimumUniformBufferOffsetAlignment);
	}
	
	return unalignedBufferSize;
}

u32 GpuBuffer::CalculateSuballocatedBufferSize(const GpuBufferInformation& information, const GpuDevice& gpuDevice)
{
	const u32 unalignedBufferSize = CalculateUnalignedGpuBufferSize(information);

	if(information.SuballocationCount > 1)
	{
		B3D_ENSURE(information.Type == GpuBufferType::Uniform); // Currently only supported for uniform buffers
		return Math::CeilToMultiple(unalignedBufferSize, gpuDevice.GetCapabilities().MinimumUniformBufferOffsetAlignment);
	}
	
	return unalignedBufferSize;
}

u32 GpuBuffer::CalculateTotalBufferSize(const GpuBufferInformation& information, const SPtr<GpuDevice>& gpuDevice)
{
	const u32 stride = CalculateSuballocatedBufferSize(information, gpuDevice);
	return stride * Math::Max(1, information.SuballocationCount);
}

namespace bs::ct
{
	GpuBuffer::GpuBuffer(const GpuBufferCreateInformation& createInformation, u32 suballocationSize)
		: mInformation(createInformation), mSuballocationSize(suballocationSize), mTotalSize(createInformation.SuballocationCount * mSuballocationSize)
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

	void GpuBuffer::FlushCache()
	{
		if(!B3D_ENSURE(mCache != nullptr))
			return;

		if(!mIsCacheDirty)
			return;

		WriteData(0, mTotalSize, mCache, BWT_NORMAL);
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

	void GpuBuffer::SyncToCore(const CoreSyncData& data, FrameAlloc& allocator)
	{
		auto* const syncPacket = data.GetSyncPacket<bs::GpuBuffer::SyncPacket>();
		if(!syncPacket)
			return;

		if(syncPacket->BufferData == nullptr)
			return;

		if(B3D_ENSURE(mTotalSize == syncPacket->BufferSize))
		{
			WriteData(0, syncPacket->BufferSize, syncPacket->BufferData);
		}

		allocator.Free(syncPacket->BufferData);
	}
}
