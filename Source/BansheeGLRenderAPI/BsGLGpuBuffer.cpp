//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLGpuBuffer.h"
#include "Debug/BsDebug.h"
#include "Profiling/BsRenderStats.h"
#include "BsGLPixelFormat.h"
#include "BsGLHardwareBufferManager.h"
#include "BsGLCommandBuffer.h"

namespace bs { namespace ct
{
	GLGpuBuffer::GLGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBuffer(desc, deviceMask), mTextureID(0), mFormat(0)
	{
		if(desc.type == GBT_APPENDCONSUME || desc.type == GBT_INDIRECTARGUMENT || desc.type == GBT_RAW)
			LOGERR("Only standard and structured buffers are supported on OpenGL.");

		if (desc.useCounter)
			LOGERR("Buffer counters not supported on OpenGL.");

		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");

		// Note: Implement OpenGL shader storage buffers, append/consume buffers, transform feedback buffers, 
		// indirect argument buffers and counter buffers

		mFormat = GLPixelUtil::getBufferFormat(desc.format);
	}

	GLGpuBuffer::~GLGpuBuffer()
	{
		glDeleteTextures(1, &mTextureID);
		BS_CHECK_GL_ERROR();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void GLGpuBuffer::initialize()
	{
		// Create buffer
		if(mProperties.getType() == GBT_STRUCTURED)
		{
#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
			const auto& props = getProperties();
			UINT32 size = props.getElementCount() * props.getElementSize();
			mBuffer.initialize(GL_SHADER_STORAGE_BUFFER, size, props.getUsage());
#else
			LOGWRN("SSBOs are not supported on the current OpenGL version.");
#endif
		}
		else
		{
			const auto& props = getProperties();
			UINT32 size = props.getElementCount() * props.getElementSize();
			mBuffer.initialize(GL_TEXTURE_BUFFER, size, props.getUsage());

			// Create texture
			glGenTextures(1, &mTextureID);
			BS_CHECK_GL_ERROR();

			glBindTexture(GL_TEXTURE_BUFFER, mTextureID);
			BS_CHECK_GL_ERROR();

			glTexBuffer(GL_TEXTURE_BUFFER, mFormat, mBuffer.getGLBufferId());
			BS_CHECK_GL_ERROR();
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);
		GpuBuffer::initialize();
	}

	void* GLGpuBuffer::lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
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

	void GLGpuBuffer::unlock()
	{
		mBuffer.unlock();
	}

	void GLGpuBuffer::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		mBuffer.readData(offset, length, dest);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
	}

	void GLGpuBuffer::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags,
									UINT32 queueIdx)
	{
		mBuffer.writeData(offset, length, source, writeFlags);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
	}

	void GLGpuBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
		bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [this](HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length)
		{
			GLGpuBuffer& glSrcBuffer = static_cast<GLGpuBuffer&>(srcBuffer);
			glSrcBuffer.mBuffer.copyData(mBuffer, srcOffset, dstOffset, length);
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
