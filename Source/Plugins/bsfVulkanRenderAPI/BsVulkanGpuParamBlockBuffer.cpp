//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuParamBlockBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

VulkanGpuParamBlockBuffer::VulkanGpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	: GpuParamBlockBuffer(size, usage, deviceMask), mDeviceMask(deviceMask)
{}

VulkanGpuParamBlockBuffer::~VulkanGpuParamBlockBuffer()
{
	if(mBuffer != nullptr)
		B3DPoolDelete(static_cast<VulkanHardwareBuffer*>(mBuffer));
}

void VulkanGpuParamBlockBuffer::Initialize()
{
	mBuffer = B3DPoolNew<VulkanHardwareBuffer>(VulkanHardwareBuffer::BT_UNIFORM, BF_UNKNOWN, mUsage, mSize, mDeviceMask);

	GpuParamBlockBuffer::Initialize();
}

VulkanBuffer* VulkanGpuParamBlockBuffer::GetResource(u32 deviceIdx) const
{
	return static_cast<VulkanHardwareBuffer*>(mBuffer)->GetResource(deviceIdx);
}
