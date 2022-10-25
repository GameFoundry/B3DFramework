//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "RenderAPI/BsHardwareBuffer.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;

GpuParamBlockBuffer::GpuParamBlockBuffer(u32 size, GpuBufferUsage usage)
	: mUsage(usage), mSize(size), mCachedData(nullptr)
{
	if(mSize > 0)
	{
		mCachedData = (u8*)bs_alloc(mSize);
		memset(mCachedData, 0, mSize);
	}
}

GpuParamBlockBuffer::~GpuParamBlockBuffer()
{
	if(mCachedData != nullptr)
		bs_free(mCachedData);
}

void GpuParamBlockBuffer::Write(u32 offset, const void* data, u32 size)
{
#if BS_DEBUG_MODE
	if((offset + size) > mSize)
	{
		BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  toString(mSize) + ". "
										"Wanted range: " +
					  toString(offset) + " .. " + toString(offset + size) + ".");
	}
#endif

	memcpy(mCachedData + offset, data, size);
	MarkCoreDirty();
}

void GpuParamBlockBuffer::Read(u32 offset, void* data, u32 size)
{
#if BS_DEBUG_MODE
	if((offset + size) > mSize)
	{
		BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  toString(mSize) + ". "
										"Wanted range: " +
					  toString(offset) + " .. " + toString(offset + size) + ".");
	}
#endif

	memcpy(data, mCachedData + offset, size);
}

void GpuParamBlockBuffer::ZeroOut(u32 offset, u32 size)
{
#if BS_DEBUG_MODE
	if((offset + size) > mSize)
	{
		BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  toString(mSize) + ". "
										"Wanted range: " +
					  toString(offset) + " .. " + toString(offset + size) + ".");
	}
#endif

	memset(mCachedData + offset, 0, size);
	MarkCoreDirty();
}

SPtr<ct::GpuParamBlockBuffer> GpuParamBlockBuffer::GetCore() const
{
	return std::static_pointer_cast<ct::GpuParamBlockBuffer>(mCoreSpecific);
}

SPtr<ct::CoreObject> GpuParamBlockBuffer::CreateCore() const
{
	return ct::HardwareBufferManager::Instance().CreateGpuParamBlockBufferInternal(mSize, mUsage);
}

CoreSyncData GpuParamBlockBuffer::SyncToCore(FrameAlloc* allocator)
{
	u8* buffer = allocator->Alloc(mSize);
	Read(0, buffer, mSize);

	return CoreSyncData(buffer, mSize);
}

SPtr<GpuParamBlockBuffer> GpuParamBlockBuffer::Create(u32 size, GpuBufferUsage usage)
{
	return HardwareBufferManager::Instance().CreateGpuParamBlockBuffer(size, usage);
}

namespace bs { namespace ct
{
GpuParamBlockBuffer::GpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	: mUsage(usage), mSize(size), mCachedData(nullptr), mGPUBufferDirty(false)
{
	if(mSize > 0)
	{
		mCachedData = (u8*)bs_alloc(mSize);
		memset(mCachedData, 0, mSize);
	}
}

GpuParamBlockBuffer::~GpuParamBlockBuffer()
{
	if(mCachedData != nullptr)
		bs_free(mCachedData);

	BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuParamBuffer);
}

void GpuParamBlockBuffer::Initialize()
{
	BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuParamBuffer);

	CoreObject::Initialize();
}

void GpuParamBlockBuffer::Write(u32 offset, const void* data, u32 size)
{
#if BS_DEBUG_MODE
	if((offset + size) > mSize)
	{
		BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  toString(mSize) + ". "
										"Wanted range: " +
					  toString(offset) + " .. " + toString(offset + size) + ".");
	}
#endif

	memcpy(mCachedData + offset, data, size);
	mGPUBufferDirty = true;
}

void GpuParamBlockBuffer::Read(u32 offset, void* data, u32 size)
{
#if BS_DEBUG_MODE
	if((offset + size) > mSize)
	{
		BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  toString(mSize) + ". "
										"Wanted range: " +
					  toString(offset) + " .. " + toString(offset + size) + ".");
	}
#endif

	memcpy(data, mCachedData + offset, size);
}

void GpuParamBlockBuffer::ZeroOut(u32 offset, u32 size)
{
#if BS_DEBUG_MODE
	if((offset + size) > mSize)
	{
		BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  toString(mSize) + ". "
										"Wanted range: " +
					  toString(offset) + " .. " + toString(offset + size) + ".");
	}
#endif

	memset(mCachedData + offset, 0, size);
	mGPUBufferDirty = true;
}

void GpuParamBlockBuffer::FlushToGpu(u32 queueIdx)
{
	if(mGPUBufferDirty)
	{
		WriteToGpu(mCachedData, queueIdx);
		mGPUBufferDirty = false;
	}
}

void GpuParamBlockBuffer::WriteToGpu(const u8* data, u32 queueIdx)
{
	mBuffer->WriteData(0, mSize, data, BWT_DISCARD, queueIdx);

	BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuParamBuffer);
}

void GpuParamBlockBuffer::SyncToCore(const CoreSyncData& data)
{
	assert(mSize == data.GetBufferSize());

	Write(0, data.GetBuffer(), data.GetBufferSize());
}

SPtr<GpuParamBlockBuffer> GpuParamBlockBuffer::Create(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateGpuParamBlockBuffer(size, usage, deviceMask);
}
}}
