//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLGpuBuffer.h"
#include "BsDebug.h"
#include "BsRenderStats.h"
#include "BsGLPixelFormat.h"
#include "BsGLHardwareBufferManager.h"

namespace BansheeEngine
{
	GLGpuBufferCore::GLGpuBufferCore(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBufferCore(desc, deviceMask), mTextureID(0), mFormat(0)
	{
		if(desc.type != GBT_STANDARD)
			LOGERR("Only standard buffers are support on OpenGL.");

		if (desc.useCounter)
			LOGERR("Buffer counters not supported on OpenGL.");

		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");

		// Note: Implement OpenGL shader storage buffers, append/consume buffers, transform feedback buffers, 
		// indirect argument buffers and counter buffers

		mFormat = GLPixelUtil::getBufferFormat(desc.format);
	}

	GLGpuBufferCore::~GLGpuBufferCore()
	{
		glDeleteTextures(1, &mTextureID);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void GLGpuBufferCore::initialize()
	{
		// Create buffer
		const auto& props = getProperties();
		UINT32 size = props.getElementCount() * props.getElementSize();
		mBuffer.initialize(GL_TEXTURE_BUFFER, size, props.getUsage());
		
		// Create texture
		glGenTextures(1, &mTextureID);
		glBindTexture(GL_TEXTURE_BUFFER, mTextureID);
		glTexBuffer(GL_TEXTURE_BUFFER, mFormat, mBuffer.getGLBufferId());

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);
		GpuBufferCore::initialize();
	}

	void* GLGpuBufferCore::lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD 
			|| options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
		}
#endif

		return mBuffer.lock(offset, length, options);
	}

	void GLGpuBufferCore::unlock()
	{
		mBuffer.unlock();
	}

	void GLGpuBufferCore::readData(UINT32 offset, UINT32 length, void* dest, UINT32 queueIdx)
	{
		mBuffer.readData(offset, length, dest);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
	}

	void GLGpuBufferCore::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags,
									UINT32 queueIdx)
	{
		mBuffer.writeData(offset, length, source, writeFlags);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
	}

	void GLGpuBufferCore::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
								   UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, UINT32 queueIdx)
	{
		GLGpuBufferCore& glSrcBuffer = static_cast<GLGpuBufferCore&>(srcBuffer);

		GLuint srcId = glSrcBuffer.getGLBufferId();
		glCopyBufferSubData(srcId, getGLBufferId(), srcOffset, dstOffset, length);
	}
}