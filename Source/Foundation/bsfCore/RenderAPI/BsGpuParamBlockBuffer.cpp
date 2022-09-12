//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "RenderAPI/BsHardwareBuffer.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"

namespace bs
{
	GpuParamBlockBuffer::GpuParamBlockBuffer(UINT32 size, GpuBufferUsage usage)
		:mUsage(usage), mSize(size), mCachedData(nullptr)
	{
		if (mSize > 0)
		{
			mCachedData = (UINT8*)bs_alloc(mSize);
			memset(mCachedData, 0, mSize);
		}
	}

	GpuParamBlockBuffer::~GpuParamBlockBuffer()
	{
		if (mCachedData != nullptr)
			bs_free(mCachedData);
	}

	void GpuParamBlockBuffer::Write(UINT32 offset, const void* data, UINT32 size)
	{
#if BS_DEBUG_MODE
		if ((offset + size) > mSize)
		{
			BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. " \
				"Available range: 0 .. " + toString(mSize) + ". " \
				"Wanted range: " + toString(offset) + " .. " + toString(offset + size) + ".");
		}
#endif

		memcpy(mCachedData + offset, data, size);
		markCoreDirty();
	}

	void GpuParamBlockBuffer::Read(UINT32 offset, void* data, UINT32 size)
	{
#if BS_DEBUG_MODE
		if ((offset + size) > mSize)
		{
			BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. " \
				"Available range: 0 .. " + toString(mSize) + ". " \
				"Wanted range: " + toString(offset) + " .. " + toString(offset + size) + ".");
		}
#endif

		memcpy(data, mCachedData + offset, size);
	}

	void GpuParamBlockBuffer::ZeroOut(UINT32 offset, UINT32 size)
	{
#if BS_DEBUG_MODE
		if ((offset + size) > mSize)
		{
			BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. " \
				"Available range: 0 .. " + toString(mSize) + ". " \
				"Wanted range: " + toString(offset) + " .. " + toString(offset + size) + ".");
		}
#endif

		memset(mCachedData + offset, 0, size);
		markCoreDirty();
	}

	SPtr<ct::GpuParamBlockBuffer> GpuParamBlockBuffer::GetCore() const
	{
		return std::static_pointer_cast<ct::GpuParamBlockBuffer>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> GpuParamBlockBuffer::CreateCore() const
	{
		return ct::HardwareBufferManager::instance().createGpuParamBlockBufferInternal(mSize, mUsage);
	}

	CoreSyncData GpuParamBlockBuffer::SyncToCore(FrameAlloc* allocator)
	{
		UINT8* buffer = allocator->alloc(mSize);
		read(0, buffer, mSize);

		return CoreSyncData(buffer, mSize);
	}

	SPtr<GpuParamBlockBuffer> GpuParamBlockBuffer::Create(UINT32 size, GpuBufferUsage usage)
	{
		return HardwareBufferManager::Instance().createGpuParamBlockBuffer(size, usage);
	}

	namespace ct
	{
	GpuParamBlockBuffer::GpuParamBlockBuffer(UINT32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
		:mUsage(usage), mSize(size), mCachedData(nullptr), mGPUBufferDirty(false)
	{
		if (mSize > 0)
		{
			mCachedData = (UINT8*)bs_alloc(mSize);
			memset(mCachedData, 0, mSize);
		}
	}

	GpuParamBlockBuffer::~GpuParamBlockBuffer()
	{
		if (mCachedData != nullptr)
			bs_free(mCachedData);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuParamBuffer);
	}

	void GpuParamBlockBuffer::Initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuParamBuffer);

		CoreObject::initialize();
	}

	void GpuParamBlockBuffer::Write(UINT32 offset, const void* data, UINT32 size)
	{
#if BS_DEBUG_MODE
		if ((offset + size) > mSize)
		{
			BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. " \
				"Available range: 0 .. " + toString(mSize) + ". " \
				"Wanted range: " + toString(offset) + " .. " + toString(offset + size) + ".");
		}
#endif

		memcpy(mCachedData + offset, data, size);
		mGPUBufferDirty = true;
	}

	void GpuParamBlockBuffer::Read(UINT32 offset, void* data, UINT32 size)
	{
#if BS_DEBUG_MODE
		if ((offset + size) > mSize)
		{
			BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. " \
				"Available range: 0 .. " + toString(mSize) + ". " \
				"Wanted range: " + toString(offset) + " .. " + toString(offset + size) + ".");
		}
#endif

		memcpy(data, mCachedData + offset, size);
	}

	void GpuParamBlockBuffer::ZeroOut(UINT32 offset, UINT32 size)
	{
#if BS_DEBUG_MODE
		if ((offset + size) > mSize)
		{
			BS_EXCEPT(InvalidParametersException, "Wanted range is out of buffer bounds. " \
				"Available range: 0 .. " + toString(mSize) + ". " \
				"Wanted range: " + toString(offset) + " .. " + toString(offset + size) + ".");
		}
#endif

		memset(mCachedData + offset, 0, size);
		mGPUBufferDirty = true;
	}

	void GpuParamBlockBuffer::FlushToGPU(UINT32 queueIdx)
	{
		if (mGPUBufferDirty)
		{
			writeToGPU(mCachedData, queueIdx);
			mGPUBufferDirty = false;
		}
	}

	void GpuParamBlockBuffer::WriteToGPU(const UINT8* data, UINT32 queueIdx)
	{
		mBuffer->writeData(0, mSize, data, BWT_DISCARD, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuParamBuffer);
	}

	void GpuParamBlockBuffer::SyncToCore(const CoreSyncData& data)
	{
		assert(mSize == data.getBufferSize());

		write(0, data.getBuffer(), data.getBufferSize());
	}

	SPtr<GpuParamBlockBuffer> GpuParamBlockBuffer::Create(UINT32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferManager::Instance().createGpuParamBlockBuffer(size, usage, deviceMask);
	}
	}
}
