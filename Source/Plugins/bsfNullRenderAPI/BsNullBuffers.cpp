//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullBuffers.h"

namespace bs { namespace ct
{
	SPtr<VertexBuffer> NullHardwareBufferManager::createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<NullVertexBuffer> ret = bs_shared_ptr_new<NullVertexBuffer>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBuffer> NullHardwareBufferManager::createIndexBufferInternal(const INDEX_BUFFER_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<NullIndexBuffer> ret = bs_shared_ptr_new<NullIndexBuffer>(desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamBlockBuffer> NullHardwareBufferManager::createGpuParamBlockBufferInternal(UINT32 size, 
		GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	{
		SPtr<GpuParamBlockBuffer> paramBlockBufferPtr = bs_shared_ptr_new<NullGpuParamBlockBuffer>(size, usage, deviceMask);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBuffer> NullHardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<NullGpuBuffer> bufferPtr = bs_shared_ptr_new<NullGpuBuffer>(desc, deviceMask);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}

	SPtr<GpuBuffer> NullHardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		SPtr<HardwareBuffer> underlyingBuffer)
	{
		SPtr<NullGpuBuffer> bufferPtr = bs_shared_ptr_new<NullGpuBuffer>(desc, std::move(underlyingBuffer));
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}

	static void deleteBuffer(HardwareBuffer* buffer)
	{
		bs_pool_delete(static_cast<NullHardwareBuffer*>(buffer));
	}

	NullGpuBuffer::NullGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuBuffer(desc, deviceMask)
	{ }

	NullGpuBuffer::NullGpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
		: GpuBuffer(desc, std::move(underlyingBuffer))
	{ }

	void NullGpuBuffer::initialize()
	{
		const GpuBufferProperties& props = getProperties();
		mBufferDeleter = &deleteBuffer;

		// Create a new buffer if not wrapping an external one
		if(!mBuffer)
			mBuffer = bs_pool_new<NullHardwareBuffer>(props.getUsage(), props.getElementCount(), props.getElementSize());

		GpuBuffer::initialize();
	}

	NullGpuParamBlockBuffer::NullGpuParamBlockBuffer(UINT32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
		:GpuParamBlockBuffer(size, usage, deviceMask)
	{ }

	void NullGpuParamBlockBuffer::initialize()
	{
		mBuffer = bs_pool_new<NullHardwareBuffer>(mUsage, 1, mSize);
		GpuParamBlockBuffer::initialize();
	}

	NullHardwareBuffer::NullHardwareBuffer(GpuBufferUsage usage, UINT32 elementCount, UINT32 elementSize)
		: HardwareBuffer(elementCount * elementSize, usage, GDF_DEFAULT)
	{ }

	void* NullHardwareBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
		assert(mStagingBuffer == nullptr);

		mStagingBuffer = bs_alloc(mSize);
		return mStagingBuffer;
	}

	void NullHardwareBuffer::unmap()
	{
		bs_free(mStagingBuffer);
		mStagingBuffer = nullptr;
	}

	NullIndexBuffer::NullIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:IndexBuffer(desc, deviceMask)
	{ }

	void NullIndexBuffer::initialize()
	{
		mBuffer = bs_pool_new<NullHardwareBuffer>(mUsage, 1, mSize);
		mBufferDeleter = &deleteBuffer;

		IndexBuffer::initialize();
	}

	NullVertexBuffer::NullVertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: VertexBuffer(desc, deviceMask)
	{ }

	void NullVertexBuffer::initialize()
	{
		mBuffer = bs_pool_new<NullHardwareBuffer>(mUsage, 1, mSize);
		mBufferDeleter = &deleteBuffer;

		VertexBuffer::initialize();
	}
}}
