//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;

GpuParamBlockBuffer::GpuParamBlockBuffer(u32 size, GpuBufferFlags flags)
	: mFlags(flags), mSize(size), mCachedData(nullptr)
{
	if(mSize > 0)
	{
		mCachedData = (u8*)B3DAllocate(mSize);
		memset(mCachedData, 0, mSize);
	}
}

GpuParamBlockBuffer::~GpuParamBlockBuffer()
{
	if(mCachedData != nullptr)
		B3DFree(mCachedData);
}

void GpuParamBlockBuffer::Write(u32 offset, const void* data, u32 size)
{
#if B3D_DEBUG
	if((offset + size) > mSize)
	{
		B3D_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  ToString(mSize) + ". "
										"Wanted range: " +
					  ToString(offset) + " .. " + ToString(offset + size) + ".");
	}
#endif

	memcpy(mCachedData + offset, data, size);
	MarkCoreDirty();
}

void GpuParamBlockBuffer::Read(u32 offset, void* data, u32 size)
{
#if B3D_DEBUG
	if((offset + size) > mSize)
	{
		B3D_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  ToString(mSize) + ". "
										"Wanted range: " +
					  ToString(offset) + " .. " + ToString(offset + size) + ".");
	}
#endif

	memcpy(data, mCachedData + offset, size);
}

void GpuParamBlockBuffer::ZeroOut(u32 offset, u32 size)
{
#if B3D_DEBUG
	if((offset + size) > mSize)
	{
		B3D_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  ToString(mSize) + ". "
										"Wanted range: " +
					  ToString(offset) + " .. " + ToString(offset + size) + ".");
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
	return ct::HardwareBufferManager::Instance().CreateGpuParamBlockBufferInternal(mSize, mFlags);
}

CoreSyncData GpuParamBlockBuffer::SyncToCore(FrameAlloc* allocator)
{
	u8* buffer = allocator->Alloc(mSize);
	Read(0, buffer, mSize);

	return CoreSyncData(buffer, mSize);
}

SPtr<GpuParamBlockBuffer> GpuParamBlockBuffer::Create(u32 size, GpuBufferFlags flags)
{
	return HardwareBufferManager::Instance().CreateGpuParamBlockBuffer(size, flags);
}

namespace bs { namespace ct
{
GpuParamBlockBuffer::GpuParamBlockBuffer(u32 size, GpuBufferFlags flags, GpuDeviceFlags deviceMask)
	: mBufferFlags(flags), mSize(size), mCachedData(nullptr), mGPUBufferDirty(false), mDeviceMask(deviceMask)
{
	if(mSize > 0)
	{
		mCachedData = (u8*)B3DAllocate(mSize);
		memset(mCachedData, 0, mSize);
	}
}

GpuParamBlockBuffer::GpuParamBlockBuffer(const SPtr<GpuBuffer>& backingMemory, u32 offset, u32 size)
	: mBufferFlags(GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU), mSize(size), mOffset(offset), mBuffer(backingMemory), mCachedData(nullptr), mGPUBufferDirty(false), mDeviceMask(GDF_DEFAULT)
{
}

GpuParamBlockBuffer::~GpuParamBlockBuffer()
{
	if(mCachedData != nullptr)
		B3DFree(mCachedData);

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_GpuParamBuffer);
}

void GpuParamBlockBuffer::Initialize()
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_GpuParamBuffer);

	if(mBuffer == nullptr)
	{
		mBuffer = HardwareBufferManager::Instance().CreateHardwareBuffer(GpuBufferType::Uniform, mSize, mBufferFlags, mDeviceMask);
	}

	CoreObject::Initialize();
}

void GpuParamBlockBuffer::Write(u32 offset, const void* data, u32 size)
{
#if B3D_DEBUG
	if((offset + size) > mSize)
	{
		B3D_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  ToString(mSize) + ". "
										"Wanted range: " +
					  ToString(offset) + " .. " + ToString(offset + size) + ".");
	}
#endif

	if(mCachedData == nullptr)
	{
		B3D_LOG(Error, RenderBackend, "Cannot write to GpuParamBlockBuffer. Buffer has external backing memory and write needs to be done externally.");
		return;
	}

	memcpy(mCachedData + offset, data, size);
	mGPUBufferDirty = true;
}

void GpuParamBlockBuffer::Read(u32 offset, void* data, u32 size)
{
#if B3D_DEBUG
	if((offset + size) > mSize)
	{
		B3D_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  ToString(mSize) + ". "
										"Wanted range: " +
					  ToString(offset) + " .. " + ToString(offset + size) + ".");
	}
#endif

	if(mCachedData == nullptr)
	{
		B3D_LOG(Error, RenderBackend, "Cannot read from GpuParamBlockBuffer. Buffer has external backing memory and read needs to be done externally.");
		return;
	}

	memcpy(data, mCachedData + offset, size);
}

void GpuParamBlockBuffer::ZeroOut(u32 offset, u32 size)
{
#if B3D_DEBUG
	if((offset + size) > mSize)
	{
		B3D_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. "
											  "Available range: 0 .. " +
					  ToString(mSize) + ". "
										"Wanted range: " +
					  ToString(offset) + " .. " + ToString(offset + size) + ".");
	}
#endif

	if(mCachedData == nullptr)
	{
		B3D_LOG(Error, RenderBackend, "Cannot write to GpuParamBlockBuffer. Buffer has external backing memory and write needs to be done externally.");
		return;
	}

	memset(mCachedData + offset, 0, size);
	mGPUBufferDirty = true;
}

void GpuParamBlockBuffer::FlushToGpu(u32 queueIdx)
{
	if(mGPUBufferDirty && mCachedData != nullptr)
	{
		WriteToGpu(mCachedData, queueIdx);
		mGPUBufferDirty = false;
	}
}

void GpuParamBlockBuffer::WriteToGpu(const u8* data, u32 queueIdx)
{
	mBuffer->WriteData(mOffset, mSize, data, BWT_NORMAL, queueIdx);

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_GpuParamBuffer);
}

void GpuParamBlockBuffer::SyncToCore(const CoreSyncData& data)
{
	B3D_ASSERT(mSize == data.GetBufferSize());

	Write(0, data.GetBuffer(), data.GetBufferSize());
}

SPtr<GpuParamBlockBuffer> GpuParamBlockBuffer::Create(u32 size, GpuBufferFlags flags, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateGpuParamBlockBuffer(size, flags, deviceMask);
}

SPtr<GpuParamBlockBuffer> GpuParamBlockBuffer::Create(const SPtr<GpuBuffer>& backingMemory, u32 offset, u32 size)
{
	return HardwareBufferManager::Instance().CreateGpuParamBlockBuffer(backingMemory, offset, size);
}
}}
