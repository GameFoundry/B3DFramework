//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLHardwareBuffer.h"
#include "BsGLHardwareBufferManager.h"
#include "Error/BsException.h"
#include "BsGLCommandBuffer.h"

using namespace bs;
using namespace bs::ct;

GLHardwareBuffer::GLHardwareBuffer(GLenum target, u32 size, GpuBufferUsage usage)
	: GpuBuffer(size, usage, GDF_DEFAULT), mTarget(target)
{
	glGenBuffers(1, &mBufferId);
	B3D_CHECK_GL_ERROR();

	if(!mBufferId)
		B3D_EXCEPT(InternalErrorException, "Cannot create GL buffer");

	glBindBuffer(target, mBufferId);
	B3D_CHECK_GL_ERROR();

	glBufferData(target, size, nullptr, GLHardwareBufferManager::GetGlUsage(usage));
	B3D_CHECK_GL_ERROR();
}

GLHardwareBuffer::~GLHardwareBuffer()
{
	if(mBufferId != 0)
	{
		glDeleteBuffers(1, &mBufferId);
		B3D_CHECK_GL_ERROR();
	}
}

void* GLHardwareBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
	// If no buffer ID it's assumed this type of buffer is unsupported and we silently fail (it's up to the creator
	// if the buffer to check for support and potentially print a warning)
	if(mBufferId == 0)
		return nullptr;

	GLenum access = 0;

	glBindBuffer(mTarget, mBufferId);
	B3D_CHECK_GL_ERROR();

	if((options == GBL_WRITE_ONLY) || (options == GBL_WRITE_ONLY_NO_OVERWRITE) || (options == GBL_WRITE_ONLY_DISCARD))
	{
		access = GL_MAP_WRITE_BIT;

		if(options == GBL_WRITE_ONLY_DISCARD)
			access |= GL_MAP_INVALIDATE_BUFFER_BIT;
		else if(options == GBL_WRITE_ONLY_NO_OVERWRITE)
			access |= GL_MAP_UNSYNCHRONIZED_BIT;
	}
	else if(options == GBL_READ_ONLY)
		access = GL_MAP_READ_BIT;
	else
		access = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;

	void* buffer = nullptr;

	if(length > 0)
	{
		buffer = glMapBufferRange(mTarget, offset, length, access);
		B3D_CHECK_GL_ERROR();

		if(buffer == nullptr)
			B3D_EXCEPT(InternalErrorException, "Cannot map OpenGL buffer.");

		mZeroLocked = false;
	}
	else
		mZeroLocked = true;

	return static_cast<void*>(static_cast<unsigned char*>(buffer));
}

void GLHardwareBuffer::Unmap()
{
	if(mBufferId == 0)
		return;

	glBindBuffer(mTarget, mBufferId);
	B3D_CHECK_GL_ERROR();

	if(!mZeroLocked)
	{
		if(!glUnmapBuffer(mTarget))
		{
			B3D_CHECK_GL_ERROR();
			B3D_EXCEPT(InternalErrorException, "Buffer data corrupted, please reload.");
		}
	}
}

void GLHardwareBuffer::ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx, u32 queueIdx)
{
	if(mBufferId == 0)
		return;

	void* bufferData = Lock(offset, length, GBL_READ_ONLY, deviceIdx, queueIdx);
	memcpy(dest, bufferData, length);
	Unlock();
}

void GLHardwareBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, u32 queueIdx)
{
	if(mBufferId == 0)
		return;

	GpuLockOptions lockOption = GBL_WRITE_ONLY;
	if(writeFlags == BWT_DISCARD)
		lockOption = GBL_WRITE_ONLY_DISCARD;
	else if(writeFlags == BTW_NO_OVERWRITE)
		lockOption = GBL_WRITE_ONLY_NO_OVERWRITE;

	void* bufferData = Lock(offset, length, lockOption, 0, queueIdx);
	memcpy(bufferData, source, length);
	Unlock();
}

void GLHardwareBuffer::CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer, const SPtr<ct::CommandBuffer>& commandBuffer)
{
	if(mBufferId == 0)
		return;

	auto executeRef = [this](GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length)
	{
		GLHardwareBuffer& glSrcBuffer = static_cast<GLHardwareBuffer&>(srcBuffer);

		glBindBuffer(GL_COPY_READ_BUFFER, glSrcBuffer.GetGlBufferId());
		B3D_CHECK_GL_ERROR();

		glBindBuffer(GL_COPY_WRITE_BUFFER, mBufferId);
		B3D_CHECK_GL_ERROR();

		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, length);
		B3D_CHECK_GL_ERROR();
	};

	if(commandBuffer == nullptr)
		executeRef(srcBuffer, srcOffset, dstOffset, length);
	else
	{
		auto execute = [&]()
		{ executeRef(srcBuffer, srcOffset, dstOffset, length); };

		SPtr<GLCommandBuffer> cb = std::static_pointer_cast<GLCommandBuffer>(commandBuffer);
		cb->QueueCommand(execute);
	}
}
