//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"

using namespace bs;
using namespace bs::ct;

static void DeleteHardwareBuffer(HardwareBuffer* buffer)
{
	B3DPoolDelete(static_cast<VulkanHardwareBuffer*>(buffer));
}

VulkanGpuBuffer::VulkanGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	: GpuBuffer(desc, deviceMask)
{}

VulkanGpuBuffer::VulkanGpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
	: GpuBuffer(desc, std::move(underlyingBuffer))
{}

VulkanGpuBuffer::~VulkanGpuBuffer()
{
	if(mBuffer)
	{
		for(u32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if(mBufferViews[i] == VK_NULL_HANDLE)
				continue;

			VulkanBuffer* buffer = static_cast<VulkanHardwareBuffer*>(mBuffer)->GetResource(i);
			buffer->FreeView(mBufferViews[i]);
		}
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_GpuBuffer);
}

void VulkanGpuBuffer::Initialize()
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_GpuBuffer);

	const GpuBufferProperties& props = GetProperties();
	mBufferDeleter = &DeleteHardwareBuffer;

	// Create a new buffer if external buffer is not provided
	if(!mBuffer)
	{
		VulkanHardwareBuffer::BufferType bufferType;
		if(props.GetType() == GBT_STRUCTURED)
			bufferType = VulkanHardwareBuffer::BT_STRUCTURED;
		else
			bufferType = VulkanHardwareBuffer::BT_GENERIC;

		u32 size = props.GetElementCount() * props.GetElementSize();
		mBuffer = B3DPoolNew<VulkanHardwareBuffer>(bufferType, props.GetFormat(), props.GetUsage(), size, mDeviceMask);
	}

	UpdateViews();

	GpuBuffer::Initialize();
}

void* VulkanGpuBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
	void* data = GpuBuffer::Map(offset, length, options, deviceIdx, queueIdx);
	UpdateViews();

	return data;
}

void VulkanGpuBuffer::Unmap()
{
	GpuBuffer::Unmap();
	UpdateViews();
}

void VulkanGpuBuffer::ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx, u32 queueIdx)
{
	GpuBuffer::ReadData(offset, length, dest, deviceIdx, queueIdx);
	UpdateViews();
}

void VulkanGpuBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, u32 queueIdx)
{
	GpuBuffer::WriteData(offset, length, source, writeFlags, queueIdx);
	UpdateViews();
}

VulkanBuffer* VulkanGpuBuffer::GetResource(u32 deviceIdx) const
{
	return static_cast<VulkanHardwareBuffer*>(mBuffer)->GetResource(deviceIdx);
}

VkBufferView VulkanGpuBuffer::GetView(u32 deviceIdx) const
{
	return mBufferViews[deviceIdx];
}

void VulkanGpuBuffer::UpdateViews()
{
	if(mProperties.GetType() == GBT_STRUCTURED)
		return;

	for(u32 i = 0; i < BS_MAX_DEVICES; i++)
	{
		VulkanBuffer* buffer = static_cast<VulkanHardwareBuffer*>(mBuffer)->GetResource(i);

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
