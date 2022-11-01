//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLGpuParamBlockBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

GLGpuParamBlockBuffer::GLGpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	: GpuParamBlockBuffer(size, usage, deviceMask)
{
	assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
}

GLGpuParamBlockBuffer::~GLGpuParamBlockBuffer()
{
	if(mBuffer)
		B3DPoolDelete(static_cast<GLHardwareBuffer*>(mBuffer));
}

void GLGpuParamBlockBuffer::Initialize()
{
	mBuffer = B3DPoolNew<GLHardwareBuffer>(GL_UNIFORM_BUFFER, mSize, mUsage);
	GpuParamBlockBuffer::Initialize();
}
