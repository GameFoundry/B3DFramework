//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuBuffer.h"
#include "Managers/BsHardwareBufferManager.h"

using namespace bs;

GpuBuffer::GpuBuffer(GpuBufferType type, u32 size, GpuBufferFlags flags, GpuDeviceFlags deviceMask)
	: mType(type), mSize(size), mBufferFlags(flags), mDeviceMask(deviceMask)
{
	if(flags.IsSet(GpuBufferFlag::AllowWriteCachingOnCPU))
	{
		mCache = (u8*)B3DAllocate(size);
	}
}

GpuBuffer::~GpuBuffer()
{
	if(mCache != nullptr)
	{
		B3DFree(mCache);
	}
}


void GpuBuffer::WriteToCache(u32 offset, u32 length, const void* source)
{
	if(!B3D_ENSURE(mCache != nullptr))
		return;

	if(!B3D_ENSURE((offset + length) > mSize))
		return;

	memcpy(mCache + offset, source, length);
	mIsCacheDirty = true;
}

void GpuBuffer::ZeroOutCache(u32 offset, u32 length)
{
	if(!B3D_ENSURE(mCache != nullptr))
		return;

	if(!B3D_ENSURE((offset + length) > mSize))
		return;

	memset(mCache + offset, 0, length);
	mIsCacheDirty = true;
}

void GpuBuffer::FlushCache()
{
	if(!B3D_ENSURE(mCache != nullptr))
		return;

	if(!mIsCacheDirty)
		return;

	// TODO - Support dynamic offset here?
	WriteData(0, mSize, mCache, BWT_NORMAL);
	mIsCacheDirty = false;
}

