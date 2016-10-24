//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLBuffer.h"
#include "BsGLHardwareBufferManager.h"
#include "BsException.h"

namespace BansheeEngine
{
	GLBuffer::GLBuffer()
		:mTarget(0), mBufferId(0), mZeroLocked(false)
	{
		
	}

	GLBuffer::GLBuffer(GLenum target, UINT32 size, GpuBufferUsage usage)
		:mTarget(target), mBufferId(0), mZeroLocked(false)
	{
		initialize(target, size, usage);
	}

	GLBuffer::~GLBuffer()
	{
		if(mBufferId != 0)
			glDeleteBuffers(1, &mBufferId);
	}

	void GLBuffer::initialize(GLenum target, UINT32 size, GpuBufferUsage usage)
	{
		assert(mBufferId == 0 && "Buffer already initialized");

		mTarget = target;

		glGenBuffers(1, &mBufferId);

		if (!mBufferId)
		{
			BS_EXCEPT(InternalErrorException, "Cannot create GL vertex buffer");
		}

		glBindBuffer(target, mBufferId);
		glBufferData(target, size, nullptr, GLHardwareBufferCoreManager::getGLUsage(usage));
	}

	void* GLBuffer::lock(UINT32 offset, UINT32 length, GpuLockOptions options)
	{
		GLenum access = 0;

		glBindBuffer(mTarget, mBufferId);

		if ((options == GBL_WRITE_ONLY) || (options == GBL_WRITE_ONLY_NO_OVERWRITE) || (options == GBL_WRITE_ONLY_DISCARD))
		{
			access = GL_MAP_WRITE_BIT;

			if (options == GBL_WRITE_ONLY_DISCARD)
				access |= GL_MAP_INVALIDATE_BUFFER_BIT;
			else if (options == GBL_WRITE_ONLY_NO_OVERWRITE)
				access |= GL_MAP_UNSYNCHRONIZED_BIT;
		}
		else if (options == GBL_READ_ONLY)
			access = GL_MAP_READ_BIT;
		else
			access = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;

		void* buffer = nullptr;

		if (length > 0)
		{
			buffer = glMapBufferRange(mTarget, offset, length, access);

			if (buffer == nullptr)
			{
				BS_EXCEPT(InternalErrorException, "Cannot map OpenGL buffer.");
			}

			mZeroLocked = false;
		}
		else
			mZeroLocked = true;

		return static_cast<void*>(static_cast<unsigned char*>(buffer));
	}

	void GLBuffer::unlock()
	{
		glBindBuffer(mTarget, mBufferId);

		if (!mZeroLocked)
		{
			if (!glUnmapBuffer(mTarget))
			{
				BS_EXCEPT(InternalErrorException, "Buffer data corrupted, please reload.");
			}
		}
	}

	void GLBuffer::readData(UINT32 offset, UINT32 length, void* pDest)
	{
		void* bufferData = lock(offset, length, GBL_READ_ONLY);
		memcpy(pDest, bufferData, length);
		unlock();
	}

	void GLBuffer::writeData(UINT32 offset, UINT32 length,
		const void* pSource, BufferWriteType writeFlags)
	{
		GpuLockOptions lockOption = GBL_WRITE_ONLY;
		if (writeFlags == BufferWriteType::Discard)
			lockOption = GBL_WRITE_ONLY_DISCARD;
		else if (writeFlags == BufferWriteType::NoOverwrite)
			lockOption = GBL_WRITE_ONLY_NO_OVERWRITE;

		void* bufferData = lock(offset, length, lockOption);
		memcpy(bufferData, pSource, length);
		unlock();
	}
}