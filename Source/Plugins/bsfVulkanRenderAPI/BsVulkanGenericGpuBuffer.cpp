//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGenericGpuBuffer.h"
#include "BsVulkanGpuBuffer.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"
#include "BsVulkanUtility.h"
#include "BsVulkanGpuDevice.h"

using namespace bs;
using namespace bs::ct;

static void DeleteHardwareBuffer(GpuBuffer* buffer)
{
	B3DPoolDelete(static_cast<VulkanGpuBuffer*>(buffer));
}

VulkanGenericGpuBuffer::VulkanGenericGpuBuffer(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: GenericGpuBuffer(desc, deviceMask)
{}

VulkanGenericGpuBuffer::VulkanGenericGpuBuffer(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer)
	: GenericGpuBuffer(desc, std::move(underlyingBuffer))
{}

VulkanGenericGpuBuffer::~VulkanGenericGpuBuffer()
{
	if(mBuffer)
	{
		for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
		{
			if(mBufferViews[i] == VK_NULL_HANDLE)
				continue;

			VulkanBuffer* buffer = static_cast<VulkanGpuBuffer*>(mBuffer)->GetResource(i);
			buffer->FreeView(mBufferViews[i]);
		}
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_GpuBuffer);
}

void VulkanGenericGpuBuffer::Initialize()
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_GpuBuffer);

	const GenericGpuBufferProperties& props = GetProperties();
	mBufferDeleter = &DeleteHardwareBuffer;

	// Create a new buffer if external buffer is not provided
	if(!mBuffer)
	{
		GpuBufferType bufferType;
		if(props.GetType() == GBT_STRUCTURED)
			bufferType = GpuBufferType::Structured;
		else
			bufferType = GpuBufferType::Generic;

		u32 size = props.GetElementCount() * props.GetElementSize();
		mBuffer = B3DPoolNew<VulkanGpuBuffer>(bufferType, props.GetFlags(), size, mDeviceMask);
	}

	UpdateViews();

	GenericGpuBuffer::Initialize();
}

void* VulkanGenericGpuBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
	void* data = GenericGpuBuffer::Map(offset, length, options, deviceIdx, queueIdx);
	UpdateViews();

	return data;
}

void VulkanGenericGpuBuffer::Unmap()
{
	GenericGpuBuffer::Unmap();
	UpdateViews();
}

void VulkanGenericGpuBuffer::ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx, u32 queueIdx)
{
	GenericGpuBuffer::ReadData(offset, length, dest, deviceIdx, queueIdx);
	UpdateViews();
}

void VulkanGenericGpuBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, u32 queueIdx)
{
	GenericGpuBuffer::WriteData(offset, length, source, writeFlags, queueIdx);
	UpdateViews();
}

VulkanBuffer* VulkanGenericGpuBuffer::GetResource(u32 deviceIdx) const
{
	return static_cast<VulkanGpuBuffer*>(mBuffer)->GetResource(deviceIdx);
}

VkBufferView VulkanGenericGpuBuffer::GetView(u32 deviceIdx) const
{
	return mBufferViews[deviceIdx];
}

void VulkanGenericGpuBuffer::UpdateViews()
{
	if(mProperties.GetType() == GBT_STRUCTURED)
		return;

	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		VulkanBuffer* buffer = static_cast<VulkanGpuBuffer*>(mBuffer)->GetResource(i);

		VkBuffer newBufferHandle = VK_NULL_HANDLE;

		if(buffer)
			newBufferHandle = buffer->GetHandle();

		if(mCachedBuffers[i] != newBufferHandle)
		{
			if(newBufferHandle != VK_NULL_HANDLE)
				mBufferViews[i] = buffer->GetView(VulkanUtility::GetBufferFormat(mProperties.GetFormat()));
			else
				mBufferViews[i] = VK_NULL_HANDLE;

			mCachedBuffers[i] = newBufferHandle;
		}
	}
}
