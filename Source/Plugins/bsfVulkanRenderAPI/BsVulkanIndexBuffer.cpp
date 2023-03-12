//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanGpuBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

static void DeleteHardwareBuffer(GpuBuffer* buffer)
{
	B3DPoolDelete(static_cast<VulkanGpuBuffer*>(buffer));
}

VulkanIndexBuffer::VulkanIndexBuffer(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: IndexBuffer(desc, deviceMask), mDeviceMask(deviceMask)
{}

void VulkanIndexBuffer::Initialize()
{
	mBuffer = B3DPoolNew<VulkanGpuBuffer>(GpuBufferType::Index, mBufferFlags, mSize, mDeviceMask);
	mBufferDeleter = &DeleteHardwareBuffer;

	IndexBuffer::Initialize();
}

VulkanBuffer* VulkanIndexBuffer::GetResource(u32 deviceIdx) const
{
	return static_cast<VulkanGpuBuffer*>(mBuffer)->GetResource(deviceIdx);
}
