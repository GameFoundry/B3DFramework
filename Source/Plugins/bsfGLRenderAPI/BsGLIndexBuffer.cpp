//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLIndexBuffer.h"
#include "BsGLHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"
#include "BsGLCommandBuffer.h"

namespace bs { namespace ct
{
	static void deleteBuffer(HardwareBuffer* buffer)
	{
		bs_pool_delete(static_cast<GLBuffer*>(buffer));
	}

	GLIndexBuffer::GLIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:IndexBuffer(desc, deviceMask)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
	}

	void GLIndexBuffer::initialize()
	{
		mBuffer = bs_pool_new<GLBuffer>(GL_ELEMENT_ARRAY_BUFFER, mSize, mUsage);
		mBufferDeleter = &deleteBuffer;

		IndexBuffer::initialize();
	}
}}
