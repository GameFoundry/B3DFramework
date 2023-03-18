//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGenericGpuBuffer.h"

#include "BsVulkanGpuBackend.h"
#include "BsVulkanGpuBuffer.h"
#include "Profiling/BsRenderStats.h"
#include "Error/BsException.h"
#include "BsVulkanUtility.h"
#include "BsVulkanGpuDevice.h"

using namespace bs;
using namespace bs::ct;

static void DeleteHardwareBuffer(ct::GpuBuffer* buffer)
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
		GpuBufferCreateInformation createInformation;
		if(props.GetType() == GBT_STRUCTURED)
		{
			createInformation.Type = GpuBufferType::StructuredStorage;
			createInformation.Flags = props.GetFlags();
			createInformation.StructuredStorage.ElementSize = props.GetElementSize();
			createInformation.StructuredStorage.Count = props.GetElementCount();
		}
		else
		{
			createInformation.Type = GpuBufferType::SimpleStorage;
			createInformation.Flags = props.GetFlags();
			createInformation.SimpleStorage.Format = props.GetFormat();
			createInformation.SimpleStorage.Count = props.GetElementCount();
		}

		mBuffer = B3DPoolNew<VulkanGpuBuffer>(*GetVulkanGpuBackend().GetVulkanDevice(0), createInformation);
		mBuffer->Initialize();
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

	VulkanBuffer* buffer = static_cast<VulkanGpuBuffer*>(mBuffer)->GetResource(0);

	VkBuffer newBufferHandle = VK_NULL_HANDLE;

	if(buffer)
		newBufferHandle = buffer->GetHandle();

	if(mCachedBuffers[0] != newBufferHandle)
	{
		if(newBufferHandle != VK_NULL_HANDLE)
			mBufferViews[0] = buffer->GetView(VulkanUtility::GetBufferFormat(mProperties.GetFormat()));
		else
			mBufferViews[0] = VK_NULL_HANDLE;

		mCachedBuffers[0] = newBufferHandle;
	}
}
