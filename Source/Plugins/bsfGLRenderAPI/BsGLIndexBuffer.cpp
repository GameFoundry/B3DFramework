//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLIndexBuffer.h"
#include "BsGLHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"
#include "BsGLCommandBuffer.h"

using namespace bs;
using namespace bs::ct;

static void DeleteBuffer(GpuBuffer* buffer)
{
	B3DPoolDelete(static_cast<GLHardwareBuffer*>(buffer));
}

GLIndexBuffer::GLIndexBuffer(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: IndexBuffer(desc, deviceMask)
{
	B3D_ASSERT((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
}

void GLIndexBuffer::Initialize()
{
	mBuffer = B3DPoolNew<GLHardwareBuffer>(GL_ELEMENT_ARRAY_BUFFER, mSize, mUsage);
	mBufferDeleter = &DeleteBuffer;

	IndexBuffer::Initialize();
}
