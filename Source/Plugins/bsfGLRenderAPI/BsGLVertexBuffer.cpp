//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLHardwareBufferManager.h"
#include "BsGLVertexBuffer.h"
#include "BsGLVertexArrayObjectManager.h"
#include "Profiling/BsRenderStats.h"
#include "BsGLCommandBuffer.h"

namespace bs { namespace ct
{
	static void deleteBuffer(HardwareBuffer* buffer)
	{
		bs_pool_delete(static_cast<GLHardwareBuffer*>(buffer));
	}

	GLVertexBuffer::GLVertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:VertexBuffer(desc, deviceMask)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
	}

	GLVertexBuffer::~GLVertexBuffer()
	{
		while (!mVAObjects.empty())
			GLVertexArrayObjectManager::instance().notifyBufferDestroyed(mVAObjects[0]);
	}

	void GLVertexBuffer::initialize()
	{
		mBuffer = bs_pool_new<GLHardwareBuffer>(GL_ARRAY_BUFFER, mSize, mUsage);
		mBufferDeleter = &deleteBuffer;
		
		VertexBuffer::initialize();
	}

	void GLVertexBuffer::registerVAO(const GLVertexArrayObject& vao)
	{
		mVAObjects.push_back(vao);
	}

	void GLVertexBuffer::unregisterVAO(const GLVertexArrayObject& vao)
	{
		const auto iterFind = std::find(mVAObjects.begin(), mVAObjects.end(), vao);

		if (iterFind != mVAObjects.end())
			mVAObjects.erase(iterFind);
	}
}}
