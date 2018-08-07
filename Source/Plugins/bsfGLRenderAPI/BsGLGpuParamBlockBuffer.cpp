//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLGpuParamBlockBuffer.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"

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
			bs_pool_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuParamBuffer);
	}

	void GLGpuParamBlockBuffer::initialize()
	{
		mBuffer = bs_pool_new<GLBuffer>(GL_UNIFORM_BUFFER, mSize, mUsage);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuParamBuffer);
		GpuParamBlockBuffer::initialize();
	}

	void GLGpuParamBlockBuffer::writeToGPU(const UINT8* data, UINT32 queueIdx)
	{
		mBuffer->writeData(0, mSize, data, BWT_DISCARD, queueIdx);
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuParamBuffer);
	}
}}