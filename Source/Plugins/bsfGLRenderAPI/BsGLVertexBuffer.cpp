//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLHardwareBufferManager.h"
#include "BsGLVertexBuffer.h"
#include "BsGLVertexArrayObjectManager.h"
#include "Profiling/BsRenderStats.h"
#include "BsGLCommandBuffer.h"

using namespace bs;
using namespace bs::ct;

static void DeleteBuffer(GpuBuffer* buffer)
{
	B3DPoolDelete(static_cast<GLHardwareBuffer*>(buffer));
}

GLVertexBuffer::GLVertexBuffer(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: VertexBuffer(desc, deviceMask)
{
	B3D_ASSERT((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
}

GLVertexBuffer::~GLVertexBuffer()
{
	while(!mVAObjects.empty())
		GLVertexArrayObjectManager::Instance().NotifyBufferDestroyed(mVAObjects[0]);
}

void GLVertexBuffer::Initialize()
{
	mBuffer = B3DPoolNew<GLHardwareBuffer>(GL_ARRAY_BUFFER, mSize, mUsage);
	mBufferDeleter = &DeleteBuffer;

	VertexBuffer::Initialize();
}

void GLVertexBuffer::RegisterVao(const GLVertexArrayObject& vao)
{
	mVAObjects.push_back(vao);
}

void GLVertexBuffer::UnregisterVao(const GLVertexArrayObject& vao)
{
	const auto iterFind = std::find(mVAObjects.begin(), mVAObjects.end(), vao);

	if(iterFind != mVAObjects.end())
		mVAObjects.erase(iterFind);
}
