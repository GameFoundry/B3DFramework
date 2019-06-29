//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLGpuBuffer.h"
#include "Profiling/BsRenderStats.h"
#include "BsGLPixelFormat.h"
#include "BsGLCommandBuffer.h"

namespace bs { namespace ct
{
	static void deleteBuffer(HardwareBuffer* buffer)
	{
		bs_pool_delete(static_cast<GLHardwareBuffer*>(buffer));
	}

	GLGpuBuffer::GLGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBuffer(desc, deviceMask)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");

		mFormat = GLPixelUtil::getBufferFormat(desc.format);
	}

	GLGpuBuffer::GLGpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
		: GpuBuffer(desc, std::move(underlyingBuffer))
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
	}

	void GLGpuBuffer::initialize()
	{
		mBufferDeleter = &deleteBuffer;

		// Create a buffer if not wrapping an external one
		if(!mBuffer)
		{
			if (mProperties.getType() == GBT_STRUCTURED)
			{
#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
				const auto& props = getProperties();
				UINT32 size = props.getElementCount() * props.getElementSize();
				mBuffer = bs_pool_new<GLHardwareBuffer>(GL_SHADER_STORAGE_BUFFER, size, props.getUsage());
#else
				BS_LOG(Warning, RenderBackend, "SSBOs are not supported on the current OpenGL version.");
#endif
			}
			else
			{
				const auto& props = getProperties();
				UINT32 size = props.getElementCount() * props.getElementSize();
				mBuffer = bs_pool_new<GLHardwareBuffer>(GL_TEXTURE_BUFFER, size, props.getUsage());
			}
		}

		if(mProperties.getType() != GBT_STRUCTURED)
		{
			// Create texture with a specific format
			glGenTextures(1, &mTextureID);
			BS_CHECK_GL_ERROR();

			glBindTexture(GL_TEXTURE_BUFFER, mTextureID);
			BS_CHECK_GL_ERROR();

			glTexBuffer(GL_TEXTURE_BUFFER, mFormat, static_cast<GLHardwareBuffer*>(mBuffer)->getGLBufferId());
			BS_CHECK_GL_ERROR();
		}

		GpuBuffer::initialize();
	}
}}
