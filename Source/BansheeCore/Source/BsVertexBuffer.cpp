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

	VertexBufferCore::VertexBufferCore(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:HardwareBuffer(desc.usage, false), mProperties(desc.numVerts, desc.vertexSize)
	{
		mSizeInBytes = mProperties.mVertexSize * mProperties.mNumVertices;
	}

	SPtr<VertexBufferCore> VertexBufferCore::create(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferCoreManager::instance().createVertexBuffer(desc, deviceMask);
	}

	VertexBuffer::VertexBuffer(const VERTEX_BUFFER_DESC& desc)
		: mProperties(desc.numVerts, desc.vertexSize), mUsage(desc.usage), mStreamOut(desc.streamOut)
    {

    }

	SPtr<CoreObjectCore> VertexBuffer::createCore() const
	{
		VERTEX_BUFFER_DESC desc;
		desc.vertexSize = mProperties.mVertexSize;
		desc.numVerts = mProperties.mNumVertices;
		desc.usage = mUsage;
		desc.streamOut = mStreamOut;

		return HardwareBufferCoreManager::instance().createVertexBufferInternal(desc);
	}

	SPtr<VertexBufferCore> VertexBuffer::getCore() const
	{
		return std::static_pointer_cast<VertexBufferCore>(mCoreSpecific);
	}

	SPtr<VertexBuffer> VertexBuffer::create(const VERTEX_BUFFER_DESC& desc)
	{
		return HardwareBufferManager::instance().createVertexBuffer(desc);
	}
}
