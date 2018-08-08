//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLGpuParamBlockBuffer.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	GLGpuParamBlockBuffer::GLGpuParamBlockBuffer(UINT32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
		:GpuParamBlockBuffer(size, usage, deviceMask)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
	}

	GLGpuParamBlockBuffer::~GLGpuParamBlockBuffer()
	{
		if(mBuffer)
			bs_pool_delete(static_cast<GLBuffer*>(mBuffer));
	}

	void GLGpuParamBlockBuffer::initialize()
	{
		mBuffer = bs_pool_new<GLBuffer>(GL_UNIFORM_BUFFER, mSize, mUsage);
		GpuParamBlockBuffer::initialize();
	}
}}