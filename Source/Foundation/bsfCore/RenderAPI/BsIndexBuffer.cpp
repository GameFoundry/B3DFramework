//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsIndexBuffer.h"
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsGenericGpuBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;

u32 CalcIndexSize(IndexType type)
{
	switch(type)
	{
	case IT_16BIT:
		return sizeof(unsigned short);
	default:
	case IT_32BIT:
		return sizeof(unsigned int);
	}
}

void CheckValidDesc(const IndexBufferCreateInformation& desc)
{
	if(desc.IndexCount == 0)
		B3D_EXCEPT(InvalidParametersException, "Index buffer index count is not allowed to be zero.");
}

IndexBufferProperties::IndexBufferProperties(IndexType idxType, u32 numIndices)
	: mIndexType(idxType), mNumIndices(numIndices), mIndexSize(CalcIndexSize(idxType))
{}

IndexBuffer::IndexBuffer(const IndexBufferCreateInformation& desc)
	: mProperties(desc.IndexType, desc.IndexCount), mBufferFlags(desc.Flags)
{
#if B3D_DEBUG
	CheckValidDesc(desc);
#endif
}

SPtr<ct::IndexBuffer> IndexBuffer::GetCore() const
{
	return std::static_pointer_cast<ct::IndexBuffer>(mCoreSpecific);
}

SPtr<ct::CoreObject> IndexBuffer::CreateCore() const
{
	IndexBufferCreateInformation desc;
	desc.IndexType = mProperties.mIndexType;
	desc.IndexCount = mProperties.mNumIndices;
	desc.Flags = mBufferFlags;

	return ct::HardwareBufferManager::Instance().CreateIndexBufferInternal(desc);
}

SPtr<IndexBuffer> IndexBuffer::Create(const IndexBufferCreateInformation& desc)
{
	return HardwareBufferManager::Instance().CreateIndexBuffer(desc);
}

namespace bs { namespace ct
{
IndexBuffer::IndexBuffer(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: GpuBuffer(GpuBufferType::Index, CalcIndexSize(desc.IndexType) * desc.IndexCount, desc.Flags, deviceMask)
	, mProperties(desc.IndexType, desc.IndexCount)
{
#if B3D_DEBUG
	CheckValidDesc(desc);
#endif
}

IndexBuffer::~IndexBuffer()
{
	if(mBuffer && !mSharedBuffer)
		mBufferDeleter(mBuffer);

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_IndexBuffer);
}

void IndexBuffer::Initialize()
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_IndexBuffer);
	CoreObject::Initialize();
}

void* IndexBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
#if B3D_PROFILING_ENABLED
	if(options == GBL_READ_ONLY || options == GBL_READ_WRITE)
	{
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_IndexBuffer);
	}

	if(options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
	{
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_IndexBuffer);
	}
#endif

	return mBuffer->Lock(offset, length, options, deviceIdx, queueIdx);
}

void IndexBuffer::Unmap()
{
	mBuffer->Unlock();
}

void IndexBuffer::ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx, u32 queueIdx)
{
	mBuffer->ReadData(offset, length, dest, deviceIdx, queueIdx);

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_IndexBuffer);
}

void IndexBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, u32 queueIdx)
{
	mBuffer->WriteData(offset, length, source, writeFlags, queueIdx);

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_IndexBuffer);
}

void IndexBuffer::CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
{
	auto& srcIndexBuffer = static_cast<IndexBuffer&>(srcBuffer);
	mBuffer->CopyData(*srcIndexBuffer.mBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
}

SPtr<GenericGpuBuffer> IndexBuffer::GetLoadStore(GenericGpuBufferType type, GpuBufferFormat format, u32 elementSize)
{
	if(!mBufferFlags.IsSet(GpuBufferFlag::AllowWritesOnTheGPU))
		return nullptr;

	for(const auto& entry : mLoadStoreViews)
	{
		const GenericGpuBufferProperties& props = entry->GetProperties();
		if(props.GetType() == type)
		{
			if(type == GBT_STANDARD && props.GetFormat() == format)
				return entry;

			if(type == GBT_STRUCTURED && props.GetElementSize() == elementSize)
				return entry;
		}
	}

	u32 elemSize = type == GBT_STANDARD ? bs::GenericGpuBuffer::GetFormatSize(format) : elementSize;
	if((mBuffer->GetSize() % elemSize) != 0)
	{
		B3D_LOG(Error, RenderBackend, "Size of the buffer isn't divisible by individual element size provided for the buffer view.");
		return nullptr;
	}

	GenericGpuBufferCreateInformation desc;
	desc.Type = type;
	desc.Format = format;
	desc.Flags = mBufferFlags;
	desc.ElementSize = elementSize;
	desc.ElementCount = mBuffer->GetSize() / elemSize;

	if(!mSharedBuffer)
		mSharedBuffer = B3DMakeSharedFromExisting(mBuffer, mBufferDeleter);

	SPtr<GenericGpuBuffer> newView = GenericGpuBuffer::Create(desc, mSharedBuffer);
	mLoadStoreViews.push_back(newView);

	return newView;
}

SPtr<IndexBuffer> IndexBuffer::Create(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateIndexBuffer(desc, deviceMask);
}
}}
