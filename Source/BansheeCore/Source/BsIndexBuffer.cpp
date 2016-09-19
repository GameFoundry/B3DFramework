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

	IndexBufferCore::IndexBufferCore(IndexType idxType, UINT32 numIndices, GpuBufferUsage usage)
		:HardwareBuffer(usage, false), mProperties(idxType, numIndices)
	{ 
		mSizeInBytes = mProperties.mIndexSize * mProperties.mNumIndices;
	}

	SPtr<IndexBufferCore> IndexBufferCore::create(IndexType itype, UINT32 numIndexes, GpuBufferUsage usage)
	{
		return HardwareBufferCoreManager::instance().createIndexBuffer(itype, numIndexes, usage);
	}

    IndexBuffer::IndexBuffer(IndexType idxType, UINT32 numIndexes, GpuBufferUsage usage) 
		:mProperties(idxType, numIndexes), mUsage(usage)
    {

	}

	SPtr<IndexBufferCore> IndexBuffer::getCore() const
	{
		return std::static_pointer_cast<IndexBufferCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> IndexBuffer::createCore() const
	{
		return HardwareBufferCoreManager::instance().createIndexBufferInternal(mProperties.mIndexType, 
			mProperties.mNumIndices, mUsage);
	}

	SPtr<IndexBuffer> IndexBuffer::create(IndexType itype, UINT32 numIndices, GpuBufferUsage usage)
	{
		return HardwareBufferManager::instance().createIndexBuffer(itype, numIndices, usage);
	}
}