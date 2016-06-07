//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLIndexBuffer.h"
#include "BsGLHardwareBufferManager.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace BansheeEngine 
{
	GLIndexBufferCore::GLIndexBufferCore(IndexType idxType, UINT32 numIndices, GpuBufferUsage usage)
		:IndexBufferCore(idxType, numIndices, usage)
	{  }

	GLIndexBufferCore::~GLIndexBufferCore()
	{
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_IndexBuffer);
	}

	void GLIndexBufferCore::initialize()
	{
		mBuffer = GLBuffer(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, mUsage);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_IndexBuffer);
		IndexBufferCore::initialize();
	}

	void* GLIndexBufferCore::lockImpl(UINT32 offset, UINT32 length, GpuLockOptions options)
	{
		return mBuffer.lock(offset, length, options);
	}

	void GLIndexBufferCore::unlockImpl()
	{
		mBuffer.unlock();
	}

	void GLIndexBufferCore::readData(UINT32 offset, UINT32 length, void* pDest)
	{
		mBuffer.readData(offset, length, pDest);
	}

	void GLIndexBufferCore::writeData(UINT32 offset, UINT32 length,
		const void* pSource, BufferWriteType writeFlags)
	{
		mBuffer.writeData(offset, length, pSource, writeFlags);
	}
}
