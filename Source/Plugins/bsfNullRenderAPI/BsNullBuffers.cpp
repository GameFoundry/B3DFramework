//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullBuffers.h"

using namespace bs;
using namespace bs::ct;

SPtr<ct::VertexBuffer> NullHardwareBufferManager::CreateVertexBufferInternal(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	SPtr<NullVertexBuffer> ret = B3DMakeShared<NullVertexBuffer>(desc, deviceMask);
	ret->SetShared(ret);

	return ret;
}

SPtr<ct::IndexBuffer> NullHardwareBufferManager::CreateIndexBufferInternal(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	SPtr<NullIndexBuffer> ret = B3DMakeShared<NullIndexBuffer>(desc, deviceMask);
	ret->SetShared(ret);

	return ret;
}

SPtr<ct::GpuParamBlockBuffer> NullHardwareBufferManager::CreateGpuParamBlockBufferInternal(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
{
	SPtr<GpuParamBlockBuffer> paramBlockBufferPtr = B3DMakeShared<NullGpuParamBlockBuffer>(size, usage, deviceMask);
	paramBlockBufferPtr->SetShared(paramBlockBufferPtr);

	return paramBlockBufferPtr;
}

SPtr<ct::GenericGpuBuffer> NullHardwareBufferManager::CreateGpuBufferInternal(const GpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	SPtr<NullGpuBuffer> bufferPtr = B3DMakeShared<NullGpuBuffer>(desc, deviceMask);
	bufferPtr->SetShared(bufferPtr);

	return bufferPtr;
}

SPtr<ct::GenericGpuBuffer> NullHardwareBufferManager::CreateGpuBufferInternal(const GpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer)
{
	SPtr<NullGpuBuffer> bufferPtr = B3DMakeShared<NullGpuBuffer>(desc, std::move(underlyingBuffer));
	bufferPtr->SetShared(bufferPtr);

	return bufferPtr;
}

static void DeleteHardwareBuffer(GpuBuffer* buffer)
{
	B3DPoolDelete(static_cast<NullHardwareBuffer*>(buffer));
}

NullGpuBuffer::NullGpuBuffer(const GpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: GenericGpuBuffer(desc, deviceMask)
{}

NullGpuBuffer::NullGpuBuffer(const GpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer)
	: GenericGpuBuffer(desc, std::move(underlyingBuffer))
{}

void NullGpuBuffer::Initialize()
{
	const GpuBufferProperties& props = GetProperties();
	mBufferDeleter = &DeleteHardwareBuffer;

	// Create a new buffer if not wrapping an external one
	if(!mBuffer)
		mBuffer = B3DPoolNew<NullHardwareBuffer>(props.GetUsage(), props.GetElementCount(), props.GetElementSize());

	GenericGpuBuffer::Initialize();
}

NullGpuParamBlockBuffer::NullGpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	: GpuParamBlockBuffer(size, usage, deviceMask)
{}

void NullGpuParamBlockBuffer::Initialize()
{
	mBuffer = B3DPoolNew<NullHardwareBuffer>(mUsage, 1, mSize);
	GpuParamBlockBuffer::Initialize();
}

NullHardwareBuffer::NullHardwareBuffer(GpuBufferUsage usage, u32 elementCount, u32 elementSize)
	: GpuBuffer(elementCount * elementSize, usage, GDF_DEFAULT)
{}

void* NullHardwareBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
	B3D_ASSERT(mStagingBuffer == nullptr);

	mStagingBuffer = B3DAllocate(mSize);
	return mStagingBuffer;
}

void NullHardwareBuffer::Unmap()
{
	B3DFree(mStagingBuffer);
	mStagingBuffer = nullptr;
}

NullIndexBuffer::NullIndexBuffer(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: IndexBuffer(desc, deviceMask)
{}

void NullIndexBuffer::Initialize()
{
	mBuffer = B3DPoolNew<NullHardwareBuffer>(mUsage, 1, mSize);
	mBufferDeleter = &DeleteHardwareBuffer;

	IndexBuffer::Initialize();
}

NullVertexBuffer::NullVertexBuffer(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: VertexBuffer(desc, deviceMask)
{}

void NullVertexBuffer::Initialize()
{
	mBuffer = B3DPoolNew<NullHardwareBuffer>(mUsage, 1, mSize);
	mBufferDeleter = &DeleteHardwareBuffer;

	VertexBuffer::Initialize();
}
