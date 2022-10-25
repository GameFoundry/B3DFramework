//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsIndexBuffer.h"
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;

u32 calcIndexSize(IndexType type)
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

void checkValidDesc(const INDEX_BUFFER_DESC& desc)
{
	if(desc.NumIndices == 0)
		BS_EXCEPT(InvalidParametersException, "Index buffer index count is not allowed to be zero.");
}

IndexBufferProperties::IndexBufferProperties(IndexType idxType, u32 numIndices)
	: mIndexType(idxType), mNumIndices(numIndices), mIndexSize(calcIndexSize(idxType))
{}

IndexBuffer::IndexBuffer(const INDEX_BUFFER_DESC& desc)
	: mProperties(desc.IndexType, desc.NumIndices), mUsage(desc.Usage)
{
#if BS_DEBUG_MODE
	checkValidDesc(desc);
#endif
}

SPtr<ct::IndexBuffer> IndexBuffer::GetCore() const
{
	return std::static_pointer_cast<ct::IndexBuffer>(mCoreSpecific);
}

SPtr<ct::CoreObject> IndexBuffer::CreateCore() const
{
	INDEX_BUFFER_DESC desc;
	desc.IndexType = mProperties.mIndexType;
	desc.NumIndices = mProperties.mNumIndices;
	desc.Usage = mUsage;

	return ct::HardwareBufferManager::Instance().CreateIndexBufferInternal(desc);
}

SPtr<IndexBuffer> IndexBuffer::Create(const INDEX_BUFFER_DESC& desc)
{
	return HardwareBufferManager::Instance().CreateIndexBuffer(desc);
}

namespace bs { namespace ct
{
IndexBuffer::IndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	: HardwareBuffer(calcIndexSize(desc.IndexType) * desc.NumIndices, desc.Usage, deviceMask)
	, mProperties(desc.IndexType, desc.NumIndices)
{
#if BS_DEBUG_MODE
	checkValidDesc(desc);
#endif
}

IndexBuffer::~IndexBuffer()
{
	if(mBuffer && !mSharedBuffer)
		mBufferDeleter(mBuffer);

	BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_IndexBuffer);
}

void IndexBuffer::Initialize()
{
	BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_IndexBuffer);
	CoreObject::Initialize();
}

void* IndexBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
#if BS_PROFILING_ENABLED
	if(options == GBL_READ_ONLY || options == GBL_READ_WRITE)
	{
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
	}

	if(options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
	{
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
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

	BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
}

void IndexBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, u32 queueIdx)
{
	mBuffer->WriteData(offset, length, source, writeFlags, queueIdx);

	BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
}

void IndexBuffer::CopyData(HardwareBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
{
	auto& srcIndexBuffer = static_cast<IndexBuffer&>(srcBuffer);
	mBuffer->CopyData(*srcIndexBuffer.mBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
}

SPtr<GpuBuffer> IndexBuffer::GetLoadStore(GpuBufferType type, GpuBufferFormat format, u32 elementSize)
{
	if((mUsage & GBU_LOADSTORE) != GBU_LOADSTORE)
		return nullptr;

	for(const auto& entry : mLoadStoreViews)
	{
		const GpuBufferProperties& props = entry->GetProperties();
		if(props.GetType() == type)
		{
			if(type == GBT_STANDARD && props.GetFormat() == format)
				return entry;

			if(type == GBT_STRUCTURED && props.GetElementSize() == elementSize)
				return entry;
		}
	}

	u32 elemSize = type == GBT_STANDARD ? bs::GpuBuffer::GetFormatSize(format) : elementSize;
	if((mBuffer->GetSize() % elemSize) != 0)
	{
		BS_LOG(Error, RenderBackend, "Size of the buffer isn't divisible by individual element size provided for the buffer view.");
		return nullptr;
	}

	GPU_BUFFER_DESC desc;
	desc.Type = type;
	desc.Format = format;
	desc.Usage = mUsage;
	desc.ElementSize = elementSize;
	desc.ElementCount = mBuffer->GetSize() / elemSize;

	if(!mSharedBuffer)
		mSharedBuffer = bs_shared_ptr(mBuffer, mBufferDeleter);

	SPtr<GpuBuffer> newView = GpuBuffer::Create(desc, mSharedBuffer);
	mLoadStoreViews.push_back(newView);

	return newView;
}

SPtr<IndexBuffer> IndexBuffer::Create(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateIndexBuffer(desc, deviceMask);
}
}}
