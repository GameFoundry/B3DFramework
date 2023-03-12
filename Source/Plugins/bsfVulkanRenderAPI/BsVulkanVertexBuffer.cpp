//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanVertexBuffer.h"
#include "BsVulkanGpuBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

static void DeleteHardwareBuffer(GpuBuffer* buffer)
{
	B3DPoolDelete(static_cast<VulkanGpuBuffer*>(buffer));
}

VulkanVertexBuffer::VulkanVertexBuffer(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: VertexBuffer(desc, deviceMask), mDeviceMask(deviceMask)
{}

void VulkanVertexBuffer::Initialize()
{
	mBuffer = B3DPoolNew<VulkanGpuBuffer>(GpuBufferType::Vertex, mBufferFlags, mSize, mDeviceMask);
	mBufferDeleter = &DeleteHardwareBuffer;

	VertexBuffer::Initialize();
}

VulkanBuffer* VulkanVertexBuffer::GetResource(u32 deviceIdx) const
{
	return static_cast<VulkanGpuBuffer*>(mBuffer)->GetResource(deviceIdx);
}
