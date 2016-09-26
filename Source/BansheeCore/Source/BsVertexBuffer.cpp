//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVertexBuffer.h"
#include "BsHardwareBufferManager.h"

namespace BansheeEngine 
{
	VertexBufferProperties::VertexBufferProperties(UINT32 numVertices, UINT32 vertexSize)
		:mNumVertices(numVertices), mVertexSize(vertexSize)
	{

	}

	VertexBufferCore::VertexBufferCore(UINT32 vertexSize, UINT32 numVertices, GpuBufferUsage usage, bool streamOut)
		:HardwareBuffer(usage, false), mProperties(numVertices, vertexSize)
	{
		mSizeInBytes = mProperties.mVertexSize * mProperties.mNumVertices;
	}

	SPtr<VertexBufferCore> VertexBufferCore::create(UINT32 vertexSize, UINT32 numVerts, GpuBufferUsage usage, bool streamOut)
	{
		return HardwareBufferCoreManager::instance().createVertexBuffer(vertexSize, numVerts, usage, streamOut);
	}

	VertexBuffer::VertexBuffer(UINT32 vertexSize, UINT32 numVertices, GpuBufferUsage usage, bool streamOut)
		: mProperties(numVertices, vertexSize), mUsage(usage), mStreamOut(streamOut)
    {

    }

	SPtr<CoreObjectCore> VertexBuffer::createCore() const
	{
		return HardwareBufferCoreManager::instance().createVertexBufferInternal(mProperties.mVertexSize, 
			mProperties.mNumVertices, mUsage, mStreamOut);
	}

	SPtr<VertexBufferCore> VertexBuffer::getCore() const
	{
		return std::static_pointer_cast<VertexBufferCore>(mCoreSpecific);
	}

	SPtr<VertexBuffer> VertexBuffer::create(UINT32 vertexSize, UINT32 numVerts, GpuBufferUsage usage, bool streamOut)
	{
		return HardwareBufferManager::instance().createVertexBuffer(vertexSize, numVerts, usage, streamOut);
	}
}
