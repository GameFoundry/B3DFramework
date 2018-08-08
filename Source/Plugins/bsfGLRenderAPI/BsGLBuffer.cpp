//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLBuffer.h"
#include "BsGLHardwareBufferManager.h"
#include "Error/BsException.h"
#include "BsGLCommandBuffer.h"

namespace bs { namespace ct
{
	GLBuffer::GLBuffer(GLenum target, UINT32 size, GpuBufferUsage usage)
		: HardwareBuffer(size, usage, GDF_DEFAULT), mTarget(target)
	{
		glGenBuffers(1, &mBufferId);
		BS_CHECK_GL_ERROR();

		if (!mBufferId)
			BS_EXCEPT(InternalErrorException, "Cannot create GL buffer");

		glBindBuffer(target, mBufferId);
		BS_CHECK_GL_ERROR();

		glBufferData(target, size, nullptr, GLHardwareBufferManager::getGLUsage(usage));
		BS_CHECK_GL_ERROR();
	}

	GLBuffer::~GLBuffer()
	{
		if (mBufferId != 0)
		{
			glDeleteBuffers(1, &mBufferId);
			BS_CHECK_GL_ERROR();
		}
	}

	void* GLBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
		// If no buffer ID it's assumed this type of buffer is unsupported and we silently fail (it's up to the creator
		// if the buffer to check for support and potentially print a warning)
		if(mBufferId == 0)
			return nullptr;

		GLenum access = 0;

		glBindBuffer(mTarget, mBufferId);
		BS_CHECK_GL_ERROR();

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
			BS_CHECK_GL_ERROR();

			if (buffer == nullptr)
				BS_EXCEPT(InternalErrorException, "Cannot map OpenGL buffer.");

			mZeroLocked = false;
		}
		else
			mZeroLocked = true;

		return static_cast<void*>(static_cast<unsigned char*>(buffer));
	}

	void GLBuffer::unmap()
	{
		if(mBufferId == 0)
			return;

		glBindBuffer(mTarget, mBufferId);
		BS_CHECK_GL_ERROR();

		if (!mZeroLocked)
		{
			if (!glUnmapBuffer(mTarget))
			{
				BS_CHECK_GL_ERROR();
				BS_EXCEPT(InternalErrorException, "Buffer data corrupted, please reload.");
			}
		}
	}

	void GLBuffer::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		if(mBufferId == 0)
			return;

		void* bufferData = lock(offset, length, GBL_READ_ONLY, deviceIdx, queueIdx);
		memcpy(dest, bufferData, length);
		unlock();
	}

	void GLBuffer::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags, UINT32 queueIdx)
	{
		if(mBufferId == 0)
			return;

		GpuLockOptions lockOption = GBL_WRITE_ONLY;
		if (writeFlags == BWT_DISCARD)
			lockOption = GBL_WRITE_ONLY_DISCARD;
		else if (writeFlags == BTW_NO_OVERWRITE)
			lockOption = GBL_WRITE_ONLY_NO_OVERWRITE;

		void* bufferData = lock(offset, length, lockOption, 0, queueIdx);
		memcpy(bufferData, source, length);
		unlock();
	}

	void GLBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length,
			bool discardWholeBuffer, const SPtr<ct::CommandBuffer>& commandBuffer)
	{
		if(mBufferId == 0)
			return;

		auto executeRef = [this](HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length)
		{
			GLBuffer& glSrcBuffer = static_cast<GLBuffer&>(srcBuffer);

			glBindBuffer(GL_COPY_READ_BUFFER, glSrcBuffer.getGLBufferId());
			BS_CHECK_GL_ERROR();

			glBindBuffer(GL_COPY_WRITE_BUFFER, mBufferId);
			BS_CHECK_GL_ERROR();

			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, length);
			BS_CHECK_GL_ERROR();
		};

		if (commandBuffer == nullptr)
			executeRef(srcBuffer, srcOffset, dstOffset, length);
		else
		{
			auto execute = [&]() { executeRef(srcBuffer, srcOffset, dstOffset, length); };

			SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

	}
}}
