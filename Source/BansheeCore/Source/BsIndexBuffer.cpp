//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsIndexBuffer.h"
#include "BsHardwareBufferManager.h"
#include "BsRenderAPI.h"

namespace BansheeEngine 
{
	IndexBufferProperties::IndexBufferProperties(IndexType idxType, UINT32 numIndices)
		:mIndexType(idxType), mNumIndices(numIndices)
	{
		switch (mIndexType)
		{
		case IT_16BIT:
			mIndexSize = sizeof(unsigned short);
			break;
		case IT_32BIT:
			mIndexSize = sizeof(unsigned int);
			break;
		}
	}

	IndexBufferCore::IndexBufferCore(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:HardwareBuffer(desc.usage, false), mProperties(desc.indexType, desc.numIndices)
	{ 
		mSizeInBytes = mProperties.mIndexSize * mProperties.mNumIndices;
	}

	SPtr<IndexBufferCore> IndexBufferCore::create(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferCoreManager::instance().createIndexBuffer(desc, deviceMask);
	}

    IndexBuffer::IndexBuffer(const INDEX_BUFFER_DESC& desc)
		:mProperties(desc.indexType, desc.numIndices), mUsage(desc.usage)
    {

	}

	SPtr<IndexBufferCore> IndexBuffer::getCore() const
	{
		return std::static_pointer_cast<IndexBufferCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> IndexBuffer::createCore() const
	{
		INDEX_BUFFER_DESC desc;
		desc.indexType = mProperties.mIndexType;
		desc.numIndices = mProperties.mNumIndices;
		desc.usage = mUsage;

		return HardwareBufferCoreManager::instance().createIndexBufferInternal(desc);
	}

	SPtr<IndexBuffer> IndexBuffer::create(const INDEX_BUFFER_DESC& desc)
	{
		return HardwareBufferManager::instance().createIndexBuffer(desc);
	}
}