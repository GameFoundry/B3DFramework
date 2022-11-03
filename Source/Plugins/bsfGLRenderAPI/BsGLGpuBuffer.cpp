//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLGpuBuffer.h"
#include "Profiling/BsRenderStats.h"
#include "BsGLPixelFormat.h"
#include "BsGLCommandBuffer.h"

using namespace bs;
using namespace bs::ct;

static void DeleteBuffer(HardwareBuffer* buffer)
{
	B3DPoolDelete(static_cast<GLHardwareBuffer*>(buffer));
}

GLGpuBuffer::GLGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	: GpuBuffer(desc, deviceMask)
{
	B3D_ASSERT((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");

	mFormat = GLPixelUtil::GetBufferFormat(desc.Format);
}

GLGpuBuffer::GLGpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
	: GpuBuffer(desc, std::move(underlyingBuffer))
{
	mFormat = GLPixelUtil::GetBufferFormat(desc.Format);
}

GLGpuBuffer::~GLGpuBuffer()
{
	if(mProperties.GetType() != GBT_STRUCTURED)
	{
		glDeleteTextures(1, &mTextureID);
		B3D_CHECK_GL_ERROR();
	}
}

void GLGpuBuffer::Initialize()
{
	mBufferDeleter = &DeleteBuffer;

	// Create a buffer if not wrapping an external one
	if(!mBuffer)
	{
		if(mProperties.GetType() == GBT_STRUCTURED)
		{
#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
			const auto& props = GetProperties();
			u32 size = props.GetElementCount() * props.GetElementSize();
			mBuffer = B3DPoolNew<GLHardwareBuffer>(GL_SHADER_STORAGE_BUFFER, size, props.GetUsage());
#else
			B3D_LOG(Warning, RenderBackend, "SSBOs are not supported on the current OpenGL version.");
#endif
		}
		else
		{
			const auto& props = GetProperties();
			u32 size = props.GetElementCount() * props.GetElementSize();
			mBuffer = B3DPoolNew<GLHardwareBuffer>(GL_TEXTURE_BUFFER, size, props.GetUsage());
		}
	}

	if(mProperties.GetType() != GBT_STRUCTURED)
	{
		// Create texture with a specific format
		glGenTextures(1, &mTextureID);
		B3D_CHECK_GL_ERROR();

		glBindTexture(GL_TEXTURE_BUFFER, mTextureID);
		B3D_CHECK_GL_ERROR();

		glTexBuffer(GL_TEXTURE_BUFFER, mFormat, static_cast<GLHardwareBuffer*>(mBuffer)->GetGlBufferId());
		B3D_CHECK_GL_ERROR();
	}

	GpuBuffer::Initialize();
}
