//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsIndexBuffer.h"
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Profiling/BsRenderStats.h"

namespace bs
{
	UINT32 CalcIndexSize(IndexType type)
	{
		switch (type)
		{
		case IT_16BIT:
			return Sizeof(unsigned short);
		default:
		case IT_32BIT:
			return Sizeof(unsigned int);
		}
	}

	void CheckValidDesc(const INDEX_BUFFER_DESC& desc)
	{
		if(desc.numIndices == 0)
			BS_EXCEPT(InvalidParametersException, "Index buffer index count is not allowed to be zero.");
	}

	IndexBufferProperties::IndexBufferProperties(IndexType idxType, UINT32 numIndices)
		:mIndexType(idxType), mNumIndices(numIndices), mIndexSize(calcIndexSize(idxType))
	{ }

	IndexBuffer::IndexBuffer(const INDEX_BUFFER_DESC& desc)
		:mProperties(desc.indexType, desc.numIndices), mUsage(desc.usage)
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
		desc.indexType = mProperties.mIndexType;
		desc.numIndices = mProperties.mNumIndices;
		desc.usage = mUsage;

		return ct::HardwareBufferManager::instance().CreateIndexBufferInternal(desc);
	}

	SPtr<IndexBuffer> IndexBuffer::Create(const INDEX_BUFFER_DESC& desc)
	{
		return HardwareBufferManager::Instance().CreateIndexBuffer(desc);
	}

	namespace ct
	{
	IndexBuffer::IndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: HardwareBuffer(calcIndexSize(desc.indexType) * desc.numIndices, desc.usage, deviceMask)
		, MProperties(desc.indexType, desc.numIndices)
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
		CoreObject::initialize();
	}

	void* IndexBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
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

	void IndexBuffer::ReadData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		mBuffer->ReadData(offset, length, dest, deviceIdx, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
	}

	void IndexBuffer::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags,
		UINT32 queueIdx)
	{
		mBuffer->WriteData(offset, length, source, writeFlags, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
	}

	void IndexBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length,
		bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto& srcIndexBuffer = static_cast<IndexBuffer&>(srcBuffer);
		mBuffer->CopyData(*srcIndexBuffer.mBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
	}

	SPtr<GpuBuffer> IndexBuffer::GetLoadStore(GpuBufferType type, GpuBufferFormat format, UINT32 elementSize)
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

		UINT32 elemSize = type == GBT_STANDARD ? bs::GpuBuffer::getFormatSize(format) : elementSize;
		if((mBuffer->GetSize() % elemSize) != 0)
		{
			BS_LOG(Error, RenderBackend,
				"Size of the buffer isn't divisible by individual element size provided for the buffer view.");
			return nullptr;
		}

		GPU_BUFFER_DESC desc;
		desc.type = type;
		desc.format = format;
		desc.usage = mUsage;
		desc.elementSize = elementSize;
		desc.elementCount = mBuffer->GetSize() / elemSize;

		if(!mSharedBuffer)
			mSharedBuffer = bs_shared_ptr(mBuffer, mBufferDeleter);

		SPtr<GpuBuffer> newView = GpuBuffer::create(desc, mSharedBuffer);
		mLoadStoreViews.push_back(newView);
		
		return newView;
	}

	SPtr<IndexBuffer> IndexBuffer::Create(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferManager::Instance().CreateIndexBuffer(desc, deviceMask);
	}
	}
}
