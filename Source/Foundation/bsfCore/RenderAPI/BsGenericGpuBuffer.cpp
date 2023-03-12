//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGenericGpuBuffer.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;

u32 GetBufferSize(const GenericGpuBufferCreateInformation& desc)
{
	u32 elementSize;

	if(desc.Type == GBT_STANDARD)
		elementSize = GenericGpuBuffer::GetFormatSize(desc.Format);
	else
		elementSize = desc.ElementSize;

	return elementSize * desc.ElementCount;
}

GenericGpuBufferProperties::GenericGpuBufferProperties(const GenericGpuBufferCreateInformation& desc)
	: mDesc(desc)
{
	if(mDesc.Type == GBT_STANDARD)
		mDesc.ElementSize = GenericGpuBuffer::GetFormatSize(mDesc.Format);
}

GenericGpuBuffer::GenericGpuBuffer(const GenericGpuBufferCreateInformation& desc)
	: mProperties(desc)
{
}

SPtr<ct::GenericGpuBuffer> GenericGpuBuffer::GetCore() const
{
	return std::static_pointer_cast<ct::GenericGpuBuffer>(mCoreSpecific);
}

SPtr<ct::CoreObject> GenericGpuBuffer::CreateCore() const
{
	return ct::HardwareBufferManager::Instance().CreateGpuBufferInternal(mProperties.mDesc);
}

u32 GenericGpuBuffer::GetFormatSize(GpuBufferFormat format)
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
		lookup[BF_64X1F] = 8;
		lookup[BF_64X2F] = 16;
		lookup[BF_64X3F] = 24;
		lookup[BF_64X4F] = 32;
		lookup[BF_64X1S] = 8;
		lookup[BF_64X2S] = 16;
		lookup[BF_64X3S] = 24;
		lookup[BF_64X4S] = 32;
		lookup[BF_64X1U] = 8;
		lookup[BF_64X2U] = 16;
		lookup[BF_64X3U] = 24;
		lookup[BF_64X4U] = 32;

		lookupInitialized = true;
	}

	if(format >= BF_COUNT)
		return 0;

	return lookup[(u32)format];
}

SPtr<GenericGpuBuffer> GenericGpuBuffer::Create(const GenericGpuBufferCreateInformation& desc)
{
	return HardwareBufferManager::Instance().CreateGpuBuffer(desc);
}

namespace bs { namespace ct
{
GenericGpuBuffer::GenericGpuBuffer(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: GpuBuffer(GpuBufferType::Generic, GetBufferSize(desc), desc.Flags, deviceMask), mProperties(desc)
{
	if(desc.Type != GBT_STANDARD)
		B3D_ASSERT(desc.Format == BF_UNKNOWN && "Format must be set to BF_UNKNOWN when using non-standard buffers");
	else
		B3D_ASSERT(desc.ElementSize == 0 && "No element size can be provided for standard buffer. Size is determined from format.");
}

GenericGpuBuffer::GenericGpuBuffer(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer)
	: GpuBuffer(GpuBufferType::Generic, GetBufferSize(desc), desc.Flags, underlyingBuffer->GetDeviceMask()), mProperties(desc), mBuffer(underlyingBuffer.get()), mSharedBuffer(std::move(underlyingBuffer)), mIsExternalBuffer(true)
{
	const auto& props = GetProperties();
	B3D_ASSERT(mSharedBuffer->GetSize() == (props.GetElementCount() * props.GetElementSize()));

	if(desc.Type != GBT_STANDARD)
		B3D_ASSERT(desc.Format == BF_UNKNOWN && "Format must be set to BF_UNKNOWN when using non-standard buffers");
	else
		B3D_ASSERT(desc.ElementSize == 0 && "No element size can be provided for standard buffer. Size is determined from format.");
}

GenericGpuBuffer::~GenericGpuBuffer()
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_GpuBuffer);

	if(mBuffer && !mSharedBuffer)
		mBufferDeleter(mBuffer);
}

void GenericGpuBuffer::Initialize()
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_GpuBuffer);
	CoreObject::Initialize();
}

void* GenericGpuBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
#if B3D_PROFILING_ENABLED
	if(options == GBL_READ_ONLY || options == GBL_READ_WRITE)
	{
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_GpuBuffer);
	}

	if(options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
	{
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_GpuBuffer);
	}
#endif

	return mBuffer->Lock(offset, length, options, deviceIdx, queueIdx);
}

void GenericGpuBuffer::Unmap()
{
	mBuffer->Unlock();
}

void GenericGpuBuffer::ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx, u32 queueIdx)
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_GpuBuffer);

	mBuffer->ReadData(offset, length, dest, deviceIdx, queueIdx);
}

void GenericGpuBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, u32 queueIdx)
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_GpuBuffer);

	mBuffer->WriteData(offset, length, source, writeFlags, queueIdx);
}

void GenericGpuBuffer::CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
{
	auto& srcGpuBuffer = static_cast<GenericGpuBuffer&>(srcBuffer);
	mBuffer->CopyData(*srcGpuBuffer.mBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
}

SPtr<GenericGpuBuffer> GenericGpuBuffer::GetView(GenericGpuBufferType type, GpuBufferFormat format, u32 elementSize)
{
	const u32 elemSize = type == GBT_STANDARD ? bs::GenericGpuBuffer::GetFormatSize(format) : elementSize;
	if((mBuffer->GetSize() % elemSize) != 0)
	{
		B3D_LOG(Error, RenderBackend, "Size of the buffer isn't divisible by individual element size provided for the buffer view.");
		return nullptr;
	}

	GenericGpuBufferCreateInformation desc;
	desc.Type = type;
	desc.Format = format;
	desc.Flags = mBufferFlags;
	desc.ElementSize = elementSize;
	desc.ElementCount = mBuffer->GetSize() / elemSize;

	if(!mSharedBuffer)
	{
		mSharedBuffer = B3DMakeSharedFromExisting(mBuffer, mBufferDeleter);
		mIsExternalBuffer = false;
	}

	SPtr<GenericGpuBuffer> newView = Create(desc, mSharedBuffer);
	return newView;
}

SPtr<GenericGpuBuffer> GenericGpuBuffer::Create(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateGpuBuffer(desc, deviceMask);
}

SPtr<GenericGpuBuffer> GenericGpuBuffer::Create(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer)
{
	return HardwareBufferManager::Instance().CreateGpuBuffer(desc, std::move(underlyingBuffer));
}
}}
