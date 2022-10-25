//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuBuffer.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;

u32 getBufferSize(const GPU_BUFFER_DESC& desc)
{
	u32 elementSize;

	if(desc.Type == GBT_STANDARD)
		elementSize = GpuBuffer::GetFormatSize(desc.Format);
	else
		elementSize = desc.ElementSize;

	return elementSize * desc.ElementCount;
}

GpuBufferProperties::GpuBufferProperties(const GPU_BUFFER_DESC& desc)
	: mDesc(desc)
{
	if(mDesc.Type == GBT_STANDARD)
		mDesc.ElementSize = GpuBuffer::GetFormatSize(mDesc.Format);
}

GpuBuffer::GpuBuffer(const GPU_BUFFER_DESC& desc)
	: mProperties(desc)
{
}

SPtr<ct::GpuBuffer> GpuBuffer::GetCore() const
{
	return std::static_pointer_cast<ct::GpuBuffer>(mCoreSpecific);
}

SPtr<ct::CoreObject> GpuBuffer::CreateCore() const
{
	return ct::HardwareBufferManager::Instance().CreateGpuBufferInternal(mProperties.mDesc);
}

u32 GpuBuffer::GetFormatSize(GpuBufferFormat format)
{
	static bool lookupInitialized = false;

	static u32 lookup[BF_COUNT];
	if(!lookupInitialized)
	{
		lookup[BF_16X1F] = 2;
		lookup[BF_16X2F] = 4;
		lookup[BF_16X4F] = 8;
		lookup[BF_32X1F] = 4;
		lookup[BF_32X2F] = 8;
		lookup[BF_32X3F] = 12;
		lookup[BF_32X4F] = 16;
		lookup[BF_8X1] = 1;
		lookup[BF_8X2] = 2;
		lookup[BF_8X4] = 4;
		lookup[BF_16X1] = 2;
		lookup[BF_16X2] = 4;
		lookup[BF_16X4] = 8;
		lookup[BF_8X1S] = 1;
		lookup[BF_8X2S] = 2;
		lookup[BF_8X4S] = 4;
		lookup[BF_16X1S] = 2;
		lookup[BF_16X2S] = 4;
		lookup[BF_16X4S] = 8;
		lookup[BF_32X1S] = 4;
		lookup[BF_32X2S] = 8;
		lookup[BF_32X3S] = 12;
		lookup[BF_32X4S] = 16;
		lookup[BF_8X1U] = 1;
		lookup[BF_8X2U] = 2;
		lookup[BF_8X4U] = 4;
		lookup[BF_16X1U] = 2;
		lookup[BF_16X2U] = 4;
		lookup[BF_16X4U] = 8;
		lookup[BF_32X1U] = 4;
		lookup[BF_32X2U] = 8;
		lookup[BF_32X3U] = 12;
		lookup[BF_32X4U] = 16;

		lookupInitialized = true;
	}

	if(format >= BF_COUNT)
		return 0;

	return lookup[(u32)format];
}

SPtr<GpuBuffer> GpuBuffer::Create(const GPU_BUFFER_DESC& desc)
{
	return HardwareBufferManager::Instance().CreateGpuBuffer(desc);
}

namespace bs { namespace ct
{
GpuBuffer::GpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	: HardwareBuffer(getBufferSize(desc), desc.Usage, deviceMask), mProperties(desc)
{
	if(desc.Type != GBT_STANDARD)
		assert(desc.Format == BF_UNKNOWN && "Format must be set to BF_UNKNOWN when using non-standard buffers");
	else
		assert(desc.ElementSize == 0 && "No element size can be provided for standard buffer. Size is determined from format.");
}

GpuBuffer::GpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
	: HardwareBuffer(getBufferSize(desc), desc.Usage, underlyingBuffer->GetDeviceMask()), mProperties(desc), mBuffer(underlyingBuffer.get()), mSharedBuffer(std::move(underlyingBuffer)), mIsExternalBuffer(true)
{
	const auto& props = GetProperties();
	assert(mSharedBuffer->GetSize() == (props.GetElementCount() * props.GetElementSize()));

	if(desc.Type != GBT_STANDARD)
		assert(desc.Format == BF_UNKNOWN && "Format must be set to BF_UNKNOWN when using non-standard buffers");
	else
		assert(desc.ElementSize == 0 && "No element size can be provided for standard buffer. Size is determined from format.");
}

GpuBuffer::~GpuBuffer()
{
	BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);

	if(mBuffer && !mSharedBuffer)
		mBufferDeleter(mBuffer);
}

void GpuBuffer::Initialize()
{
	BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);
	CoreObject::Initialize();
}

void* GpuBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
#if BS_PROFILING_ENABLED
	if(options == GBL_READ_ONLY || options == GBL_READ_WRITE)
	{
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
	}

	if(options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
	{
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
	}
#endif

	return mBuffer->Lock(offset, length, options, deviceIdx, queueIdx);
}

void GpuBuffer::Unmap()
{
	mBuffer->Unlock();
}

void GpuBuffer::ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx, u32 queueIdx)
{
	BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);

	mBuffer->ReadData(offset, length, dest, deviceIdx, queueIdx);
}

void GpuBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, u32 queueIdx)
{
	BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);

	mBuffer->WriteData(offset, length, source, writeFlags, queueIdx);
}

void GpuBuffer::CopyData(HardwareBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
{
	auto& srcGpuBuffer = static_cast<GpuBuffer&>(srcBuffer);
	mBuffer->CopyData(*srcGpuBuffer.mBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
}

SPtr<GpuBuffer> GpuBuffer::GetView(GpuBufferType type, GpuBufferFormat format, u32 elementSize)
{
	const u32 elemSize = type == GBT_STANDARD ? bs::GpuBuffer::GetFormatSize(format) : elementSize;
	if((mBuffer->GetSize() % elemSize) != 0)
	{
		BS_LOG(Error, RenderBackend, "Size of the buffer isn't divisible by individual element size provided for the buffer view.");
		return nullptr;
	}

	GPU_BUFFER_DESC desc;
	desc.Type = type;
	desc.Format = format;
	desc.Usage = mUsage;
	desc.ElementSize = elementSize;
	desc.ElementCount = mBuffer->GetSize() / elemSize;

	if(!mSharedBuffer)
	{
		mSharedBuffer = bs_shared_ptr(mBuffer, mBufferDeleter);
		mIsExternalBuffer = false;
	}

	SPtr<GpuBuffer> newView = Create(desc, mSharedBuffer);
	return newView;
}

SPtr<GpuBuffer> GpuBuffer::Create(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateGpuBuffer(desc, deviceMask);
}

SPtr<GpuBuffer> GpuBuffer::Create(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
{
	return HardwareBufferManager::Instance().CreateGpuBuffer(desc, std::move(underlyingBuffer));
}
}}
