//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuParamBlockBuffer.h"
#include "BsVulkanGpuBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

VulkanGpuParamBlockBuffer::VulkanGpuParamBlockBuffer(u32 size, GpuBufferFlags flags, GpuDeviceFlags deviceMask)
	: GpuParamBlockBuffer(size, flags, deviceMask)
{}

VulkanGpuParamBlockBuffer::VulkanGpuParamBlockBuffer(const SPtr<GpuBuffer>& backingMemory, u32 offset, u32 size)
	: GpuParamBlockBuffer(backingMemory, offset, size)
{}

VulkanBuffer* VulkanGpuParamBlockBuffer::GetResource(u32 deviceIdx) const
{
	return static_cast<VulkanGpuBuffer*>(mBuffer.get())->GetResource(deviceIdx);
}
