//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLIndexBuffer.h"
#include "BsGLHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"
#include "BsGLCommandBuffer.h"

namespace bs { namespace ct
{
	GLIndexBuffer::GLIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:IndexBuffer(desc, deviceMask), mUsage(desc.usage)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
	}

	GLIndexBuffer::~GLIndexBuffer()
	{
		if(mBuffer)
			bs_pool_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_IndexBuffer);
	}

	void GLIndexBuffer::initialize()
	{
		mBuffer = bs_pool_new<GLBuffer>(GL_ELEMENT_ARRAY_BUFFER, mSize, mUsage);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_IndexBuffer);
		IndexBuffer::initialize();
	}

	void* GLIndexBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
		return mBuffer->lock(offset, length, options, deviceIdx, queueIdx);
	}

	void GLIndexBuffer::unmap()
	{
		mBuffer->unlock();
	}

	void GLIndexBuffer::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		mBuffer->readData(offset, length, dest);
	}

	void GLIndexBuffer::writeData(UINT32 offset, UINT32 length,
		const void* pSource, BufferWriteType writeFlags, UINT32 queueIdx)
	{
		mBuffer->writeData(offset, length, pSource, writeFlags);
	}

	void GLIndexBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
		bool discardWholeBuffer, const SPtr<ct::CommandBuffer>& commandBuffer)
	{
		auto executeRef = [this](HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length)
		{
			GLIndexBuffer& glSrcBuffer = static_cast<GLIndexBuffer&>(srcBuffer);
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
