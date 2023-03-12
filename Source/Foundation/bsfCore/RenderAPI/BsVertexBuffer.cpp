//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsVertexBuffer.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsGenericGpuBuffer.h"

using namespace bs;

void CheckValidDesc(const VertexBufferCreateInformation& desc)
{
	if(desc.VertexSize == 0)
		B3D_EXCEPT(InvalidParametersException, "Vertex buffer vertex size is not allowed to be zero.");

	if(desc.VertexCount == 0)
		B3D_EXCEPT(InvalidParametersException, "Vertex buffer vertex count is not allowed to be zero.");
}

VertexBufferProperties::VertexBufferProperties(u32 vertexCount, u32 vertexSize)
	: mVertexCount(vertexCount), mVertexSize(vertexSize)
{}

VertexBuffer::VertexBuffer(const VertexBufferCreateInformation& desc)
	: mProperties(desc.VertexCount, desc.VertexSize), mFlags(desc.Flags), mStreamOut(desc.StreamOut)
{
#if B3D_DEBUG
	CheckValidDesc(desc);
#endif
}

SPtr<ct::CoreObject> VertexBuffer::CreateCore() const
{
	VertexBufferCreateInformation desc;
	desc.VertexSize = mProperties.mVertexSize;
	desc.VertexCount = mProperties.mVertexCount;
	desc.Flags = mFlags;
	desc.StreamOut = mStreamOut;

	return ct::HardwareBufferManager::Instance().CreateVertexBufferInternal(desc);
}

SPtr<ct::VertexBuffer> VertexBuffer::GetCore() const
{
	return std::static_pointer_cast<ct::VertexBuffer>(mCoreSpecific);
}

SPtr<VertexBuffer> VertexBuffer::Create(const VertexBufferCreateInformation& desc)
{
	return HardwareBufferManager::Instance().CreateVertexBuffer(desc);
}

namespace bs { namespace ct
{
VertexBuffer::VertexBuffer(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: GpuBuffer(GpuBufferType::Vertex, desc.VertexSize * desc.VertexCount, desc.Flags, deviceMask), mProperties(desc.VertexCount, desc.VertexSize)
{
#if B3D_DEBUG
	CheckValidDesc(desc);
#endif
}

VertexBuffer::~VertexBuffer()
{
	if(mBuffer && !mSharedBuffer)
		mBufferDeleter(mBuffer);

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_VertexBuffer);
}

void VertexBuffer::Initialize()
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_VertexBuffer);
	CoreObject::Initialize();
}

void* VertexBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
#if B3D_PROFILING_ENABLED
	if(options == GBL_READ_ONLY || options == GBL_READ_WRITE)
	{
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_VertexBuffer);
	}

	if(options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
	{
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_VertexBuffer);
	}
#endif

	return mBuffer->Lock(offset, length, options, deviceIdx, queueIdx);
}

void VertexBuffer::Unmap()
{
	mBuffer->Unlock();
}

void VertexBuffer::ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx, u32 queueIdx)
{
	mBuffer->ReadData(offset, length, dest, deviceIdx, queueIdx);
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_VertexBuffer);
}

void VertexBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, u32 queueIdx)
{
	mBuffer->WriteData(offset, length, source, writeFlags, queueIdx);
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_VertexBuffer);
}

void VertexBuffer::CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
{
	auto& srcVertexBuffer = static_cast<VertexBuffer&>(srcBuffer);
	mBuffer->CopyData(*srcVertexBuffer.mBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
}

SPtr<GenericGpuBuffer> VertexBuffer::GetLoadStore(GenericGpuBufferType type, GpuBufferFormat format, u32 elementSize)
{
	if(!mBufferFlags.IsSet(GpuBufferFlag::AllowWritesOnTheGPU))
		return nullptr;

	for(const auto& entry : mLoadStoreViews)
	{
		const GenericGpuBufferProperties& props = entry->GetProperties();
		if(props.GetType() == type)
		{
			if(type == GBT_STANDARD && props.GetFormat() == format)
				return entry;

			if(type == GBT_STRUCTURED && props.GetElementSize() == elementSize)
				return entry;
		}
	}

	u32 elemSize = type == GBT_STANDARD ? bs::GenericGpuBuffer::GetFormatSize(format) : elementSize;
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
		mSharedBuffer = B3DMakeSharedFromExisting(mBuffer, mBufferDeleter);

	SPtr<GenericGpuBuffer> newView = GenericGpuBuffer::Create(desc, mSharedBuffer);
	mLoadStoreViews.push_back(newView);

	return newView;
}

SPtr<VertexBuffer> VertexBuffer::Create(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateVertexBuffer(desc, deviceMask);
}
}}
