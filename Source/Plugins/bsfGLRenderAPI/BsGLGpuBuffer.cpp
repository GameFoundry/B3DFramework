//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLGpuBuffer.h"
#include "Profiling/BsRenderStats.h"
#include "BsGLPixelFormat.h"
#include "BsGLHardwareBufferManager.h"
#include "BsGLCommandBuffer.h"

namespace bs { namespace ct
{
	GLGpuBuffer::GLGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBuffer(desc, deviceMask)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");

		mFormat = GLPixelUtil::getBufferFormat(desc.format);
	}

	GLGpuBuffer::GLGpuBuffer(const GPU_BUFFER_DESC& desc, const SPtr<GLBuffer>& underlyingBuffer)
		: GpuBuffer(desc, underlyingBuffer), mBuffer(underlyingBuffer.get())
	{
		mFormat = GLPixelUtil::getBufferFormat(desc.format);
	}

	GLGpuBuffer::~GLGpuBuffer()
	{
		if(mProperties.getType() != GBT_STRUCTURED)
		{
			glDeleteTextures(1, &mTextureID);
			BS_CHECK_GL_ERROR();
		}

		if(mBuffer && !mExternalBuffer)
			bs_pool_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void GLGpuBuffer::initialize()
	{
		// Create a buffer if not wrapping an external one
		if(!mBuffer)
		{
			if (mProperties.getType() == GBT_STRUCTURED)
			{
#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
				const auto& props = getProperties();
				UINT32 size = props.getElementCount() * props.getElementSize();
				mBuffer = bs_pool_new<GLBuffer>(GL_SHADER_STORAGE_BUFFER, size, props.getUsage());
#else
				LOGWRN("SSBOs are not supported on the current OpenGL version.");
#endif
			}
			else
			{
				const auto& props = getProperties();
				UINT32 size = props.getElementCount() * props.getElementSize();
				mBuffer = bs_pool_new<GLBuffer>(GL_TEXTURE_BUFFER, size, props.getUsage());
			}
		}

		if(mProperties.getType() != GBT_STRUCTURED)
		{
			// Create texture with a specific format
			glGenTextures(1, &mTextureID);
			BS_CHECK_GL_ERROR();

			glBindTexture(GL_TEXTURE_BUFFER, mTextureID);
			BS_CHECK_GL_ERROR();

			glTexBuffer(GL_TEXTURE_BUFFER, mFormat, mBuffer->getGLBufferId());
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

		return mBuffer->lock(offset, length, options);
	}

	void GLGpuBuffer::unlock()
	{
		mBuffer->unlock();
	}

	void GLGpuBuffer::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		mBuffer->readData(offset, length, dest, deviceIdx, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
	}

	void GLGpuBuffer::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags,
									UINT32 queueIdx)
	{
		mBuffer->writeData(offset, length, source, writeFlags);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
	}

	void GLGpuBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
		bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [this](HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length)
		{
			GLGpuBuffer& glSrcBuffer = static_cast<GLGpuBuffer&>(srcBuffer);
			glSrcBuffer.mBuffer->copyData(*mBuffer, srcOffset, dstOffset, length);
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
